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

def export(src, dst, *args):
    manager, scene = InitializeSdkObjects()

    if not LoadScene(manager, scene, src):
        raise RuntimeError('Failed to load scene')

    output_dir = os.path.dirname(dst)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    uid_to_mesh_path = {}
    root = doc.Node('root')
    objects = root.add_child('children')

    root_node = scene.GetRootNode()
    for i in range(root_node.GetChildCount()):
        node = root_node.GetChild(i)
        node_attr = node.GetNodeAttribute()
        if node_attr is None or node_attr.GetAttributeType() != FbxNodeAttribute.eMesh:
            continue

        mesh = node.GetMesh()
        mesh_name = node.GetName().strip().replace(' ', '')
        try:
            mesh_path = uid_to_mesh_path[mesh.GetUniqueID()]
        except KeyError:
            output_file = (os.path.splitext(os.path.basename(src))[0] + '_' + mesh_name + '.mmesh').lower()
            print('Exporting %s' % output_file)
            if not compile_fbx.build_node(manager, node, os.path.join(output_dir, output_file), *args):
                raise RuntimeError('Failed to export mesh')
            mesh_path = os.path.join(os.path.relpath(output_dir, 'assets'), output_file).replace('\\', '/').lower()
            uid_to_mesh_path[mesh.GetUniqueID()] = mesh_path

        material_path = None
        if node.GetMaterialCount() > 1:
            print('WARNING: Object has multiple materials: %s' % mesh_name)
        if node.GetMaterialCount() > 0:
            mat = node.GetMaterial(0)
            print('Material name: %s' % mat.GetName())
            material_path = 'materials/%s.mdoc' % mat.GetName().replace(' ', '_').lower()
        
        user_props = {}
        prop = node.GetFirstProperty()
        while prop.IsValid():
            if prop.GetFlag(FbxPropertyAttr.eUser):
                if prop.GetName() == 'UDP3DSMAX':
                    user_props = _parse_user_props(FbxPropertyString(prop).Get().Buffer())
                    break
            prop = node.GetNextProperty(prop)

        flags = doc.Node('flags')
        user_flags = user_props.get('flags', '')
        for f in user_flags.split():
            flags.add_child(f)
        for i in range(node.GetDstObjectCount()):
            obj = node.GetDstObject(i)
            if obj.GetTypeName() == 'DisplayLayer':
                if obj.GetName() == 'physics':
                    flags.add_child('physics')

        trans = node.LclTranslation.Get()
        angles = node.LclRotation.Get()
        n = doc.Node('mesh_entity')
        if len(flags):
            n.add_child(flags)
        n.add_child('mesh').add_child(mesh_path)
        if material_path:
            n.add_child('material').add_child(material_path)
        if any(trans):
            n.add_child('pos').add_children(trans)
        if any(angles):
            n.add_child('angles').add_children(angles)
        objects.add_child(n)

    with open(dst, 'w') as file:
        root.serialize(file)

if __name__ == '__main__':
    assert len(sys.argv) > 2, 'Must provide src and dst as command line arguments'
    export(sys.argv[1], sys.argv[2], *sys.argv[3:])
