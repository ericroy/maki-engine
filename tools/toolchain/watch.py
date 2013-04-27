import sys
import os
import time
import win32file
import win32con
import win32event
import threading
from collections import defaultdict
from . import util
from . import CONFIG

FILE_LIST_DIRECTORY = 0x0001
FILE_NOTIFY_CHANGE_CREATION = 0x00000040
FILE_NOTIFY_CHANGE_LAST_WRITE = 0x00000010

class UDPNotifier(object):
    def __init__(self, host, port):
        self._host = host
        self._port = port

    def __call__(self, path):
        print('Notify %s' % path)

    def close(self):
        pass

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
    print('Build: %s' % path)

def watch_forever():
    print('Watching...')
    udp_notify = UDPNotifier('127.0.0.1', 11001)
    obs1 = Observer(CONFIG['assets_path'], _builder)
    obs2 = Observer(CONFIG['assets_bin_path'], udp_notify)
    obs1.start()
    obs2.start()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        obs1.stop()
        obs2.stop()
        obs1.join()
        obs2.join()
        udp_notify.close()