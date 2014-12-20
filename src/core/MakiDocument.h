#pragma once
#include "core/core_stdafx.h"
#include <cstdlib>

namespace maki
{
	namespace core
	{

		class document_t
		{
			friend class document_text_serializer_t;
			friend class document_binary_serializer_t;

		public:
			class node_t
			{
				friend class document_t;
				friend class document_text_serializer_t;
				friend class document_binary_serializer_t;
		
			public:
				node_t(const char *data, uint32 length);
				node_t(char *data, uint32 length, bool should_clone_data);
				~node_t();

				void append_child(node_t *n);
				inline void append_sibling(node_t *n) { assert(parent_ != nullptr); parent_->append_child(n); }
				node_t *remove_child(uint32 index);
				bool remove_child(node_t *n);
				void detach();

				inline bool value_equals(const char *v) const { return strcmp(v, value_) == 0; }
			
				int32 value_as_int(int32 default_value = 0) const;
				bool value_as_int(int32 *out) const;

				uint32 value_as_uint(uint32 default_value = 0) const;
				bool value_as_uint(uint32 *out) const;

				float value_as_float(float default_value = 0.0f) const;
				bool value_as_float(float *out) const;

				bool value_as_bool(bool default_value = false) const;
				bool value_as_bool(bool *out) const;
			
				// Path resolution helpers:
				node_t *resolve(const char *node_path);
				inline char *resolve_value(const char *node_path) { document_t::node_t *n = resolve(node_path); if(n) { return n->value_; } return nullptr; }
				inline bool resolve_as_int(const char *node_path, int32 *out) { document_t::node_t *n = resolve(node_path); if(n) { return n->value_as_int(out); } return false; }
				inline int32 resolve_as_int(const char *node_path, int32 default_value = 0) { document_t::node_t *n = resolve(node_path); if(n) { return n->value_as_int(default_value); } return default_value; }
				inline bool resolve_as_uint(const char *node_path, uint32 *out) { document_t::node_t *n = resolve(node_path); if(n) { return n->value_as_uint(out); } return false; }
				inline uint32 resolve_as_uint(const char *node_path, uint32 default_value = 0) { document_t::node_t *n = resolve(node_path); if(n) { return n->value_as_uint(default_value); } return default_value; }
				inline bool resolve_as_float(const char *node_path, float *out) { document_t::node_t *n = resolve(node_path); if(n) { return n->value_as_float(out); } return false; }
				inline float resolve_as_float(const char *node_path, float default_value = 0.0f) { document_t::node_t *n = resolve(node_path); if(n) { return n->value_as_float(default_value); } return default_value; }
				inline bool resolve_as_bool(const char *node_path, bool *out) { document_t::node_t *n = resolve(node_path); if(n) { return n->value_as_bool(out); } return false; }
				inline bool resolve_as_bool(const char *node_path, bool default_value = false) { document_t::node_t *n = resolve(node_path); if(n) { return n->value_as_bool(default_value); } return default_value; }
			
				inline bool resolve_as_vector_n(const char *node_path, uint32 count, float *out) {
					document_t::node_t *n = resolve(node_path);
					if(n) {
						assert(n->count_ >= count);
						for(uint32 i = 0; i < count; i++) {
							out[i] = n->children_[i]->value_as_float();
						}
						return true;
					}
					return false;
				}

			public:
				node_t *parent_;
				char *value_;
				uint32 count_;
				uint32 capacity_;
				node_t **children_;
			};
	
	
		public:
			document_t();
			virtual ~document_t();
		
			void print(const char *indent_token = "  ");
			bool load(char *data, uint32 length);
	#ifndef MAKI_TOOLS
			bool load(rid_t rid);
	#endif

		public:
			node_t *root_;
		};


		class document_text_serializer_t
		{
		private:
			static const int32 stack_children_count_threshold_ = 4;

		public:
			document_text_serializer_t(document_t &doc);
			~document_text_serializer_t();

			bool serialize(char *path, const char *indent_token = "\t");
			bool serialize(std::ostream &out, const char *indent_token = "\t");
			bool deserialize(char *data, uint32 length);

		private:
			bool add_node(document_t::node_t *n, int32 indent_level, document_t::node_t **previous, int32 &previous_indent_level, bool &clean_line, bool &append_sibling);
			void serialize_node(document_t::node_t *n, uint32 depth, std::ostream &out, bool stacking, bool first_in_stack, bool last_in_stack, const char *indent_token);

		private:
			document_t &doc_;
		};


		class document_binary_serializer_t
		{
		private:
			static const uint8 BINARY_HEADER[4];

		public:
			static bool IsBinaryDocument(char *data, uint32 length);

		public:
			document_binary_serializer_t(document_t &doc);
			~document_binary_serializer_t();

			bool serialize(char *path);
			bool serialize(std::ostream &out);
			bool deserialize(char *data, uint32 length);

		private:
			uint16 get_or_add(std::vector<std::string> &string_table, char *str);
			void serialize_node(document_t::node_t *n, std::ostream &body, std::vector<std::string> &string_table, uint32 &level);

		private:
			document_t &doc_;
		};


	} // namespace core

} // namespace maki
