#pragma once
#include <vector>
#include "core/MakiTypes.h"
#include "core/MakiMacros.h"
#include "core/MakiResource.h"
#include "core/MakiBoundingBox.h"
#include "core/MakiMaterialManager.h"
#include "core/MakiVertexFormat.h"

namespace maki {
	namespace core {

		class mesh_t : public resource_t {
			MAKI_NO_COPY(mesh_t);

		public:
			enum mesh_flag_t {
				 mesh_flag_has_translucency = 1 << 0,
			};

			enum object_t {
				object_rect = 0,
				object_count,
			};

			struct object_args_t {
			};

			struct rect_args_t : public object_args_t {
				vector4_t facing_axis;
				float left;
				float right;
				float top;
				float bottom;
			};

		public:
			mesh_t(bool dynamic_ = false);
			mesh_t(object_t type, const object_args_t &args);
			mesh_t(mesh_t &&other);
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

			inline uint8_t has_mesh_flag(mesh_flag_t flag) const {
				return (mesh_flags_ & flag) != 0;
			}

			inline void set_mesh_flag(mesh_flag_t flag, bool on = true) {
				if(on)
					mesh_flags_ |= flag;
				else
					mesh_flags_ &= ~flag;
			}

			inline uint8_t vertex_stride() const {
				return vertex_stride_;
			}

			inline uint32_t vertex_count() const {
				return vertex_count_;
			}

			int32_t attribute_offset(vertex_format_t::attribute_t attr);

			inline uint8_t bytes_per_index() const {
				return bytes_per_index_;
			}
			
			inline uint8_t indices_per_face() const {
				return indices_per_face_;
			}

			inline uint32_t face_count() const {
				return face_count_;
			}

			inline const bounding_box_t &bounds() const {
				return bounds_;
			}

			inline const ::std::vector<handle_t> &siblings() const {
				return siblings_;
			}

			inline handle_t vertex_format() const {
				return vertex_format_;
			}

			//inline void set_vertex_count(uint32_t count) { vertex_count_ = count; }
			inline char *vertex_data() {
				return vertex_data_;
			}

			inline char *index_data() {
				return index_data_;
			}

			// Retrieves the gpu buffer_ associated with this mesh (creating it if necessary)
			inline void *buffer() {
				if (buffer_ == nullptr)
					upload();
				return buffer_;
			}

			// Uploads (or re-uploads) data to gpu buffer
			void upload();

			void calculate_bounds();

		private:
			uint32_t load_mesh_data(char *data, bool upload);
			void make_rect(const rect_args_t &args);

		private:
			::std::vector<handle_t> siblings_;
			handle_t vertex_format_ = HANDLE_NONE;
			bounding_box_t bounds_;

			uint8_t mesh_flags_ = 0;
			uint8_t vertex_attribute_flags_ = 0;
			uint8_t vertex_stride_ = 0;
			uint8_t indices_per_face_ = 0;
			uint8_t bytes_per_index_ = 0;
			uint32_t vertex_count_ = 0;
			uint32_t face_count_ = 0;

			uint32_t vertex_insertion_index_ = 0;
			uint32_t vertex_data_size_ = 0;
			char *vertex_data_ = nullptr;

			uint32_t index_insertion_index_ = 0;
			uint32_t index_data_size_ = 0;
			char *index_data_ = nullptr;

			// The data sizes at the time of last upload
			uint32_t old_vertex_data_size_ = -1;
			uint32_t old_index_data_size_ = -1;

			bool dynamic_ = false;
			void *buffer_ = nullptr;
		};

	} // namespace core
} // namespace maki
