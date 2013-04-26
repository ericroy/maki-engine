import os

def clean_path(path):
    return os.path.normpath(path).replace('\\', '/')

def walk(top_dir):
    for root, dirs, files in os.walk(top_dir):
        for f in files:
            yield clean_path(os.path.join(os.path.relpath(root, top_dir), f))
