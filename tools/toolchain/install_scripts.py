import sys
import os
import shutil
import tempfile
import subprocess
import traceback

SEE_MASK_NOCLOSEPROCESS = 0x00000040
INFINITE = 0xffffffff

TOOLS_DIR = os.path.expandvars('$MAKI_DIR/tools')

PATHS = {
	'photoshop_scripts': (	
		r'%ProgramW6432%\Adobe\Adobe Photoshop CS6 (64 Bit)\Presets\Scripts',
		r'%ProgramFiles(x86)%\Adobe\Adobe Photoshop CS6\Presets\Scripts',
		r'%ProgramW6432%\Adobe\Adobe Photoshop CC (64 Bit)\Presets\Scripts',
		r'%ProgramFiles(x86)%\Adobe\Adobe Photoshop CC\Presets\Scripts'
		),
	'extension_manager': (
		r'%ProgramFiles(x86)%\Adobe\Adobe Extension Manager CS6\Adobe Extension Manager CS6.exe'
		r'%ProgramFiles(x86)%\Adobe\Adobe Extension Manager CC\Adobe Extension Manager CC.exe'
		),
	'flash': (
		),
	}

def _get_path(key):
	for p in PATHS[key]:
		p = os.path.expandvars(p)
		if os.path.exists(p):
			return p
	return None

def _existing_paths(key):
	for p in PATHS[key]:
		p = os.path.expandvars(p)
		if os.path.exists(p):
			yield p	

def _install_scripts():
	print('Started')
	paths = list(_existing_paths('photoshop_scripts'))
	if not any(paths):
		raise RuntimeError('Cannot find Adobe Photoshop scripts directory')
	for p in paths:
		shutil.copyfile(os.path.join(TOOLS_DIR, 'ps/Maki Export Level.jsx'), p)
		print('Installed Maki Export Level script to: ' + p)

	p = _get_path('extension_manager')
	if p is None:
		raise RuntimeError('Cannot find Adobe Extension Manager')
	subprocess.check_call([p, '-suppress', '-install', 'zxp="%s"' % os.path.join(TOOLS_DIR, 'ps/MakiMeta.zxp')])
	print('Installed MakiMeta extension')
	print('Done')


def install_scripts():
	import ctypes
	import win32com.shell.shell as shell
	with tempfile.NamedTemporaryFile(mode='w+', delete=False) as temp_file:
		command_line = '"%s" "%s"' % (os.path.realpath(__file__), os.path.abspath(temp_file.name))
		print(command_line)

		res = shell.ShellExecuteEx(lpVerb='runas', lpFile=sys.executable, lpParameters=command_line, fMask=SEE_MASK_NOCLOSEPROCESS)
		ctypes.windll.kernel32.WaitForSingleObject(int(res['hProcess']), INFINITE)

		temp_file.seek(0)
		s = temp_file.read()
		if s:
			print(s)

if __name__ == '__main__':
	with open(sys.argv[1], 'w') as out:
		sys.stdout = out
		sys.stderr = out
		try:
			_install_scripts()
		except:
			traceback.print_exc()
