import os
import logging
import shutil
import subprocess

def compile(src, dst, *args):
    old_cwd = os.getcwd()
    cwd = os.path.expandvars('$MAKI_DIR/tools')
    src = os.path.abspath(src)
    dst = os.path.abspath(dst)
    rel_src = os.path.relpath(src, cwd)
    rel_dst = os.path.relpath(dst, cwd)
    cmd = ['luajit.exe', '-b', rel_src, rel_dst]

    os.chdir(cwd)
    try:
        subprocess.check_call(cmd)
    finally:
        os.chdir(old_cwd)
