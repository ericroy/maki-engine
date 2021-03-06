import os
import logging
import shutil
import subprocess
from .. import doc, util, CONFIG

NVDXT = os.path.expandvars('$MAKI_DIR/tools/nvdxt.exe')

def compile(arc_name, src, dst):
    conf = CONFIG['assets'][arc_name]
    with open(src) as file:
        root = doc.deserialize(file.read())
    sheets = root.resolve('sprite_sheets')
    for sheet in sheets.children():
        val = sheet.resolve('path.#0')
        image_path_src = val.value
        image_path_dst = os.path.splitext(dst)[0] + '.dds'
        val.value = util.clean_path(os.path.relpath(image_path_dst, conf['dst']))
        subprocess.check_call([NVDXT, '-file', image_path_src, '-output', image_path_dst, '-dxt5'])

    with open(dst, 'w') as file:
        root.serialize(file, max_stack=6)
