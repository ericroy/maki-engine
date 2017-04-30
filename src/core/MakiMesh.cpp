#include "core/core_stdafx.h"
#include "core/MakiMesh.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiRenderer.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiMeshManager.h"


namespace maki
{
	namespace core
	{

		mesh_t::mesh_t(bool dynamic_)
			: resource_t(),
			vertex_count_(0),
			face_count_(0),
			mesh_flags_(0),
			vertex_attribute_flags_(0),
			vertex_stride_(0),
			indices_per_face_(0),
			bytes_per_index_(0),
			vertex_data_size_(0),
			vertex_data_(nullptr),
			index_data_size_(0),
			index_data_(nullptr),
			vertex_format_(HANDLE_NONE),
			buffer_(nullptr),
			vertex_insertion_index_(0),
			index_insertion_index_(0),
			dynamic_(dynamic_),
			old_vertex_data_size_(-1),
			old_index_data_size_(-1)
		{
		}

		mesh_t::mesh_t(object_t type, const object_args_t &args)
			: resource_t(),
			vertex_count_(0),
			face_count_(0),
			mesh_flags_(0),
			vertex_attribute_flags_(0),
			vertex_stride_(0),
			indices_per_face_(0),
			bytes_per_index_(0),
			vertex_data_size_(0),
			vertex_data_(nullptr),
			index_data_size_(0),
			index_data_(nullptr),
			vertex_format_(HANDLE_NONE),
			buffer_(nullptr),
			vertex_insertion_index_(0),
			index_insertion_index_(0),
			dynamic_(false),
			old_vertex_data_size_(-1),
			old_index_data_size_(-1)
		{
			switch(type) {
			case object_rect_:
				make_rect((const rect_args_t &)args);
				break;
			default:
				console_t::error("Invalid premade object type: %d", type);
				assert(false);
			}
			calculate_bounds();
			upload();
		}

		mesh_t::mesh_t(const move_token_t<mesh_t> &other)
			: resource_t((const move_token_t<resource_t> &)other),
			vertex_count_(0),
			face_count_(0),
			mesh_flags_(0),
			vertex_attribute_flags_(0),
			vertex_stride_(0),
			indices_per_face_(0),
			bytes_per_index_(0),
			vertex_data_size_(0),
			vertex_data_(nullptr),
			index_data_size_(0),
			index_data_(nullptr),
			vertex_format_(HANDLE_NONE),
			buffer_(nullptr),
			vertex_insertion_index_(0),
			index_insertion_index_(0),
			dynamic_(false),
			old_vertex_data_size_(-1),
			old_index_data_size_(-1)
		{
			std::swap(siblings_, other.obj_->siblings_);
			std::swap(vertex_count_, other.obj_->vertex_count_);
			std::swap(face_count_, other.obj_->face_count_);
			std::swap(mesh_flags_, other.obj_->mesh_flags_);
			std::swap(vertex_attribute_flags_, other.obj_->vertex_attribute_flags_);
			std::swap(vertex_stride_, other.obj_->vertex_stride_);
			std::swap(indices_per_face_, other.obj_->indices_per_face_);
			std::swap(bytes_per_index_, other.obj_->bytes_per_index_);
			std::swap(vertex_data_size_, other.obj_->vertex_data_size_);
			std::swap(vertex_data_, other.obj_->vertex_data_);
			std::swap(index_data_size_, other.obj_->index_data_size_);
			std::swap(index_data_, other.obj_->index_data_);
			std::swap(vertex_format_, other.obj_->vertex_format_);
			std::swap(buffer_, other.obj_->buffer_);
			std::swap(vertex_insertion_index_, other.obj_->vertex_insertion_index_);
			std::swap(index_insertion_index_, other.obj_->index_insertion_index_);
			std::swap(dynamic_, other.obj_->dynamic_);
			std::swap(old_vertex_data_size_, other.obj_->old_vertex_data_size_);
			std::swap(old_index_data_size_, other.obj_->old_index_data_size_);
			std::swap(bounds_, other.obj_->bounds_);
		}


		mesh_t::~mesh_t()
		{
			if(buffer_ != nullptr) {
				engine_t::get()->renderer_->free_buffer(buffer_);
			}
			vertex_format_manager_t::free(vertex_format_);
			mesh_manager_t::free(siblings_.size(), siblings_.data());
			MAKI_SAFE_FREE(vertex_data_);
			MAKI_SAFE_FREE(index_data_);
		}

		void mesh_t::clear_data()
		{
			mesh_flags_ = 0;
			vertex_attribute_flags_ = 0;
			vertex_stride_ = 0;
			vertex_count_ = 0;
			face_count_ = 0;
			bytes_per_index_ = 0;
			vertex_insertion_index_ = 0;
			index_insertion_index_ = 0;

			mesh_manager_t::free(siblings_.size(), siblings_.data());
			siblings_.clear();
		}

		void mesh_t::push_vertex_data(uint32_t size_in_bytes, char *data)
		{
			assert(size_in_bytes % vertex_stride_ == 0);

			if(vertex_insertion_index_+size_in_bytes > vertex_data_size_) {
				// Enlarge by some margin
				uint32_t overflow = vertex_insertion_index_ + size_in_bytes - vertex_data_size_;
				uint32_t more = std::max(std::max(overflow, 256U), vertex_data_size_/2);
				vertex_data_size_ += more;
				vertex_data_ = (char *)allocator_t::realloc(vertex_data_, vertex_data_size_);
				assert(vertex_data_);
			}
			assert(vertex_insertion_index_+size_in_bytes <= vertex_data_size_);

			if(data != nullptr) {
				memcpy(&vertex_data_[vertex_insertion_index_], data, size_in_bytes);
			}
			vertex_insertion_index_ += size_in_bytes;
			vertex_count_ += size_in_bytes / vertex_stride_;
		}

		void mesh_t::push_index_data(uint32_t size_in_bytes, char *data)
		{
			assert(size_in_bytes % (bytes_per_index_ * indices_per_face_) == 0);

			if(index_insertion_index_+size_in_bytes > index_data_size_) {
				// Enlarge by some margin
				uint32_t overflow = index_insertion_index_ + size_in_bytes - index_data_size_;
				uint32_t more = std::max(std::max(overflow, 256U), index_data_size_/2);
				index_data_size_ += more;
				index_data_ = (char *)allocator_t::realloc(index_data_, index_data_size_);
				assert(index_data_);
			}
			assert(index_insertion_index_+size_in_bytes <= index_data_size_);

			if(data != nullptr) {
				memcpy(&index_data_[index_insertion_index_], data, size_in_bytes);
			}
			index_insertion_index_ += size_in_bytes;
			face_count_ += size_in_bytes / (indices_per_face_ * bytes_per_index_);
		}

		bool mesh_t::load(rid_t rid, bool upload)
		{
			uint32_t bytes_read;
			char *data;
			char *start = data = engine_t::get()->assets_->alloc_read(rid, &bytes_read);
			if(start == nullptr) {
				return false;
			}

			if(strncmp(data, "maki", 4) != 0) {
				console_t::error("Invalid binary file type identifier <rid %d>", rid);
				MAKI_SAFE_FREE(start);
				return false;
			}
			data += sizeof(uint8_t)*8;

			const uint32_t mesh_count = *(uint32_t *)data;
			data += sizeof(uint32_t);

			core_managers_t *res = core_managers_t::get();
			data += load_mesh_data(data, upload);
			for(uint32_t i = 1; i < mesh_count; i++) {
				mesh_t next_mesh;
				data += next_mesh.load_mesh_data(data, upload);
				siblings_.push_back(res->mesh_manager_->add(maki_move(next_mesh)));
			}

			if((uint32_t)(data - start) > bytes_read) {
				console_t::error("Read past the end of the mesh data!");
			} else if((uint32_t)(data - start) < bytes_read) {
				console_t::error("Still more bytes to be read in the mesh data!");
			}
			assert(data == start + bytes_read);
			MAKI_SAFE_FREE(start);

			calculate_bounds();

			this->rid_ = rid;
			return true;
		}

		uint32_t mesh_t::load_mesh_data(char *start, bool upload)
		{
			char *data = start;
		
			// Read mesh properties
			vertex_count_ = *(uint32_t *)data;			data += sizeof(uint32_t);
			face_count_ = *(uint32_t *)data;			data += sizeof(uint32_t);
			set_vertex_attributes(*(uint8_t *)data);	data += sizeof(uint8_t);
			indices_per_face_ = *(uint8_t *)data;		data += sizeof(uint8_t);
			bytes_per_index_ = *(uint8_t *)data;			data += sizeof(uint8_t);
													data += sizeof(uint8_t);	// Pad byte
		
			// Allocate a buffer_ for the vertex data
			vertex_data_size_ = std::max(vertex_data_size_, vertex_stride_ * vertex_count_);
			vertex_data_ = (char *)allocator_t::realloc(vertex_data_, vertex_data_size_);
			assert(vertex_data_);

			// Fill the buffer_ with vertex data
			memcpy(vertex_data_, data, vertex_stride_ * vertex_count_);
			data += vertex_stride_ * vertex_count_;

			// Align to the nearest word boundary
			if(((intptr_t)data & 0x3) != 0) {
				data += 0x4-((intptr_t)data & 0x3);
			}

			// Allocate a buffer_ for index data
			index_data_size_ = std::max(index_data_size_, bytes_per_index_ * indices_per_face_ * face_count_);
			index_data_ = (char *)allocator_t::realloc(index_data_, index_data_size_);
			assert(index_data_);

			// Fill the buffer_ with index data
			memcpy(index_data_, data, bytes_per_index_ * indices_per_face_ * face_count_);
			data += bytes_per_index_ * indices_per_face_ * face_count_;

			// Align to the nearest word boundary
			if(((intptr_t)data & 0x3) != 0) {
				data += 0x4-((intptr_t)data & 0x3);
			}

			// Build gpu buffers from the vertex and index data
			if(upload) {
				this->upload();
			}

			// Return how much we have advanced the pointer
			return data - start;
		}

		void mesh_t::set_vertex_attributes(uint32_t vertex_attribute_flags_)
		{
			this->vertex_attribute_flags_ = vertex_attribute_flags_;
			vertex_stride_ = 3*sizeof(float);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_normal_) != 0) {
				vertex_stride_ += 3*sizeof(float);
			}
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_tangent_) != 0) {
				vertex_stride_ += 3*sizeof(float);
			}
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_color_) != 0) {
				vertex_stride_ += 4*sizeof(uint8_t);
			}
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_color1_) != 0) {
				vertex_stride_ += 4*sizeof(uint8_t);
			}
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_text_coord_) != 0) {
				vertex_stride_ += 2*sizeof(float);
			}
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_bone_weight_) != 0) {
				vertex_stride_ += 4*sizeof(uint32_t);
			}
		}

		void mesh_t::set_index_attributes(uint8_t indices_per_face_, uint8_t bytes_per_index_)
		{
			this->indices_per_face_ = indices_per_face_;
			this->bytes_per_index_ = bytes_per_index_;
		}

		int32_t mesh_t::get_attribute_offset(vertex_format_t::attribute_t attr)
		{
			uint32_t offset = 0;
			if(attr == vertex_format_t::attribute_position_) {
				return offset;
			}
			offset += sizeof(float)*3;
		
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_normal_) != 0) {
				if(attr == vertex_format_t::attribute_normal_) {
					return offset;
				}
				offset += sizeof(float)*3;
			}

			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_tangent_) != 0) {
				if(attr == vertex_format_t::attribute_tangent_) {
					return offset;
				}
				offset += sizeof(float)*3;
			}

			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_color_) != 0) {
				if(attr == vertex_format_t::attribute_color_) {
					return offset;
				}
				offset += sizeof(uint8_t)*4;
			}

			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_color1_) != 0) {
				if(attr == vertex_format_t::attribute_color1_) {
					return offset;
				}
				offset += sizeof(uint8_t)*4;
			}

			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_text_coord_) != 0) {
				if(attr == vertex_format_t::attribute_tex_coord_) {
					return offset;
				}
				offset += sizeof(float)*2;
			}

			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_bone_weight_) != 0) {
				if(attr == vertex_format_t::attribute_bone_weight_) {
					return offset;
				}
				offset += sizeof(uint32_t)*4;
			}

			return -1;
		}
	
		void mesh_t::upload()
		{		
			vertex_format_t vf;
			vf.push_attribute(vertex_format_t::attribute_position_, vertex_format_t::data_type_float_, 3);
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_normal_) != 0) {
				vf.push_attribute(vertex_format_t::attribute_normal_, vertex_format_t::data_type_float_, 3);	
			}
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_tangent_) != 0) {
				vf.push_attribute(vertex_format_t::attribute_tangent_, vertex_format_t::data_type_float_, 3);
			}
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_color_) != 0) {
				vf.push_attribute(vertex_format_t::attribute_color_, vertex_format_t::data_type_unsigned_int8_, 4);
			}
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_color1_) != 0) {
				vf.push_attribute(vertex_format_t::attribute_color1_, vertex_format_t::data_type_unsigned_int8_, 4);
			}
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_text_coord_) != 0) {
				vf.push_attribute(vertex_format_t::attribute_tex_coord_, vertex_format_t::data_type_float_, 2);
			}
			if((vertex_attribute_flags_ & vertex_format_t::attribute_flag_bone_weight_) != 0) {
				vf.push_attribute(vertex_format_t::attribute_bone_weight_, vertex_format_t::data_type_unsigned_int32_, 4);
			}

			bool length_changed = old_vertex_data_size_ != vertex_data_size_ || old_index_data_size_ != index_data_size_;

			buffer_ = engine_t::get()->renderer_->upload_buffer(buffer_, &vf, vertex_data_, vertex_count_, index_data_, face_count_, indices_per_face_, bytes_per_index_, dynamic_, length_changed);

			// get or create vertex format
			handle_t new_vertex_format = core_managers_t::get()->vertex_format_manager_->find_or_add(vf);
			vertex_format_manager_t::free(vertex_format_);
			vertex_format_ = new_vertex_format;

			// Record the data sizes so if upload is called again later, we can see if the buffers have changed length
			old_vertex_data_size_ = vertex_data_size_;
			old_index_data_size_ = index_data_size_;
		}

		void mesh_t::calculate_bounds()
		{
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
			for(uint32_t i = 0; i < sibling_count; i++) {
				mesh_t *m = mesh_manager_t::get(siblings_[i]);
				m->calculate_bounds();
				bounds_.merge(m->bounds_);
			}
		}

		void mesh_t::make_rect(const rect_args_t &args)
		{
			set_vertex_attributes(vertex_format_t::attribute_flag_color_|vertex_format_t::attribute_flag_text_coord_);
			indices_per_face_ = 3;
			bytes_per_index_ = 2;

			struct V {
				float pos[3];
				uint8_t col[4];
				float uv[2];
			};
			V v[4] = {
				{0, 0, 0, 255, 255, 255, 255, 0, 1},
				{0, 1, 0, 255, 255, 255, 255, 0, 0},
				{1, 1, 0, 255, 255, 255, 255, 1, 0},
				{1, 0, 0, 255, 255, 255, 255, 1, 1},
			};

			push_vertex_data(sizeof(v), (char *)v);
		
			uint16_t f[6] = {0, 1, 2, 0, 2, 3};
			push_index_data(sizeof(f), (char *)f);
		}


	} // namespace core

} // namespace maki
