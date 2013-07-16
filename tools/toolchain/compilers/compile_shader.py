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

def compile(src, dst, *args):
    cgc = os.path.expandvars('$MAKI_DIR/tools/cgc.exe')
    binary = '0'

    ps_lines = {}
    vs_lines = {}
    line_buffer = None
    with open(src) as file:
        for line in file:
            if line.startswith('vertex_shader'):
                line_buffer = vs_lines
            elif line.startswith('pixel_shader'):
                line_buffer = ps_lines    
            assert line_buffer != None, "invalid mshad file"
            line = line.strip()
            if not line:
                continue
            keyvals = line.split(" ", 1)
            if len(keyvals) > 1:
                key = keyvals[0]
                vals = keyvals[1].split()
            else:
                key = keyvals[0]
                vals = []
            line_buffer[key] = vals

    shaders = {'pixel_shader': ps_lines, 'vertex_shader': vs_lines}
    assets_path = os.path.join(CONFIG['project_root'], CONFIG['assets_path'])
    compiled_shaders = {
        'pixel_shader': {},
        'vertex_shader': {}
    }

    for shader_type, shader in shaders.items():
        for variant in ('', 'shadow', 'depth'):
            for api in ('d3d', 'ogl'):
                cmd = [cgc, '-q']
                if api == 'd3d':
                    cmd.append('-d3d')
                cmd += ['-entry', shader['entry_point'][0]]
                target = shader['target'][0]
                if api is 'ogl':
                    target = 'glsl' + ('f' if shader_type is 'pixel_shader' else 'v')
                cmd += ['-profile', target]
                
                filename = os.path.normpath(os.path.join(assets_path, shader['file_name'][0]))
                
                output_file = tempfile.NamedTemporaryFile(delete=False)
                output_file.close()
                
                cmd += ["-o", output_file.name]
                cmd.append(filename)
                subprocess.check_call(cmd)

                with open(output_file.name, 'rb') as file:
                    data = file.read()
                compiled_shaders[shader_type][variant] = data
                os.remove(output_file.name)

    with open(dst, 'w') as out:
        for shader_type, variants in compiled_shaders.items():
            shader_root = doc.Node(shader_type)
            for variant, data in variants.items():
                variant_data_node = variant+'_data' if variant else 'data'
                variant_meta_node = variant+'_meta' if variant else 'meta'
                shader_root.add_child(variant_data_node).add_child(b64encode(data).decode('utf-8'))
                shader_root.add_child(variant_meta_node) #.add_child(b64encode(data))
            shader_root.serialize(out)
