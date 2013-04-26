import os
import sys
import shutil
import subprocess
from io import BytesIO

def compile(src, dst, *args):
    binary = '0'
    cmd = [os.path.expandvars('$MAKI_DIR/tools/MakiShader.exe'), src, dst, binary]
    subprocess.check_call(cmd)