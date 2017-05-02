#pragma once
#include "core/MakiTypes.h"
#include "core/MakiVector4.h"
#include "core/MakiBoundingBox.h"
#include "core/MakiMaterialManager.h"
#include "core/MakiVertexFormat.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		class mesh_t {
			MAKI_NO_COPY(mesh_t);
		public:
			enum mesh_flag_t {
				 mesh_flag_has_translucency = 1 << 0,
			};

			enum object_t {
				object_rect = 0,
				object_max = object_rect,
			};

			static void build_quad(mesh_t &m);

		public:
			mesh_t(bool dynamic_ = false);
			mesh_t(mesh_t &&other);
			~mesh_t();

			// Populate object directly using these
			void set_vertex_attributes(uint32_t vertex_attribute_flags);
			void set_index_attributes(uint8_t indices_per_face, uint8_t bytes_per_index);
			
			// If data is null, simply reserves the requested number of bytes but does not initialize the memory
			void push_vertex_data(uint32_t size_in_bytes, char *data);
			void push_index_data(uint32_t size_in_bytes, char *data);
			
			void reset(bool keep_buffers = true);

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

			inline const vector<ref_t<mesh_t>> &siblings() const {
				return siblings_;
			}

			inline const ref_t<vertex_format_t> &vertex_format() const {
				return vertex_format_;
			}

			//inline void set_vertex_count(uint32_t count) { vertex_count_ = count; }
			inline char *vertex_data() {
				return vertex_data_.data();
			}

			inline char *index_data() {
				return index_data_.data();
			}

			// Retrieves the gpu buffer associated with this mesh (creating it if necessary)
			inline void *buffer() {
				if (buffer_ == nullptr)
					upload();
				return buffer_;
			}

			// Uploads (or re-uploads) data to gpu buffer
			void upload();

			void calculate_bounds();

		public:
			rid_t rid = RID_NONE;

		private:
			vector<ref_t<mesh_t>> siblings_;
			ref_t<vertex_format_t> vertex_format_;
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
			array_t<char> vertex_data_;

			uint32_t index_insertion_index_ = 0;
			uint32_t index_data_size_ = 0;
			array_t<char> index_data_;

			// The data sizes at the time of last upload
			uint32_t old_vertex_data_size_ = -1;
			uint32_t old_index_data_size_ = -1;

			bool dynamic_ = false;
			void *buffer_ = nullptr;
		};

		class mesh_loader_t {
		public:
			// Initialize object from mesh file
			static bool load(ref_t<mesh_t> &out, rid_t rid, bool upload = true);

		private:
			static uint32_t load_data(char *data, bool upload);
		};

		class mesh_builder_t {
		public:
			static void make_quad(ref_t<mesh_t> &out, bool upload = true);
		};

	} // namespace core
} // namespace maki
