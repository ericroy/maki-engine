import sys
import os
import json
import optparse

CONFIG = None
CONFIG_PATH = None

def _init_config():
    global CONFIG, CONFIG_PATH
    parser = optparse.OptionParser()
    parser.add_option('-c', '--config', dest='config', help='Specifies which project config file to use')
    options, args = parser.parse_args()
    CONFIG_PATH = options.config
    if CONFIG_PATH is None:
        CONFIG_PATH = os.environ.get('MAKI_PROJECT_CONFIG', 'project.json')
    with open(CONFIG_PATH) as file:
        CONFIG = json.loads(file.read())
        CONFIG['project_root'] = os.path.expanduser(os.path.expandvars(CONFIG['project_root']))

_init_config()
    
_fbx_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'fbx')
if _fbx_dir not in sys.path:
    sys.path.append(_fbx_dir)
from . import fbx
from . import compilers
from . import exporters
from . import archive
from . import doc
from . import manifest
from . import util
from . import watch