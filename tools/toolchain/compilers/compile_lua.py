import os
import logging
import shutil
import subprocess
import string

def _clean(path):
    drive, tail = os.path.splitdrive(os.path.normpath(path))
    return drive.lower() + tail

def compile(arc_name, src, dst):
    old_cwd = os.getcwd()
    cwd = _clean(os.path.expandvars('$MAKI_DIR/tools'))
    src = _clean(os.path.abspath(src))
    dst = _clean(os.path.abspath(dst))
    #print(cwd, src, dst)
    rel_src = os.path.relpath(src, cwd)
    rel_dst = os.path.relpath(dst, cwd)
    cmd = ['luajit.exe', '-b', rel_src, rel_dst]

    os.chdir(cwd)
    try:
        subprocess.check_call(cmd)
    finally:
        os.chdir(old_cwd)
