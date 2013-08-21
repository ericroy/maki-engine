import sys
import logging
import os
import time
import socket
import struct
import win32file
import win32con
import win32event
import threading
import subprocess
from collections import defaultdict
from . import CONFIG, CONFIG_PATH, util

FILE_LIST_DIRECTORY = 0x0001
FILE_NOTIFY_CHANGE_CREATION = 0x00000040
FILE_NOTIFY_CHANGE_LAST_WRITE = 0x00000010

MANAGE_SCRIPT = os.path.expandvars('$MAKI_DIR/tools/manage.py')

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

class UDPNotifier(object):
    def __init__(self, port):
        self._threshold = 0.2
        self._cache = {}
        self._port = port
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self._socket.bind(('127.0.0.1', 0))

    def __call__(self, path):
        now = time.clock()
        try:
            if now - self._cache[path] < self._threshold:
                return
        except KeyError:
            pass
        self._cache[path] = now
        print('Notify %s' % path)
        b = path.encode('utf-8')
        data = struct.pack('I%ds' % len(b), 0, b)
        self._socket.sendto(data, ('<broadcast>', self._port))

    def close(self):
        self._socket.close()

class Observer(threading.Thread):
    def __init__(self, path, callback):
        threading.Thread.__init__(self)
        self._path = path
        self._callback = callback
        self._handle = None
        self._abort = False

        self._buffer = win32file.AllocateReadBuffer(4096)
        self._handle = win32file.CreateFile(self._path, FILE_LIST_DIRECTORY,
            win32con.FILE_SHARE_READ|win32con.FILE_SHARE_WRITE|win32con.FILE_SHARE_DELETE,
            None, win32con.OPEN_EXISTING, win32con.FILE_FLAG_BACKUP_SEMANTICS|win32con.FILE_FLAG_OVERLAPPED, None)
        self._overlapped = win32file.OVERLAPPED()
        self._overlapped.hEvent = win32event.CreateEvent(None, False, 0, None)

    def run(self):
        print('Observer started (%s)' % threading.current_thread().ident)
        try:
            while not self._abort:
                flags = FILE_NOTIFY_CHANGE_CREATION|FILE_NOTIFY_CHANGE_LAST_WRITE
                ret = win32file.ReadDirectoryChangesW(self._handle, self._buffer, True, flags, self._overlapped)
                if ret == 0:
                    raise WindowsError('ReadDirectoryChangesW failed')
                ret = win32event.WaitForSingleObject(self._overlapped.hEvent, win32event.INFINITE)
                if ret == win32event.WAIT_OBJECT_0:
                    if self._abort:
                        break
                    bytes = win32file.GetOverlappedResult(self._handle, self._overlapped, False)
                    files_changed = defaultdict(list)
                    for action, file in win32file.FILE_NOTIFY_INFORMATION(self._buffer, bytes):
                        if action in (1, 3):
                            files_changed[file].append(action)
                    for file in files_changed:    
                        # Created or updated
                        path = util.clean_path(os.path.join(self._path, file))
                        self._callback(path)
        finally:
            win32file.CancelIo(self._handle)
            win32file.CloseHandle(self._overlapped.hEvent)
            win32file.CloseHandle(self._handle)
        print('Observer exited (%s)' % threading.current_thread().ident)

    def stop(self):
        self._abort = True
        win32file.CancelIo(self._handle)
        win32event.SetEvent(self._overlapped.hEvent)

def _builder(path):
    if os.path.isdir(path):
        return
    if os.path.splitext(path)[1].lower() == '.tmp':
        return
    cmd = ['python', MANAGE_SCRIPT, 'build', path]
    if CONFIG_PATH is not None:
        cmd += ['--project', CONFIG_PATH]
    try:
        subprocess.check_call(cmd)
    except:
        logger.exception('Build attempt failed')


def watch_forever():
    observers = []
    udp_notify = UDPNotifier(11001)

    for arc_name, conf in CONFIG['assets'].items():
        logger.info('Watching %s and %s', conf['src'], conf['dst'])
        obs1 = Observer(conf['src'], _builder)
        obs2 = Observer(conf['dst'], udp_notify)
        observers += [obs1, obs2]
        obs1.start()
        obs2.start()

    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        logger.info('Stopping observers')
        for obs in observers:
            obs.stop()
        logger.info('Joining observers')
        for obs in observers:
            obs.join()
        logger.info('Done')
        udp_notify.close()