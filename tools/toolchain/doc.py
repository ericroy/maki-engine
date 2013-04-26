from io import StringIO

class Node(object):
	def __init__(self, value):
		self._value = value
		self._children = []

	def __len__(self):
		return len(self._children)

	def __contains__(self, item):
		for c in self._children:
			if c._value == item:
				return True
		return False

	def index_of(self, item):
		for i, c in enumerate(self._children):
			if c._value == item:
				return i
		return -1

	def add_child(self, child):
		if not isinstance(child, Node):
			child = Node(str(child))
		self._children.append(child)
		return self._children[-1]

	def add_children(self, children):
		for c in children:
			self.add_child(c)

	def _quote(self, value):
		for ch in ',# \t\r\n':
			if ch in value:
				return '"%s"' % value.replace('"', '\\"')
		return value

	def serialize(self, out, indent_level=0, stacking=False):
		if not stacking:
			out.write('\t'*indent_level)
		out.write(self._quote(self._value))
		
		should_stack = len(self._children) <= 4
		for c in self._children:
			if len(c._children) != 0:
				should_stack = False
				break

		if self._children:
			if should_stack:
				out.write(' ')
				for c in self._children[:-1]:
					c.serialize(out, indent_level, True)
					out.write(', ')
				self._children[-1].serialize(out, indent_level, True)
				out.write('\n')
			else:
				out.write('\n')
				for c in self._children:
					c.serialize(out, indent_level+1, False)
		elif not stacking:
			out.write('\n')

def main():
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

if __name__ == '__main__':
	main()