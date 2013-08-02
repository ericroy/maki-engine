import sys
import os
import tempfile
import subprocess
import struct
import re
from ctypes import *
from base64 import b64encode

from .. import doc
from .. import CONFIG

BUFFERS = ('enginePerObject', 'enginePerFrame', 'material')

D3D_COMPILER = os.path.expandvars('$MAKI_DIR/tools/fxc.exe')
D3D_BUFFER_MEMBER = re.compile(r'//\s*\S+\s+([A-Za-z0-9_]+)(?:\[\d+\])?;\s*//\s*Offset:\s*(\d+)\s+Size:\s*(\d+)\s*')
D3D_PROFILE = {'vs': 'vs_4_0', 'ps': 'ps_4_0'}

GL = ctypes.windll.opengl32

GL_FRAGMENT_SHADER = 0x8B30
GL_VERTEX_SHADER = 0x8B31
GL_COMPILE_STATUS = 0x8B81
GL_LINK_STATUS = 0x8B82


def _data(node_name, compiled):
    n = doc.Node(node_name)
    n.add_child(b64encode(compiled).decode('utf-8'))
    return n

def _d3d_external_compiler(source_file, profile_string, entry_point, defines):
    listing_file = tempfile.NamedTemporaryFile(delete=False)
    listing_file.close()
    out_file = tempfile.NamedTemporaryFile(delete=False)
    out_file.close()
    try:
        cmd = [D3D_COMPILER, '/Zi', '/nologo', '/T:'+profile_string, '/E:'+entry_point, '/Fc:'+os.path.normpath(listing_file.name),
            '/Fo:'+os.path.normpath(out_file.name), os.path.normpath(source_file)]
        cmd += ['/D%s=%s' % (var, val) for var, val in defines]
        #print(cmd)
        subprocess.check_call(cmd)
        with open(listing_file.name) as listing:
            with open(out_file.name, 'rb') as bin:
                return listing.read(), bin.read()
    finally:
        os.remove(out_file.name)
        os.remove(listing_file.name)

def _d3d_meta(node_name, listing, compiled, input_attrs=None):
    buffer_slots = {}
    buffer_contents = {}

    # Parse comments in head of listing file
    lines = iter(listing.split('\n'))
    try:
        line = next(lines).strip()
        while True:
            if line.startswith('// Resource Bindings:'):
                next(lines), next(lines), next(lines)
                line = next(lines).strip()
                while line != '//':
                    parts = line.split()
                    buffer_name = parts[1]
                    if buffer_name in BUFFERS:
                        buffer_slots[buffer_name] = int(parts[5])
                    line = next(lines).strip()
            elif input_attrs is not None and line.startswith('// Input signature:'):
                next(lines), next(lines), next(lines)
                line = next(lines).strip()
                while line != '//':
                    input_attrs.append(line.split()[1])
                    line = next(lines).strip()
            elif line.startswith('// cbuffer'):
                buffer_name = line.split()[2]
                if buffer_name in BUFFERS:
                    buffer_lines = []
                    line = next(lines).strip()
                    while True:
                        parts = line.split()
                        if len(parts) == 2 and parts[1] == '}':
                            break
                        buffer_lines.append(line)
                        line = next(lines).strip()
                    buffer_contents[buffer_name] = re.findall(D3D_BUFFER_MEMBER, '\n'.join(buffer_lines))
            line = next(lines).strip()
    except StopIteration:
        pass

    n = doc.Node(node_name)
    for buffer_name, slot in buffer_slots.items():
        buffer_node = n.add_child(buffer_name)
        buffer_node.add_child('slot').add_child(str(slot))
        uniform_node = buffer_node.add_child('uniforms')
        for var_name, var_offset, var_length in buffer_contents[buffer_name]:
            uniform_node.add_child(var_name).add_children([str(var_offset), str(var_length)])
    return n

def _d3d_compile(arc_name, is_vertex, shader_node):
    target_profile = D3D_PROFILE['vs'] if is_vertex else D3D_PROFILE['ps']
    entry_point = shader_node.resolve('entry_point.#0').get_value()
    shader_path = os.path.join(CONFIG['assets'][arc_name]['src'], shader_node.resolve('file_name.#0').get_value())

    defines = []
    try:
        for define in shader_node['defines']:
            defines.append((define.get_value(), define[0].get_value()))
    except KeyError:
        pass
    programs = {}

    input_attrs = [] if is_vertex else None

    # Generate standard version of the shader
    listing, compiled = _d3d_external_compiler(shader_path, target_profile, entry_point, defines)
    programs[''] = []
    programs[''].append(_d3d_meta('meta', listing, compiled, input_attrs))
    programs[''].append(_data('data', compiled))

    # Generate each variant
    try:
        variants = shader_node['variants']
    except KeyError:
        pass
    else:
        for variant in variants:
            variant_defines = [(variant[0], variant[0][0])]
            listing, compiled = _d3d_external_compiler(shader_path, target_profile, entry_point, defines + variant_defines)
            programs[variant.get_value()] = []
            programs[variant.get_value()].append(_d3d_meta(variant.get_value()+'_meta', listing, compiled))
            programs[variant.get_value()].append(_data(variant.get_value()+'_data', compiled))

    return (len(input_attrs), programs) if is_vertex else programs




def _ogl_meta(node_name, prog, input_attrs=None):
    buffer_slots = {}
    buffer_contents = {}

    glCreateShader

    program = GL.glCreateProgram()
    GL.glAttachShader(program, ps)
    GL.glAttachShader(program, vs)

    n = doc.Node(node_name)
    for buffer_name, slot in buffer_slots.items():
        buffer_node = n.add_child(buffer_name)
        buffer_node.add_child('slot').add_child(str(slot))
        uniform_node = buffer_node.add_child('uniforms')
        for var_name, var_offset, var_length in buffer_contents[buffer_name]:
            uniform_node.add_child(var_name).add_children([str(var_offset), str(var_length)])
    return n

def _ogl_compile_glsl(source):
    source = create_string_buffer(source)
    sh = GL.glCreateShader(GL_VERTEX_SHADER if shader_node == vs_node else GL_FRAGMENT_SHADER)
    shader_handles.append(sh)
    source_length = c_int(len(final_source))
    GL.glShaderSource(sh, 1, byref(final_source), byref(source_length))
    status = c_int()
    GL.glGetShaderiv(sh, GL_COMPILE_STATUS, byref(status))
    if status == 0:
        raise ValueError('GL shader compilation failed')
    return sh

def _ogl_compile(arc_name, vs_node, ps_node):
    shaders = {}
    for shader_node in (vs_node, ps_node):
        entry_point = shader_node.resolve('entry_point.#0').get_value()
        shader_path = os.path.join(CONFIG['assets'][arc_name]['src'], shader_node.resolve('file_name.#0').get_value())

        defines = []
        try:
            for define in shader_node['defines']:
                defines.append((define.get_value(), define[0].get_value()))
        except KeyError:
            pass
        define_code = '\n'.join(['#define %s %s' % (key, value) for key, value in defines]) + '\n'

        with open(shader_path) as file:
            source = file.read()
        
        shaders[''][shader_node.get_value()] = _ogl_compile_glsl(define_code + source)
        try:
            variants = shader_node['variants']
        except KeyError:
            pass
        else:
            for variant in variants:
                variant_define_code = '#define %s %s\n\n' % (variant[0], variant[0][0])
                shaders[variant][shader_node.get_value()] = _ogl_compile_glsl(variant_define_code + define_code + source)

    # prog = GL.glCreateProgram()
    # GL.glAttachShader(prog, shader_handles[0])
    # GL.glAttachShader(prog, shader_handles[1])

    # GL.glLinkProgram(prog)
    # status = c_int()
    # GL.glGetShaderiv(sh, GL_LINK_STATUS, byref(status))
    # if status == 0:
    #     raise ValueError('GL shader link failed')





def _compile(arc_name, vs_node, ps_node):
    if CONFIG['render_api'] == 'd3d':
        input_attr_count, vs_programs = _d3d_compile(arc_name, True, vs_node)
        ps_programs = _d3d_compile(arc_name, False, ps_node)
    else:
        input_attr_count, vs_programs, ps_programs = _ogl_compile(arc_name, vs_node, ps_node)
    return input_attr_count, vs_programs, ps_programs

def compile(arc_name, src, dst):
    with open(src) as file:
        root = doc.deserialize(file.read())

    input_attr_count, vs_programs, ps_programs = _compile_shader(arc_name, root['vertex_shader'], root['pixel_shader'])
    
    out_nodes = []

    iac_node = doc.Node('input_attribute_count')
    iac_node.add_child(str(input_attr_count))
    out_nodes.append(iac_node)

    vs_node = doc.Node('vertex_shader')
    for key, nodes in vs_programs.items():
        for node in nodes:
            vs_node.add_child(node)
    out_nodes.append(vs_node)

    ps_node = doc.Node('pixel_shader')
    for key, nodes in ps_programs.items():
        for node in nodes:
            ps_node.add_child(node)
    out_nodes.append(ps_node)

    with open(dst, 'w') as out:
        for n in out_nodes:
            n.serialize(out)
    