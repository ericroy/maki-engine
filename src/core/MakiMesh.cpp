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
			swap(vertex_data_, other.vertex_data_);
			
			index_data_ = move(other.index_data_);
			swap(index_data_size_, other.index_data_size_);

			swap(vertex_count_, other.vertex_count_);
			swap(face_count_, other.face_count_);
			swap(mesh_flags_, other.mesh_flags_);
			swap(vertex_attribute_flags_, other.vertex_attribute_flags_);
			swap(vertex_stride_, other.vertex_stride_);
			swap(indices_per_face_, other.indices_per_face_);
			swap(bytes_per_index_, other.bytes_per_index_);
			swap(vertex_data_size_, other.vertex_data_size_);
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

		void mesh_t::reset(bool keep_buffers) {
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
			if (!keep_buffers) {
				vertex_data_.set_length(0);
				index_data_.set_length(0);
			}
		}

		void mesh_t::push_vertex_data(uint32_t size_in_bytes, char *data) {
			MAKI_ASSERT(size_in_bytes % vertex_stride_ == 0);

			if(vertex_insertion_index_ + size_in_bytes > vertex_data_size_) {
				// Enlarge by some margin
				uint32_t overflow = vertex_insertion_index_ + size_in_bytes - vertex_data_size_;
				uint32_t more = max(max(overflow, 256U), vertex_data_size_/2);
				vertex_data_size_ += more;
				vertex_data_.set_length(vertex_data_size_);
			}
			MAKI_ASSERT(vertex_insertion_index_ + size_in_bytes <= vertex_data_size_);

			if(data)
				memcpy(&vertex_data_.data()[vertex_insertion_index_], data, size_in_bytes);

			vertex_insertion_index_ += size_in_bytes;
			vertex_count_ += size_in_bytes / vertex_stride_;
		}

		void mesh_t::push_index_data(uint32_t size_in_bytes, char *data) {
			MAKI_ASSERT(size_in_bytes % (bytes_per_index_ * indices_per_face_) == 0);

			if(index_insertion_index_+size_in_bytes > index_data_size_) {
				// Enlarge by some margin
				uint32_t overflow = index_insertion_index_ + size_in_bytes - index_data_size_;
				uint32_t more = max(max(overflow, 256U), index_data_size_/2);
				index_data_size_ += more;
				index_data_.set_length(index_data_size_);
			}
			MAKI_ASSERT(index_insertion_index_+size_in_bytes <= index_data_size_);

			if(data != nullptr)
				memcpy(&index_data_.data()[index_insertion_index_], data, size_in_bytes);

			index_insertion_index_ += size_in_bytes;
			face_count_ += size_in_bytes / (indices_per_face_ * bytes_per_index_);
		}

		bool mesh_t::load(rid_t rid, bool upload) {
			array_t<char> buffer = engine_t::get()->assets->alloc_read(rid);
			if (!buffer)
				return false;

			char *start = buffer.data();
			char *data = buffer.data();

			if(strncmp(data, "maki", 4) != 0) {
				console_t::error("Invalid binary file type identifier <rid %u>", rid);
				MAKI_SAFE_FREE(start);
				return false;
			}
			data += sizeof(uint8_t) * 8;

			const uint32_t mesh_count = *(uint32_t *)data;
			data += sizeof(uint32_t);

			core_managers_t *res = core_managers_t::get();
			data += load_mesh_data(data, upload);
			for(uint32_t i = 1; i < mesh_count; i++) {
				mesh_t next_mesh;
				data += next_mesh.load_mesh_data(data, upload);
				siblings_.push_back(res->mesh_manager->add(move(next_mesh)));
			}

			if((uint32_t)(data - start) > bytes_read)
				console_t::error("Read past the end of the mesh data!");
			else if((uint32_t)(data - start) < bytes_read) {
				console_t::error("Still more bytes to be read in the mesh data!");
			MAKI_ASSERT(data == start + bytes_read);

			calculate_bounds();
			this->rid = rid;
			return true;
		}

		uint32_t mesh_t::load_mesh_data(char *start, bool upload) {
			char *data = start;
		
			// Read mesh properties
			vertex_count_ = *(uint32_t *)data;			data += sizeof(uint32_t);
			face_count_ = *(uint32_t *)data;			data += sizeof(uint32_t);
			set_vertex_attributes(*(uint8_t *)data);	data += sizeof(uint8_t);
			indices_per_face_ = *(uint8_t *)data;		data += sizeof(uint8_t);
			bytes_per_index_ = *(uint8_t *)data;		data += sizeof(uint8_t);
														data += sizeof(uint8_t);	// Pad byte
		
			// Allocate a buffer for the vertex data
			vertex_data_size_ = max(vertex_data_size_, vertex_stride_ * vertex_count_);
			vertex_data_.set_length(vertex_data_size_);

			// Fill the buffer with vertex data
			memcpy(vertex_data_.data(), data, vertex_stride_ * vertex_count_);
			data += vertex_stride_ * vertex_count_;

			// Align to the nearest word boundary
			if(((intptr_t)data & 0x3) != 0)
				data += 0x4-((intptr_t)data & 0x3);

			// Allocate a buffer_ for index data
			index_data_size_ = max(index_data_size_, bytes_per_index_ * indices_per_face_ * face_count_);
			index_data_.set_length(index_data_size_);

			// Fill the buffer_ with index data
			memcpy(index_data_.data(), data, bytes_per_index_ * indices_per_face_ * face_count_);
			data += bytes_per_index_ * indices_per_face_ * face_count_;

			// Align to the nearest word boundary
			if(((intptr_t)data & 0x3) != 0)
				data += 0x4-((intptr_t)data & 0x3);

			// Build gpu buffers from the vertex and index data
			if(upload)
				this->upload();

			// Return how much we have advanced the pointer
			return data - start;
		}

		void mesh_t::set_vertex_attributes(uint32_t vertex_attribute_flags) {
			vertex_attribute_flags_ = vertex_attribute_flags;
			vertex_stride_ = 3 * sizeof(float);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_normal) != 0)
				vertex_stride_ += 3 * sizeof(float);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_tangent) != 0)
				vertex_stride_ += 3 * sizeof(float);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_color) != 0)
				vertex_stride_ += 4 * sizeof(uint8_t);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_color1) != 0)
				vertex_stride_ += 4 * sizeof(uint8_t);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_text_coord) != 0)
				vertex_stride_ += 2 * sizeof(float);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_bone_weight) != 0)
				vertex_stride_ += 4 * sizeof(uint32_t);
		}

		void mesh_t::set_index_attributes(uint8_t indices_per_face, uint8_t bytes_per_index) {
			indices_per_face_ = indices_per_face;
			bytes_per_index_ = bytes_per_index;
		}

		int32_t mesh_t::get_attribute_offset(vertex_format_t::attribute_t attr) {
			uint32_t offset = 0;
			if(attr == vertex_format_t::attribute_position)
				return offset;
			offset += sizeof(float) * 3;
		
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_normal) != 0) {
				if(attr == vertex_format_t::attribute_normal)
					return offset;
				offset += sizeof(float) * 3;
			}

			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_tangent) != 0) {
				if(attr == vertex_format_t::attribute_tangent)
					return offset;
				offset += sizeof(float) * 3;
			}

			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_color) != 0) {
				if(attr == vertex_format_t::attribute_color)
					return offset;
				offset += sizeof(uint8_t) * 4;
			}

			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_color1) != 0) {
				if(attr == vertex_format_t::attribute_color1)
					return offset;
				offset += sizeof(uint8_t) * 4;
			}

			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_tex_coord) != 0) {
				if(attr == vertex_format_t::attribute_tex_coord)
					return offset;
				offset += sizeof(float) * 2;
			}

			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_bone_weight) != 0) {
				if(attr == vertex_format_t::attribute_bone_weight)
					return offset;
				offset += sizeof(uint32_t) * 4;
			}

			return -1;
		}
	
		void mesh_t::upload() {		
			vertex_format_t vf;
			vf.push_attribute(vertex_format_t::attribute_position, vertex_format_t::data_type_float, 3);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_normal) != 0)
				vf.push_attribute(vertex_format_t::attribute_normal, vertex_format_t::data_type_float, 3);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_tangent) != 0)
				vf.push_attribute(vertex_format_t::attribute_tangent, vertex_format_t::data_type_float, 3);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_color) != 0)
				vf.push_attribute(vertex_format_t::attribute_color, vertex_format_t::data_type_unsigned_int8, 4);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_color1) != 0)
				vf.push_attribute(vertex_format_t::attribute_color1, vertex_format_t::data_type_unsigned_int8, 4);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_tex_coord) != 0)
				vf.push_attribute(vertex_format_t::attribute_tex_coord, vertex_format_t::data_type_float, 2);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_bone_weight) != 0)
				vf.push_attribute(vertex_format_t::attribute_bone_weight, vertex_format_t::data_type_unsigned_int32, 4);

			bool length_changed = old_vertex_data_size_ != vertex_data_size_ || old_index_data_size_ != index_data_size_;

			buffer_ = engine_t::get()->renderer->upload_buffer(buffer_, &vf, vertex_data_, vertex_count_, index_data_, face_count_, indices_per_face_, bytes_per_index_, dynamic_, length_changed);

			// get or create vertex format
			vertex_format_ = core_managers_t::get()->vertex_format_manager->get_or_add(vf);

			// Record the data sizes so if upload is called again later, we can see if the buffers have changed length
			old_vertex_data_size_ = vertex_data_size_;
			old_index_data_size_ = index_data_size_;
		}

		void mesh_t::calculate_bounds() {
			bounds_.reset();

			if(vertex_data_ != nullptr) {
				char *p = vertex_data_;
				for(uint32_t i = 0; i < vertex_count_; i++) {
					vector3_t *v = (vector3_t *)p;
					bounds_.merge(*v);
					p += vertex_stride_;
				}
			}

			const uint32_t sibling_count = siblings_.size();
			for (auto &m : siblings_) {
				m.calculate_bounds();
				bounds_.merge(m.bounds);
			}
		}









		bool mesh_loader_t::load(ref_t<mesh_t> &out, rid_t rid, bool upload) {

		}
		uint32_t mesh_loader_t::load_data(char *data, bool upload) {

		}

		void mesh_builder_t::make_quad(ref_t<mesh_t> &out, bool upload) {
			out->reset();
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
			out->set_vertex_attributes(vertex_format_t::attribute_flag_color | vertex_format_t::attribute_flag_text_coord);
			out->set_index_attributes(3, 2);
			out->push_vertex_data(sizeof(v), (char *)v);
			out->push_index_data(sizeof(f), (char *)f);
			out->calculate_bounds();
			if (upload)
				out->upload();
		}


	} // namespace core
} // namespace maki
