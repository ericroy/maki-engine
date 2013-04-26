import shutil
from . import compile_fbx
from . import compile_doc
from . import compile_shader
from . import doc_toucher

COMPILERS = {
    'mdoc': compile_doc.compile,    
    'mshad': compile_shader.compile,
    'fbx': compile_fbx.compile,
    'mskel': shutil.copyfile,
    'manim': shutil.copyfile,
    'mmesh': shutil.copyfile,
    'ps': doc_toucher.touch,
    'vs': doc_toucher.touch,
    'dds': shutil.copyfile,
    'png': shutil.copyfile,
    'jpg': shutil.copyfile,
    'ttf': shutil.copyfile,
}