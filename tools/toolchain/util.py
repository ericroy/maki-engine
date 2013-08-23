import os
from . import CONFIG

def clean_path(path):
    return os.path.normpath(path).replace('\\', '/')

def walk(top_dir):
    for root, dirs, files in os.walk(top_dir):
        for f in files:
            yield clean_path(os.path.join(os.path.relpath(root, top_dir), f))

def archive_containing(file_path):
    file_path = os.path.normpath(file_path)
    for arc_name, conf in CONFIG['assets'].items():
        src_path = os.path.normpath(conf['src'])
        p = file_path
        while len(p) > 0:
            if os.path.samefile(src_path, p):
                #print('Determined that file %s belongs to archive %s' % (file_path, arc_name))
                return arc_name
            p = os.path.split(p)[0]
    raise RuntimeError('None of the archives specified in the project file contain %s' % file_path)

def archive_relative_path(file_path):
    arc_name = archive_containing(file_path)
    conf = CONFIG['assets'][arc_name]
    return clean_path(os.path.relpath(file_path, conf['src']))
