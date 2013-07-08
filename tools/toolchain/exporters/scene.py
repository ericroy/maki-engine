import sys
import os
import struct
import operator
import functools

from ..compilers import compile_fbx
from .. import doc
from ..fbx.FbxCommon import *

def _parse_user_props_string(s):
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

def _get_user_props(node):
    user_props = {}
    prop = node.GetFirstProperty()
    while prop.IsValid():
        if prop.GetFlag(FbxPropertyAttr.eUser):
            if prop.GetName() == 'UDP3DSMAX':
                user_props = _parse_user_props_string(FbxPropertyString(prop).Get().Buffer())
                break
        prop = node.GetNextProperty(prop)
    return user_props

def _get_display_layer_name(node):
    for i in range(node.GetDstObjectCount()):
        obj = node.GetDstObject(i)
        if obj.GetTypeName() == 'DisplayLayer':
            return obj.GetName()
    return None

def _get_material_path(node):
    material_path = None
    if node.GetMaterialCount() > 1:
        print('WARNING: Object has multiple materials' % node.GetName())
    if node.GetMaterialCount() > 0:
        mat = node.GetMaterial(0)
        print('Material name: %s' % mat.GetName())
        material_path = 'materials/%s.mdoc' % mat.GetName().replace(' ', '_').lower()
    return material_path

class MeshLibrary(object):
    def __init__(self, manager, src, output_dir, *args):
        self._manager = manager
        self._src = src
        self._output_dir = output_dir
        self._args = args
        self._map = {}

    def build_and_get_path(self, node, mesh, entity_name):
        try:
            mesh_path = self._map[mesh.GetUniqueID()]
        except KeyError:
            output_file = (os.path.splitext(os.path.basename(self._src))[0] + '_' + entity_name + '.mmesh').lower()
            print('Exporting %s' % output_file)
            if not compile_fbx.build_node(self._manager, node, os.path.join(self._output_dir, output_file), *self._args):
                raise RuntimeError('Failed to export mesh')
            mesh_path = os.path.join(os.path.relpath(self._output_dir, 'assets'), output_file).replace('\\', '/').lower()
            self._map[mesh.GetUniqueID()] = mesh_path
        return mesh_path

def _export_node(manager, node, doc_container, src, mesh_lib, *args):
    ent = doc_container.add_child('entity')

    user_props = _get_user_props(node)

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

    layer_name = _get_display_layer_name(node)
    if layer_name == 'physics':
        mesh = node.GetMesh()
        assert mesh, 'Physics object had no mesh'
        
        phys = ent.add_child('physics')
        phys.add_child('type').add_child(user_props.get('type', 'static'))

        is_rect_prism = False
        if mesh.GetControlPointsCount() == 8:
            mesh.ComputeBBox()
            min_corner = mesh.BBoxMin.Get()
            max_corner = mesh.BBoxMax.Get()
            eps = 0.0000000000001
            control_points = mesh.GetControlPoints()
            is_rect_prism = True
            for i in range(8):
                cp = control_points[i]
                for j in range(3):
                    if abs(cp[j] - min_corner[j]) > eps and abs(cp[j] - max_corner[j]) > eps:
                        is_rect_prism = False
                        break

        if is_rect_prism:
            phys.add_child('shape').add_child('box')
            phys.add_child('min').add_children(min_corner)
            phys.add_child('max').add_children(max_corner)
        else:
            phys.add_child('shape').add_child('mesh')
            mesh_path = mesh_lib.build_and_get_path(node, mesh, entity_name)
            phys.add_child('mesh').add_child(mesh_path)

    elif layer_name == 'nav':
        mesh = node.GetMesh()
        assert mesh, 'Nav object had no mesh'
        nav = ent.add_child('nav_mesh')
        mesh_path = mesh_lib.build_and_get_path(node, mesh, entity_name)
        nav.add_child('mesh').add_child(mesh_path)

    else:
        mesh = node.GetMesh()
        if mesh:
            mesh_path = mesh_lib.build_and_get_path(node, mesh, entity_name)
            material_path = _get_material_path(node)
            assert material_path, 'Mesh object must have material'
            m = ent.add_child('mesh')
            m.add_child('mesh').add_child(mesh_path)
            m.add_child('material').add_child(material_path)

    children = None
    for i in range(node.GetChildCount()):
        child_node = node.GetChild(i)
        child_node_attr = node.GetNodeAttribute()
        if child_node_attr is None or child_node_attr.GetAttributeType() != FbxNodeAttribute.eMesh:
            continue
        if children is None:
            children = ent.add_child('children')
        _export_node(manager, child_node, children, src, mesh_lib, *args)

def export(src, dst, *args):
    manager, scene = InitializeSdkObjects()

    if not LoadScene(manager, scene, src):
        raise RuntimeError('Failed to load scene')

    output_dir = os.path.dirname(dst)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    mesh_lib = MeshLibrary(manager, src, output_dir, *args)
    root = doc.Node('root')

    root_node = scene.GetRootNode()
    for i in range(root_node.GetChildCount()):
        node = root_node.GetChild(i)
        node_attr = node.GetNodeAttribute()
        if node_attr is None or node_attr.GetAttributeType() != FbxNodeAttribute.eMesh:
            continue
        _export_node(manager, node, root, src, mesh_lib, *args)

    with open(dst, 'w') as file:
        for c in root.children():
            c.serialize(file)

if __name__ == '__main__':
    assert len(sys.argv) > 2, 'Must provide src and dst as command line arguments'
    export(sys.argv[1], sys.argv[2], *sys.argv[3:])
