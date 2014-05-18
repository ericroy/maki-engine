import platform
from paver.easy import *

DEBUG = False
CONFIGURATION = 'Release'
PLATFORM = 'win' if platform.system() == 'Windows' else 'nix'

KNOWN_PATHS = {
	'sdl': {
		'win': 'deps/SDL2-2.0.0/include',
		'nix': '/usr/local/include/SDL2'
	},
	'bullet': {
		'win': 'deps/bullet-2.81-rev2613/src',
		'nix': ''
	},
	'lua': {
		'win': 'deps/LuaJIT-2.0.2/src',
		'nix': ''
	}
}

def known_path(name):
	return KNOWN_PATHS[name][PLATFORM]

def relative(series, basedir):
	return [basedir.relpathto(p) for p in series]

def listify(series, prefix=''):
	return ' '.join(['%s%s' % (prefix, path(d).normpath().replace('\\', '/')) for d in series])

def lib_listify(series):
	if PLATFORM == 'win':
		return listify(series, '')
	return listify(series, '-l')

def include_listify(series):
	if PLATFORM == 'win':
		return listify(series, '/I')
	return listify(series, '-I')

def defines_listify(series):
	def __all():
		if DEBUG:
			yield '_DEBUG=1'
		for item in series:
			yield item
	if PLATFORM == 'win':
		return listify(__all(), '/D')
	return listify(__all(), '-D')

def lib_name(basename):
	return basename+'.lib' if PLATFORM == 'win' else basename

def object_name(filename):
	if PLATFORM == 'win':
		return path(filename).basename().stripext()+'.obj'
	return path(filename).basename().stripext()+'.o'

def compile_lib(sources, build_dir, out, include_dirs=[], libs=[], defines=[]):
	if not out.dirname().exists():
		out.dirname().makedirs()
	out = build_dir.relpathto(out)
	include_dirs = include_listify(relative(include_dirs, build_dir))
	libs = lib_listify(relative(libs, build_dir))
	defines = defines_listify(defines)
	sources = list(sources)
	objects = listify([object_name(s) for s in sources])
	sources = listify(relative(sources, build_dir))
	with pushd(build_dir):
		print(out)
		if PLATFORM == 'win':
			# Build static libraries on windows
			sh('cl /MP /EHsc /c %s %s' % (include_dirs, sources))
			sh('lib /OUT:%s %s' % (out, objects))
		else:
			# Build shared libraries on *nix
			out = out.dirname() / ('lib' + out.basename() + '.so')
			sh('clang++ -std=c++11 -fno-rtti -fPIC -pthreads -o %s %s %s -shared -lSDL2 -ldl -lrt' % (out, sources, include_dirs))

@task
def debug():
	global DEBUG, CONFIGURATION
	DEBUG = True
	CONFIGURATION = 'Debug'

@task
def core():
	build_dir = path('build/core')
	try:
		build_dir.rmtree()
		build_dir.makedirs()
		out = path('bin') / path(CONFIGURATION) /  lib_name('MakiCore')
		sources = path('src/core').walkfiles('*.cpp')
		compile_lib(sources, build_dir, out, include_dirs=['src', known_path('sdl')])
	finally:
		build_dir.rmtree()

@task
def framework():
	build_dir = path('build/framework')
	try:
		build_dir.rmtree()
		build_dir.makedirs()
		out = path('bin') / path(CONFIGURATION) /  lib_name('MakiFramework')
		sources = path('src/framework').walkfiles('*.cpp')
		compile_lib(sources, build_dir, out, include_dirs=['src', known_path('sdl'), known_path('bullet'), known_path('lua')])
	finally:
		build_dir.rmtree()

@task
def d3d():
	build_dir = path('build/d3d')
	try:
		build_dir.rmtree()
		build_dir.makedirs()
		out = path('bin') / path(CONFIGURATION) /  lib_name('MakiD3D')
		sources = path('src/d3d').walkfiles('*.cpp')
		compile_lib(sources, build_dir, out, include_dirs=['src', known_path('sdl')])
	finally:
		build_dir.rmtree()

@task
def ogl():
	build_dir = path('build/ogl')
	try:
		build_dir.rmtree()
		build_dir.makedirs()
		out = path('bin') / path(CONFIGURATION) / lib_name('MakiOGL')
		sources = path('src/ogl').walkfiles('*.cpp')
		compile_lib(sources, build_dir, out, include_dirs=['src', known_path('sdl')])
	finally:
		build_dir.rmtree()

@needs('core', 'framework', 'd3d', 'ogl')
@task
def maki():
	pass
