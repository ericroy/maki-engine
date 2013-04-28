#pragma once
#include "core/core_stdafx.h"
#include <vector>
#include "core/MakiResource.h"
#include "core/MakiMaterialManager.h"
#include "core/MakiVertexFormat.h"

namespace Maki
{
	class Mesh : public Resource
	{
	public:
		enum MeshFlag
		{
			 MeshFlag_HasTranslucency = 1 << 0,
		};

		enum Object
		{
			Object_Rect = 0,
			ObjectCount,
		};

		struct ObjectArgs {};
		struct RectArgs : public ObjectArgs
		{
			Vector4 facingAxis;
			float left;
			float right;
			float top;
			float bottom;
		};

	public:
		Mesh(bool dynamic = false);
		Mesh(Object type, const ObjectArgs &args);
		Mesh(const MoveToken<Mesh> &other);
		~Mesh();

		// Initialize object from mesh file
		bool Load(Rid rid, bool upload = true);
		
		// Populate object directly using these
		void SetVertexAttributes(uint32 vertexAttributeFlags);
		void SetIndexAttributes(uint8 indicesPerFace, uint8 bytesPerIndex);
		void PushVertexData(uint32 sizeInBytes, char *data);
		void PushIndexData(uint32 sizeInBytes, char *data);
		void ClearData();

		inline uint8 GetMeshFlag(MeshFlag flag) const { return (meshFlags & flag) != 0; }
		inline void SetMeshFlag(MeshFlag flag, bool on = true) { if(on) { meshFlags |= flag; } else { meshFlags &= ~flag; } }

		// Retrieves the gpu buffer associated with this mesh (creating it if necessary)
		inline void *GetBuffer() { if(buffer == nullptr) { Upload(); } return buffer; }

		inline uint8 GetVertexStride() const { return vertexStride; }
		inline uint32 GetVertexCount() const { return vertexCount; }
		//inline void SetVertexCount(uint32 count) { vertexCount = count; }
		inline char *GetVertexData() { return vertexData; }
		int32 GetAttributeOffset(VertexFormat::Attribute attr);

		inline uint8 GetBytesPerIndex() const { return bytesPerIndex; }
		inline uint8 GetIndicesPerFace() const { return indicesPerFace; }
		inline uint32 GetFaceCount() const { return faceCount; }
		inline char *GetIndexData() { return indexData; }

		// Uploads (or re-uploads) data to gpu buffer
		void Upload();

		void CalculateBounds();

	private:
		Mesh(const Mesh &) {}
		uint32 LoadMeshData(char *data, bool upload);
		void MakeRect(const RectArgs &args);

	public:
		std::vector<Handle> siblings;
		Handle vertexFormat;
		BoundingBox bounds;

	private:
		uint8 meshFlags;
		uint8 vertexAttributeFlags;
		uint8 vertexStride;
		uint8 indicesPerFace;
		uint8 bytesPerIndex;
		uint32 vertexCount;
		uint32 faceCount;

		uint32 vertexInsertionIndex;
		uint32 vertexDataSize;
		char *vertexData;

		uint32 indexInsertionIndex;
		uint32 indexDataSize;
		char *indexData;

		bool dynamic;
		void *buffer;
	};

} // namespace Maki