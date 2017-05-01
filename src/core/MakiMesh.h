#pragma once
#include <vector>
#include "core/MakiResource.h"
#include "core/MakiBoundingBox.h"
#include "core/MakiMaterialManager.h"
#include "core/MakiVertexFormat.h"

namespace maki {
	namespace core {

		

		class mesh_t : public resource_t
		{
		public:
			enum mesh_flag_t
			{
				 mesh_flag_has_translucency_ = 1 << 0,
			};

			enum object_t
			{
				object_rect_ = 0,
				object_count_,
			};

			struct object_args_t {};
			struct rect_args_t : public object_args_t
			{
				vector4_t facing_axis_;
				float left_;
				float right_;
				float top_;
				float bottom_;
			};

		public:
			mesh_t(bool dynamic_ = false);
			mesh_t(object_t type, const object_args_t &args);
			mesh_t(mesh_t &&other);
			mesh_t(const mesh_t &) { assert(false && "mesh_t copy construction not allowed"); }
			~mesh_t();

			// Initialize object from mesh file
			bool load(rid_t rid, bool upload = true);
		
			// Populate object directly using these
			void set_vertex_attributes(uint32_t vertex_attribute_flags_);
			void set_index_attributes(uint8_t indices_per_face_, uint8_t bytes_per_index_);
			
			// If data is null, simply reserves the requested number of bytes but does not initialize the memory
			void push_vertex_data(uint32_t size_in_bytes, char *data);
			void push_index_data(uint32_t size_in_bytes, char *data);
			
			void clear_data();

			inline uint8_t get_mesh_flag(mesh_flag_t flag) const { return (mesh_flags_ & flag) != 0; }
			inline void set_mesh_flag(mesh_flag_t flag, bool on = true) { if(on) { mesh_flags_ |= flag; } else { mesh_flags_ &= ~flag; } }

			// Retrieves the gpu buffer_ associated with this mesh (creating it if necessary)
			inline void *get_buffer() { if(buffer_ == nullptr) { upload(); } return buffer_; }

			inline uint8_t get_vertex_stride() const { return vertex_stride_; }
			inline uint32_t get_vertex_count() const { return vertex_count_; }
			//inline void set_vertex_count(uint32_t count) { vertex_count_ = count; }
			inline char *get_vertex_data() { return vertex_data_; }
			int32_t get_attribute_offset(vertex_format_t::attribute_t attr);

			inline uint8_t get_bytes_per_index() const { return bytes_per_index_; }
			inline uint8_t get_indices_per_face() const { return indices_per_face_; }
			inline uint32_t get_face_count() const { return face_count_; }
			inline char *get_index_data() { return index_data_; }

			// Uploads (or re-uploads) data to gpu buffer
			void upload();

			void calculate_bounds();

		private:
			uint32_t load_mesh_data(char *data, bool upload);
			void make_rect(const rect_args_t &args);

		public:
			std::vector<handle_t> siblings;
			handle_t vertex_format;
			bounding_box_t bounds;

		private:
			uint8_t mesh_flags_;
			uint8_t vertex_attribute_flags_;
			uint8_t vertex_stride_;
			uint8_t indices_per_face_;
			uint8_t bytes_per_index_;
			uint32_t vertex_count_;
			uint32_t face_count_;

			uint32_t vertex_insertion_index_;
			uint32_t vertex_data_size_;
			char *vertex_data_;

			uint32_t index_insertion_index_;
			uint32_t index_data_size_;
			char *index_data_;

			// The data sizes at the time of last upload
			uint32_t old_vertex_data_size_;
			uint32_t old_index_data_size_;

			bool dynamic_;
			void *buffer_;
		};

	} // namespace core
} // namespace maki
