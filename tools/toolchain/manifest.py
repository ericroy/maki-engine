import sys
import os
from . import CONFIG
from . import compilers
from . import util

ENUM_NAME_REPLACEMENTS = ('/', ' ', '.', '_', '-')

TEMPLATE_HEADER = '''
/* Programmatically generated, do not modify! */
#pragma once
#include "Maki.h"

class %(class_name)s : public maki::core::asset_manifest_t {
public:
    static const unsigned int asset_count_ = %(resource_count)s;
    static const char *asset_paths_[asset_count_];

private:
    static maki::rid_t rids_[asset_count_];

public:
    %(public_vars)s

public:
    %(class_name)s(void) : maki::core::asset_manifest_t(asset_count_, rids_, asset_paths_, "%(common_prefix)s") {}
    virtual ~%(class_name)s() {}
};

'''

TEMPLATE_SOURCE = '''
/* Programmatically generated, do not modify! */
#include "%(file_name)s.h"

maki::rid_t %(class_name)s::rids_[%(class_name)s::asset_count_] = {%(rid_list)s};

%(public_var_defs)s

const char *%(class_name)s::asset_paths_[asset_count_] = {
%(paths)s
};
'''


def _write_manifest(conf, resources):
    manifest_name = conf['manifest']
    file_name = _titlecase(manifest_name)
    class_name = manifest_name + '_t'
    paths = ''
    public_vars = ''
    public_var_defs = ''
    rid_list = ','.join(['{%s}' % i for i in range(len(resources))])
    common_prefix = util.clean_path(os.path.relpath(os.path.join(CONFIG['archive_path'], conf['dst']), CONFIG['bin_path'])) + '/'

    for i, (name, rel_path) in enumerate(resources):
        public_vars += 'static const maki::rid_t &%s_; // %d\n\t' % (name, i)
        public_var_defs += 'const maki::rid_t &%s::%s_ = %s::rids_[%s];\n' % (class_name, name, class_name, i)
        paths += '\t"%s%s",\n' % (common_prefix, rel_path)

    with open(os.path.join(CONFIG['manifest_output_path'], '%s.h' % file_name), 'w') as file:
        file.write(TEMPLATE_HEADER % {
            'class_name': class_name,
            'file_name': file_name,
            'resource_count': len(resources),
            'public_vars': public_vars,
            'common_prefix': common_prefix
        })

    with open(os.path.join(CONFIG['manifest_output_path'], '%s.cpp' % file_name), 'w') as file:
        file.write(TEMPLATE_SOURCE % {
            'class_name': class_name,
            'file_name': file_name,
            'rid_list': rid_list,
            'public_var_defs': public_var_defs,
            'paths': paths
        })


def _titlecase(s):
    parts = []
    s = s.lower().replace('_', ' ').replace('-', ' ')
    for word in s.split(' '):
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
            name = name.replace(repl, '_')
        parts.append(name.lower())
    return '_'.join(parts) + '_' + ext


def manifest(arc_name):
    conf = CONFIG['assets'][arc_name]
    resources = []
    for path in util.walk(conf['dst']):
        ext = os.path.splitext(path)[1]
        ext = ext.strip('.')
        if ext in compilers.COMPILERS:
            name = _make_enum_name(path)
            resources.append([name, path])
    _write_manifest(conf, resources)


if __name__ == '__main__':
    assert len(sys.argv) == 2, 'Manifest requires <arc_name>'
    manifest(sys.argv[1])
