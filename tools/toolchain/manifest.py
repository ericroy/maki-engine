import sys
import os
from . import CONFIG
from . import compilers
from . import util

ENUM_NAME_REPLACEMENTS = (
    '/', ' ', '.', '_', '-',
)

COMMON_PREFIX = '../assets_bin/'

TEMPLATE_HEADER = '''
/* Programmatically generated, do not modify! */
#pragma once
#include "Maki.h"

class %(name)s : public Maki::Core::AssetManifest {
public:
    static const unsigned int AssetCount = %(resource_count)s;
    static const char *assetPaths[AssetCount];

private:
    static Maki::Rid rids[AssetCount];

public:
    %(public_vars)s

public:
    Assets(void) : Maki::Core::AssetManifest(AssetCount, rids, assetPaths, "%(common_prefix)s") {}
    virtual ~Assets() {}
};

'''

TEMPLATE_SOURCE = '''
/* Programmatically generated, do not modify! */
#include "%(name)s.h"

Maki::Rid %(name)s::rids[%(name)s::AssetCount] = {%(rid_list)s};

%(public_var_defs)s

const char *Assets::assetPaths[AssetCount] = {
%(paths)s
};
'''

def _write_manifest(manifest_name, resources):
    paths = ''
    public_vars = ''
    private_var_defs = ''
    public_var_defs = ''
    rid_list = ','.join(['{%s}' % i for i in range(len(resources))])
    common_prefix = util.clean_path(os.path.relpath(CONFIG['assets_bin_path'], CONFIG['bin_path']))+'/'

    for i, (name, rel_path) in enumerate(resources):
        public_vars += 'static const Maki::Rid &%s; // %d\n\t' % (name, i)
        private_var_defs += 'Maki::Rid %s::_%s = {%s};\n' % (manifest_name, name, i)
        public_var_defs += 'const Maki::Rid &%s::%s = %s::rids[%s];\n' % (manifest_name, name, manifest_name, i)
        paths += '\t"%s%s",\n' % (common_prefix, rel_path)


    with open(os.path.join(CONFIG['manifest_output_path'], '%s.h' % manifest_name), 'w') as file:
        file.write(TEMPLATE_HEADER % {
            'name': manifest_name,
            'resource_count': len(resources),
            'public_vars': public_vars,
            'common_prefix': common_prefix
        })
    with open(os.path.join(CONFIG['manifest_output_path'], '%s.cpp' % manifest_name), 'w') as file:
        file.write(TEMPLATE_SOURCE % {
            'name': manifest_name,
            'rid_list': rid_list,
            'public_var_defs': public_var_defs,
            'paths': paths
        })

def _titlecase(s):
    parts = []
    for word in s.lower().split(' '):
        if word:
            parts.append('%s%s' % (word[0].capitalize(), word[1:]))
    return ''.join(parts)

def _make_enum_name(p):
    p = util.clean_path(p)
    base, ext = os.path.splitext(p)
    ext = ext[1:]
    parts = []
    for name in base.split('/'):
        if name == '.':
            continue
        for repl in ENUM_NAME_REPLACEMENTS:
            name = name.replace(repl, ' ')
        parts.append(_titlecase(name))
    return '_'.join(parts)+'_'+ext

def manifest(src_dir, manifest_name, *args):
    resources = []
    for path in util.walk(src_dir):
        ext = os.path.splitext(path)[1]
        ext = ext.strip('.')
        if ext in compilers.COMPILERS:
            name = _make_enum_name(path)
            resources.append([name, path])
    _write_manifest(manifest_name, resources)

if __name__ == '__main__':
    assert len(sys.argv) > 3, 'Manifest requires src dir and manifest name'
    manifest(sys.argv[1], sys.argv[2], *sys.argv[3:])