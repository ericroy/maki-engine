import sys
import os
import tempfile
import subprocess
import struct
from ctypes import *
from base64 import b64encode

from .. import doc
from .. import CONFIG

VS_PROFILE = 'vs_4_0'
PS_PROFILE = 'ps_4_0'

Self = c_void_p

class Guid(Structure):
    _fields_ = [
        ("Data1", c_uint),
        ("Data2", c_ushort),
        ("Data3", c_ushort),
        ("Data4", c_ubyte * 8)
    ]
    def __str__(self):
        return '%08x' % self.Data1 + '%04x' % self.Data2 + '%04x' % self.Data3 + ''.join(['%02x' % x for x in self.Data4])

class IUnknown(Structure):
    _fields_ = [
        #('_reserved', c_void_p),
        ('QueryInterface', WINFUNCTYPE(HRESULT)),
        ('AddRef', WINFUNCTYPE(c_ulong)),
        ('Release', WINFUNCTYPE(c_ulong))
    ]

class D3D11_SHADER_DESC(Structure):
    _fields_ = [
        ('Version', c_uint),
        ('Creator', c_char_p),
        ('Flags', c_uint),
        ('ConstantBuffers', c_uint),
        ('BoundResources', c_uint),
        ('InputParameters', c_uint),
        ('OutputParameters', c_uint),
        ('InstructionCount', c_uint),
        ('TempRegisterCount', c_uint),
        ('TempArrayCount', c_uint),
        ('DefCount', c_uint),
        ('DclCount', c_uint),
        ('TextureNormalInstructions', c_uint),
        ('TextureLoadInstructions', c_uint),
        ('TextureCompInstructions', c_uint),
        ('TextureBiasInstructions', c_uint),
        ('TextureGradientInstructions', c_uint),
        ('FloatInstructionCount', c_uint),
        ('IntInstructionCount', c_uint),
        ('c_uintInstructionCount', c_uint),
        ('StaticFlowControlCount', c_uint),
        ('DynamicFlowControlCount', c_uint),
        ('MacroInstructionCount', c_uint),
        ('ArrayInstructionCount', c_uint),
        ('CutInstructionCount', c_uint),
        ('EmitInstructionCount', c_uint),
        ('GSOutputTopology', c_int),
        ('GSMaxOutputVertexCount', c_uint),
        ('InputPrimitive', c_int),
        ('PatchConstantParameters', c_uint),
        ('cGSInstanceCount', c_uint),
        ('cControlPoints', c_uint),
        ('HSOutputPrimitive', c_int),
        ('HSPartitioning', c_int),
        ('TessellatorDomain', c_int),
        ('cBarrierInstructions', c_uint),
        ('cInterlockedInstructions', c_uint),
        ('cTextureStoreInstructions', c_uint),
    ]

class ID3D11ShaderReflection(Structure):
    _fields_ = IUnknown._fields_ + [
        ('QueryInterface', WINFUNCTYPE(HRESULT)),
        ('AddRef', WINFUNCTYPE(c_ulong)),
        ('Release', WINFUNCTYPE(c_ulong)),
        ('GetDesc', WINFUNCTYPE(HRESULT, POINTER(D3D11_SHADER_DESC))),
        ('GetConstantBufferByIndex', WINFUNCTYPE(HRESULT)),
        ('GetConstantBufferByName', WINFUNCTYPE(HRESULT)),
        ('GetResourceBindingDesc', WINFUNCTYPE(HRESULT)),
        ('GetInputParameterDesc', WINFUNCTYPE(HRESULT)),
        ('GetOutputParameterDesc', WINFUNCTYPE(HRESULT)),
        ('GetPatchConstantParameterDesc', WINFUNCTYPE(HRESULT)),
        ('GetVariableByName', WINFUNCTYPE(HRESULT)),
        ('GetResourceBindingDescByName', WINFUNCTYPE(HRESULT)),
        ('GetMovInstructionCount', WINFUNCTYPE(HRESULT)),
        ('GetMovcInstructionCount', WINFUNCTYPE(HRESULT)),
        ('GetConversionInstructionCount', WINFUNCTYPE(HRESULT)),
        ('GetBitwiseInstructionCount', WINFUNCTYPE(HRESULT)),
        ('GetGSInputPrimitive', WINFUNCTYPE(HRESULT)),
        ('IsSampleFrequencyShader', WINFUNCTYPE(HRESULT)),
        ('GetNumInterfaceSlots', WINFUNCTYPE(HRESULT)),
        ('GetMinFeatureLevel', WINFUNCTYPE(HRESULT)),
        ('GetThreadGroupSize', WINFUNCTYPE(HRESULT)),
        ('GetRequiresFlags', WINFUNCTYPE(HRESULT)),
    ]



D3DCLIB = windll.LoadLibrary('C:/Program Files (x86)/Windows Kits/8.0/bin/x64/d3dcompiler_46.dll')
IID_ID3D11ShaderReflection = Guid(0x8d536ca1, 0x0cca, 0x4956, (c_ubyte * 8)(0xa8, 0x37, 0x78, 0x69, 0x63, 0x75, 0x55, 0x84))

ASSETS_PATH = os.path.join(CONFIG['project_root'], CONFIG['assets_path'])
D3D_COMPILER = os.path.expandvars('$MAKI_DIR/tools/fxc.exe')

def _d3d_compile(source_file, profile_string, entry_point, defines):
    out_file = tempfile.NamedTemporaryFile(delete=False)
    out_file.close()
    try:
        cmd = [D3D_COMPILER, '/Zi', '/nologo', '/T:'+profile_string, '/E:'+entry_point, '/Fo:'+os.path.normpath(out_file.name), os.path.normpath(source_file)]
        cmd += ['/D%s=%s' % (var, val) for var, val in defines]
        #print(cmd)
        subprocess.check_call(cmd)
        with open(out_file.name, 'rb') as f:
            return f.read()
    finally:
        os.remove(out_file.name)

def _meta(node_name, compiled):
    reflect = POINTER(c_byte)()
    buff = create_string_buffer(compiled)
    hr = D3DCLIB.D3DReflect(buff, c_size_t(len(buff)-1), byref(IID_ID3D11ShaderReflection), byref(reflect))
    if hr < 0:
        raise RuntimeError('D3DReflect failed (0x%08X)' % c_ulong(hr).value)
    
    print(reflect)
    reflect = cast(reflect, POINTER(ID3D11ShaderReflection))

    desc = D3D11_SHADER_DESC()
    hr = reflect.contents.GetDesc(byref(desc))
    if hr < 0:
        raise RuntimeError('reflect.GetDesc failed (0x%08X)' % c_ulong(hr).value)
    
    print(desc.Creator)

    n = doc.Node(node_name)
    return n

def _data(node_name, compiled):
    n = doc.Node(node_name)
    n.add_child(b64encode(compiled).decode('utf-8'))
    return n

def _compile_shader(shader_node):
    is_vertex = shader_node.get_value() == 'vertex_shader'
    target_profile = VS_PROFILE if is_vertex else PS_PROFILE
    entry_point = shader_node.resolve('entry_point.#0').get_value()
    shader_path = os.path.join(ASSETS_PATH, shader_node.resolve('file_name.#0').get_value())

    defines = []
    try:
        for define in shader_node['defines']:
            defines.append((define.get_value(), define[0].get_value()))
    except KeyError:
        pass
    programs = {}
    
    # Generate standard version of the shader
    compiled = _d3d_compile(shader_path, target_profile, entry_point, defines)
    programs[''] = []
    programs[''].append(_meta('meta', compiled))
    programs[''].append(_data('data', compiled))

    # Generate each variant
    for variant in shader_node['variants']:
        variant_defines = [(variant[0], variant[0][0])]
        compiled = _d3d_compile(shader_path, target_profile, entry_point, defines + variant_defines)
        programs[variant.get_value()] = []
        programs[variant.get_value()].append(_meta(variant.get_value()+'_meta', compiled))
        programs[variant.get_value()].append(_data(variant.get_value()+'_data', compiled))

    input_attr_count = 0
    return (input_attr_count, programs) if is_vertex else programs


def compile(src, dst, *args):
    with open(src) as file:
        root = doc.deserialize(file.read())

    input_attr_count, vs_programs = _compile_shader(root['vertex_shader'])
    ps_programs = _compile_shader(root['pixel_shader'])

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
    