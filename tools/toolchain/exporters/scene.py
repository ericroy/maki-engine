import sys
import os
import struct
import operator
import functools

from ..compilers import compile_fbx
from .. import doc
from ..fbx.FbxCommon import *

def _parse_user_props(s):
    user_props = {}
    for pair in s.split('\n'):
        pair = pair.strip()
        if not pair:
            continue
        parts = pair.split('=', 1)
        if len(parts) == 2:
            user_props[parts[0].strip()] = parts[1].strip()
        else:
            user_props[parts[0].strip()] = None
    return user_props

def _export_node(manager, node, doc_container, src, output_dir, uid_to_mesh_path, *args):
    ent = doc_container.add_child('entity')

    entity_name = node.GetName().strip().replace(' ', '')
    ent.add_child('name').add_child(entity_name)


    trans = node.LclTranslation.Get()
    angles = node.LclRotation.Get()
    if any(trans) or any(angles):
        t = ent.add_child('transform')
        if any(trans):
            t.add_child('pos').add_children(trans)
        if any(angles):
            t.add_child('angles').add_children(angles)

    physics = False
    for i in range(node.GetDstObjectCount()):
        obj = node.GetDstObject(i)
        if obj.GetTypeName() == 'DisplayLayer':
            if obj.GetName() == 'physics':
                physics = True

    mesh = node.GetMesh()
    if mesh:
        try:
            mesh_path = uid_to_mesh_path[mesh.GetUniqueID()]
        except KeyError:
            output_file = (os.path.splitext(os.path.basename(src))[0] + '_' + entity_name + '.mmesh').lower()
            print('Exporting %s' % output_file)
            if not compile_fbx.build_node(manager, node, os.path.join(output_dir, output_file), *args):
                raise RuntimeError('Failed to export mesh')
            mesh_path = os.path.join(os.path.relpath(output_dir, 'assets_bin'), output_file).replace('\\', '/').lower()
            uid_to_mesh_path[mesh.GetUniqueID()] = mesh_path

        material_path = None
        if node.GetMaterialCount() > 1:
            print('WARNING: Object has multiple materials: %s' % entity_name)
        if node.GetMaterialCount() > 0:
            mat = node.GetMaterial(0)
            print('Material name: %s' % mat.GetName())
            material_path = 'materials/%s.mdoc' % mat.GetName().replace(' ', '_').lower()
    
        if physics:
            p = ent.add_child('physics')
            p.add_child('mesh').add_child(mesh_path)
        else:
            m = ent.add_child('mesh')
            m.add_child('mesh').add_child(mesh_path)
            if material_path:
                m.add_child('material').add_child(material_path)

    # user_props = {}
    # prop = node.GetFirstProperty()
    # while prop.IsValid():
    #     if prop.GetFlag(FbxPropertyAttr.eUser):
    #         if prop.GetName() == 'UDP3DSMAX':
    #             user_props = _parse_user_props(FbxPropertyString(prop).Get().Buffer())
    #             break
    #     prop = node.GetNextProperty(prop)

    children = None
    for i in range(node.GetChildCount()):
        child_node = node.GetChild(i)
        child_node_attr = node.GetNodeAttribute()
        if child_node_attr is None or child_node_attr.GetAttributeType() != FbxNodeAttribute.eMesh:
            continue
        if children is None:
            children = ent.add_child('children')
        _export_node(manager, child_node, children, src, output_dir, uid_to_mesh_path, *args)

def export(src, dst, *args):
    manager, scene = InitializeSdkObjects()

    if not LoadScene(manager, scene, src):
        raise RuntimeError('Failed to load scene')

    output_dir = os.path.dirname(dst)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    uid_to_mesh_path = {}
    root = doc.Node('root')
    children = root.add_child('children')

    root_node = scene.GetRootNode()
    for i in range(root_node.GetChildCount()):
        node = root_node.GetChild(i)
        node_attr = node.GetNodeAttribute()
        if node_attr is None or node_attr.GetAttributeType() != FbxNodeAttribute.eMesh:
            continue
        _export_node(manager, node, children, src, output_dir, uid_to_mesh_path, *args)

    with open(dst, 'w') as file:
        root.serialize(file)

if __name__ == '__main__':
    assert len(sys.argv) > 2, 'Must provide src and dst as command line arguments'
    export(sys.argv[1], sys.argv[2], *sys.argv[3:])
