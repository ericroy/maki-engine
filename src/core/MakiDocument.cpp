#include <algorithm>
#include <fstream>
#include "core/MakiDocument.h"
#include "core/MakiConsole.h"

#ifndef MAKI_TOOLS
#include "core/MakiEngine.h"
#include "core/MakiAssetLibrary.h"
#endif

#include <sstream>

namespace maki {
	namespace core {

		document_t::node_t::node_t(const char *data, uint32_t length) {
			value_ = new char[length+1];
			memcpy(value_, data, length+1);
			value_[length] = 0;
		}

		document_t::node_t::node_t(char *data, uint32_t length, bool should_clone_data) {
			if(should_clone_data) {
				value_ = new char[length+1];
				memcpy(value_, data, length+1);
				value_[length] = 0;
			} else {
				value_ = data;
			}
		}

		document_t::node_t::~node_t() {
			MAKI_SAFE_DELETE_ARRAY(value_);
			for(uint32_t i = 0; i < length_; i++)
				MAKI_SAFE_DELETE(children_[i]);
			MAKI_SAFE_FREE(children_);
		}

		void document_t::node_t::append_child(node_t *n) {
			n->parent_ = this;
			if(length_ == capacity_) {
				capacity_ = std::max<uint32_t>(8, capacity_ * 2);
				children_ = (node_t **)allocator_t::realloc(children_, capacity_ * sizeof(node_t *), std::alignment_of<node_t *>::value);
				MAKI_ASSERT(children_ != nullptr);
			}
			children_[length_++] = n;
		}

		document_t::node_t *document_t::node_t::remove_child(uint32_t index) {
			document_t::node_t *ret = children_[index];
			memmove(&children_[index], &children_[index + 1], (length_ - index - 1)*sizeof(node_t *));
			return ret;
		}

		bool document_t::node_t::remove_child(document_t::node_t *n) {
			for(uint32_t i = 0; i < length_; i++) {
				if(children_[i] == n) {
					remove_child(i);
					return true;
				}
			}
			return false;
		}

		void document_t::node_t::detach() {
			if(parent_ != nullptr)
				parent_->remove_child(this);
		}
	
		int32_t document_t::node_t::value_as_int(int32_t default_value) const {
			int32_t v;
			if(value_as_int(&v))
				return v;
			return default_value;
		}
		bool document_t::node_t::value_as_int(int32_t *out) const {
			char *end;
			int32_t v = strtol(value_, &end, 10);
			if(end == value_)
				return false;
			if(out != nullptr)
				*out = v;
			return true;
		}

		uint32_t document_t::node_t::value_as_uint(uint32_t default_value) const {
			uint32_t v;
			if(value_as_uint(&v))
				return v;
			return default_value;
		}
		bool document_t::node_t::value_as_uint(uint32_t *out) const {
			char *end;
			uint32_t v = strtoul(value_, &end, 10);
			if(end == value_)
				return false;
			if(out != nullptr)
				*out = v;
			return true;
		}

		float document_t::node_t::value_as_float(float default_value) const {
			float v;
			if(value_as_float(&v))
				return v;
			return default_value;
		}
		bool document_t::node_t::value_as_float(float *out) const {
			char *end;
			float v = (float)strtod(value_, &end);
			if(end == value_)
				return false;
			if(out != nullptr)
				*out = v;
			return true;
		}

		bool document_t::node_t::value_as_bool(bool default_value) const {
			bool v;
			if(value_as_bool(&v))
				return v;
			return default_value;
		}
		bool document_t::node_t::value_as_bool(bool *out) const {
			if(out != nullptr)
				*out = value_[0] == 'T' || value_[0] == 't' || value_[0] == '1' || value_[0] == 'Y' || value_[0] == 'y';
			return true;
		}

		const document_t::node_t *document_t::node_t::resolve(const char *node_path) const {
			if(*node_path == 0)
				return this;
		
			const char *p = node_path;

			// Referencing a child by index
			if(*p == '#') {
				p++;
				char *end = nullptr;
				uint32_t i = strtoul(p, &end, 10);
				p = end;
				if(i >= length_)
					return nullptr;
				return children_[i]->resolve(p);
			}

			// Referencing a child by name
			while(*p != 0 && *p != '.')
				p++;

			for(uint32_t i = 0; i < length_; i++) {
				if(strncmp(children_[i]->value_, node_path, p-node_path) == 0) {
					if(*p == '.')
						p++;
					return children_[i]->resolve(p);
				}
			}
			return nullptr;
		}

		const char *document_t::node_t::resolve_value(const char *node_path) const {
			const auto *n = resolve(node_path);
			if (n)
				return n->value();
			return nullptr;
		}

		bool document_t::node_t::resolve_as_int(const char *node_path, int32_t *out) const {
			const auto *n = resolve(node_path);
			if (n)
				return n->value_as_int(out);
			return false;
		}

		int32_t document_t::node_t::resolve_as_int(const char *node_path, int32_t default_value) const {
			const auto *n = resolve(node_path);
			if (n)
				return n->value_as_int(default_value);
			return default_value;
		}

		bool document_t::node_t::resolve_as_uint(const char *node_path, uint32_t *out) const {
			const auto *n = resolve(node_path);
			if (n)
				return n->value_as_uint(out);
			return false;
		}
		
		uint32_t document_t::node_t::resolve_as_uint(const char *node_path, uint32_t default_value) const {
			const auto *n = resolve(node_path);
			if (n)
				return n->value_as_uint(default_value);
			return default_value;
		}

		bool document_t::node_t::resolve_as_float(const char *node_path, float *out) const {
			const auto *n = resolve(node_path);
			if (n)
				return n->value_as_float(out);
			return false;
		}

		float document_t::node_t::resolve_as_float(const char *node_path, float default_value) const {
			const auto *n = resolve(node_path);
			if (n)
				return n->value_as_float(default_value);
			return default_value;
		}

		bool document_t::node_t::resolve_as_bool(const char *node_path, bool *out) const {
			const auto *n = resolve(node_path);
			if (n)
				return n->value_as_bool(out);
			return false;
		}

		bool document_t::node_t::resolve_as_bool(const char *node_path, bool default_value) const {
			const auto *n = resolve(node_path);
			if (n)
				return n->value_as_bool(default_value);
			return default_value;
		}

		bool document_t::node_t::resolve_as_vector_n(const char *node_path, uint64_t count, float *out) const {
			const auto *n = resolve(node_path);
			if(n) {
				MAKI_ASSERT(n->length() >= count);
				for(uint64_t i = 0; i < count; i++) {
					out[i] = n->children_[i]->value_as_float();
				}
				return true;
			}
			return false;
		}









		document_t::document_t() {
			root_ = new node_t("<root_>", 6);
		}

		document_t::~document_t() {
			MAKI_SAFE_DELETE(root_);
		}

	#ifndef MAKI_TOOLS
		bool document_t::load(rid_t rid) {
			auto data = engine_t::get()->assets->alloc_read(rid);
			if(!data) {
				console_t::error("Failed to alloc_read document bytes");
				return false;
			}
			return load(data.data(), data.length());
		}
	#endif

		bool document_t::load(char *data, size_t length) {
			if(document_binary_serializer_t::is_binary_document(data, length)) {
				document_binary_serializer_t serial(*this);
				return serial.deserialize(data, length);
			} else {
				document_text_serializer_t serial(*this);
				return serial.deserialize(data, length);
			}
		}

		void document_t::print(const char *indent_token) {
			if(root_ == nullptr)
				return;
			std::ostringstream out;
			document_text_serializer_t serial(*this);
			serial.serialize(out, indent_token);
			console_t::info(out.str().c_str());
		}






		document_text_serializer_t::document_text_serializer_t(document_t &doc_) : doc_(doc_) {
		}

		bool document_text_serializer_t::serialize(char *path, const char *indent_token) {
			if(doc_.root_ == nullptr) {
				console_t::error("Could not serialize text document, root_ was null");
				return false;
			}

			std::ofstream file(path, std::ios_base::out);
			if(!file.good()) {
				file.close();
				console_t::error("Failed to open file for text document serialization");
				return false;
			}

			for(uint32_t i = 0; i < doc_.root().length(); i++) {
				serialize_node(&doc_.root()[i], 0, file, false, false, false, indent_token);
			}

			file.close();
			return true;
		}

		bool document_text_serializer_t::serialize(std::ostream &out, const char *indent_token) {
			if(doc_.root_ == nullptr) {
				console_t::error("Could not serialize text document, root_ was null");
				return false;
			}

			for(uint32_t i = 0; i < doc_.root().length(); i++) {
				serialize_node(&doc_.root()[i], 0, out, false, false, false, indent_token);
			}
			return true;
		}

		void document_text_serializer_t::serialize_node(document_t::node_t *n, uint32_t depth, std::ostream &out, bool stacking, bool first_in_stack, bool last_in_stack, const char *indent_token) {
			if(stacking) {
				out << " ";
			} else {
				for(uint32_t i = 0; i < depth; i++) {
					out << indent_token;
				}
			}
		
			std::string s = n->value_;
		
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
			bool stack_children = false;
			if(n->length() > 0 && n->length() <= stack_children_count_threshold_) {
				stack_children = true;
				for(uint32_t i = 0; i < n->length(); i++) {
					if(n->children_[i]->length() != 0) {
						stack_children = false;
						break;
					}
				}
			}

			if(stacking && !last_in_stack) {
				out << ",";
			} else {
				if(!stack_children) {
					out << std::endl;
				}
			}

			for(uint32_t i = 0; i < n->length(); i++) {
				serialize_node(n->children_[i], depth+1, out, stack_children, i == 0, i == n->length()-1, indent_token);
			}
		}

		bool document_text_serializer_t::deserialize(char *data, size_t length) {
			MAKI_SAFE_DELETE(doc_.root_);
			doc_.root_ = new document_t::node_t("<root_>", 6);
		
			uint32_t character = 0;
			uint32_t line = 0;
			uint32_t indent_width = 0;

			bool clean_line = true;
			bool append_sibling = false;
			int32_t indent_level = 0;
			int32_t previous_indent_level = -1;
			document_t::node_t *previous = doc_.root_;
		
			char *p = data;

			while(*p != 0) {
				if(*p == '#') {
					do { p++; character++; } while(*p != 0 && *p != '\r' && *p != '\n');
				} else if(*p == ' ' || *p == '\t') {
					do { p++; character++; } while(*p == ' ' || *p == '\t');
					indent_level++;
				} else if(*p == '\n' || *p == '\r') {
					clean_line = true;
					do { p++; line++; character = 0; indent_level = 0; } while(*p == '\n' || *p == '\r');
					if(*p == ' ' || *p == '\t') {
						uint32_t length_ = 0;
						do { p++; character++; length_++; } while(*p == ' ' || *p == '\t');

						indent_level += indent_width != 0 ? length_/indent_width : length_;
						if(indent_level > previous_indent_level+1) {
							if(indent_width == 0) {
								indent_width = indent_level - previous_indent_level;
								indent_level = previous_indent_level+1;
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
					uint32_t length = 0;

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
					uint32_t new_length = 0;
					for(uint32_t i = 0; i < length; i++) {
						if(i > 0 && *q == '"' && *(q-1) == '\\') {
							// Back-stomp on the escape character
							new_length--;
						}
						buffer[new_length++] = *q++;
					}
					buffer[new_length] = 0;

					// We give ownership of the buffer we allocated to the node
					if(!add_node(new document_t::node_t(buffer, new_length, false), indent_level, &previous, previous_indent_level, clean_line, append_sibling)) {
						console_t::error("Parse error near line %d, char %d", line, character);
						return false;
					}
				} else {
					char *start = p;
					uint32_t length = 0;
					while(*p != 0 && *p != ' ' && *p != '\t' && *p != '\r' && *p != ',') {
						p++; character++; length++;
					}
					if(!add_node(new document_t::node_t(start, length, true), indent_level, &previous, previous_indent_level, clean_line, append_sibling)) {
						console_t::error("Parse error near line %d, char %d", line, character);
						return false;
					}
				}
			}
			return true;
		}

		bool document_text_serializer_t::add_node(document_t::node_t *n, int32_t indent_level, document_t::node_t **previous, int32_t &previous_indent_level, bool &clean_line, bool &append_sibling) {
			if(!clean_line || indent_level > previous_indent_level) {
				if(append_sibling) {
					(*previous)->append_sibling(n);
				} else {
					(*previous)->append_child(n);
					*previous = n;
					previous_indent_level = indent_level;
				}
			} else {
				document_t::node_t *parent_ = *previous;
				for(int32_t i = indent_level; i <= previous_indent_level; i++) {
					parent_ = parent_->parent_;
					if(parent_ == nullptr) {
						console_t::error("Parsing document failed, suspect indentation");
						return false;
					}
				}
				if(append_sibling) {
					parent_->append_sibling(n);
				} else {
					parent_->append_child(n);
					*previous = n;
					previous_indent_level = indent_level;
				}
			}
			clean_line = false;
			append_sibling = false;
			return true;
		}







		const uint8_t document_binary_serializer_t::BINARY_HEADER[4] = {'\0', 'H', 'D', '\0'};

		bool document_binary_serializer_t::is_binary_document(char *data, size_t length) {
			return data != nullptr && length >= sizeof(BINARY_HEADER) && memcmp(data, BINARY_HEADER, sizeof(BINARY_HEADER)) == 0;
		}

		document_binary_serializer_t::document_binary_serializer_t(document_t &doc_) : doc_(doc_) {
		}

		bool document_binary_serializer_t::serialize(char *path) {
			if(doc_.root_ == nullptr) {
				console_t::error("Could not serialize binary document, root_ was null");
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

		bool document_binary_serializer_t::serialize(std::ostream &out) {
			if(doc_.root_ == nullptr) {
				console_t::error("Could not serialize binary document, root_ was null");
				return false;
			}

			std::vector<std::string> table;
			std::ostringstream body;

			// serialize our document to the stringstream, constructing a stringtable along the way
			for(uint32_t i = 0; i < doc_.root().length(); i++) {
				uint32_t level = 0;
				serialize_node(&doc_.root()[i], body, table, level);
			}

			// write header ident
			uint32_t offset = 0;
			out.write((char *)BINARY_HEADER, sizeof(BINARY_HEADER));
			offset += sizeof(BINARY_HEADER);

			// write string table length_
			uint32_t string_count = (uint32_t)table.size();
			MAKI_ASSERT(string_count < 1<<16);
			uint16_t sc = string_count;
			out.write((char *)&sc, sizeof(sc));
			offset += sizeof(sc);
		
			// write each string, null terminated
			for(uint32_t i = 0; i < string_count; i++) {
				const std::string &s = table[i];
				uint32_t bytes = (uint32_t)(s.size() + 1);
				out.write(s.c_str(), bytes);
				offset += bytes;
			}

			// Align
			uint8_t pad = 0xff;
			while(offset % 2 != 0) {
				out.write((char *)&pad, 1);
				offset++;
			}		

			// write the document
			std::string body_string = body.str();
			out.write(body_string.c_str(), body_string.size());
		
			return true;
		}

		uint16_t document_binary_serializer_t::get_or_add(std::vector<std::string> &string_table, char *str) {
			std::vector<std::string>::iterator iter = std::find(std::begin(string_table), std::end(string_table), str);
			if(iter != std::end(string_table))
				return (uint16_t)(iter - std::begin(string_table));
			string_table.push_back(str);
			uint32_t index = (uint32_t)(string_table.size() - 1);
			MAKI_ASSERT(index < 1 << 16);
			return (uint16_t)index;
		}

		void document_binary_serializer_t::serialize_node(document_t::node_t *n, std::ostream &body, std::vector<std::string> &string_table, uint32_t &level) {
			// write indentation depth
			MAKI_ASSERT(level < 1<<16);
			uint16_t l = level;
			body.write((char *)&l, sizeof(l));
		
			// write node value_ (index into string table actually)
			uint16_t string_index = get_or_add(string_table, n->value_);
			body.write((char *)&string_index, sizeof(string_index));

			level++;
			for(uint32_t i = 0; i < n->length(); i++)
				serialize_node(n->children_[i], body, string_table, level);
			level--;
		}

		bool document_binary_serializer_t::deserialize(char *data, size_t length) {
			if(!is_binary_document(data, length)) {
				console_t::error("Data did not contain a binary document");
				return false;
			}

			MAKI_SAFE_DELETE(doc_.root_);
			doc_.root_ = new document_t::node_t("<root_>", 6);

			char *p = data; p += sizeof(BINARY_HEADER);

			// Parse string table, finding the offsets to the start of each string
			uint16_t string_table_count = *(uint16_t *)p; p += sizeof(uint16_t);
			uint16_t *string_table_offsets = new uint16_t[string_table_count+1];
			char *string_table_start = p;
			for(uint16_t i = 0; i < string_table_count; i++) {
				string_table_offsets[i] = (uint16_t)(p - string_table_start);
				while(*p != 0)
					p++;
				p++;
			}
			string_table_offsets[string_table_count] = (uint16_t)(p - string_table_start);

			// Align
			while((p-data) % 2 != 0)
				p++;

			int32_t previous_level = -1;
			document_t::node_t *previous = doc_.root_;

			while(p < data+length) {
				// Read indentation depth
				uint16_t level = *(uint16_t *)p; p += sizeof(uint16_t);

				// Read string table index which specifies the node value_
				uint16_t string_index = *(uint16_t *)p; p += sizeof(uint16_t);

				document_t::node_t *n = new document_t::node_t(string_table_start+string_table_offsets[string_index], string_table_offsets[string_index+1]-string_table_offsets[string_index], true);
				if(level > previous_level) {
					previous->append_child(n);
				} else {
					while(level <= previous_level) {
						previous = previous->parent_;
						MAKI_ASSERT(previous != nullptr);
						previous_level--;
					}
					previous->append_child(n);
				}
				previous = n;
				previous_level = level;
			}

			MAKI_SAFE_DELETE_ARRAY(string_table_offsets);
			return true;
		}

	} // namespace core
} // namespace maki
