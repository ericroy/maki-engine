import os
import time

def touch_mshad(src, dst, *args):
    now = time.time()
    touch_path = os.path.splitext(src)[0]+'.mshad'
    try:
        os.utime(touch_path, (now, now))
    except:
        pass