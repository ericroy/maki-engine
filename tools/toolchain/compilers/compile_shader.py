import os
import sys
import shutil
import subprocess
import tempfile
import platform
from ctypes import *
from base64 import b64encode
from io import BytesIO
from .. import CONFIG
from .. import util
from .. import doc


CG_SOURCE = 4112
CG_COMPILED_PROGRAM = 4106

CG_NO_ERROR = 0
CG_COMPILER_ERROR = 1
CG_INVALID_PARAM_HANDLE_ERROR = 18
CG_BUFFER_INDEX_OUT_OF_RANGE_ERROR = 58

CG_BEHAVIOR_3100 = 3000

CG_GLOBAL = 4108
CG_PROGRAM = 4109

CG_BUFFER = 1319
CG_UNIFORMBUFFER = 1320
CG_STRUCT = 1


CG_PROFILE_UNKNOWN = 6145
CG_PROFILE_VP20    = 6146
CG_PROFILE_FP20    = 6147
CG_PROFILE_VP30    = 6148
CG_PROFILE_FP30    = 6149
CG_PROFILE_ARBVP1  = 6150
CG_PROFILE_FP40    = 6151
CG_PROFILE_ARBFP1  = 7000
CG_PROFILE_VP40    = 7001
CG_PROFILE_GLSLV   = 7007 # GLSL vertex shader
CG_PROFILE_GLSLF   = 7008 # GLSL fragment shader
CG_PROFILE_GLSLG   = 7016 # GLSL geometry shader
CG_PROFILE_GLSLC   = 7009 # Combined GLSL program
CG_PROFILE_GPU_FP  = 7010 # Deprecated alias for CG_PROFILE_GP4FP
CG_PROFILE_GPU_VP  = 7011 # Deprecated alias for CG_PROFILE_GP4VP
CG_PROFILE_GPU_GP  = 7012 # Deprecated alias for CG_PROFILE_GP4GP
CG_PROFILE_GP4FP   = 7010 # NV_gpu_program4 fragment program
CG_PROFILE_GP4VP   = 7011 # NV_gpu_program4 vertex program
CG_PROFILE_GP4GP   = 7012 # NV_gpu_program4 geometry program
CG_PROFILE_GP5FP   = 7017 # NV_gpu_program5 fragment program
CG_PROFILE_GP5VP   = 7018 # NV_gpu_program5 vertex program
CG_PROFILE_GP5GP   = 7019 # NV_gpu_program5 geometry program
CG_PROFILE_GP5TCP  = 7020 # NV_tessellation_program5 tessellation control program
CG_PROFILE_GP5TEP  = 7021 # NV_tessellation_program5 tessellation evaluation prog
CG_PROFILE_VS_1_1  = 6153
CG_PROFILE_VS_2_0  = 6154
CG_PROFILE_VS_2_X  = 6155
CG_PROFILE_VS_2_SW = 6156
CG_PROFILE_PS_1_1  = 6159
CG_PROFILE_PS_1_2  = 6160
CG_PROFILE_PS_1_3  = 6161
CG_PROFILE_PS_2_0  = 6162
CG_PROFILE_PS_2_X  = 6163
CG_PROFILE_PS_2_SW = 6164
CG_PROFILE_VS_3_0  = 6157 # DX9 vertex shader
CG_PROFILE_PS_3_0  = 6165 # DX9 pixel shader
CG_PROFILE_HLSLV   = 6158 # DX9 HLSL vertex shader
CG_PROFILE_HLSLF   = 6166 # DX9 HLSL fragment shader
CG_PROFILE_VS_4_0  = 6167 # DX10 vertex shader
CG_PROFILE_PS_4_0  = 6168 # DX10 pixel shader
CG_PROFILE_GS_4_0  = 6169 # DX10 geometry shader
CG_PROFILE_VS_5_0  = 6170 # DX11 vertex shader
CG_PROFILE_PS_5_0  = 6171 # DX11 pixel shader
CG_PROFILE_GS_5_0  = 6172 # DX11 geometry shader
CG_PROFILE_HS_5_0  = 6173 # DX11 hull shader (tessellation control
CG_PROFILE_DS_5_0  = 6174 # DX11 domain shader (tessellation evaluation
CG_PROFILE_GENERIC = 7002



CG_GL_GLSL_DEFAULT = 0
CG_GL_GLSL_100     = 1
CG_GL_GLSL_110     = 2
CG_GL_GLSL_120     = 3




API = 'ogl' if platform.system() == 'Windows' else 'ogl'


ASSETS_PATH = os.path.join(CONFIG['project_root'], CONFIG['assets_path'])
D3D_COMPILER = os.path.expandvars('$MAKI_DIR/tools/fxc.exe')

PROFILES = {
    'vertex_shader': {
        'd3d': CG_PROFILE_VS_4_0,
        'ogl': CG_PROFILE_GLSLV
    },
    'pixel_shader': {
        'd3d': CG_PROFILE_PS_4_0,
        'ogl': CG_PROFILE_GLSLF
    },
}

CG = CDLL(os.path.expandvars('$MAKI_DIR/tools/cg.dll'))

class CGError(Exception):
    def __init__(self, context, err=None):
        if err is None:
            err = CG.cgGetError()
        if err == CG_COMPILER_ERROR:
            msg = c_char_p(CG.cgGetLastListing(context)).value
        else:
            msg = c_char_p(CG.cgGetErrorString(err)).value
        Exception.__init__(self, msg)

def _check_error(context):
    err = CG.cgGetError()
    if err != CG_NO_ERROR:
        raise CGError(context, err)

def _build_meta_data(context, program):
    #print('Building meta')
    meta_nodes = []
    for name in ('enginePerFrame', 'enginePerObject', 'material'):
        buff_param = CG.cgGetNamedProgramUniformBuffer(program, c_char_p(name))
        if buff_param == 0:
            # Consume the error
            CG.cgGetError()
            continue

        print('Found param named:', name)
        buff_type = CG.cgGetParameterType(buff_param)
        if buff_type != CG_UNIFORMBUFFER:
            raise RuntimeError('%s must be a uniform buffer (actual type was %d)' % (name, buff_type))
        _check_error(context)

        print('Found buffer %s' % name)

        n = doc.Node(name)
        unis = doc.Node('uniforms')
        slot = -1

        param = CG.cgGetFirstUniformBufferParameter(buff_param)
        _check_error(context)
        if param == 0:
            raise RuntimeError('Failed to get first param of buffer')
        while param != 0:
            name = c_char_p(CG.cgGetParameterName(param)).value
            _check_error(context)

            slot = CG.cgGetParameterBufferIndex(param)

            name = name.split('.')[1]
            var = unis.add_child(name)
            var.add_child('%d' % CG.cgGetParameterBufferOffset(param))
            _check_error(context)

            var.add_child('%d' % CG.cgGetParameterResourceSize(param))
            _check_error(context)

            param = CG.cgGetNextParameter(param)
            _check_error(context)
        
        if slot >= 0:
            n.add_child('slot').add_child('%d' % slot)
            n.add_child(unis)
            meta_nodes.append(n)

    #print('Done building meta')
    return meta_nodes

def _count_input_params(context, program):
    count = 0

    param = CG.cgGetFirstParameter(program, CG_PROGRAM)
    _check_error(context)
    while param != 0:

        name = c_char_p(CG.cgGetParameterName(param)).value
        _check_error(context)
        print('Program param name: %s' % name)

        param_type = CG.cgGetParameterType(param)
        _check_error(context)
        if param_type == CG_STRUCT:
            p = CG.cgGetFirstStructParameter(param)
            _check_error(context)
            while p != 0:
                count += 1
                name = c_char_p(CG.cgGetParameterName(p)).value
                _check_error(context)
                print('Program input struct param name: %s' % name)

                p = CG.cgGetNextParameter(p)
                _check_error(context)
        else:
            assert False, "Haven't handled case where main funtion takes loose parameters (must take a single struct arg atm).  Type of arg was: %s" % param_type

        param = CG.cgGetNextParameter(param)
        _check_error(context)

    return count

def _d3d_compile(source_code, profile_string, entry_point):
    in_file = tempfile.NamedTemporaryFile(delete=False)
    out_file = tempfile.NamedTemporaryFile(delete=False)
    out_file.close()

    try:
        in_file.write(source_code)
        in_file.close()

        cmd = [D3D_COMPILER, '/Zi', '/nologo', '/T:'+profile_string, '/E:'+entry_point, '/Fo:'+os.path.normpath(out_file.name), os.path.normpath(in_file.name)]
        subprocess.check_call(cmd)
                
        with open(out_file.name, 'rb') as f:
            return f.read()

    finally:
        os.remove(in_file.name)
        os.remove(out_file.name)

def _cg_compile(api, shader_type, variant, shader):
    num_inputs = 0

    filename = os.path.abspath(os.path.join(ASSETS_PATH, shader['file_name'][0]))
    print('Compiling file: %s' % filename)

    #print('Creating context')
    context = CG.cgCreateContext()
    if context == 0:
        raise RuntimeError('Could not create CG context')

    try:
        CG.cgSetContextBehavior(context, CG_BEHAVIOR_3100)
        _check_error(context)

        profile = PROFILES[shader_type][api]
        args = []
        if api == 'd3d':
            args += ['-d3d', '-po', 'pad16']
        else:
            args += ['-po', 'version=330']

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
        _check_error(context)

        try:
            meta_nodes = _build_meta_data(context, program)
            compiled_program = bytes(c_char_p(CG.cgGetProgramString(program, CG_COMPILED_PROGRAM)).value, 'utf-8')
            _check_error(context)

            if shader_type == 'vertex_shader':
                num_inputs = _count_input_params(context, program)
                _check_error(context)

        finally:
            CG.cgDestroyProgram(program)
    finally:
        CG.cgDestroyContext(context)

    if api == 'd3d':
        compiled_program = _d3d_compile(compiled_program, 'vs_4_0' if shader_type == 'vertex_shader' else 'ps_4_0', shader['entry_point'][0])

    #print(compiled_program)
    return compiled_program, meta_nodes, num_inputs

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
    input_attribute_count = None

    for shader_type, shader in shaders.items():
        for variant in shader['variants']:
            data, meta, num_inputs = _cg_compile(API, shader_type, variant, shader)
            compiled_shaders[shader_type][variant] = data
            meta_nodes[shader_type][variant] = meta
            if shader_type == 'vertex_shader':
                if input_attribute_count is not None:
                    assert input_attribute_count == num_inputs, "Different variants of the shader disagreed on the number of input attributes"
                input_attribute_count = num_inputs


    with open(dst, 'w') as out:
        attr_count = doc.Node('input_attribute_count')
        attr_count.add_child(input_attribute_count)
        attr_count.serialize(out)
        for shader_type, variants in compiled_shaders.items():
            shader_root = doc.Node(shader_type)
            for variant, data in variants.items():
                variant_data_node = variant+'_data' if variant else 'data'
                variant_meta_node = variant+'_meta' if variant else 'meta'
                shader_root.add_child(variant_data_node).add_child(b64encode(data).decode('utf-8'))
                shader_root.add_child(variant_meta_node).add_children(meta_nodes[shader_type][variant])
            shader_root.serialize(out)
