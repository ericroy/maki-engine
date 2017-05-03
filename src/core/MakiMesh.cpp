#include "core/MakiMesh.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiRenderer.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiMeshManager.h"
#include "core/MakiConsole.h"

using ::std::swap;
using ::std::max;

namespace maki {
	namespace core {

		mesh_t::mesh_t(bool dynamic) : dynamic_(dynamic) {
		}

		mesh_t::mesh_t(mesh_t &&other) {
			siblings_ = move(other.siblings_);
			vertex_format_ = move(other.vertex_format_);
			vertex_data_ = move(other.vertex_data_);
			index_data_ = move(other.index_data_);

			swap(vertex_count_, other.vertex_count_);
			swap(face_count_, other.face_count_);
			swap(mesh_flags_, other.mesh_flags_);
			swap(vertex_attribute_flags_, other.vertex_attribute_flags_);
			swap(vertex_stride_, other.vertex_stride_);
			swap(indices_per_face_, other.indices_per_face_);
			swap(bytes_per_index_, other.bytes_per_index_);
			swap(buffer_, other.buffer_);
			swap(vertex_insertion_index_, other.vertex_insertion_index_);
			swap(index_insertion_index_, other.index_insertion_index_);
			swap(dynamic_, other.dynamic_);
			swap(old_vertex_data_size_, other.old_vertex_data_size_);
			swap(old_index_data_size_, other.old_index_data_size_);
			swap(bounds_, other.bounds_);
		}

		mesh_t::~mesh_t() {
			if(buffer_ != nullptr)
				engine_t::get()->renderer->free_buffer(buffer_);
		}

		void mesh_t::reset() {
			mesh_flags_ = 0;
			vertex_attribute_flags_ = 0;
			vertex_stride_ = 0;
			vertex_count_ = 0;
			face_count_ = 0;
			bytes_per_index_ = 0;
			vertex_insertion_index_ = 0;
			index_insertion_index_ = 0;
			siblings_.clear();
			vertex_format_.release();
			vertex_data_.free();
			index_data_.free();
		}

		void mesh_t::push_vertex_data(char *data, uint32_t size_in_bytes) {
			MAKI_ASSERT(size_in_bytes % vertex_stride_ == 0);

			if(vertex_insertion_index_ + size_in_bytes > vertex_data_.length()) {
				// Enlarge by some margin
				size_t overflow = vertex_insertion_index_ + size_in_bytes - vertex_data_.length();
				size_t more = max(max<size_t>(overflow, 256u), vertex_data_.length() / 2);
				vertex_data_.set_length(vertex_data_.length() + more);
			}
			MAKI_ASSERT(vertex_insertion_index_ + size_in_bytes <= vertex_data_.length());

			if(data)
				memcpy(&vertex_data_.data()[vertex_insertion_index_], data, size_in_bytes);

			vertex_insertion_index_ += size_in_bytes;
			vertex_count_ += size_in_bytes / vertex_stride_;
		}

		void mesh_t::push_index_data(char *data, uint32_t size_in_bytes) {
			MAKI_ASSERT(size_in_bytes % (bytes_per_index_ * indices_per_face_) == 0);

			if(index_insertion_index_ + size_in_bytes > index_data_.length()) {
				// Enlarge by some margin
				size_t overflow = index_insertion_index_ + size_in_bytes - index_data_.length();
				size_t more = max(max<size_t>(overflow, 256u), index_data_.length() / 2);
				index_data_.set_length(index_data_.length() + more);
			}
			MAKI_ASSERT(index_insertion_index_ + size_in_bytes <= index_data_.length());

			if(data != nullptr)
				memcpy(&index_data_.data()[index_insertion_index_], data, size_in_bytes);

			index_insertion_index_ += size_in_bytes;
			face_count_ += size_in_bytes / (indices_per_face_ * bytes_per_index_);
		}

		void mesh_t::set_vertex_attributes(uint32_t vertex_attribute_flags) {
			vertex_attribute_flags_ = vertex_attribute_flags;
			vertex_stride_ = 3 * sizeof(float);
			if((vertex_attribute_flags_ & attribute_flag_normal) != 0)
				vertex_stride_ += 3 * sizeof(float);
			if((vertex_attribute_flags_ & attribute_flag_tangent) != 0)
				vertex_stride_ += 3 * sizeof(float);
			if((vertex_attribute_flags_ & attribute_flag_color) != 0)
				vertex_stride_ += 4 * sizeof(uint8_t);
			if((vertex_attribute_flags_ & attribute_flag_color1) != 0)
				vertex_stride_ += 4 * sizeof(uint8_t);
			if((vertex_attribute_flags_ & attribute_flag_tex_coord) != 0)
				vertex_stride_ += 2 * sizeof(float);
			if((vertex_attribute_flags_ & attribute_flag_bone_weight) != 0)
				vertex_stride_ += 4 * sizeof(uint32_t);
		}

		void mesh_t::set_index_attributes(uint8_t indices_per_face, uint8_t bytes_per_index) {
			indices_per_face_ = indices_per_face;
			bytes_per_index_ = bytes_per_index;
		}

		int32_t mesh_t::get_attribute_offset(attribute_t attr) {
			uint32_t offset = 0;
			if(attr == attribute_position)
				return offset;
			offset += sizeof(float) * 3;
		
			if((vertex_attribute_flags_ & attribute_flag_normal) != 0) {
				if(attr == attribute_normal)
					return offset;
				offset += sizeof(float) * 3;
			}

			if((vertex_attribute_flags_ & attribute_flag_tangent) != 0) {
				if(attr == attribute_tangent)
					return offset;
				offset += sizeof(float) * 3;
			}

			if((vertex_attribute_flags_ & attribute_flag_color) != 0) {
				if(attr == attribute_color)
					return offset;
				offset += sizeof(uint8_t) * 4;
			}

			if((vertex_attribute_flags_ & attribute_flag_color1) != 0) {
				if(attr == attribute_color1)
					return offset;
				offset += sizeof(uint8_t) * 4;
			}

			if((vertex_attribute_flags_ & attribute_flag_tex_coord) != 0) {
				if(attr == attribute_tex_coord)
					return offset;
				offset += sizeof(float) * 2;
			}

			if((vertex_attribute_flags_ & attribute_flag_bone_weight) != 0) {
				if(attr == attribute_bone_weight)
					return offset;
				offset += sizeof(uint32_t) * 4;
			}

			return -1;
		}
	
		void mesh_t::upload() {		
			vertex_format_t vf;
			vf.push_attribute(attribute_position, attribute_type_float, 3);
			if((vertex_attribute_flags_ & attribute_flag_normal) != 0)
				vf.push_attribute(attribute_normal, attribute_type_float, 3);
			if((vertex_attribute_flags_ & attribute_flag_tangent) != 0)
				vf.push_attribute(attribute_tangent, attribute_type_float, 3);
			if((vertex_attribute_flags_ & attribute_flag_color) != 0)
				vf.push_attribute(attribute_color, attribute_type_unsigned_int8, 4);
			if((vertex_attribute_flags_ & attribute_flag_color1) != 0)
				vf.push_attribute(attribute_color1, attribute_type_unsigned_int8, 4);
			if((vertex_attribute_flags_ & attribute_flag_tex_coord) != 0)
				vf.push_attribute(attribute_tex_coord, attribute_type_float, 2);
			if((vertex_attribute_flags_ & attribute_flag_bone_weight) != 0)
				vf.push_attribute(attribute_bone_weight, attribute_type_unsigned_int32, 4);

			bool length_changed = old_vertex_data_size_ != vertex_data_.length() || old_index_data_size_ != index_data_.length();

			buffer_ = engine_t::get()->renderer->upload_buffer(buffer_, &vf, vertex_data_.data(), vertex_count_, index_data_.data(), face_count_, indices_per_face_, bytes_per_index_, dynamic_, length_changed);

			// get or create vertex format
			vertex_format_ = move(core_managers_t::get()->vertex_format_manager->get_or_add(vf));

			// Record the data sizes so if upload is called again later, we can see if the buffers have changed length
			old_vertex_data_size_ = (uint32_t)vertex_data_.length();
			old_index_data_size_ = (uint32_t)index_data_.length();
		}

		void mesh_t::calculate_bounds() {
			bounds_.reset();

			if(vertex_data_) {
				char *p = vertex_data_.data();
				for(uint32_t i = 0; i < vertex_count_; i++) {
					vector3_t *v = (vector3_t *)p;
					bounds_.merge(*v);
					p += vertex_stride_;
				}
			}

			for (auto &m : siblings_) {
				m->calculate_bounds();
				bounds_.merge(m->bounds());
			}
		}









		ref_t<mesh_t> mesh_loader_t::load(rid_t rid) {
			auto buffer = engine_t::get()->assets->alloc_read(rid);
			if (!buffer)
				return nullptr;

			char *data = buffer.data();

			if (strncmp(data, "maki", 4) != 0) {
				console_t::error("Invalid binary file type identifier <rid %u>", rid);
				return nullptr;
			}
			data += sizeof(uint8_t) * 8;

			const uint32_t mesh_count = *(uint32_t *)data;
			data += sizeof(uint32_t);

			auto *res = core_managers_t::get();
			auto mesh = res->mesh_manager->create();
			data += load_mesh(data, mesh);
			for (uint32_t i = 1; i < mesh_count; i++) {
				auto next_mesh = res->mesh_manager->create();
				data += load_mesh(data, next_mesh);
				mesh->siblings().push_back(move(next_mesh));
			}

			mesh->calculate_bounds();
			mesh->set_rid(rid);
			return mesh;
		}

		uint32_t mesh_loader_t::load_mesh(char *data, ref_t<mesh_t> &out) {
			char *start = data;

			// Read mesh properties
			auto vertex_count = *(uint32_t *)data;
			data += sizeof(uint32_t);

			auto face_count = *(uint32_t *)data;
			data += sizeof(uint32_t);
			
			out->set_vertex_attributes(*(uint8_t *)data);
			data += sizeof(uint8_t);

			auto indices_per_face = *(uint8_t *)data;
			data += sizeof(uint8_t);

			auto bytes_per_index = *(uint8_t *)data;
			data += sizeof(uint8_t);

			out->set_index_attributes(indices_per_face, bytes_per_index);

			// Pad byte
			data += sizeof(uint8_t);

			// Add the vertex data
			out->push_vertex_data(data, out->vertex_stride() * vertex_count);
			data += out->vertex_stride() * vertex_count;

			// Align to the nearest word boundary
			if (((uintptr_t)data & 0x3) != 0)
				data += 0x4 - ((uintptr_t)data & 0x3);

			// Add the index data
			out->push_index_data(data, bytes_per_index * indices_per_face * face_count);
			data += bytes_per_index * indices_per_face * face_count;

			// Align to the nearest word boundary
			if (((uintptr_t)data & 0x3) != 0)
				data += 0x4 - ((uintptr_t)data & 0x3);

			// Build gpu buffers from the vertex and index data
			out->upload();

			// Return how much we have advanced the pointer
			return (uint32_t)(data - start);
		}

		ref_t<mesh_t> mesh_builder_t::make_quad() {
			struct vertex_t {
				float pos[3];
				uint8_t col[4];
				float uv[2];
			};
			vertex_t v[4] = {
				{ 0, 0, 0, 255, 255, 255, 255, 0, 1 },
				{ 0, 1, 0, 255, 255, 255, 255, 0, 0 },
				{ 1, 1, 0, 255, 255, 255, 255, 1, 0 },
				{ 1, 0, 0, 255, 255, 255, 255, 1, 1 },
			};
			uint16_t f[6] = { 0, 1, 2, 0, 2, 3 };

			auto *res = core_managers_t::get();
			auto mesh = res->mesh_manager->create();
			mesh->set_vertex_attributes(attribute_flag_color | attribute_flag_tex_coord);
			mesh->set_index_attributes(3, 2);
			mesh->push_vertex_data((char *)v, sizeof(v));
			mesh->push_index_data((char *)f, sizeof(f));
			mesh->calculate_bounds();
			mesh->upload();
			return mesh;
		}


	} // namespace core
} // namespace maki
