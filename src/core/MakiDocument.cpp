#include "core/core_stdafx.h"
#include "core/MakiDocument.h"

#ifndef MAKI_TOOLS
#include "core/MakiEngine.h"
#include "core/MakiAssetLibrary.h"
#endif

#include <sstream>

namespace Maki
{
	namespace Core
	{


		Document::Node::Node(char *data, uint32 length, bool shouldCloneData)
			: parent(nullptr), value(nullptr), count(0), capacity(0), children(nullptr)
		{
			if(shouldCloneData) {
				value = new char[length+1];
				memcpy(value, data, length+1);
				value[length] = 0;
			} else {
				value = data;
			}
		}

		Document::Node::~Node()
		{
			SAFE_DELETE_ARRAY(value);
			for(uint32 i = 0; i < count; i++) {
				SAFE_DELETE(children[i]);
			}
			SAFE_FREE(children);
		}

		void Document::Node::AppendChild(Node *n)
		{
			n->parent = this;
			if(count == capacity) {
				capacity = std::max<uint32>(8, capacity*2);
				children = (Node **)Allocator::Realloc(children, capacity * sizeof(Node *), std::alignment_of<Node *>::value);
				assert(children != nullptr);
			}
			children[count++] = n;
		}

		Document::Node *Document::Node::RemoveChild(uint32 index)
		{
			Document::Node *ret = children[index];
			memmove(&children[index], &children[index+1], (count-index-1)*sizeof(Node *));
			return ret;
		}

		bool Document::Node::RemoveChild(Document::Node *n)
		{
			for(uint32 i = 0; i < count; i++) {
				if(children[i] == n) {
					RemoveChild(i);
					return true;
				}
			}
			return false;
		}

		void Document::Node::Detach()
		{
			if(parent != nullptr) {
				parent->RemoveChild(this);
			}
		}
	
		int32 Document::Node::ValueAsInt(int32 defaultValue) const
		{
			int32 v;
			if(ValueAsInt(&v)) {
				return v;
			}
			return defaultValue;
		}
		bool Document::Node::ValueAsInt(int32 *out) const
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

		uint32 Document::Node::ValueAsUInt(uint32 defaultValue) const
		{
			uint32 v;
			if(ValueAsUInt(&v)) {
				return v;
			}
			return defaultValue;
		}
		bool Document::Node::ValueAsUInt(uint32 *out) const
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

		float Document::Node::ValueAsFloat(float defaultValue) const
		{
			float v;
			if(ValueAsFloat(&v)) {
				return v;
			}
			return defaultValue;
		}
		bool Document::Node::ValueAsFloat(float *out) const
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

		bool Document::Node::ValueAsBool(bool defaultValue) const
		{
			bool v;
			if(ValueAsBool(&v)) {
				return v;
			}
			return defaultValue;
		}
		bool Document::Node::ValueAsBool(bool *out) const
		{
			if(out != nullptr) {
				*out = value[0] == 'T' || value[0] == 't' || value[0] == '1' || value[0] == 'Y' || value[0] == 'y';
			}
			return true;
		}

		Document::Node *Document::Node::Resolve(const char *nodePath)
		{
			if(*nodePath == 0) {
				return this;
			}
		
			const char *p = nodePath;

			// Referencing a child by index
			if(*p == '#') {
				p++;
				char *end = nullptr;
				uint32 i = strtoul(p, &end, 10);
				p = end;
				if(i >= count) {
					return nullptr;
				}
				return children[i]->Resolve(p);
			}

			// Referencing a child by name
			while(*p != 0 && *p != '.') {
				p++;
			}
			for(uint32 i = 0; i < count; i++) {
				if(strncmp(children[i]->value, nodePath, p-nodePath) == 0) {
					if(*p == '.') {
						p++;
					}
					return children[i]->Resolve(p);
				}
			}
			return nullptr;
		}









		Document::Document()
		:	root(nullptr)
		{
			root = new Node("<root>", 6, true);
		}

		Document::~Document()
		{
			SAFE_DELETE(root);
		}

	#ifndef MAKI_TOOLS
		bool Document::Load(Rid rid)
		{
			uint32 bytes;
			char *data = Engine::Get()->assets->AllocRead(rid, &bytes);
			if(data == nullptr) {
				Console::Error("Failed to AllocRead document bytes");
				return false;
			}
			bool success = Load(data, bytes);
			SAFE_FREE(data);
			return success;
		}
	#endif

		bool Document::Load(char *data, uint32 length)
		{
			if(DocumentBinarySerializer::IsBinaryDocument(data, length)) {
				DocumentBinarySerializer serial(*this);
				return serial.Deserialize(data, length);
			} else {
				DocumentTextSerializer serial(*this);
				return serial.Deserialize(data, length);
			}
		}

		void Document::Print(const char *indentToken)
		{
			if(root == nullptr) {
				return;
			}
			std::ostringstream out;
			DocumentTextSerializer serial(*this);
			serial.Serialize(out, indentToken);
			Console::Info(out.str().c_str());
		}






		DocumentTextSerializer::DocumentTextSerializer(Document &doc)
			: doc(doc)
		{
		}

		DocumentTextSerializer::~DocumentTextSerializer()
		{
		}

		bool DocumentTextSerializer::Serialize(char *path, const char *indentToken)
		{
			if(doc.root == nullptr) {
				Console::Error("Could not serialize text document, root was null");
				return false;
			}

			std::ofstream file(path, std::ios_base::out);
			if(!file.good()) {
				file.close();
				Console::Error("Failed to open file for text document serialization");
				return false;
			}

			for(uint32 i = 0; i < doc.root->count; i++) {
				SerializeNode(doc.root->children[i], 0, file, false, false, false, indentToken);
			}

			file.close();
			return true;
		}

		bool DocumentTextSerializer::Serialize(std::ostream &out, const char *indentToken)
		{
			if(doc.root == nullptr) {
				Console::Error("Could not serialize text document, root was null");
				return false;
			}

			for(uint32 i = 0; i < doc.root->count; i++) {
				SerializeNode(doc.root->children[i], 0, out, false, false, false, indentToken);
			}
			return true;
		}

		void DocumentTextSerializer::SerializeNode(Document::Node *n, uint32 depth, std::ostream &out, bool stacking, bool firstInStack, bool lastInStack, const char *indentToken)
		{
			if(stacking) {
				out << " ";
			} else {
				for(uint32 i = 0; i < depth; i++) {
					out << indentToken;
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
			if(n->count > 0 && n->count <= STACK_CHILDREN_COUNT_THRESHOLD) {
				stackChildren = true;
				for(uint32 i = 0; i < n->count; i++) {
					if(n->children[i]->count != 0) {
						stackChildren = false;
						break;
					}
				}
			}

			if(stacking && !lastInStack) {
				out << ",";
			} else {
				if(!stackChildren) {
					out << std::endl;
				}
			}

			for(uint32 i = 0; i < n->count; i++) {
				SerializeNode(n->children[i], depth+1, out, stackChildren, i == 0, i == n->count-1, indentToken);
			}
		}

		bool DocumentTextSerializer::Deserialize(char *data, uint32 length)
		{
			SAFE_DELETE(doc.root);
			doc.root = new Document::Node("<root>", 6, true);
		
			uint32 character = 0;
			uint32 line = 0;
			uint32 indentWidth = 0;

			bool cleanLine = true;
			bool appendSibling = false;
			int32 indentLevel = 0;
			int32 previousIndentLevel = -1;
			Document::Node *previous = doc.root;
		
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
						if(indentLevel > previousIndentLevel+1) {
							if(indentWidth == 0) {
								indentWidth = indentLevel - previousIndentLevel;
								indentLevel = previousIndentLevel+1;
							} else {
								Console::Error("Indentation error at line %d, char %d", line, character);
								return false;
							}
						}
					}
				} else if(*p == ',') {
					p++; character++;
					appendSibling = true;
				} else if(*p == '"') {
					p++; character++;
					char prior = *p;
					char *start = p;
					uint32 length = 0;

					while(*p != 0 && !(*p == '"' && prior != '\\')) {
						prior = *p; p++; character++; length++;
					}
					if(*p != '"') {
						Console::Error("Expected closing quote at line %d, char %d", line, character);
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
					if(!AddNode(new Document::Node(buffer, newLength, false), indentLevel, &previous, previousIndentLevel, cleanLine, appendSibling)) {
						Console::Error("Parse error near line %d, char %d", line, character);
						return false;
					}
				} else {
					char *start = p;
					uint32 length = 0;
					while(*p != 0 && *p != ' ' && *p != '\t' && *p != '\r' && *p != ',') {
						p++; character++; length++;
					}
					if(!AddNode(new Document::Node(start, length, true), indentLevel, &previous, previousIndentLevel, cleanLine, appendSibling)) {
						Console::Error("Parse error near line %d, char %d", line, character);
						return false;
					}
				}
			}
			return true;
		}

		bool DocumentTextSerializer::AddNode(Document::Node *n, int32 indentLevel, Document::Node **previous, int32 &previousIndentLevel, bool &cleanLine, bool &appendSibling)
		{
			if(!cleanLine || indentLevel > previousIndentLevel) {
				if(appendSibling) {
					(*previous)->AppendSibling(n);
				} else {
					(*previous)->AppendChild(n);
					*previous = n;
					previousIndentLevel = indentLevel;
				}
			} else {
				Document::Node *parent = *previous;
				for(int32 i = indentLevel; i <= previousIndentLevel; i++) {
					parent = parent->parent;
					if(parent == nullptr) {
						Console::Error("Parsing document failed, suspect indentation");
						return false;
					}
				}
				if(appendSibling) {
					parent->AppendSibling(n);
				} else {
					parent->AppendChild(n);
					*previous = n;
					previousIndentLevel = indentLevel;
				}
			}
			cleanLine = false;
			appendSibling = false;
			return true;
		}







		const uint8 DocumentBinarySerializer::BINARY_HEADER[4] = {'\0', 'H', 'D', '\0'};

		bool DocumentBinarySerializer::IsBinaryDocument(char *data, uint32 length)
		{
			return data != nullptr && length >= sizeof(BINARY_HEADER) && memcmp(data, BINARY_HEADER, sizeof(BINARY_HEADER)) == 0;
		}

		DocumentBinarySerializer::DocumentBinarySerializer(Document &doc)
			: doc(doc)
		{
		}
	
		DocumentBinarySerializer::~DocumentBinarySerializer()
		{
		}

		bool DocumentBinarySerializer::Serialize(char *path)
		{
			if(doc.root == nullptr) {
				Console::Error("Could not serialize binary document, root was null");
				return false;
			}

			std::ofstream file(path, std::ios_base::out | std::ios_base::binary);
			if(!file.good()) {
				file.close();
				Console::Error("Failed to open file for binary document serialization");
				return false;
			}

			bool success = Serialize(file);
			file.close();
			return success;
		}

		bool DocumentBinarySerializer::Serialize(std::ostream &out)
		{
			if(doc.root == nullptr) {
				Console::Error("Could not serialize binary document, root was null");
				return false;
			}

			std::vector<std::string> table;
			std::ostringstream body;

			// Serialize our document to the stringstream, constructing a stringtable along the way
			for(uint32 i = 0; i < doc.root->count; i++) {
				uint32 level = 0;
				SerializeNode(doc.root->children[i], body, table, level);
			}

			// Write header ident
			uint32 offset = 0;
			out.write((char *)BINARY_HEADER, sizeof(BINARY_HEADER));
			offset += sizeof(BINARY_HEADER);

			// Write string table count
			uint32 stringCount = table.size();
			assert(stringCount < 1<<16);
			uint16 sc = stringCount;
			out.write((char *)&sc, sizeof(sc));
			offset += sizeof(sc);
		
			// Write each string, null terminated
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

			// Write the document
			std::string bodyString = body.str();
			out.write(bodyString.c_str(), bodyString.size());
		
			return true;
		}

		uint16 DocumentBinarySerializer::GetOrAdd(std::vector<std::string> &stringTable, char *str)
		{
			std::vector<std::string>::iterator iter = std::find(stringTable.begin(), stringTable.end(), str);
			if(iter != stringTable.end()) {
				return iter - stringTable.begin();
			}
			stringTable.push_back(str);
			uint32 index = stringTable.size()-1;
			assert(index < 1<<16);
			return (uint16)index;
		}

		void DocumentBinarySerializer::SerializeNode(Document::Node *n, std::ostream &body, std::vector<std::string> &stringTable, uint32 &level)
		{
			// Write indentation depth
			assert(level < 1<<16);
			uint16 l = level;
			body.write((char *)&l, sizeof(l));
		
			// Write node value (index into string table actually)
			uint16 stringIndex = GetOrAdd(stringTable, n->value);
			body.write((char *)&stringIndex, sizeof(stringIndex));

			level++;
			for(uint32 i = 0; i < n->count; i++) {
				SerializeNode(n->children[i], body, stringTable, level);
			}
			level--;
		}

		bool DocumentBinarySerializer::Deserialize(char *data, uint32 length)
		{
			if(!IsBinaryDocument(data, length)) {
				Console::Error("Data did not contain a binary document");
				return false;
			}

			SAFE_DELETE(doc.root);
			doc.root = new Document::Node("<root>", 6, true);

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
			Document::Node *previous = doc.root;

			while(p < data+length) {
				// Read indentation depth
				uint16 level = *(uint16 *)p; p += sizeof(uint16);

				// Read string table index which specifies the node value
				uint16 stringIndex = *(uint16 *)p; p += sizeof(uint16);

				Document::Node *n = new Document::Node(stringTableStart+stringTableOffsets[stringIndex], stringTableOffsets[stringIndex+1]-stringTableOffsets[stringIndex], true);
				if(level > previousLevel) {
					previous->AppendChild(n);
				} else {
					while(level <= previousLevel) {
						previous = previous->parent;
						assert(previous != nullptr);
						previousLevel--;
					}
					previous->AppendChild(n);
				}
				previous = n;
				previousLevel = level;
			}

			SAFE_DELETE_ARRAY(stringTableOffsets);
			return true;
		}

	} // namespace Core

} // namespace Maki