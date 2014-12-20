#include "core/core_stdafx.h"
#include "core/MakiDocument.h"

#ifndef MAKI_TOOLS
#include "core/MakiEngine.h"
#include "core/MakiAssetLibrary.h"
#endif

#include <sstream>

namespace maki
{
	namespace core
	{

		document_t::node_t::node_t(const char *data, uint32 length)
			: parent(nullptr), value(nullptr), count(0), capacity(0), children(nullptr)
		{
			value = new char[length+1];
			memcpy(value, data, length+1);
			value[length] = 0;
		}

		document_t::node_t::node_t(char *data, uint32 length, bool should_clone_data)
			: parent(nullptr), value(nullptr), count(0), capacity(0), children(nullptr)
		{
			if(should_clone_data) {
				value = new char[length+1];
				memcpy(value, data, length+1);
				value[length] = 0;
			} else {
				value = data;
			}
		}

		document_t::node_t::~node_t()
		{
			MAKI_SAFE_DELETE_ARRAY(value);
			for(uint32 i = 0; i < count; i++) {
				MAKI_SAFE_DELETE(children[i]);
			}
			MAKI_SAFE_FREE(children);
		}

		void document_t::node_t::append_child(node_t *n)
		{
			n->parent = this;
			if(count == capacity) {
				capacity = std::max<uint32>(8, capacity*2);
				children = (node_t **)allocator_t::realloc(children, capacity * sizeof(node_t *), std::alignment_of<node_t *>::value_);
				assert(children != nullptr);
			}
			children[count++] = n;
		}

		document_t::node_t *document_t::node_t::remove_child(uint32 index)
		{
			document_t::node_t *ret = children[index];
			memmove(&children[index], &children[index+1], (count-index-1)*sizeof(node_t *));
			return ret;
		}

		bool document_t::node_t::remove_child(document_t::node_t *n)
		{
			for(uint32 i = 0; i < count; i++) {
				if(children[i] == n) {
					remove_child(i);
					return true;
				}
			}
			return false;
		}

		void document_t::node_t::detach()
		{
			if(parent != nullptr) {
				parent->remove_child(this);
			}
		}
	
		int32 document_t::node_t::value_as_int(int32 default_value) const
		{
			int32 v;
			if(value_as_int(&v)) {
				return v;
			}
			return default_value;
		}
		bool document_t::node_t::value_as_int(int32 *out) const
		{
			char *end;
			int32 v = strtol(value, &end, 10);
			if(end == value) {
				return false;
			}
			if(out != nullptr) {
				*out = v;
			}
			return true;
		}

		uint32 document_t::node_t::value_as_uint(uint32 default_value) const
		{
			uint32 v;
			if(value_as_uint(&v)) {
				return v;
			}
			return default_value;
		}
		bool document_t::node_t::value_as_uint(uint32 *out) const
		{
			char *end;
			uint32 v = strtoul(value, &end, 10);
			if(end == value) {
				return false;
			}
			if(out != nullptr) {
				*out = v;
			}
			return true;
		}

		float document_t::node_t::value_as_float(float default_value) const
		{
			float v;
			if(value_as_float(&v)) {
				return v;
			}
			return default_value;
		}
		bool document_t::node_t::value_as_float(float *out) const
		{
			char *end;
			float v = (float)strtod(value, &end);
			if(end == value) {
				return false;
			}
			if(out != nullptr) {
				*out = v;
			}
			return true;
		}

		bool document_t::node_t::value_as_bool(bool default_value) const
		{
			bool v;
			if(value_as_bool(&v)) {
				return v;
			}
			return default_value;
		}
		bool document_t::node_t::value_as_bool(bool *out) const
		{
			if(out != nullptr) {
				*out = value[0] == 'T' || value[0] == 't' || value[0] == '1' || value[0] == 'Y' || value[0] == 'y';
			}
			return true;
		}

		document_t::node_t *document_t::node_t::resolve(const char *node_path)
		{
			if(*node_path == 0) {
				return this;
			}
		
			const char *p = node_path;

			// Referencing a child by index
			if(*p == '#') {
				p++;
				char *end = nullptr;
				uint32 i = strtoul(p, &end, 10);
				p = end;
				if(i >= count) {
					return nullptr;
				}
				return children[i]->resolve(p);
			}

			// Referencing a child by name
			while(*p != 0 && *p != '.') {
				p++;
			}
			for(uint32 i = 0; i < count; i++) {
				if(strncmp(children[i]->value, node_path, p-node_path) == 0) {
					if(*p == '.') {
						p++;
					}
					return children[i]->resolve(p);
				}
			}
			return nullptr;
		}









		document_t::document_t()
		:	root(nullptr)
		{
			root = new node_t("<root>", 6);
		}

		document_t::~document_t()
		{
			MAKI_SAFE_DELETE(root);
		}

	#ifndef MAKI_TOOLS
		bool document_t::load(rid_t rid)
		{
			uint32 bytes;
			char *data = engine_t::get()->assets->alloc_read(rid, &bytes);
			if(data == nullptr) {
				console_t::error("Failed to alloc_read document bytes");
				return false;
			}
			bool success = load(data, bytes);
			MAKI_SAFE_FREE(data);
			return success;
		}
	#endif

		bool document_t::load(char *data, uint32 length)
		{
			if(document_binary_serializer_t::IsBinaryDocument(data, length)) {
				document_binary_serializer_t serial(*this);
				return serial.deserialize(data, length);
			} else {
				document_text_serializer_t serial(*this);
				return serial.deserialize(data, length);
			}
		}

		void document_t::print(const char *indent_token)
		{
			if(root == nullptr) {
				return;
			}
			std::ostringstream out;
			document_text_serializer_t serial(*this);
			serial.serialize(out, indent_token);
			console_t::info(out.str().c_str());
		}






		document_text_serializer_t::document_text_serializer_t(document_t &doc)
			: doc(doc)
		{
		}

		document_text_serializer_t::~document_text_serializer_t()
		{
		}

		bool document_text_serializer_t::serialize(char *path, const char *indent_token)
		{
			if(doc.root == nullptr) {
				console_t::error("Could not serialize text document, root was null");
				return false;
			}

			std::ofstream file(path, std::ios_base::out);
			if(!file.good()) {
				file.close();
				console_t::error("Failed to open file for text document serialization");
				return false;
			}

			for(uint32 i = 0; i < doc.root->count; i++) {
				serialize_node(doc.root->children[i], 0, file, false, false, false, indent_token);
			}

			file.close();
			return true;
		}

		bool document_text_serializer_t::serialize(std::ostream &out, const char *indent_token)
		{
			if(doc.root == nullptr) {
				console_t::error("Could not serialize text document, root was null");
				return false;
			}

			for(uint32 i = 0; i < doc.root->count; i++) {
				serialize_node(doc.root->children[i], 0, out, false, false, false, indent_token);
			}
			return true;
		}

		void document_text_serializer_t::serialize_node(document_t::node_t *n, uint32 depth, std::ostream &out, bool stacking, bool first_in_stack, bool last_in_stack, const char *indent_token)
		{
			if(stacking) {
				out << " ";
			} else {
				for(uint32 i = 0; i < depth; i++) {
					out << indent_token;
				}
			}
		
			std::string s = n->value;
		
			// Escape quotes
			std::size_t start = 0;
			std::size_t loc = s.find('"', start);
			while(loc != std::string::npos) {
				s.replace(loc, 1, "\\\"");
				start = loc+2;
				loc = s.find('"', start);
			}

			// Quote string if necessary
			loc = s.find_first_of(" \t\r\n,#");
			if(loc != std::string::npos) {
				out << "\"" << s << "\"" ;
			} else {
				out << s;
			}

			// Check for a special case where we will stack a small number of leaf nodes on the
			// same line for readability (vectors, particularly)
			bool stackChildren = false;
			if(n->count > 0 && n->count <= stack_children_count_threshold_) {
				stackChildren = true;
				for(uint32 i = 0; i < n->count; i++) {
					if(n->children[i]->count != 0) {
						stackChildren = false;
						break;
					}
				}
			}

			if(stacking && !last_in_stack) {
				out << ",";
			} else {
				if(!stackChildren) {
					out << std::endl;
				}
			}

			for(uint32 i = 0; i < n->count; i++) {
				serialize_node(n->children[i], depth+1, out, stackChildren, i == 0, i == n->count-1, indent_token);
			}
		}

		bool document_text_serializer_t::deserialize(char *data, uint32 length)
		{
			MAKI_SAFE_DELETE(doc.root);
			doc.root = new document_t::node_t("<root>", 6);
		
			uint32 character = 0;
			uint32 line = 0;
			uint32 indentWidth = 0;

			bool cleanLine = true;
			bool append_sibling = false;
			int32 indentLevel = 0;
			int32 previous_indent_level = -1;
			document_t::node_t *previous = doc.root;
		
			char *p = data;

			while(*p != 0) {
				if(*p == '#') {
					do { p++; character++; } while(*p != 0 && *p != '\r' && *p != '\n');
				} else if(*p == ' ' || *p == '\t') {
					do { p++; character++; } while(*p == ' ' || *p == '\t');
					indentLevel++;
				} else if(*p == '\n' || *p == '\r') {
					cleanLine = true;
					do { p++; line++; character = 0; indentLevel = 0; } while(*p == '\n' || *p == '\r');
					if(*p == ' ' || *p == '\t') {
						uint32 count = 0;
						do { p++; character++; count++; } while(*p == ' ' || *p == '\t');

						indentLevel += indentWidth != 0 ? count/indentWidth : count;
						if(indentLevel > previous_indent_level+1) {
							if(indentWidth == 0) {
								indentWidth = indentLevel - previous_indent_level;
								indentLevel = previous_indent_level+1;
							} else {
								console_t::error("Indentation error at line %d, char %d", line, character);
								return false;
							}
						}
					}
				} else if(*p == ',') {
					p++; character++;
					append_sibling = true;
				} else if(*p == '"') {
					p++; character++;
					char prior = *p;
					char *start = p;
					uint32 length = 0;

					while(*p != 0 && !(*p == '"' && prior != '\\')) {
						prior = *p; p++; character++; length++;
					}
					if(*p != '"') {
						console_t::error("Expected closing quote at line %d, char %d", line, character);
						return false;
					}
					p++; character++;

					// Unescape quotes in string
					char *buffer = new char[length+1];
					char *q = start;
					uint32 newLength = 0;
					for(uint32 i = 0; i < length; i++) {
						if(i > 0 && *q == '"' && *(q-1) == '\\') {
							// Back-stomp on the escape character
							newLength--;
						}
						buffer[newLength++] = *q++;
					}
					buffer[newLength] = 0;

					// We give ownership of the buffer we allocated to the node
					if(!add_node(new document_t::node_t(buffer, newLength, false), indentLevel, &previous, previous_indent_level, cleanLine, append_sibling)) {
						console_t::error("Parse error near line %d, char %d", line, character);
						return false;
					}
				} else {
					char *start = p;
					uint32 length = 0;
					while(*p != 0 && *p != ' ' && *p != '\t' && *p != '\r' && *p != ',') {
						p++; character++; length++;
					}
					if(!add_node(new document_t::node_t(start, length, true), indentLevel, &previous, previous_indent_level, cleanLine, append_sibling)) {
						console_t::error("Parse error near line %d, char %d", line, character);
						return false;
					}
				}
			}
			return true;
		}

		bool document_text_serializer_t::add_node(document_t::node_t *n, int32 indentLevel, document_t::node_t **previous, int32 &previous_indent_level, bool &cleanLine, bool &append_sibling)
		{
			if(!cleanLine || indentLevel > previous_indent_level) {
				if(append_sibling) {
					(*previous)->append_sibling(n);
				} else {
					(*previous)->append_child(n);
					*previous = n;
					previous_indent_level = indentLevel;
				}
			} else {
				document_t::node_t *parent = *previous;
				for(int32 i = indentLevel; i <= previous_indent_level; i++) {
					parent = parent->parent;
					if(parent == nullptr) {
						console_t::error("Parsing document failed, suspect indentation");
						return false;
					}
				}
				if(append_sibling) {
					parent->append_sibling(n);
				} else {
					parent->append_child(n);
					*previous = n;
					previous_indent_level = indentLevel;
				}
			}
			cleanLine = false;
			append_sibling = false;
			return true;
		}







		const uint8 document_binary_serializer_t::BINARY_HEADER[4] = {'\0', 'H', 'D', '\0'};

		bool document_binary_serializer_t::IsBinaryDocument(char *data, uint32 length)
		{
			return data != nullptr && length >= sizeof(BINARY_HEADER) && memcmp(data, BINARY_HEADER, sizeof(BINARY_HEADER)) == 0;
		}

		document_binary_serializer_t::document_binary_serializer_t(document_t &doc)
			: doc(doc)
		{
		}
	
		document_binary_serializer_t::~document_binary_serializer_t()
		{
		}

		bool document_binary_serializer_t::serialize(char *path)
		{
			if(doc.root == nullptr) {
				console_t::error("Could not serialize binary document, root was null");
				return false;
			}

			std::ofstream file(path, std::ios_base::out | std::ios_base::binary);
			if(!file.good()) {
				file.close();
				console_t::error("Failed to open file for binary document serialization");
				return false;
			}

			bool success = serialize(file);
			file.close();
			return success;
		}

		bool document_binary_serializer_t::serialize(std::ostream &out)
		{
			if(doc.root == nullptr) {
				console_t::error("Could not serialize binary document, root was null");
				return false;
			}

			std::vector<std::string> table;
			std::ostringstream body;

			// serialize our document to the stringstream, constructing a stringtable along the way
			for(uint32 i = 0; i < doc.root->count; i++) {
				uint32 level = 0;
				serialize_node(doc.root->children[i], body, table, level);
			}

			// write header ident
			uint32 offset = 0;
			out.write((char *)BINARY_HEADER, sizeof(BINARY_HEADER));
			offset += sizeof(BINARY_HEADER);

			// write string table count
			uint32 stringCount = table.size();
			assert(stringCount < 1<<16);
			uint16 sc = stringCount;
			out.write((char *)&sc, sizeof(sc));
			offset += sizeof(sc);
		
			// write each string, null terminated
			for(uint32 i = 0; i < stringCount; i++) {
				const std::string &s = table[i];
				uint32 bytes = s.size()+1;
				out.write(s.c_str(), bytes);
				offset += bytes;
			}

			// Align
			uint8 pad = 0xff;
			while(offset % 2 != 0) {
				out.write((char *)&pad, 1);
				offset++;
			}		

			// write the document
			std::string bodyString = body.str();
			out.write(bodyString.c_str(), bodyString.size());
		
			return true;
		}

		uint16 document_binary_serializer_t::get_or_add(std::vector<std::string> &string_table, char *str)
		{
			std::vector<std::string>::iterator iter = std::find(string_table.begin(), string_table.end(), str);
			if(iter != string_table.end()) {
				return iter - string_table.begin();
			}
			string_table.push_back(str);
			uint32 index = string_table.size()-1;
			assert(index < 1<<16);
			return (uint16)index;
		}

		void document_binary_serializer_t::serialize_node(document_t::node_t *n, std::ostream &body, std::vector<std::string> &string_table, uint32 &level)
		{
			// write indentation depth
			assert(level < 1<<16);
			uint16 l = level;
			body.write((char *)&l, sizeof(l));
		
			// write node value (index into string table actually)
			uint16 stringIndex = get_or_add(string_table, n->value);
			body.write((char *)&stringIndex, sizeof(stringIndex));

			level++;
			for(uint32 i = 0; i < n->count; i++) {
				serialize_node(n->children[i], body, string_table, level);
			}
			level--;
		}

		bool document_binary_serializer_t::deserialize(char *data, uint32 length)
		{
			if(!IsBinaryDocument(data, length)) {
				console_t::error("Data did not contain a binary document");
				return false;
			}

			MAKI_SAFE_DELETE(doc.root);
			doc.root = new document_t::node_t("<root>", 6);

			char *p = data; p += sizeof(BINARY_HEADER);

			// Parse string table, finding the offsets to the start of each string
			uint16 stringTableCount = *(uint16 *)p; p += sizeof(uint16);
			uint16 *stringTableOffsets = new uint16[stringTableCount+1];
			char *stringTableStart = p;
			for(uint16 i = 0; i < stringTableCount; i++) {
				stringTableOffsets[i] = p - stringTableStart;
				while(*p != 0) {
					p++;
				}
				p++;
			}
			stringTableOffsets[stringTableCount] = p - stringTableStart;

			// Align
			while((p-data) % 2 != 0) {
				p++;
			}	

			int32 previousLevel = -1;
			document_t::node_t *previous = doc.root;

			while(p < data+length) {
				// Read indentation depth
				uint16 level = *(uint16 *)p; p += sizeof(uint16);

				// Read string table index which specifies the node value
				uint16 stringIndex = *(uint16 *)p; p += sizeof(uint16);

				document_t::node_t *n = new document_t::node_t(stringTableStart+stringTableOffsets[stringIndex], stringTableOffsets[stringIndex+1]-stringTableOffsets[stringIndex], true);
				if(level > previousLevel) {
					previous->append_child(n);
				} else {
					while(level <= previousLevel) {
						previous = previous->parent;
						assert(previous != nullptr);
						previousLevel--;
					}
					previous->append_child(n);
				}
				previous = n;
				previousLevel = level;
			}

			MAKI_SAFE_DELETE_ARRAY(stringTableOffsets);
			return true;
		}

	} // namespace core

} // namespace maki
