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

		mesh_t::mesh_t(bool dynamic)
			: resource_t(),
			vertex_count(0),
			face_count(0),
			meshFlags(0),
			vertexAttributeFlags(0),
			vertexStride(0),
			indices_per_face(0),
			bytes_per_index(0),
			vertexDataSize(0),
			vertex_data(nullptr),
			indexDataSize(0),
			index_data(nullptr),
			vertex_format_(HANDLE_NONE),
			buffer(nullptr),
			vertexInsertionIndex(0),
			indexInsertionIndex(0),
			dynamic(dynamic),
			oldVertexDataSize(-1),
			oldIndexDataSize(-1)
		{
		}

		mesh_t::mesh_t(object_t type, const object_args_t &args)
			: resource_t(),
			vertex_count(0),
			face_count(0),
			meshFlags(0),
			vertexAttributeFlags(0),
			vertexStride(0),
			indices_per_face(0),
			bytes_per_index(0),
			vertexDataSize(0),
			vertex_data(nullptr),
			indexDataSize(0),
			index_data(nullptr),
			vertex_format_(HANDLE_NONE),
			buffer(nullptr),
			vertexInsertionIndex(0),
			indexInsertionIndex(0),
			dynamic(false),
			oldVertexDataSize(-1),
			oldIndexDataSize(-1)
		{
			switch(type) {
			case Object_Rect:
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
			vertex_count(0),
			face_count(0),
			meshFlags(0),
			vertexAttributeFlags(0),
			vertexStride(0),
			indices_per_face(0),
			bytes_per_index(0),
			vertexDataSize(0),
			vertex_data(nullptr),
			indexDataSize(0),
			index_data(nullptr),
			vertex_format_(HANDLE_NONE),
			buffer(nullptr),
			vertexInsertionIndex(0),
			indexInsertionIndex(0),
			dynamic(false),
			oldVertexDataSize(-1),
			oldIndexDataSize(-1)
		{
			std::swap(siglings_, other.obj->siglings_);
			std::swap(vertex_count, other.obj->vertex_count);
			std::swap(face_count, other.obj->face_count);
			std::swap(meshFlags, other.obj->meshFlags);
			std::swap(vertexAttributeFlags, other.obj->vertexAttributeFlags);
			std::swap(vertexStride, other.obj->vertexStride);
			std::swap(indices_per_face, other.obj->indices_per_face);
			std::swap(bytes_per_index, other.obj->bytes_per_index);
			std::swap(vertexDataSize, other.obj->vertexDataSize);
			std::swap(vertex_data, other.obj->vertex_data);
			std::swap(indexDataSize, other.obj->indexDataSize);
			std::swap(index_data, other.obj->index_data);
			std::swap(vertex_format_, other.obj->vertex_format_);
			std::swap(buffer, other.obj->buffer);
			std::swap(vertexInsertionIndex, other.obj->vertexInsertionIndex);
			std::swap(indexInsertionIndex, other.obj->indexInsertionIndex);
			std::swap(dynamic, other.obj->dynamic);
			std::swap(oldVertexDataSize, other.obj->oldVertexDataSize);
			std::swap(oldIndexDataSize, other.obj->oldIndexDataSize);
			std::swap(bounds, other.obj->bounds);
		}


		mesh_t::~mesh_t()
		{
			if(buffer != nullptr) {
				engine_t::get()->renderer_->free_buffer(buffer);
			}
			vertex_format_manager_t::free(vertex_format_);
			mesh_manager_t::free(siglings_.size(), siglings_.data_());
			MAKI_SAFE_FREE(vertex_data);
			MAKI_SAFE_FREE(index_data);
		}

		void mesh_t::clear_data()
		{
			meshFlags = 0;
			vertexAttributeFlags = 0;
			vertexStride = 0;
			vertex_count = 0;
			face_count = 0;
			bytes_per_index = 0;
			vertexInsertionIndex = 0;
			indexInsertionIndex = 0;

			mesh_manager_t::free(siglings_.size(), siglings_.data_());
			siglings_.clear();
		}

		void mesh_t::push_vertex_data(uint32 sizeInBytes, char *data)
		{
			assert(sizeInBytes % vertexStride == 0);

			if(vertexInsertionIndex+sizeInBytes > vertexDataSize) {
				// Enlarge by some margin
				uint32 overflow = vertexInsertionIndex + sizeInBytes - vertexDataSize;
				uint32 more = std::max(std::max(overflow, 256U), vertexDataSize/2);
				vertexDataSize += more;
				vertex_data = (char *)allocator_t::realloc(vertex_data, vertexDataSize);
				assert(vertex_data);
			}
			assert(vertexInsertionIndex+sizeInBytes <= vertexDataSize);

			if(data != nullptr) {
				memcpy(&vertex_data[vertexInsertionIndex], data, sizeInBytes);
			}
			vertexInsertionIndex += sizeInBytes;
			vertex_count += sizeInBytes / vertexStride;
		}

		void mesh_t::push_index_data(uint32 sizeInBytes, char *data)
		{
			assert(sizeInBytes % (bytes_per_index * indices_per_face) == 0);

			if(indexInsertionIndex+sizeInBytes > indexDataSize) {
				// Enlarge by some margin
				uint32 overflow = indexInsertionIndex + sizeInBytes - indexDataSize;
				uint32 more = std::max(std::max(overflow, 256U), indexDataSize/2);
				indexDataSize += more;
				index_data = (char *)allocator_t::realloc(index_data, indexDataSize);
				assert(index_data);
			}
			assert(indexInsertionIndex+sizeInBytes <= indexDataSize);

			if(data != nullptr) {
				memcpy(&index_data[indexInsertionIndex], data, sizeInBytes);
			}
			indexInsertionIndex += sizeInBytes;
			face_count += sizeInBytes / (indices_per_face * bytes_per_index);
		}

		bool mesh_t::load(rid_t rid, bool upload)
		{
			uint32 bytesRead;
			char *data;
			char *start = data = engine_t::get()->assets->alloc_read(rid, &bytesRead);
			if(start == nullptr) {
				return false;
			}

			if(strncmp(data, "maki", 4) != 0) {
				console_t::error("Invalid binary file type identifier <rid %d>", rid);
				MAKI_SAFE_FREE(start);
				return false;
			}
			data += sizeof(uint8)*8;

			const uint32 meshCount = *(uint32 *)data;
			data += sizeof(uint32);

			core_managers_t *res = core_managers_t::get();
			data += load_mesh_data(data, upload);
			for(uint32 i = 1; i < meshCount; i++) {
				mesh_t nextMesh;
				data += nextMesh.load_mesh_data(data, upload);
				siglings_.push_back(res->mesh_manager_->add(maki_move(nextMesh)));
			}

			if((uint32)(data - start) > bytesRead) {
				console_t::error("Read past the end of the mesh data!");
			} else if((uint32)(data - start) < bytesRead) {
				console_t::error("Still more bytes to be read in the mesh data!");
			}
			assert(data == start + bytesRead);
			MAKI_SAFE_FREE(start);

			calculate_bounds();

			this->rid_ = rid;
			return true;
		}

		uint32 mesh_t::load_mesh_data(char *start, bool upload)
		{
			char *data = start;
		
			// Read mesh properties
			vertex_count = *(uint32 *)data;			data += sizeof(uint32);
			face_count = *(uint32 *)data;			data += sizeof(uint32);
			set_vertex_attributes(*(uint8 *)data);	data += sizeof(uint8);
			indices_per_face = *(uint8 *)data;		data += sizeof(uint8);
			bytes_per_index = *(uint8 *)data;			data += sizeof(uint8);
													data += sizeof(uint8);	// Pad byte
		
			// Allocate a buffer for the vertex data
			vertexDataSize = std::max(vertexDataSize, vertexStride * vertex_count);
			vertex_data = (char *)allocator_t::realloc(vertex_data, vertexDataSize);
			assert(vertex_data);

			// Fill the buffer with vertex data
			memcpy(vertex_data, data, vertexStride * vertex_count);
			data += vertexStride * vertex_count;

			// Align to the nearest word boundary
			if(((intptr_t)data & 0x3) != 0) {
				data += 0x4-((intptr_t)data & 0x3);
			}

			// Allocate a buffer for index data
			indexDataSize = std::max(indexDataSize, bytes_per_index * indices_per_face * face_count);
			index_data = (char *)allocator_t::realloc(index_data, indexDataSize);
			assert(index_data);

			// Fill the buffer with index data
			memcpy(index_data, data, bytes_per_index * indices_per_face * face_count);
			data += bytes_per_index * indices_per_face * face_count;

			// Align to the nearest word boundary
			if(((intptr_t)data & 0x3) != 0) {
				data += 0x4-((intptr_t)data & 0x3);
			}

			// Build gpu buffers from the vertex and index data
			if(upload) {
				upload();
			}

			// Return how much we have advanced the pointer
			return data - start;
		}

		void mesh_t::set_vertex_attributes(uint32 vertexAttributeFlags)
		{
			this->vertexAttributeFlags = vertexAttributeFlags;
			vertexStride = 3*sizeof(float);
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_Normal) != 0) {
				vertexStride += 3*sizeof(float);
			}
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_Tangent) != 0) {
				vertexStride += 3*sizeof(float);
			}
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_Color) != 0) {
				vertexStride += 4*sizeof(uint8);
			}
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_Color1) != 0) {
				vertexStride += 4*sizeof(uint8);
			}
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_TexCoord) != 0) {
				vertexStride += 2*sizeof(float);
			}
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_BoneWeight) != 0) {
				vertexStride += 4*sizeof(uint32);
			}
		}

		void mesh_t::set_index_attributes(uint8 indices_per_face, uint8 bytes_per_index)
		{
			this->indices_per_face = indices_per_face;
			this->bytes_per_index = bytes_per_index;
		}

		int32 mesh_t::get_attribute_offset(vertex_format_t::Attribute attr)
		{
			uint32 offset = 0;
			if(attr == vertex_format_t::Attribute_Position) {
				return offset;
			}
			offset += sizeof(float)*3;
		
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_Normal) != 0) {
				if(attr == vertex_format_t::Attribute_Normal) {
					return offset;
				}
				offset += sizeof(float)*3;
			}

			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_Tangent) != 0) {
				if(attr == vertex_format_t::Attribute_Tangent) {
					return offset;
				}
				offset += sizeof(float)*3;
			}

			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_Color) != 0) {
				if(attr == vertex_format_t::Attribute_Color) {
					return offset;
				}
				offset += sizeof(uint8)*4;
			}

			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_Color1) != 0) {
				if(attr == vertex_format_t::Attribute_Color1) {
					return offset;
				}
				offset += sizeof(uint8)*4;
			}

			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_TexCoord) != 0) {
				if(attr == vertex_format_t::Attribute_TexCoord) {
					return offset;
				}
				offset += sizeof(float)*2;
			}

			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_BoneWeight) != 0) {
				if(attr == vertex_format_t::Attribute_BoneWeight) {
					return offset;
				}
				offset += sizeof(uint32)*4;
			}

			return -1;
		}
	
		void mesh_t::upload()
		{		
			vertex_format_t vf;
			vf.PushAttribute(vertex_format_t::Attribute_Position, vertex_format_t::DataType_Float, 3);
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_Normal) != 0) {
				vf.PushAttribute(vertex_format_t::Attribute_Normal, vertex_format_t::DataType_Float, 3);	
			}
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_Tangent) != 0) {
				vf.PushAttribute(vertex_format_t::Attribute_Tangent, vertex_format_t::DataType_Float, 3);
			}
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_Color) != 0) {
				vf.PushAttribute(vertex_format_t::Attribute_Color, vertex_format_t::DataType_UnsignedInt8, 4);
			}
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_Color1) != 0) {
				vf.PushAttribute(vertex_format_t::Attribute_Color1, vertex_format_t::DataType_UnsignedInt8, 4);
			}
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_TexCoord) != 0) {
				vf.PushAttribute(vertex_format_t::Attribute_TexCoord, vertex_format_t::DataType_Float, 2);
			}
			if((vertexAttributeFlags & vertex_format_t::AttributeFlag_BoneWeight) != 0) {
				vf.PushAttribute(vertex_format_t::Attribute_BoneWeight, vertex_format_t::DataType_UnsignedInt32, 4);
			}

			bool length_changed = oldVertexDataSize != vertexDataSize || oldIndexDataSize != indexDataSize;

			buffer = engine_t::get()->renderer_->upload_buffer(buffer, &vf, vertex_data, vertex_count, index_data, face_count, indices_per_face, bytes_per_index, dynamic, length_changed);

			// get or create vertex format
			handle_t newVertexFormat = core_managers_t::get()->vertex_format_manager_->FindOrAdd(vf);
			vertex_format_manager_t::free(vertex_format_);
			vertex_format_ = newVertexFormat;

			// Record the data sizes so if upload is called again later, we can see if the buffers have changed length
			oldVertexDataSize = vertexDataSize;
			oldIndexDataSize = indexDataSize;
		}

		void mesh_t::calculate_bounds()
		{
			bounds.reset();

			if(vertex_data != nullptr) {
				char *p = vertex_data;
				for(uint32 i = 0; i < vertex_count; i++) {
					vector3_t *v = (vector3_t *)p;
					bounds.merge(*v);
					p += vertexStride;
				}
			}

			const uint32 siblingCount = siglings_.size();
			for(uint32 i = 0; i < siblingCount; i++) {
				mesh_t *m = mesh_manager_t::get(siglings_[i]);
				m->calculate_bounds();
				bounds.merge(m->bounds);
			}
		}

		void mesh_t::make_rect(const rect_args_t &args)
		{
			set_vertex_attributes(vertex_format_t::AttributeFlag_Color|vertex_format_t::AttributeFlag_TexCoord);
			indices_per_face = 3;
			bytes_per_index = 2;

			struct V {
				float pos[3];
				uint8 col[4];
				float uv[2];
			};
			V v[4] = {
				{0, 0, 0, 255, 255, 255, 255, 0, 1},
				{0, 1, 0, 255, 255, 255, 255, 0, 0},
				{1, 1, 0, 255, 255, 255, 255, 1, 0},
				{1, 0, 0, 255, 255, 255, 255, 1, 1},
			};

			push_vertex_data(sizeof(v), (char *)v);
		
			uint16 f[6] = {0, 1, 2, 0, 2, 3};
			push_index_data(sizeof(f), (char *)f);
		}


	} // namespace core

} // namespace maki
