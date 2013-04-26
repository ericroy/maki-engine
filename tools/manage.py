import sys
import os
import glob
import shutil
import traceback
from optparse import OptionParser
from toolchain import *

def _build(*args):
    assert len(args) >= 1, 'build command expects src (and optional dst)'
    src = util.clean_path(args[0])
    if len(args) > 1:
        dst = util.clean_path(args[1])
    else:
        dst = os.path.join(CONFIG['assets_bin_path'], os.path.relpath(src, CONFIG['assets_path']))
    compile_args = args[2:]
    ext = os.path.splitext(src)[1]
    ext = ext.strip('.')
    try:
        compiler = compilers.COMPILERS[ext]
    except KeyError as e:
        print('Skipping file, no compiler for extension: %s' % ext)
    else:
        print('%s => %s' % (src, dst))
        compiler(src, dst, *compile_args)

def _build_all(*args):
    if os.path.exists(CONFIG['assets_bin_path']):
        for f in glob.glob(CONFIG['assets_bin_path']+'/*'):
            if os.path.isdir(f):
                shutil.rmtree(f)
            else:
                os.remove(f)
    else:
        os.mkdir(CONFIG['assets_bin_path'])

    for path in util.walk(CONFIG['assets_path']):
        src_path = os.path.join(CONFIG['assets_path'], path)
        dest_path = os.path.join(CONFIG['assets_bin_path'], path)
        dest_dir = os.path.dirname(dest_path)
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)
        _build(src_path, dest_path, *args)
    # Rebuild all manifests
    _manifest()

def _archive(*args):
    if len(args) >= 2:
        archive.archive(args[0], args[1], *args[2:])
    else:
        # Archive all
        for src_dir, dst_file in CONFIG.get('archives', []):
            archive.archive(src_dir, dst_file)

def _manifest(*args):
    if len(args) >= 2:
        manifest.manifest(args[0], args[1], *args[2:])
    else:
        # Make all manifests
        for src_dir, manifest_name in CONFIG.get('manifests', []):
            print('Generating manifest "%s" from directory: %s' % (manifest_name, src_dir))
            manifest.manifest(src_dir, manifest_name)

def _export_scene(*args):
    assert len(args) >= 2, 'export_scene command expects src file'
    exporters.export_scene(args[0], args[1], *args[2:])

def _export_animation(*args):
    assert len(args) >= 2, 'export_animation command expects src file'
    exporters.export_animation(args[0], args[1], *args[2:])

def _export_skeleton(*args):
    assert len(args) >= 2, 'export_skeleton command expects src file'
    exporters.export_skeleton(args[0], args[1], *args[2:])

COMMANDS = {
    'build': _build,
    'build_all': _build_all,
    'archive': _archive,
    'manifest': _manifest,
    'export_scene': _export_scene,
    'export_skeleton': _export_skeleton,
    'export_animation': _export_animation,
}

def main(command, *args, **kwargs):
    os.chdir(CONFIG['project_root'])
    try:
        func = COMMANDS[command]
    except KeyError as e:
        raise KeyError('No such command: %s' % command) from e
    
    ret = 0
    try:
        func(*args)
    except Exception:
        print(traceback.format_exc()+'\n')
        ret = 1
    return ret

if __name__ == '__main__':
    print('Project config: %s' % CONFIG_PATH)
    
    parser = OptionParser()
    options, args = parser.parse_args()

    assert len(args) >= 1, 'Must provide a command'
    ret = main(*args, **vars(options))
    exit(ret)

