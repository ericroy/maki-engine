#include "core/core_stdafx.h"
#include "core/MakiMesh.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiRenderer.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiMeshManager.h"


namespace Maki
{
	namespace Core
	{

		Mesh::Mesh(bool dynamic)
			: Resource(),
			vertexCount(0),
			faceCount(0),
			meshFlags(0),
			vertexAttributeFlags(0),
			vertexStride(0),
			indicesPerFace(0),
			bytesPerIndex(0),
			vertexDataSize(0),
			vertexData(nullptr),
			indexDataSize(0),
			indexData(nullptr),
			vertexFormat(HANDLE_NONE),
			buffer(nullptr),
			vertexInsertionIndex(0),
			indexInsertionIndex(0),
			dynamic(dynamic),
			oldVertexDataSize(-1),
			oldIndexDataSize(-1)
		{
		}

		Mesh::Mesh(Object type, const ObjectArgs &args)
			: Resource(),
			vertexCount(0),
			faceCount(0),
			meshFlags(0),
			vertexAttributeFlags(0),
			vertexStride(0),
			indicesPerFace(0),
			bytesPerIndex(0),
			vertexDataSize(0),
			vertexData(nullptr),
			indexDataSize(0),
			indexData(nullptr),
			vertexFormat(HANDLE_NONE),
			buffer(nullptr),
			vertexInsertionIndex(0),
			indexInsertionIndex(0),
			dynamic(false),
			oldVertexDataSize(-1),
			oldIndexDataSize(-1)
		{
			switch(type) {
			case Object_Rect:
				MakeRect((const RectArgs &)args);
				break;
			default:
				Console::Error("Invalid premade object type: %d", type);
				assert(false);
			}
			CalculateBounds();
			Upload();
		}

		Mesh::Mesh(const MoveToken<Mesh> &other)
			: Resource((const MoveToken<Resource> &)other),
			vertexCount(0),
			faceCount(0),
			meshFlags(0),
			vertexAttributeFlags(0),
			vertexStride(0),
			indicesPerFace(0),
			bytesPerIndex(0),
			vertexDataSize(0),
			vertexData(nullptr),
			indexDataSize(0),
			indexData(nullptr),
			vertexFormat(HANDLE_NONE),
			buffer(nullptr),
			vertexInsertionIndex(0),
			indexInsertionIndex(0),
			dynamic(false),
			oldVertexDataSize(-1),
			oldIndexDataSize(-1)
		{
			std::swap(siblings, other.obj->siblings);
			std::swap(vertexCount, other.obj->vertexCount);
			std::swap(faceCount, other.obj->faceCount);
			std::swap(meshFlags, other.obj->meshFlags);
			std::swap(vertexAttributeFlags, other.obj->vertexAttributeFlags);
			std::swap(vertexStride, other.obj->vertexStride);
			std::swap(indicesPerFace, other.obj->indicesPerFace);
			std::swap(bytesPerIndex, other.obj->bytesPerIndex);
			std::swap(vertexDataSize, other.obj->vertexDataSize);
			std::swap(vertexData, other.obj->vertexData);
			std::swap(indexDataSize, other.obj->indexDataSize);
			std::swap(indexData, other.obj->indexData);
			std::swap(vertexFormat, other.obj->vertexFormat);
			std::swap(buffer, other.obj->buffer);
			std::swap(vertexInsertionIndex, other.obj->vertexInsertionIndex);
			std::swap(indexInsertionIndex, other.obj->indexInsertionIndex);
			std::swap(dynamic, other.obj->dynamic);
			std::swap(oldVertexDataSize, other.obj->oldVertexDataSize);
			std::swap(oldIndexDataSize, other.obj->oldIndexDataSize);
			std::swap(bounds, other.obj->bounds);
		}


		Mesh::~Mesh()
		{
			if(buffer != nullptr) {
				Engine::Get()->renderer->FreeBuffer(buffer);
			}
			VertexFormatManager::Free(vertexFormat);
			MeshManager::Free(siblings.size(), siblings.data());
			SAFE_FREE(vertexData);
			SAFE_FREE(indexData);
		}

		void Mesh::ClearData()
		{
			meshFlags = 0;
			vertexAttributeFlags = 0;
			vertexStride = 0;
			vertexCount = 0;
			faceCount = 0;
			bytesPerIndex = 0;
			vertexInsertionIndex = 0;
			indexInsertionIndex = 0;

			MeshManager::Free(siblings.size(), siblings.data());
			siblings.clear();
		}

		void Mesh::PushVertexData(uint32 sizeInBytes, char *data)
		{
			assert(sizeInBytes % vertexStride == 0);

			if(vertexInsertionIndex+sizeInBytes > vertexDataSize) {
				// Enlarge by some margin
				uint32 overflow = vertexInsertionIndex + sizeInBytes - vertexDataSize;
				uint32 more = std::max(std::max(overflow, 256U), vertexDataSize/2);
				vertexDataSize += more;
				vertexData = (char *)Allocator::Realloc(vertexData, vertexDataSize);
				assert(vertexData);
			}
			assert(vertexInsertionIndex+sizeInBytes <= vertexDataSize);

			memcpy(&vertexData[vertexInsertionIndex], data, sizeInBytes);
			vertexInsertionIndex += sizeInBytes;
			vertexCount += sizeInBytes / vertexStride;
		}

		void Mesh::PushIndexData(uint32 sizeInBytes, char *data)
		{
			assert(sizeInBytes % (bytesPerIndex * indicesPerFace) == 0);

			if(indexInsertionIndex+sizeInBytes > indexDataSize) {
				// Enlarge by some margin
				uint32 overflow = indexInsertionIndex + sizeInBytes - indexDataSize;
				uint32 more = std::max(std::max(overflow, 256U), indexDataSize/2);
				indexDataSize += more;
				indexData = (char *)Allocator::Realloc(indexData, indexDataSize);
				assert(indexData);
			}
			assert(indexInsertionIndex+sizeInBytes <= indexDataSize);

			memcpy(&indexData[indexInsertionIndex], data, sizeInBytes);
			indexInsertionIndex += sizeInBytes;
			faceCount += sizeInBytes / (indicesPerFace * bytesPerIndex);
		}

		bool Mesh::Load(Rid rid, bool upload)
		{
			uint32 bytesRead;
			char *data;
			char *start = data = Engine::Get()->assets->AllocRead(rid, &bytesRead);
			if(start == nullptr) {
				return false;
			}

			if(strncmp(data, "maki", 4) != 0) {
				Console::Error("Invalid binary file type identifier <rid %d>", rid);
				SAFE_FREE(start);
				return false;
			}
			data += sizeof(uint8)*8;

			const uint32 meshCount = *(uint32 *)data;
			data += sizeof(uint32);

			CoreManagers *res = CoreManagers::Get();
			data += LoadMeshData(data, upload);
			for(uint32 i = 1; i < meshCount; i++) {
				Mesh nextMesh;
				data += nextMesh.LoadMeshData(data, upload);
				siblings.push_back(res->meshManager->Add(Move(nextMesh)));
			}

			if((uint32)(data - start) > bytesRead) {
				Console::Error("Read past the end of the mesh data!");
			} else if((uint32)(data - start) < bytesRead) {
				Console::Error("Still more bytes to be read in the mesh data!");
			}
			assert(data == start + bytesRead);
			SAFE_FREE(start);

			CalculateBounds();

			this->rid = rid;
			return true;
		}

		uint32 Mesh::LoadMeshData(char *start, bool upload)
		{
			char *data = start;
		
			// Read mesh properties
			vertexCount = *(uint32 *)data;			data += sizeof(uint32);
			faceCount = *(uint32 *)data;			data += sizeof(uint32);
			SetVertexAttributes(*(uint8 *)data);	data += sizeof(uint8);
			indicesPerFace = *(uint8 *)data;		data += sizeof(uint8);
			bytesPerIndex = *(uint8 *)data;			data += sizeof(uint8);
													data += sizeof(uint8);	// Pad byte
		
			// Allocate a buffer for the vertex data
			vertexDataSize = std::max(vertexDataSize, vertexStride * vertexCount);
			vertexData = (char *)Allocator::Realloc(vertexData, vertexDataSize);
			assert(vertexData);

			// Fill the buffer with vertex data
			memcpy(vertexData, data, vertexStride * vertexCount);
			data += vertexStride * vertexCount;

			// Align to the nearest word boundary
			if(((intptr_t)data & 0x3) != 0) {
				data += 0x4-((intptr_t)data & 0x3);
			}

			// Allocate a buffer for index data
			indexDataSize = std::max(indexDataSize, bytesPerIndex * indicesPerFace * faceCount);
			indexData = (char *)Allocator::Realloc(indexData, indexDataSize);
			assert(indexData);

			// Fill the buffer with index data
			memcpy(indexData, data, bytesPerIndex * indicesPerFace * faceCount);
			data += bytesPerIndex * indicesPerFace * faceCount;

			// Align to the nearest word boundary
			if(((intptr_t)data & 0x3) != 0) {
				data += 0x4-((intptr_t)data & 0x3);
			}

			// Build gpu buffers from the vertex and index data
			if(upload) {
				Upload();
			}

			// Return how much we have advanced the pointer
			return data - start;
		}

		void Mesh::SetVertexAttributes(uint32 vertexAttributeFlags)
		{
			this->vertexAttributeFlags = vertexAttributeFlags;
			vertexStride = 3*sizeof(float);
			if((vertexAttributeFlags & VertexFormat::AttributeFlag_Normal) != 0) {
				vertexStride += 3*sizeof(float);
			}
			if((vertexAttributeFlags & VertexFormat::AttributeFlag_Tangent) != 0) {
				vertexStride += 3*sizeof(float);
			}
			if((vertexAttributeFlags & VertexFormat::AttributeFlag_Color) != 0) {
				vertexStride += 4*sizeof(uint8);
			}
			if((vertexAttributeFlags & VertexFormat::AttributeFlag_TexCoord) != 0) {
				vertexStride += 2*sizeof(float);
			}
			if((vertexAttributeFlags & VertexFormat::AttributeFlag_BoneWeight) != 0) {
				vertexStride += 4*sizeof(uint32);
			}
		}

		void Mesh::SetIndexAttributes(uint8 indicesPerFace, uint8 bytesPerIndex)
		{
			this->indicesPerFace = indicesPerFace;
			this->bytesPerIndex = bytesPerIndex;
		}

		int32 Mesh::GetAttributeOffset(VertexFormat::Attribute attr)
		{
			uint32 offset = 0;
			if(attr == VertexFormat::Attribute_Position) {
				return offset;
			}
			offset += sizeof(float)*3;
		
			if((vertexAttributeFlags & VertexFormat::AttributeFlag_Normal) != 0) {
				if(attr == VertexFormat::Attribute_Normal) {
					return offset;
				}
				offset += sizeof(float)*3;
			}

			if((vertexAttributeFlags & VertexFormat::AttributeFlag_Tangent) != 0) {
				if(attr == VertexFormat::Attribute_Tangent) {
					return offset;
				}
				offset += sizeof(float)*3;
			}

			if((vertexAttributeFlags & VertexFormat::AttributeFlag_Color) != 0) {
				if(attr == VertexFormat::Attribute_Color) {
					return offset;
				}
				offset += sizeof(uint8)*4;
			}

			if((vertexAttributeFlags & VertexFormat::AttributeFlag_TexCoord) != 0) {
				if(attr == VertexFormat::Attribute_TexCoord) {
					return offset;
				}
				offset += sizeof(float)*2;
			}

			if((vertexAttributeFlags & VertexFormat::AttributeFlag_BoneWeight) != 0) {
				if(attr == VertexFormat::Attribute_BoneWeight) {
					return offset;
				}
				offset += sizeof(uint32)*4;
			}

			return -1;
		}
	
		void Mesh::Upload()
		{		
			VertexFormat vf;
			vf.PushAttribute(VertexFormat::Attribute_Position, VertexFormat::DataType_Float, 3);
			if((vertexAttributeFlags & VertexFormat::AttributeFlag_Normal) != 0) {
				vf.PushAttribute(VertexFormat::Attribute_Normal, VertexFormat::DataType_Float, 3);	
			}
			if((vertexAttributeFlags & VertexFormat::AttributeFlag_Tangent) != 0) {
				vf.PushAttribute(VertexFormat::Attribute_Tangent, VertexFormat::DataType_Float, 3);
			}
			if((vertexAttributeFlags & VertexFormat::AttributeFlag_Color) != 0) {
				vf.PushAttribute(VertexFormat::Attribute_Color, VertexFormat::DataType_UnsignedInt8, 4);
			}
			if((vertexAttributeFlags & VertexFormat::AttributeFlag_TexCoord) != 0) {
				vf.PushAttribute(VertexFormat::Attribute_TexCoord, VertexFormat::DataType_Float, 2);
			}
			if((vertexAttributeFlags & VertexFormat::AttributeFlag_BoneWeight) != 0) {
				vf.PushAttribute(VertexFormat::Attribute_BoneWeight, VertexFormat::DataType_UnsignedInt32, 4);
			}

			bool lengthChanged = oldVertexDataSize != vertexDataSize || oldIndexDataSize != indexDataSize;

			buffer = Engine::Get()->renderer->UploadBuffer(buffer, &vf, vertexData, vertexCount, indexData, faceCount, indicesPerFace, bytesPerIndex, dynamic, lengthChanged);

			// Get or create vertex format
			Handle newVertexFormat = CoreManagers::Get()->vertexFormatManager->FindOrAdd(vf);
			VertexFormatManager::Free(vertexFormat);
			vertexFormat = newVertexFormat;

			// Record the data sizes so if Upload is called again later, we can see if the buffers have changed length
			oldVertexDataSize = vertexDataSize;
			oldIndexDataSize = indexDataSize;
		}

		void Mesh::CalculateBounds()
		{
			bounds.Reset();

			if(vertexData != nullptr) {
				char *p = vertexData;
				for(uint32 i = 0; i < vertexCount; i++) {
					Vector3 *v = (Vector3 *)p;
					bounds.Merge(*v);
					p += vertexStride;
				}
			}

			const uint32 siblingCount = siblings.size();
			for(uint32 i = 0; i < siblingCount; i++) {
				Mesh *m = MeshManager::Get(siblings[i]);
				m->CalculateBounds();
				bounds.Merge(m->bounds);
			}
		}

		void Mesh::MakeRect(const RectArgs &args)
		{
			SetVertexAttributes(VertexFormat::AttributeFlag_Color|VertexFormat::AttributeFlag_TexCoord);
			indicesPerFace = 3;
			bytesPerIndex = 2;

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

			PushVertexData(sizeof(v), (char *)v);
		
			uint16 f[6] = {0, 1, 2, 0, 2, 3};
			PushIndexData(sizeof(f), (char *)f);
		}


	} // namespace Core

} // namespace Maki