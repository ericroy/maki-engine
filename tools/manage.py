import sys
import os
import glob
import shutil
import traceback
from optparse import OptionParser
from toolchain import option_parser, CONFIG, util, manifest, archive, watch, compilers, exporters, install_scripts


def _watch(*args):
    watch.watch_forever()


def _build(src_file, arc_name=None):
    src = util.clean_path(src_file)
    ext = os.path.splitext(src)[1]
    ext = ext.strip('.')
    try:
        compiler = compilers.COMPILERS[ext]
    except KeyError as e:
        print('Skipping file, no compiler for extension: %s' % ext)
        return
    if arc_name is None:
        arc_name = util.archive_containing(src_file)
    conf = CONFIG['assets'][arc_name]
    dst = os.path.join(conf['dst'], os.path.relpath(src, conf['src']))
    print('%s => %s' % (src, dst))
    dest_dir = os.path.dirname(dst)
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
    compiler(arc_name, src, dst)


def _build_all():
    for arc_name, conf in CONFIG['assets'].items():
        if os.path.exists(conf['dst']):
            for f in glob.glob(conf['dst'] + '/*'):
                if os.path.isdir(f):
                    try:
                        shutil.rmtree(f)
                    except:
                        shutil.rmtree(f)
                else:
                    os.remove(f)
        else:
            os.mkdir(conf['dst'])

        for path in util.walk(conf['src']):
            _build(os.path.join(conf['src'], path), arc_name)
        _manifest(arc_name)
        print()


def _archive(arc_name):
    conf = CONFIG['assets'][arc_name]
    if conf['archive']:
        archive.archive(arc_name)
    else:
        # If the configuration says that we shouldn't archive, then just copy the loose files instead
        dst = os.path.join(CONFIG['archive_path'], conf['dst'])
        if os.path.exists(dst):
            shutil.rmtree(dst)
        shutil.copytree(conf['src'], dst)


def _archive_all():
    for arc_name in CONFIG['assets'].keys():
        _archive(arc_name)
        print()


def _manifest(arc_name):
    conf = CONFIG['assets'][arc_name]
    print('Generating manifest for "%s" from directory: %s' % (arc_name, conf['src']))
    manifest.manifest(arc_name)


def _manifest_all():
    for arc_name in CONFIG['assets'].keys():
        _manifest(arc_name)
        print()


def _export_scene(*args):
    assert len(args) >= 2, 'export_scene command expects src and dst files'
    exporters.export_scene(args[0], args[1], *args[2:])


def _export_animation(*args):
    assert len(args) >= 2, 'export_animation command expects src and dst files'
    exporters.export_animation(args[0], args[1], *args[2:])


def _export_skeleton(*args):
    assert len(args) >= 2, 'export_skeleton command expects src and dst files'
    exporters.export_skeleton(args[0], args[1], *args[2:])


def _install_scripts(*args):
    install_scripts.install_scripts()


COMMANDS = {
    'watch': _watch,
    'build': _build,
    'build_all': _build_all,
    'archive': _archive,
    'archive_all': _archive_all,
    'manifest': _manifest,
    'manifest_all': _manifest_all,
    'export_scene': _export_scene,
    'export_skeleton': _export_skeleton,
    'export_animation': _export_animation,
    'install_scripts': _install_scripts,
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
        print(traceback.format_exc() + '\n')
        ret = 1
    return ret


if __name__ == '__main__':
    options, args = option_parser.parse_args()
    assert len(args) >= 1, 'Must provide a command'
    ret = main(*args, **vars(options))
    exit(ret)
