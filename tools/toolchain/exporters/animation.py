import sys
import os
import struct
from collections import OrderedDict
import pprint
from ..fbx.FbxCommon import *

def _enum_nodes(node):
    if node:
        yield node
        for i in range(node.GetChildCount()):
            for child in _enum_nodes(node.GetChild(i)):
                yield child

def _export_animation(scene, anim_layer, dst):
    bones = OrderedDict()
    anim_name = ''

    for n in _enum_nodes(scene.GetRootNode()):
        node_attr = n.GetNodeAttribute()
        if node_attr and node_attr.GetAttributeType() == FbxNodeAttribute.eSkeleton:
            node_type = node_attr.GetSkeletonType()
            if node_type == FbxSkeleton.eRoot:
                bone_index = 0
                anim_name = n.GetName()
            if node_type not in (FbxSkeleton.eLimbNode, FbxSkeleton.eRoot):
                continue
            # This will skip the helper widgets at the tips of skeleton fingers, feet, head, etc
            # The skin does not bind to these, so they aren't necessary bones
            if n.GetChildCount() == 0:
                continue

            key_frames = OrderedDict()
            name = n.GetName().lower().replace(' ', '_')
            for xform_letter, xform_type in (('T', 'LclTranslation'), ('R', 'LclRotation')):
                for component in ('X', 'Y', 'Z'):
                    anim_curve = getattr(n, xform_type).GetCurve(anim_layer, '%s' % component)
                    if anim_curve:
                        for ki in range(anim_curve.KeyGetCount()):
                            key_value = anim_curve.KeyGetValue(ki)
                            key_time  = anim_curve.KeyGetTime(ki)
                            frame = key_frames.setdefault(key_time.GetFrameCount(), {})
                            xform = frame.setdefault(xform_letter, {})
                            xform[component] = float(key_value)
            bones[bone_index] = key_frames
            bone_index += 1
    bone_count = bone_index

    with open(dst, 'w') as file:
        file.write('bone_count %d\n' % bone_count)
        file.write('frame_rate %f\n' % float(FbxTime.GetFrameRate(FbxTime.GetGlobalTimeMode())))
        
        timeline_span = scene.GetGlobalSettings().GetTimelineDefaultTimeSpan()
        timeline_start = timeline_span.GetStart().GetFrameCount()
        timeline_stop  = timeline_span.GetStop().GetFrameCount()
        file.write('frame_count %d\n' % (timeline_stop-timeline_start+1))

        for bone_index in range(bone_count):
            file.write('bone\n')
            key_frames = bones.get(bone_index)
            for key_time, xform in key_frames.items():
                t = (xform['T']['X'], xform['T']['Y'], xform['T']['Z'])
                r = (xform['R']['X'], xform['R']['Y'], xform['R']['Z'])
                file.write('\t%d %f, %f, %f, %f, %f, %f\n' % (
                    key_time-timeline_start, t[0], t[1], t[2], r[0], r[1], r[2]))

def export(src, dst, *args):
    manager, scene = InitializeSdkObjects()
    if not LoadScene(manager, scene, src):
        raise RuntimeError('Failed to load scene')
    for i in range(scene.GetSrcObjectCount(FbxAnimStack.ClassId)):
        animStack = scene.GetSrcObject(FbxAnimStack.ClassId, i)
        for j in range(animStack.GetSrcObjectCount(FbxAnimLayer.ClassId)):
            _export_animation(scene, animStack.GetSrcObject(FbxAnimLayer.ClassId, j), dst)
            break
        break
    return True

if __name__ == '__main__':
    assert len(sys.argv) > 2, 'Must provide at least a src file and dst dir'
    export(sys.argv[1], sys.argv[2], *sys.argv[3:])
