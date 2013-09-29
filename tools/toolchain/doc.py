import sys
from io import StringIO

class Node(object):
    def __init__(self, value):
        self.value = value
        self.parent = None
        self._children = []

    def __len__(self):
        return len(self._children)

    def __contains__(self, item):
        for c in self._children:
            if c.value == item:
                return True
        return False

    def __str__(self):
        return self.value

    def __getitem__(self, key):
        if isinstance(key, int):
            return self._children[key]
        for c in iter(self._children):
            if c.value == key:
                return c
        raise KeyError('Node has no child with value "%s"' % key)

    def resolve(self, path):
        parts = path if isinstance(path, (list, tuple)) else path.split('.')
        if len(parts) == 0:
            return self
        key = parts[0]
        if key.startswith('#'):
            return self._children[int(key[1:])].resolve(parts[1:])
        return self[key].resolve(parts[1:])

    def children(self):
        return iter(self._children)

    def index_of(self, item):
        for i, c in enumerate(self._children):
            if c.value == item:
                return i
        return -1

    def add_child(self, child):
        if not isinstance(child, Node):
            child = Node(str(child))
        self._children.append(child)
        child.parent = self
        return self._children[-1]

    def remove_child(self, child):
        self._children.remove(child)

    def add_children(self, children):
        for c in children:
            self.add_child(c)

    def _quote(self, value):
        for ch in ',# \t\r\n':
            if ch in value:
                return '"%s"' % value.replace('"', '\\"')
        return value

    def serialize(self, out, max_stack=4):
        if self.value == "<root>":
            for child in self.children():
                child._serialize(out, max_stack)
        else:
            self._serialize(out, max_stack)

    def _serialize(self, out, max_stack, indent_level=0, stacking=False):
        if not stacking:
            out.write('\t'*indent_level)
        out.write(self._quote(self.value))
        
        should_stack = len(self._children) <= max_stack
        for c in self._children:
            if len(c._children) != 0:
                should_stack = False
                break

        if self._children:
            if should_stack:
                out.write(' ')
                for c in self._children[:-1]:
                    c._serialize(out, max_stack, indent_level, True)
                    out.write(', ')
                self._children[-1]._serialize(out, max_stack, indent_level, True)
                out.write('\n')
            else:
                out.write('\n')
                for c in self._children:
                    c._serialize(out, max_stack, indent_level+1, False)
        elif not stacking:
            out.write('\n')




TOKEN_INDENT = 0
TOKEN_EOL = 1
TOKEN_COMMA = 2
TOKENvalue = 3
TOKEN_COMMENT = 4

TOKEN_NAMES = {
    TOKEN_INDENT: 'INDENT',
    TOKEN_EOL: 'EOL',
    TOKEN_COMMA: 'COMMA',
    TOKENvalue: 'VALUE',
    TOKEN_COMMENT: 'COMMENT',
}

def _tokens(source):
    prev_token_type = None
    c = next(source)
    try:
        while True:
            token = []
            if c == '#':
                while c not in ('\r', '\n'):
                    token.append(c)
                    c = next(source)
                yield TOKEN_COMMENT, ''.join(token)
                prev_token_type = TOKEN_COMMENT
            elif c in (' ', '\t'):
                ws_type = c
                while c in (' ', '\t'):
                    if c != ws_type:
                        raise ValueError('Maki doc contained inconsistent whitespace')
                    token.append(c)
                    c = next(source)
                if prev_token_type == TOKEN_EOL:
                    yield TOKEN_INDENT, ''.join(token)
                    prev_token_type = TOKEN_INDENT
            elif c == ',':
                c = next(source)
                yield TOKEN_COMMA, ','
                prev_token_type = TOKEN_COMMA
            elif c == '"':
                if prev_token_type == TOKEN_EOL:
                    yield TOKEN_INDENT, ''
                    prev_token_type = TOKEN_INDENT
                c = next(source)
                while not (c == '"' and token[-1] != '\\'):
                    token.append(c)
                    c = next(source)
                yield TOKENvalue, ''.join(token)
                prev_token_type = TOKENvalue
                c = next(source)
            elif c in ('\r', '\n'):
                while c in ('\r', '\n'):
                    c = next(source)
                yield TOKEN_EOL, '\n'
                prev_token_type = TOKEN_EOL
            else:
                if prev_token_type == TOKEN_EOL:
                    yield TOKEN_INDENT, ''
                    prev_token_type = TOKEN_INDENT
                while c not in (' ', ',', '\r', '\n'):
                    token.append(c)
                    c = next(source)
                while c == ' ':
                    c = next(source)
                yield TOKENvalue, ''.join(token)
                prev_token_type = TOKENvalue
    except StopIteration:
        tokenvalue = ''.join(token)
        if tokenvalue.strip():
            yield TOKENvalue, tokenvalue
        raise

def deserialize(s):
    root = Node('<root>')
    it = iter(s)
    parent = root
    last_node = root
    indent = 0
    indent_format = None
    for tok_type, tok in _tokens(it):
        #print(TOKEN_NAMES[tok_type], repr(tok))
        if tok_type == TOKENvalue:
            indent += 1
            n = Node(tok)
            parent.add_child(n)
            last_node = n
            parent = n
        elif tok_type == TOKEN_INDENT:
            old_indent = indent
            if indent_format is None:
                if len(tok) > 0:
                    indent_format = tok
                    indent = len(tok) // len(indent_format)
                else:
                    indent = 0
            else:
                indent = len(tok) // len(indent_format)
            steps = indent - old_indent
            if steps < 0:
                for i in range(0, -steps):
                    parent = parent.parent
        elif tok_type == TOKEN_EOL:
            pass
        elif tok_type == TOKEN_COMMA:
            indent -= 1
            parent = last_node.parent
    return root














def test_serialize():
    root = Node('root')
    subnode1 = root.add_child('subnode1')
    subnode2 = root.add_child('subnode2')
    for i in range(3):
        subnode1.add_child('child%d' % i)
    for i in range(3):
        subnode2.add_child('child%d' % i)
    root.add_child('test').add_child('more').add_child('stuff')
    root.add_child('another thing').add_children(['one', 'two', 3, 4, 'five'])

    buffer = StringIO()
    root.serialize(buffer)
    buffer.seek(0)
    print(buffer.read())

def test_deserialize():
    s1 = """\
entity
    name Hull001
    transform
        pos -0.00339609384537, 0.0075023365207, -0.437618970871
    physics
        type dynamic
        shape box
        min -1.76784539223, -2.44680047035, 0.0
        max 1.76784539223, 2.44680047035, 1.96512675285
    children
        entity
            name Cylinder001
            transform
                pos 0.00339609384537, -0.00750235980377, 0.974541425705
            mesh
                mesh scenes/module/module_cylinder001.mmesh
                material materials/module.mdoc
        entity
            name Nav001
            transform
                pos 0.00339609384537, -0.00750234676525, 0.676456511021
            nav_mesh
                mesh scenes/module/module_nav001.mmesh
"""
    s2 = """\
vertex_shader
    file_name shaders/pos_norm_tan_uv/pos_norm_tan_uv.vs
    target vs_4_0
    entry_point main
    variants
        shadow SHADOW_VARIANT 1
        depth DEPTH_VARIANT 1
    defines
        NUM_LIGHTS 7
        NUM_SHADOW_LIGHTS 1
        NUM_CASCADED_SHADOW_LIGHTS 1
pixel_shader
    file_name shaders/pos_norm_tan_uv/pos_norm_tan_uv.ps
    target ps_4_0
    entry_point main
    variants
        shadow SHADOW_VARIANT 1
        depth DEPTH_VARIANT 1
    defines
        NUM_LIGHTS 7
        NUM_SHADOW_LIGHTS 1
        NUM_CASCADED_SHADOW_LIGHTS 1
"""
    root = deserialize(s2)
    #print(root.resolve('entity.transform.pos'))
    #print(root.resolve('entity.transform.pos.#1'))
    root.serialize(sys.stdout)

if __name__ == '__main__':
    test_deserialize()