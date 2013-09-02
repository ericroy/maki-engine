import os
import logging
import shutil
import subprocess
from .. import doc, util, CONFIG

NVDXT = os.path.expandvars('$MAKI_DIR/tools/nvdxt.exe')

PPU = 150

def compile(arc_name, src, dst):
    conf = CONFIG['assets'][arc_name]
    with open(src) as file:
        root = doc.deserialize(file.read())

    out_root = doc.Node('<root>')
    out = out_root.add_child('entity')
    out.add_child('transform').add_child('pos').add_children(['0']*3)
    out = out.add_child('children')

    for layer in root.children():
        name = layer.resolve('name.#0').value
        rectNode = layer.resolve('pos')
        layer_pos = int(rectNode.resolve('#0').value), int(rectNode.resolve('#1').value)

        out_layer = out.add_child('entity')
        out_layer.add_child('name').add_child(name)
        out_layer.add_child('transform').add_child('pos').add_children(
            map(str, [layer_pos[0] / PPU, -layer_pos[1] / PPU, 0])
            )
        out_layer = out_layer.add_child('children')

        tiles = layer.resolve('tiles')
        for tile in tiles.children():
            val = tile.resolve('path.#0')
            image_path_src = val.value
            image_path_dst = os.path.splitext(image_path_src)[0] + '.dds'
            texture_path = util.clean_path(os.path.relpath(image_path_dst, conf['src']))
            subprocess.check_call([NVDXT, '-file', image_path_src, '-output', image_path_dst, '-dxt5'])

            rectNode = tile.resolve('rect')
            tileRect = [int(rectNode.resolve('#%s' % i).value) for i in range(4)]

            out_tile = out_layer.add_child('entity')
            out_tile.add_child('transform').add_child('pos').add_children(
                map(str, [tileRect[0] / PPU, -tileRect[1] / PPU, 0])
                )
            out_sprite = out_tile.add_child('sprite')
            out_sprite.add_child('texture').add_child(texture_path)
            out_sprite.add_child('region').add_children(
                    map(str, [0, 0, tileRect[2], tileRect[3]])
                )

    with open(dst, 'w') as file:
        out_root.serialize(file, max_stack=6)
