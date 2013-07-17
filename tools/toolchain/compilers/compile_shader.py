import os
import sys
import shutil
import subprocess
import tempfile
from base64 import b64encode
from io import BytesIO
from .. import CONFIG
from .. import util
from .. import doc

API = 'd3d'
CGC = os.path.expandvars('$MAKI_DIR/tools/cgc.exe')
ASSETS_PATH = os.path.join(CONFIG['project_root'], CONFIG['assets_path'])

def _cgc(api, shader_type, variant, shader):
    cmd = [CGC, '-q', '-O3']       #, '-dumpinputbind', 'bind.txt']
    if api == 'd3d':
        cmd.append('-d3d')
    cmd += ['-entry', shader['entry_point'][0]]
    target = shader['target'][0]
    if api == 'ogl':
        target = 'glsl' + ('f' if shader_type == 'pixel_shader' else 'v')
    cmd += ['-profile', target]

    defs = shader['variants'][variant]
    assert len(defs) % 2 == 0
    for i in range(0, len(defs), 2):
        cmd.append('-D%s=%s' % (defs[i], defs[i+1]))
    
    filename = os.path.normpath(os.path.join(ASSETS_PATH, shader['file_name'][0]))
    
    output_file = tempfile.NamedTemporaryFile(delete=False)
    output_file.close()
    
    cmd += ["-o", output_file.name]
    cmd.append(filename)
    #print(cmd)
    subprocess.check_call(cmd)

    with open(output_file.name, 'rb') as file:
        data = file.read()
    os.remove(output_file.name)
    return data

def _parse_line(line):
    line = line.strip()
    if not line:
        return None, None
    keyvals = line.split(" ", 1)
    if len(keyvals) > 1:
        key = keyvals[0]
        vals = keyvals[1].split()
    else:
        key = keyvals[0]
        vals = []
    return key, vals

def _leading_spaces(line):
    return len(line.rstrip()) - len(line.strip())

def _read_shader_defs(filename):
    ps_lines = {}
    ps_variants = {'': []}
    vs_lines = {}
    vs_variants = {'': []}
    
    with open(filename) as file:
        lines = list(file)
    
    line_buffer = None
    variant_buffer = None
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith('vertex_shader'):
            line_buffer, variant_buffer = vs_lines, vs_variants
        elif line.startswith('pixel_shader'):
            line_buffer, variant_buffer = ps_lines, ps_variants
        assert line_buffer != None, "invalid mshad file"
        key, vals = _parse_line(line)
        i += 1
        if not key:
            continue
        line_buffer[key] = vals
        if key == 'variants':
            leading_spaces = _leading_spaces(line)
            while i < len(lines) and _leading_spaces(lines[i]) > leading_spaces:
                # This is a variant
                key, vals = _parse_line(lines[i])
                variant_buffer[key] = vals
                i += 1

    ps_lines['variants'] = ps_variants
    vs_lines['variants'] = vs_variants
    shaders = {'pixel_shader': ps_lines, 'vertex_shader': vs_lines}
    return shaders

def compile(src, dst, *args):
    shaders = _read_shader_defs(src)
    compiled_shaders = {'pixel_shader': {}, 'vertex_shader': {}}

    for shader_type, shader in shaders.items():
        for variant in shader['variants']:
            compiled_shaders[shader_type][variant] = _cgc(API, shader_type, variant, shader)

    with open(dst, 'w') as out:
        for shader_type, variants in compiled_shaders.items():
            shader_root = doc.Node(shader_type)
            for variant, data in variants.items():
                variant_data_node = variant+'_data' if variant else 'data'
                variant_meta_node = variant+'_meta' if variant else 'meta'
                shader_root.add_child(variant_data_node).add_child(b64encode(data).decode('utf-8'))
                shader_root.add_child(variant_meta_node) #.add_child(b64encode(data))
            shader_root.serialize(out)
