import sys
import os
import struct
import operator
import functools
from ..fbx.FbxCommon import *

# Must match flags defined in Mesh.h
FLAG_HAS_NORMALS = 1 << 0
FLAG_HAS_TANGENTS = 1 << 1
FLAG_HAS_COLORS = 1 << 2
FLAG_HAS_TEXCOORDS = 1 << 3
FLAG_HAS_BONEWEIGHTS = 1 << 4

BONE_INDEX_BITS = 7
BONE_WEIGHT_BITS = (32-BONE_INDEX_BITS)
BONE_WEIGHT_MAX = (1<<BONE_WEIGHT_BITS)-1
NO_WEIGHTS = [[0, 0.0], [0, 0.0], [0, 0.0], [0, 0.0]]

def _enum_nodes(node):
    if node:
        yield node
        for i in range(node.GetChildCount()):
            for child in _enum_nodes(node.GetChild(i)):
                yield child

def _get_skeleton_heirarchy(node):
    # Root bone is not included by meshes with skins
    skel = []
    for n in _enum_nodes(node):
        node_attr = n.GetNodeAttribute()
        if node_attr and node_attr.GetAttributeType() == FbxNodeAttribute.eSkeleton:
            if node_attr.GetSkeletonType() in (FbxSkeleton.eLimbNode, FbxSkeleton.eRoot):
                # This will skip the helper widgets at the tips of skeleton fingers, feet, head, etc
                # The skin does not bind to these, so they aren't necessary bones
                if n.GetChildCount() == 0:
                    continue
                skel.append(n)
    return skel

def _get_skeleton_root(node):
    while node:
        node_attr = node.GetNodeAttribute()
        if node_attr is None:
            return None
        if node_attr.GetAttributeType() == FbxNodeAttribute.eSkeleton and node_attr.GetSkeletonType() == FbxSkeleton.eRoot:
            return node            
        node = node.GetParent()

def _get_skins(node):
    skeleton_heirarchy = None
    skins = []
    mesh = node.GetMesh()
    cluster_mode = None
    for i in range(mesh.GetDeformerCount(FbxDeformer.eSkin)):
        skin_weights = {}
        skin = mesh.GetDeformer(i, FbxDeformer.eSkin)
        skinning_type = skin.GetSkinningType()
        assert(skinning_type == FbxSkin.eRigid)
        for j in range(skin.GetClusterCount()):
            cluster = skin.GetCluster(j)
            link = cluster.GetLink()
            if skeleton_heirarchy is None:
                skeleton_root = _get_skeleton_root(link)
                if skeleton_root is None:
                    raise ValueError("If mesh is skinned, you must export the biped along with the mesh")
                skeleton_heirarchy = _get_skeleton_heirarchy(skeleton_root)
            try:
                bone_index = skeleton_heirarchy.index(link)
            except ValueError:
                raise ValueError("Could not find referenced bone in our skeleton (so far).  Bone name is %s. Skeleton heirarchy is:\n%s" % (
                    link.GetName(), '\n'.join([s.GetName() for s in skeleton_heirarchy])))

            cluster_mode = cluster.GetLinkMode()
            assert(cluster_mode == FbxCluster.eNormalize)
            indices = cluster.GetControlPointIndices()
            weights = cluster.GetControlPointWeights()
            for k in range(cluster.GetControlPointIndicesCount()):
                index = indices[k]
                influences = skin_weights.setdefault(index, [])
                weight = float(weights[k])
                if weight == 0.0:
                    continue
                influences.append([bone_index, weight])
        skins.append(skin_weights)
    return skins

def _pad(out_file, write_location, byte_multiple):
    added = 0
    while write_location % byte_multiple != 0:
        out_file.write(struct.pack('x'))
        write_location += 1
        added += 1
    return added

def _make_list(obj, field_count):
    if type(obj) == FbxColor:
        assert field_count == 4
        return [int(obj.mRed*255), int(obj.mGreen*255), int(obj.mBlue*255), int(obj.mAlpha*255)]
    else:
        return [obj[i] for i in range(field_count)]

def _get_obj(elements, elements_name, cpi, vi):
    obj = None
    if elements.GetMappingMode() == FbxLayerElement.eByControlPoint:
        if elements.GetReferenceMode() == FbxLayerElement.eDirect:
            obj = elements.GetDirectArray().GetAt(cpi)
        elif elements.GetReferenceMode() == FbxLayerElement.eIndexToDirect:
            obj = elements.GetDirectArray().GetAt(elements.GetIndexArray().GetAt(cpi))
        else:
            print('Warning, unsupported %s reference mode: %d' % (elements_name, elements.GetReferenceMode()))
    elif elements.GetMappingMode() == FbxLayerElement.eByPolygonVertex:
        if elements.GetReferenceMode() == FbxLayerElement.eDirect:
            obj = elements.GetDirectArray().GetAt(vi)
        elif elements.GetReferenceMode() == FbxLayerElement.eIndexToDirect:
            obj = elements.GetDirectArray().GetAt(elements.GetIndexArray().GetAt(vi))
        else:
            print('Warning, unsupported %s reference mode: %d' % (elements_name, elements.GetReferenceMode()))
    else:
        print('Warning, unsupported %s mapping mode: %d' % (elements_name, elements.GetMappingMode()))
    return obj



def _write_mesh(manager, node, out):
    #print('Mesh name:', node.GetName())
    converter = FbxGeometryConverter(manager)
    mesh = converter.Triangulate(node.GetMesh(), True)
    vertex_count = mesh.GetPolygonVertexCount()
    polygon_count = mesh.GetPolygonCount()

    indices_per_face = 3
    bytes_per_index = 2
    index_formatter = 'H'
    if vertex_count >= 2**16:
        bytes_per_index = 4
        index_formatter = 'I'

    flags = 0
    normals = mesh.GetLayer(0).GetNormals()
    if normals:
        #print("Has normals")
        flags |= FLAG_HAS_NORMALS
    tangents = mesh.GetLayer(0).GetTangents()
    if tangents:
        #print("Has tangents")
        flags |= FLAG_HAS_TANGENTS
    colors = mesh.GetLayer(0).GetVertexColors()
    if colors:
        #print("Has colors")
        flags |= FLAG_HAS_COLORS
    uvs = mesh.GetLayer(0).GetUVs()
    if uvs:
        #print("Has uvs")
        flags |= FLAG_HAS_TEXCOORDS
    skins = _get_skins(node)
    if skins:
        #print("Has bone weights")
        flags |= FLAG_HAS_BONEWEIGHTS

    # Generate tangents if necessary
    # if normals and not tangents:
    #     print("Generating tangents")
    #     if mesh.GenerateTangentsDataForAllUVSets(True):
    #         tangents = mesh.GetLayer(0).GetTangents()
    #         if tangents:
    #             flags |= FLAG_HAS_TANGENTS
    #     else:
    #         raise RuntimeError('Failed to generate tangets')

    print('Vertex count: %s' % vertex_count)
    print('Index count: %s' % (polygon_count * indices_per_face))

    out.write(struct.pack('I', vertex_count))
    out.write(struct.pack('I', polygon_count))
    out.write(struct.pack('B', flags))
    out.write(struct.pack('B', indices_per_face))
    out.write(struct.pack('Bx', bytes_per_index))

    control_point_count = mesh.GetControlPointsCount()
    control_points = mesh.GetControlPoints()

    #print('%d polygons' % polygon_count)
    #print('%d control points' % control_point_count)
    #print('%d polygon vertices' % vertex_count)


    bytes_written = 0
    vi = 0
    vertices_exported = 0
    for i in range(polygon_count):
        poly_size = mesh.GetPolygonSize(i)
        assert(poly_size == 3)
        for j in range(poly_size):
            cpi = mesh.GetPolygonVertex(i, j)
            out.write(struct.pack('fff', *_make_list(control_points[cpi], 3)))
            bytes_written += 12
            
            if normals:
                obj = _get_obj(normals, 'normals', cpi, vi)
                if obj:
                    #print(obj)
                    out.write(struct.pack('fff', *_make_list(obj, 3)))
                    bytes_written += 12
            else:
                assert flags & FLAG_HAS_NORMALS == 0
            if tangents:
                obj = _get_obj(tangents, 'tangents', cpi, vi)
                if obj:
                    #print(obj)
                    out.write(struct.pack('fff', *_make_list(obj, 3)))
                    bytes_written += 12
            else:
                assert flags & FLAG_HAS_TANGENTS == 0
            if colors:
                obj = _get_obj(colors, 'colors', cpi, vi)
                if obj:
                    #print(obj)
                    out.write(struct.pack('BBBB', *_make_list(obj, 4)))
                    bytes_written += 4
            else:
                assert flags & FLAG_HAS_COLORS == 0
            if uvs:
                obj = _get_obj(uvs, 'uvs', cpi, vi)
                if obj:
                    #print(obj)
                    out.write(struct.pack('ff', *_make_list(obj, 2)))
                    bytes_written += 8
            else:
                assert flags & FLAG_HAS_TEXCOORDS == 0
            if skins:
                influences = skins[0].get(cpi, [])[:4]
                influences += NO_WEIGHTS[len(influences):]
                #assert(abs(sum([w for i, w in influences])-1) < 0.00001)

                total_weight = sum([w for i, w in influences])
                adjustment_factor = 1.0 / total_weight

                for bone_index, weight in influences:
                    assert(bone_index < 2**BONE_INDEX_BITS)
                    packed = (bone_index << BONE_WEIGHT_BITS) | int(weight * adjustment_factor * BONE_WEIGHT_MAX)
                    out.write(struct.pack('L', packed))
                    bytes_written += 4
            else:
                assert flags & FLAG_HAS_BONEWEIGHTS == 0
            vi += 1
            vertices_exported += 1

    print('%d vertices exported (%d bytes)' % (vertices_exported, bytes_written))
    assert(vertices_exported == vertex_count)

    bytes_written += _pad(out, bytes_written, 4)

    vi = 0
    index_start_offset = bytes_written
    for i in range(polygon_count):
        poly_size = mesh.GetPolygonSize(i)
        for j in range(poly_size):
            out.write(struct.pack(index_formatter, vi))
            bytes_written += bytes_per_index
            vi += 1

    print('%d indices exported (%d bytes)' % (vi, bytes_written - index_start_offset))
    assert(vi == polygon_count * indices_per_face)

    bytes_written += _pad(out, bytes_written, 4)

def _enum_meshes(node):
    for n in _enum_nodes(node):
        node_attr = n.GetNodeAttribute()
        if node_attr and node_attr.GetAttributeType() == FbxNodeAttribute.eMesh:
            yield n

def build_node(manager, base_node, dst):
    with open(dst, 'wb') as out:
        out.write(struct.pack('8s', b'maki'))
        mesh_count = len(list(_enum_meshes(base_node)))
        #print('Exporting %d meshes' % mesh_count)
        out.write(struct.pack('I', mesh_count))
        for node in _enum_meshes(base_node):
            _write_mesh(manager, node, out)
    return True

def compile(arc_name, src, dst):
    manager, scene = InitializeSdkObjects()
    if not LoadScene(manager, scene, src):
        raise RuntimeError('Failed to load scene')
    build_node(manager, scene.GetRootNode(), dst)

if __name__ == '__main__':
    assert len(sys.argv) > 2, 'Must provide src and dst as command line arguments'
    build(sys.argv[1], sys.argv[2])
