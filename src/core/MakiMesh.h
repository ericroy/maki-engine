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

		public:
			mesh_t(bool dynamic_ = false);
			mesh_t(mesh_t &&other);
			~mesh_t();

			inline rid_t rid() const { return rid_; }
			inline void set_rid(rid_t rid) { rid_ = rid; }

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

			int32_t get_attribute_offset(attribute_t attr);

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

			inline vector<ref_t<mesh_t>> &siblings() {
				return siblings_;
			}

			inline const ref_t<vertex_format_t> &vertex_format() const {
				return vertex_format_;
			}

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

			// Populate object directly using these
			void set_vertex_attributes(uint32_t vertex_attribute_flags);
			void set_index_attributes(uint8_t indices_per_face, uint8_t bytes_per_index);

			// If data is null, simply reserves the requested number of bytes but does not initialize the memory
			void push_vertex_data(char *data, uint32_t size_in_bytes);
			void push_index_data(char *data, uint32_t size_in_bytes);

			// Uploads (or re-uploads) data to gpu buffer
			void upload();

			void calculate_bounds();

			void reset();

		private:
			rid_t rid_ = RID_NONE;
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
			array_t<char> vertex_data_;

			uint32_t index_insertion_index_ = 0;
			array_t<char> index_data_;

			// The data sizes at the time of last upload
			uint32_t old_vertex_data_size_ = -1;
			uint32_t old_index_data_size_ = -1;

			bool dynamic_ = false;
			void *buffer_ = nullptr;
		};

		class mesh_loader_t {
		public:
			static ref_t<mesh_t> load(rid_t rid);

		private:
			static uint32_t load_mesh(char *data, ref_t<mesh_t> &out);
		};

		class mesh_builder_t {
		public:
			static ref_t<mesh_t> make_quad();
		};

	} // namespace core
} // namespace maki
