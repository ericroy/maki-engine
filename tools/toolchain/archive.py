import sys
import os
import struct
import zlib
from io import BytesIO
from . import CONFIG

COMPRESSION_ENABLED = True
ZLIB_COMPRESSION_LEVEL = 6

FLAG_ZLIB_COMPRESSED = 1<<0

def _should_compress(file_size, extension):
    if not COMPRESSION_ENABLED:
        return False
    if extension.lower() in []:
        return False
    return True

def _pad(out_file, byte_multiple):
    while out_file.tell() % byte_multiple != 0:
        out_file.write(struct.pack('x'))

def _walk(top):
    for root, dirs, files in os.walk(top):
        for f in files:
            yield os.path.join(os.path.relpath(root, top), f)

def _clean_path(p):
    return os.path.normpath(p).replace('\\', '/')

def archive(arc_name):
    conf = CONFIG['assets'][arc_name]
    files = []
    toc = BytesIO()
    body = BytesIO()
    for file_path in _walk(conf['dst']):
        files.append(file_path)

    for file_path in files:
        file_path = _clean_path(file_path)
        flags = 0
        with open(os.path.join(conf['dst'], file_path), 'rb') as file:
            raw_data = file.read()
            compressed_data = raw_data

            if _should_compress(len(raw_data), os.path.splitext(file_path)[1]):
                flags |= FLAG_ZLIB_COMPRESSED
                compressed_data = zlib.compress(raw_data, ZLIB_COMPRESSION_LEVEL)

            toc.write(struct.pack('II', flags, len(file_path)))
            toc.write(file_path.encode("utf8"))
            _pad(toc, 8)
            toc.write(struct.pack('QII', body.tell(), len(raw_data), len(compressed_data)))
            body.write(compressed_data)
            _pad(body, 8)
            print(file_path)

    with open(os.path.join(CONFIG['bin_path'], arc_name+'.marc'), 'wb') as archive:
        len_file_header = 8
        toc_count = len(files)
        body_offset = toc.tell()+len_file_header
        assert(body_offset % 8 == 0)
        file_header = struct.pack('II', body_offset, toc_count)
        assert(len(file_header) == len_file_header)

        archive.write(file_header)
        toc.seek(0)
        archive.write(toc.read())
        body.seek(0)
        archive.write(body.read())
        print("Wrote archive (%d bytes)" % archive.tell())

if __name__ == '__main__':
    assert len(sys.argv) == 2, 'Archive requires <arc_name>'
    archive(sys.argv[1])