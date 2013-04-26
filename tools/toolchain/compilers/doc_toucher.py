import time

def touch(in_path, out_path, *args):
    now = time.time()
    try:
        os.utime(os.path.splitext(in_path)[0]+'.hsh', (now, now))
    except:
        pass
    # Didn't produce output file
    return False