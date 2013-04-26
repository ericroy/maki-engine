import sys
import os
import struct
from ..fbx.FbxCommon import *

def _enum_nodes(node):
    if node:
        yield node
        for i in range(node.GetChildCount()):
            for child in _enum_nodes(node.GetChild(i)):
                yield child

def _export_skeleton(root, dst):
    parts = []
    with open(dst, 'w') as file:
        for n in _enum_nodes(root):
            node_attr = n.GetNodeAttribute()
            if node_attr and node_attr.GetAttributeType() == FbxNodeAttribute.eSkeleton:
                if node_attr.GetSkeletonType() in (FbxSkeleton.eLimbNode, FbxSkeleton.eRoot):
                    # This will skip the helper widgets at the tips of skeleton fingers, feet, head, etc
                    # The skin does not bind to these, so they aren't necessary bones
                    if n.GetChildCount() == 0:
                        continue
                    
                    trans = n.LclTranslation.Get()
                    rot = n.LclRotation.Get()
                    parts.append(n)
                    parent_index = -1 if n == root else parts.index(n.GetParent())
                    file.write('%s %d, %f, %f, %f, %f, %f, %f\n' % (
                        n.GetName().lower().replace(' ', '_'),
                        parent_index,
                        trans[0], trans[1], trans[2],
                        rot[0], rot[1], rot[2]))
                elif node_attr.GetSkeletonType() in (FbxSkeleton.eLimb, FbxSkeleton.eEffector):
                    assert(False)


def export(src, dst, *args):
    manager, scene = InitializeSdkObjects()
    if not LoadScene(manager, scene, src):
        raise RuntimeError('Failed to load scene')
    for node in _enum_nodes(scene.GetRootNode()):
        node_attr = node.GetNodeAttribute()
        if node_attr and node_attr.GetAttributeType() == FbxNodeAttribute.eSkeleton:
            if node_attr.GetSkeletonType() == FbxSkeleton.eRoot:
                _export_skeleton(node, dst)
    return True

if __name__ == '__main__':
    assert len(sys.argv) > 2, 'Must provide at least a src file and dst dir'
    export(sys.argv[1], sys.argv[2], *sys.argv[3:])
