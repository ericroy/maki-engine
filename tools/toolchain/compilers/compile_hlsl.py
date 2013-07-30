from base64 import b64encode
from .. import doc

def _d3d_compile(source_code, profile_string, entry_point, defines):
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

def _meta(node_name, compiled):
    n = doc.Node(node_name)

    return n

def _compile_shader(shader_node):
    is_vertex = shader_node.get_value() == 'vertex_shader'
    target_profile = 'vs_4_0' if is_vertex else 'ps_4_0'
    entry_point = shader_node.resolve('entry_point.#0').get_value()
    shader_path = shader_node.resolve('file_name.#0').get_value()
    with open(shader_path) as file:
        source = file.read()
    defines = []
    for define in shader_node['defines']:
        defines.append((define.get_value(), define[0].get_value()))
    programs = {}
    
    # Generate standard version of the shader
    compiled = _d3d_compile(source, target_profile, entry_point, defines)
    programs['meta'] = _meta('meta', compiled)
    programs['data'] = b64encode(compiled).decode('utf-8')

    # Generate each variant
    for variant in shader_node['variants']:
        variant_defines = [(variant[0], variant[0][0])]
        compiled = _d3d_compile(source, target_profile, entry_point, defines + variant_defines)
        programs[variant.get_value()+'_meta'] = _meta(variant.get_value()+'_meta', compiled)
        programs[variant.get_value()+'_data'] = b64encode(compiled).decode('utf-8')

    input_attr_count = 0
    return programs, input_attr_count if is_vertex else programs


def compile(src, dst, *args):
    root = doc.deserialize(src)
    input_attr_count = vs_programs = _compile_shader(root['vertex_shader'])
    ps_programs = _compile_shader(root['pixel_shader'])

    out_nodes = []

    iac_node = doc.Node('input_attribute_count')
    iac_node.add_child(str(input_attr_count))
    out_nodes.append(iac_node)

    vs_node = doc.Node('vertex_shader')
    for key, node in vs_programs.items():
        vs_node.add_child('key').add_child(node)
    out_nodes.append(vs_node)

    ps_node = doc.Node('pixel_shader')
    for key, node in ps_programs.items():
        ps_node.add_child('key').add_child(node)
    out_nodes.append(ps_node)

    with open(dst, 'w') as out:
        for n in out_nodes:
            n.serialize(out)
    