import shutil
import platform
from . import compile_fbx
from . import compile_doc
from . import compile_hlsl
from . import compile_glsl
from . import compile_lua
from . import doc_toucher

def _copy(arc_name, src, dst):
    shutil.copyfile(src, dst)

COMPILERS = {
    'mdoc': _copy,    
    'mshad': compile_hlsl.compile if platform.system() == 'Windows' else compile_glsl.compile,
    'fbx': compile_fbx.compile,
    'mskel': _copy,
    'manim': _copy,
    'mmesh': _copy,
    'dds': _copy,
    'png': _copy,
    'jpg': _copy,
    'ttf': _copy,
    'lua': compile_lua.compile,
    'ps': doc_toucher.touch_mshad,
    'vs': doc_toucher.touch_mshad,
}