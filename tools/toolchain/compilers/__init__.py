import shutil
import platform
from . import compile_fbx
from . import compile_doc
from . import compile_shader
from . import compile_hlsl
from . import compile_glsl
from . import compile_lua
from . import doc_toucher

COMPILERS = {
    'mdoc': shutil.copyfile,    
    'mshad': compile_hlsl.compile if platform.system() == 'Windows' else compile_glsl.compile,
    'fbx': compile_fbx.compile,
    'mskel': shutil.copyfile,
    'manim': shutil.copyfile,
    'mmesh': shutil.copyfile,
    'dds': shutil.copyfile,
    'png': shutil.copyfile,
    'jpg': shutil.copyfile,
    'ttf': shutil.copyfile,
    'lua': compile_lua.compile,
    'ps': doc_toucher.touch,
    'vs': doc_toucher.touch,
    'cg': doc_toucher.touch,
}