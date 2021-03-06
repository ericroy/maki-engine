import os
import logging
import shutil
import subprocess
from .. import doc, util, CONFIG

NVDXT = os.path.expandvars('$MAKI_DIR/tools/nvdxt.exe')

PPU = 150

def try_get_value(node, key, default=None):
    try:
        return node.resolve(key).value
    except KeyError:
        return default

def try_get(node, key, default=None):
    try:
        return node.resolve(key)
    except KeyError:
        return default

def compile(arc_name, src, dst):
    conf = CONFIG['assets'][arc_name]
    with open(src) as file:
        root = doc.deserialize(file.read())

    out_root = doc.Node('<root>')
    out = out_root.add_child('entity')
    out.add_child('transform').add_child('pos').add_children(['0']*3)
    out = out.add_child('children')

    entities = []

    for layer_index, layer in enumerate(root.children()):
        name = layer.resolve('name.#0').value
        rectNode = layer.resolve('pos')
        layer_pos = int(rectNode.resolve('#0').value), int(rectNode.resolve('#1').value)

        try:
            meta = layer.resolve('meta')
        except KeyError:
            meta = doc.Node('meta')

        depth = float(try_get_value(meta, 'depth.#0', 0))

        entity = try_get(meta, 'entity')
        if entity is not None:
            meta.remove_child(entity)
            entities.append(entity)
            entity.add_child('transform').add_child('pos').add_children(map(str, [layer_pos[0] / PPU, -layer_pos[1] / PPU, depth]))
            continue

        out_layer = out.add_child('entity')
        out_layer.add_child('name').add_child(name)
        out_layer.add_child('transform').add_child('pos').add_children(map(str, [layer_pos[0] / PPU, -layer_pos[1] / PPU, depth]))
        out_layer.add_child(meta)
        out_layer = out_layer.add_child('children')

        tiles = layer.resolve('tiles')
        for tile in tiles.children():
            val = tile.resolve('path.#0')
            image_path_src = val.value
            image_path_dst = os.path.splitext(image_path_src)[0] + '.dds'
            texture_path = util.clean_path(os.path.relpath(image_path_dst, conf['src']))

            subprocess.check_call([NVDXT, '-file', image_path_src, '-output', os.path.join(conf['dst'], texture_path), '-dxt5', '-nomipmap'])

            rectNode = tile.resolve('rect')
            tileRect = [int(rectNode.resolve('#%s' % i).value) for i in range(4)]

            out_tile = out_layer.add_child('entity')
            out_tile.add_child('transform').add_child('pos').add_children(map(str, [tileRect[0] / PPU, -tileRect[1] / PPU, 0]))
            out_sprite = out_tile.add_child('mesh')
            out_sprite.add_child('material').add_child('materials/sprite.mdoc')
            out_sprite_props = out_sprite.add_child('sprite')

            out_sprite_props.add_child('texture').add_child(texture_path)
            out_sprite_props.add_child('rect').add_children(map(str, [0, 0, tileRect[2], tileRect[3]]))

    # At the root of the document, add the entities that were specified in the meta nodes
    out.add_children(entities)

    with open(dst, 'w') as file:
        out_root.serialize(file, max_stack=6)
