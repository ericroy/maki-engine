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
from .gl_constants import *

SDL_GL_RED_SIZE = 0
SDL_GL_GREEN_SIZE = 1
SDL_GL_BLUE_SIZE = 2
SDL_GL_ALPHA_SIZE = 3
SDL_GL_BUFFER_SIZE = 4
SDL_GL_DOUBLEBUFFER = 5
SDL_GL_DEPTH_SIZE = 6
SDL_GL_STENCIL_SIZE = 7
SDL_GL_ACCUM_RED_SIZE = 8
SDL_GL_ACCUM_GREEN_SIZE = 9
SDL_GL_ACCUM_BLUE_SIZE = 10
SDL_GL_ACCUM_ALPHA_SIZE = 11
SDL_GL_STEREO = 12
SDL_GL_MULTISAMPLEBUFFERS = 13
SDL_GL_MULTISAMPLESAMPLES = 14
SDL_GL_ACCELERATED_VISUAL = 15
SDL_GL_RETAINED_BACKING = 16
SDL_GL_CONTEXT_MAJOR_VERSION = 17
SDL_GL_CONTEXT_MINOR_VERSION = 18
SDL_GL_CONTEXT_EGL = 19
SDL_GL_CONTEXT_FLAGS = 20
SDL_GL_CONTEXT_PROFILE_MASK = 21
SDL_GL_SHARE_WITH_CURRENT_CONTEXT = 22
SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG = 2
SDL_WINDOW_OPENGL = 0x00000002
SDL_WINDOW_SHOWN = 0x00000004
SDL_WINDOW_HIDDEN = 0x00000008
SDL_WINDOW_RESIZABLE = 32
SDL_WINDOW_INPUT_FOCUS = 512
SDL_INIT_VIDEO = 0x00000020

if sys.platform.startswith('win'):
    GL = windll.LoadLibrary('opengl32')
    GLU = windll.LoadLibrary('glu32')
    _gl_func_type = WINFUNCTYPE
elif sys.platform.startswith('darwin'):
    GL = windll.LoadLibrary('OpenGL')
    GLU = windll.LoadLibrary('GLU')
    _gl_func_type = CFUNCTYPE
else:
    GL = windll.LoadLibrary('GL')
    GLU = windll.LoadLibrary('GLU')
    _gl_func_type = CFUNCTYPE


SDL = cdll.LoadLibrary(os.path.expandvars('$MAKI_DIR/tools/SDL2.dll'))
def _check_sdl_error():
    msg = c_char_p(SDL.SDL_GetError())
    if len(msg.value) > 0:
        raise RuntimeError('SDL error: %s' % msg.value)

def _check_gl_error():
    err = GL.glGetError()
    if err != 0:
        raise RuntimeError('GL error: %s' % c_char_p(GLU.gluErrorString(err)).value.decode('utf-8'))

def _glGetProcAddress(name, *args):
    proc_name = create_string_buffer(name.encode('utf-8'))
    addr = SDL.SDL_GL_GetProcAddress(proc_name)
    #addr = GL.wglGetProcAddress(proc_name)
    assert addr != 0, 'Could not get proc address for: %s' % name
    return _gl_func_type(*args)(addr)

_ogl_initialized = False
def _init_ogl():
    global _ogl_initialized
    if _ogl_initialized:
        return

    SDL.SDL_Init(SDL_INIT_VIDEO)
    _check_sdl_error()

    window = SDL.SDL_CreateWindow('', 0, 0, 50, 50, SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN)
    _check_sdl_error()
    assert window != 0

    SDL.SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)
    SDL.SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1)
    SDL.SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)
    SDL.SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8)
    SDL.SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8)
    SDL.SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8)
    SDL.SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32)
    SDL.SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG)
    _check_sdl_error()

    context = SDL.SDL_GL_CreateContext(window)
    _check_sdl_error()
    assert context != 0

    global glCreateShader; glCreateShader = _glGetProcAddress('glCreateShader', c_uint, c_int)
    global glShaderSource; glShaderSource = _glGetProcAddress('glShaderSource', None, c_uint, c_size_t, POINTER(c_char_p), POINTER(c_int))
    global glCompileShader; glCompileShader = _glGetProcAddress('glCompileShader', None, c_uint)
    global glGetShaderiv; glGetShaderiv = _glGetProcAddress('glGetShaderiv', None, c_uint, c_int, POINTER(c_int))
    global glGetProgramiv; glGetProgramiv = _glGetProcAddress('glGetProgramiv', None, c_uint, c_int, POINTER(c_int))
    global glCreateProgram; glCreateProgram = _glGetProcAddress('glCreateProgram', c_uint)
    global glAttachShader; glAttachShader = _glGetProcAddress('glAttachShader', None, c_uint, c_uint)
    global glLinkProgram; glLinkProgram = _glGetProcAddress('glLinkProgram', None, c_uint)
    global glGetUniformBlockIndex; glGetUniformBlockIndex = _glGetProcAddress('glGetUniformBlockIndex', c_uint, c_uint, c_char_p)
    global glBindBuffer; glBindBuffer = _glGetProcAddress('glBindBuffer', None, c_int, c_uint)
    global glGetShaderInfoLog; glGetShaderInfoLog = _glGetProcAddress('glGetShaderInfoLog', None, c_uint, c_size_t, POINTER(c_size_t), c_char_p)
    global glGetProgramInfoLog; glGetProgramInfoLog = _glGetProcAddress('glGetProgramInfoLog', None, c_uint, c_size_t, POINTER(c_size_t), c_char_p)
    global glGetActiveUniform; glGetActiveUniform = _glGetProcAddress('glGetActiveUniform', None, c_uint, c_uint, c_size_t, POINTER(c_size_t), POINTER(c_int), POINTER(c_int), c_char_p)
    global glGetActiveUniformBlockiv; glGetActiveUniformBlockiv = _glGetProcAddress('glGetActiveUniformBlockiv', None, c_uint, c_uint, c_int, POINTER(c_int))
    global glGetActiveUniformsiv; glGetActiveUniformsiv = _glGetProcAddress('glGetActiveUniformsiv', None, c_uint, c_size_t, POINTER(c_uint), c_int, POINTER(c_int))

    _ogl_initialized = True









BUFFERS = ('enginePerObject', 'enginePerFrame', 'material')
D3D_COMPILER = os.path.expandvars('$MAKI_DIR/tools/fxc.exe')
D3D_BUFFER_MEMBER = re.compile(r'//\s*\S+\s+([A-Za-z0-9_]+)(?:\[\d+\])?;\s*//\s*Offset:\s*(\d+)\s+Size:\s*(\d+)\s*')
D3D_PROFILE = {'vs': 'vs_4_0', 'ps': 'ps_4_0'}
OGL_PROFILE = '330'
OGL_INCLUDE = re.compile(r'(#pragma include "([^"]+)")')


def _data(node_name, compiled):
    n = doc.Node(node_name)
    n.add_child(b64encode(compiled).decode('utf-8'))
    return n

def _meta(node_name, buffer_slots, buffer_contents):
    n = doc.Node(node_name)
    for buffer_name, slot in buffer_slots.items():
        buffer_node = n.add_child(buffer_name)
        buffer_node.add_child('slot').add_child(str(slot))
        uniform_node = buffer_node.add_child('uniforms')
        for var_name, var_offset, var_length in buffer_contents[buffer_name]:
            uniform_node.add_child(var_name).add_children([str(var_offset), str(var_length)])
    return n




def _d3d_listing_and_bytecode(source_file, profile_string, entry_point, defines):
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

def _d3d_create_meta_node(node_name, listing, compiled, input_attrs=None):
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
    return _meta(node_name, buffer_slots, buffer_contents)

def _d3d(arc_name, variants, vs_entry_point, vs_path, vs_defines, ps_entry_point, ps_path, ps_defines):
    input_attr_count = 0
    vs_programs = {}
    ps_programs = {}
    for variant, variant_defs in variants.items():
        meta_node_name = (variant+'_meta').strip('_')
        data_node_name = (variant+'_data').strip('_')
        
        input_attrs = []
        listing, compiled = _d3d_listing_and_bytecode(vs_path, D3D_PROFILE['vs'], vs_entry_point, vs_defines + variant_defs)
        vs_programs[variant] = [_d3d_create_meta_node(meta_node_name, listing, compiled, input_attrs), _data(data_node_name, compiled)]
        input_attr_count = len(input_attrs)
        
        listing, compiled = _d3d_listing_and_bytecode(ps_path, D3D_PROFILE['ps'], ps_entry_point, ps_defines + variant_defs)
        ps_programs[variant] = [_d3d_create_meta_node(meta_node_name, listing, compiled), _data(data_node_name, compiled)]

    return input_attr_count, vs_programs, ps_programs



def _ogl_process_includes(file_path, source):
    base_path = os.path.split(file_path)[0]
    while True:
        result_iter = re.finditer(OGL_INCLUDE, source)
        try:
            statement, included_path = next(result_iter).groups()
        except StopIteration:
            break
        else:
            included_path = os.path.join(base_path, included_path)
            with open(included_path) as file:
                included_source = file.read()
            included_source = _ogl_process_includes(included_path, included_source)
            source = source.replace(statement, included_source)
    return source

def _ogl_uniform_size(data_type, size, array_stride, matrix_stride):
    if array_stride > 0:
        return array_stride * size
    elif matrix_stride > 0:
        if data_type in (GL_FLOAT_MAT2, GL_FLOAT_MAT2x3, GL_FLOAT_MAT2x4, GL_DOUBLE_MAT2, GL_DOUBLE_MAT2x3, GL_DOUBLE_MAT2x4):
            return 2 * matrix_stride
        elif data_type in (GL_FLOAT_MAT3, GL_FLOAT_MAT3x2, GL_FLOAT_MAT3x4, GL_DOUBLE_MAT3, GL_DOUBLE_MAT3x2, GL_DOUBLE_MAT3x4):
            return 3 * matrix_stride
        elif data_type in (GL_FLOAT_MAT4, GL_FLOAT_MAT4x2, GL_FLOAT_MAT4x3, GL_DOUBLE_MAT4, GL_DOUBLE_MAT4x2, GL_DOUBLE_MAT4x3):
            return 4 * matrix_stride
        else:
            assert False, "Don't know the size of this type %s" % data_type
    else:
        return GL_TYPE_BYTE_SIZES[data_type]

def _ogl_get_active_uniforms(prog, indices, enum):
    values = (c_int * len(indices))()
    glGetActiveUniformsiv(prog, c_size_t(len(indices)), indices, enum, values)
    _check_gl_error()
    return values

def _ogl_create_meta_node(node_name, prog, shader_type):
    print('Creating %s for %s' % (node_name, shader_type))
    buffer_slots = {}
    buffer_contents = {}
    for buffer_name in BUFFERS:

        bn = create_string_buffer(buffer_name.encode('utf-8'))
        slot = glGetUniformBlockIndex(prog, bn)
        _check_gl_error()
        if slot == GL_INVALID_INDEX:
            print('Buffer "%s" not found' % buffer_name)
            continue
        
        referenced = c_int(0)
        enum = GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER if shader_type == 'vs' else GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER
        glGetActiveUniformBlockiv(prog, slot, enum, byref(referenced))
        _check_gl_error()
        if referenced.value == 0:
            print('Buffer "%s" not referenced' % buffer_name)
            continue

        buffer_slots[buffer_name] = int(slot)

        glBindBuffer(GL_UNIFORM_BUFFER, slot)
        _check_gl_error()

        count = c_int()
        glGetActiveUniformBlockiv(prog, slot, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, byref(count))
        _check_gl_error()

        indices = (c_uint * count.value)()
        glGetActiveUniformBlockiv(prog, slot, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, cast(indices, POINTER(c_int)))
        _check_gl_error()        

        offsets = _ogl_get_active_uniforms(prog, indices, GL_UNIFORM_OFFSET)
        types = _ogl_get_active_uniforms(prog, indices, GL_UNIFORM_TYPE)
        sizes = _ogl_get_active_uniforms(prog, indices, GL_UNIFORM_SIZE)
        array_strides = _ogl_get_active_uniforms(prog, indices, GL_UNIFORM_ARRAY_STRIDE)
        matrix_strides = _ogl_get_active_uniforms(prog, indices, GL_UNIFORM_MATRIX_STRIDE)
        name_lengths = _ogl_get_active_uniforms(prog, indices, GL_UNIFORM_NAME_LENGTH)
        byte_lengths = [_ogl_uniform_size(types[i], sizes[i], array_strides[i], matrix_strides[i]) for i in range(count.value)]

        name_buffer = create_string_buffer(max(name_lengths)+1)
        for i in range(count.value):
            size = c_int()
            gl_type = c_int()
            length = c_size_t(0)
            glGetActiveUniform(prog, indices[i], len(name_buffer), byref(length), byref(size), byref(gl_type), name_buffer)
            _check_gl_error()

            name = name_buffer.value.decode('utf-8')
            buffer_contents.setdefault(buffer_name, []).append((name, offsets[i], byte_lengths[i]))

    return _meta(node_name, buffer_slots, buffer_contents)

def _ogl_compile_glsl(file_name, source, shader_type):
    sh = glCreateShader(GL_VERTEX_SHADER if shader_type == 'vs' else GL_FRAGMENT_SHADER)
    _check_gl_error()

    source_buffer = create_string_buffer(source)
    buffer_ptr = c_char_p(source_buffer.value)
    source_length = c_int(len(source_buffer))
    glShaderSource(sh, 1, byref(buffer_ptr), byref(source_length))
    _check_gl_error()

    glCompileShader(sh)
    _check_gl_error()

    status = c_int()
    glGetShaderiv(sh, GL_COMPILE_STATUS, byref(status))
    _check_gl_error()
    if status.value == 0:
        max_log_length = c_size_t(1024 * 10)
        log = create_string_buffer(max_log_length.value)
        log_length = c_size_t()
        glGetShaderInfoLog(sh, max_log_length, byref(log_length), log)
        raise RuntimeError('GL shader compilation failed (%s):\n%s' % (file_name, log.value.decode('utf-8')))
    return sh

def _ogl_link(vs, ps):
    prog = glCreateProgram()
    _check_gl_error()
    
    glAttachShader(prog, vs)
    _check_gl_error()
    
    glAttachShader(prog, ps)
    _check_gl_error()
    
    glLinkProgram(prog)
    _check_gl_error()

    status = c_int()
    glGetProgramiv(prog, GL_LINK_STATUS, byref(status))
    _check_gl_error()
    if status.value == 0:
        max_log_length = c_size_t(1024 * 10)
        log = create_string_buffer(max_log_length.value)
        log_length = c_size_t()
        glGetProgramInfoLog(sh, max_log_length, byref(log_length), log)
        raise RuntimeError('GL shader program link failed:\n%s' % log.value.decode('utf-8'))
    return prog

def _ogl_defines_prefix(defines):
    source = []
    for var, value in defines:
        source.append('#define %s %s\n' % (var, value))
    return ''.join(source)

def _ogl(arc_name, variants, vs_entry_point, vs_path, vs_defines, ps_entry_point, ps_path, ps_defines):
    input_attr_count = c_int(0)
    vs_programs = {}
    ps_programs = {}

    with open(vs_path) as file:
        vs_source = file.read()
    vs_source = _ogl_process_includes(vs_path, vs_source)

    with open(ps_path) as file:
        ps_source = file.read()
    ps_source = _ogl_process_includes(ps_path, ps_source)

    for variant, variant_defs in variants.items():
        meta_node_name = (variant+'_meta').strip('_')
        data_node_name = (variant+'_data').strip('_')

        vs_final_source = (('#version %s\n' % OGL_PROFILE) + _ogl_defines_prefix(vs_defines + variant_defs) + vs_source).encode('utf-8')
        ps_final_source = (('#version %s\n' % OGL_PROFILE) + _ogl_defines_prefix(ps_defines + variant_defs) + ps_source).encode('utf-8')

        vs = _ogl_compile_glsl(vs_path, vs_final_source, 'vs')
        ps = _ogl_compile_glsl(ps_path, ps_final_source, 'ps')
        prog = _ogl_link(vs, ps)

        if variant == '':
            glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTES, byref(input_attr_count))
            _check_gl_error()
            print(input_attr_count.value)

        vs_programs[variant] = [_ogl_create_meta_node(meta_node_name, prog, 'vs'), _data(data_node_name, vs_final_source)]
        ps_programs[variant] = [_ogl_create_meta_node(meta_node_name, prog, 'ps'), _data(data_node_name, ps_final_source)]

    return input_attr_count.value, vs_programs, ps_programs





def compile(arc_name, src, dst):
    with open(src) as file:
        root = doc.deserialize(file.read())

    variants = {'': []}
    variants_node = root.resolve('variants')
    if variants_node:
        for variant in variants_node.children():
            variants.setdefault(variant.get_value(), []).append((variant[0].get_value(), variant[0][0].get_value()))

    vs_defines = []
    try:
        defines_node = root.resolve('vertex_shader.defines')
    except KeyError:
        pass
    else:
        for define in defines_node.children():
            vs_defines.append((define.get_value(), define[0].get_value()))

    ps_defines = []
    try:
        defines_node = root.resolve('pixel_shader.defines')
    except KeyError:
        pass
    else:
        for define in defines_node.children():
            ps_defines.append((define.get_value(), define[0].get_value()))
    
    vs_entry_point = root.resolve('vertex_shader.entry_point.#0').get_value()
    ps_entry_point = root.resolve('pixel_shader.entry_point.#0').get_value()
    vs_path = os.path.join(CONFIG['assets'][arc_name]['src'], root.resolve('vertex_shader.file_name.#0').get_value())
    ps_path = os.path.join(CONFIG['assets'][arc_name]['src'], root.resolve('pixel_shader.file_name.#0').get_value())

    if CONFIG['render_api'] == 'd3d':
        input_attr_count, vs_programs, ps_programs = _d3d(arc_name, variants, vs_entry_point, vs_path, vs_defines, ps_entry_point, ps_path, ps_defines)
    else:
        _init_ogl()
        input_attr_count, vs_programs, ps_programs = _ogl(arc_name, variants, vs_entry_point, vs_path, vs_defines, ps_entry_point, ps_path, ps_defines)

    with open(dst, 'w') as out:
        n = doc.Node('input_attribute_count')
        n.add_child(str(input_attr_count))
        n.serialize(out)
        n = doc.Node('vertex_shader')
        for key, nodes in vs_programs.items():
            for node in nodes:
                n.add_child(node)
        n.serialize(out)
        n = doc.Node('pixel_shader')
        for key, nodes in ps_programs.items():
            for node in nodes:
                n.add_child(node)
        n.serialize(out)
