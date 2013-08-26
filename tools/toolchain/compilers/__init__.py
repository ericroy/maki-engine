import shutil
import platform
from . import compile_fbx
from . import compile_shader
from . import compile_flash
from . import compile_lua
from . import doc_toucher
from .. import CONFIG

def _copy(arc_name, src, dst):
    shutil.copyfile(src, dst)

COMPILERS = {
    'mdoc': _copy,    
    'mshad': compile_shader.compile,
    'mflas': compile_flash.compile,
    'fbx': compile_fbx.compile,
    'mskel': _copy,
    'manim': _copy,
    'mmesh': _copy,
    'dds': _copy,
    'lua': compile_lua.compile,
    'ps': doc_toucher.touch_mshad,
    'vs': doc_toucher.touch_mshad,
}