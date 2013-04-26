import os
import logging
import shutil
import subprocess

def compile(src, dst, *args):
    binary = '1'
    cmd = [os.path.expandvars('$MAKI_DIR/tools/MakiDoc.exe'), src, dst, binary]
    subprocess.check_call(cmd)
