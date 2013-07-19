import os
import sys
import shutil
import subprocess
import tempfile
from ctypes import *
from base64 import b64encode
from io import BytesIO
from .. import CONFIG
from .. import util
from .. import doc


CG_SOURCE = 4112
CG_COMPILED_PROGRAM = 4106

CG_PROFILE_VS_4_0 = 6167
CG_PROFILE_PS_4_0 = 6168
CG_PROFILE_GLSLV = 7007
CG_PROFILE_GLSLF = 7008

CG_PROFILE_UNKNOWN = 6145
CG_PROFILE_VS_2_0 = 6154
CG_PROFILE_VS_1_1 = 6153

CG_NO_ERROR = 0
CG_COMPILER_ERROR = 1
CG_INVALID_PARAM_HANDLE_ERROR = 18
CG_BUFFER_INDEX_OUT_OF_RANGE_ERROR = 58

CG_BEHAVIOR_3100 = 3000


API = 'd3d'
ASSETS_PATH = os.path.join(CONFIG['project_root'], CONFIG['assets_path'])
PROFILES = {
    'pixel_shader': {
        'd3d': CG_PROFILE_PS_4_0,
        'ogl': CG_PROFILE_GLSLF
    },
    'vertex_shader': {
        'd3d': CG_PROFILE_VS_4_0,
        'ogl': CG_PROFILE_GLSLV
    },
}

CG = CDLL(os.path.expandvars('$MAKI_DIR/tools/cg.dll'))
#CGD3D = windll.LoadLibrary(os.path.expandvars('$MAKI_DIR/tools/cgD3D10.dll'))
#CGGL = windll.LoadLibrary(os.path.expandvars('$MAKI_DIR/tools/cgGL.dll'))

class CGError(Exception):
    def __init__(self, context, err=None):
        if err is None:
            err = CG.cgGetError()
        if err == CG_COMPILER_ERROR:
            s = c_char_p(CG.cgGetLastListing(context)).value
        else:
            s = c_char_p(CG.cgGetLastErrorString(byref(err))).value
        Exception.__init__(self, s)

def _build_meta_data(context, program):
    print('Building meta')
    meta_nodes = []
    for name in ('enginePerFrame', 'enginePerObject', 'material'):
        buff = CG.cgGetNamedProgramUniformBuffer(program, c_char_p(name))
        if buff == 0:
            continue
        print('Found buffer')

        n = doc.Node(buffer_name)
        n.add_child('slot').add_child('%d' % i)
        unis = n.add_child('uniforms')

        param = CG.cgGetFirstUniformBufferParameter(buff)
        while param != 0:
            var = unis.add_child(c_char_p(CG.cgGetParameterName(param)).value)
            var.add_child('%d' % CG.cgGetParameterBufferOffset(param))
            var.add_child('%d' % CG.cgGetParameterResourceSize(param))
            param = CG.cgGetNextParameter(param)
        meta_nodes.append(n)
        i += 1

    print('Done building meta')
    return meta_nodes


def _cg_compile(api, shader_type, variant, shader):
    filename = os.path.abspath(os.path.join(ASSETS_PATH, shader['file_name'][0]))
    print('Compiling file: %s' % filename)

    print('Creating context')
    context = CG.cgCreateContext()
    if context == 0:
        raise CGError(context)
    CG.cgSetContextBehavior(CG_BEHAVIOR_3100)

    try:
        profile = PROFILES[shader_type][api]
        args = []
        if api == 'd3d':
            args += ['-d3d', '-po', 'pad16']
        else:
            args += ['-po', 'version=140']

        defs = []
        try:
            defs += shader['variants'][variant]
        except KeyError:
            pass
        defs += shader.get('defines', [])
        
        for k, v in defs:
            args.append('-D%s=%s' % (k, v))
        args.append(None)
        
        program = CG.cgCreateProgramFromFile(context, CG_SOURCE, c_char_p(filename), profile, c_char_p(shader['entry_point'][0]), (c_char_p * len(args))(*args))
        if program == 0:
            raise CGError(context)
        try:
            meta_nodes = _build_meta_data(context, program)
            compiled_program = bytes(c_char_p(CG.cgGetProgramString(program, CG_COMPILED_PROGRAM)).value, 'utf-8')
            if len(compiled_program) == 0:
                raise CGError(context)
        finally:
            CG.cgDestroyProgram(program)
    finally:
        CG.cgDestroyContext(context)

    print(compiled_program)
    return compiled_program, meta_nodes

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
    vs_lines = {}
    
    with open(filename) as file:
        lines = list(file)
    
    line_buffer = None
    variant_buffer = None
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith('vertex_shader'):
            line_buffer = vs_lines
        elif line.startswith('pixel_shader'):
            line_buffer = ps_lines
        assert line_buffer != None, "invalid mshad file"
        key, vals = _parse_line(line)
        i += 1
        if not key:
            continue
        if key == 'variants':
            line_buffer[key] = {'': []}
            leading_spaces = _leading_spaces(line)
            while i < len(lines) and _leading_spaces(lines[i]) > leading_spaces:
                key2, vals = _parse_line(lines[i])
                assert len(vals) == 2
                line_buffer[key].setdefault(key2, []).append(vals)
                i += 1
        elif key == 'defines':
            line_buffer[key] = []
            leading_spaces = _leading_spaces(line)
            while i < len(lines) and _leading_spaces(lines[i]) > leading_spaces:
                key2, vals = _parse_line(lines[i])
                assert len(vals) == 1
                line_buffer[key].append([key2, vals[0]])
                i += 1
        else:
            line_buffer[key] = vals

    shaders = {'pixel_shader': ps_lines, 'vertex_shader': vs_lines}
    return shaders

def compile(src, dst, *args):
    shaders = _read_shader_defs(src)
    compiled_shaders = {'pixel_shader': {}, 'vertex_shader': {}}
    meta_nodes = {'pixel_shader': {}, 'vertex_shader': {}}

    for shader_type, shader in shaders.items():
        for variant in shader['variants']:
            data, meta = _cg_compile(API, shader_type, variant, shader)
            compiled_shaders[shader_type][variant] = data
            meta_nodes[shader_type][variant] = meta

    with open(dst, 'w') as out:
        for shader_type, variants in compiled_shaders.items():
            shader_root = doc.Node(shader_type)
            for variant, data in variants.items():
                variant_data_node = variant+'_data' if variant else 'data'
                variant_meta_node = variant+'_meta' if variant else 'meta'
                shader_root.add_child(variant_data_node).add_child(b64encode(data).decode('utf-8'))
                shader_root.add_child(variant_meta_node).add_children(meta_nodes[shader_type][variant])
            shader_root.serialize(out)
