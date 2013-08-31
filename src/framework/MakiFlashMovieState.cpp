#include "framework/framework_stdafx.h"
#include "framework/MakiFlashMovie.h"
#include "framework/MakiFlashMovieManager.h"
#include "framework/MakiFlashMovieState.h"

namespace Maki
{
	namespace Framework
	{
		FlashMovieState::ElementGroup::ElementGroup()
			:	mesh(HANDLE_NONE),
				activeElementCount(0)
		{
		}

		FlashMovieState::ElementGroup::~ElementGroup()
		{
			MeshManager::Free(mesh);
		}


		FlashMovieState::FlashMovieState(Handle movie)
			:	movie(HANDLE_NONE),
				playhead(0.0f),
				finished(false)
		{
			// Hold a reference to the movie
			FlashMovieManager::AddRef(movie);
			this->movie = movie;

			FlashMovie *mov = FlashMovieManager::Get(movie);
			groups.SetSize(mov->sheets.count);
			for(uint32 i = 0; i < groups.count; i++) {
				new(&groups[i]) ElementGroup();
				PrepareGroup(groups[i], mov, i);
			}

			if(mov->maxMetaElementsInSingleFrame > 0) {
				PrepareMetaGroup(mov);
			}

			currentKeyFrames.SetSize(mov->layers.count);
			currentKeyFrames.Zero();
		}

		FlashMovieState::~FlashMovieState()
		{
			for(uint32 i = 0; i < groups.count; i++) {
				groups[i].~ElementGroup();
			}
			FlashMovieManager::Free(movie);
		}

		void FlashMovieState::PrepareGroup(ElementGroup &g, FlashMovie *mov, uint32 sheetIndex)
		{
			Mesh m(true);
			m.SetIndexAttributes(3, 2);
			m.SetVertexAttributes(VertexFormat::AttributeFlag_Color|VertexFormat::AttributeFlag_Color1|VertexFormat::AttributeFlag_TexCoord);
			m.SetMeshFlag(Mesh::MeshFlag_HasTranslucency);
					
			uint32 maxQuads = mov->sheets[sheetIndex].maxElementsInSingleFrame;
			m.PushVertexData(sizeof(FlashMovie::Vertex) * 4 * maxQuads, nullptr);
			m.PushIndexData(maxQuads * 6 * sizeof(uint16), nullptr);

			// We can initialize the indices now, since they are constants, regardless of the quad
			// data being rendered
			uint16 *indices = (uint16 *)m.GetIndexData();
			for(uint32 k = 0; k < maxQuads; k++) {
				uint16 base = k * 4;
				*indices++ = base;
				*indices++ = base+1;
				*indices++ = base+2;
				*indices++ = base;
				*indices++ = base+2;
				*indices++ = base+3;
			}

			// Must upload before adding this mesh to the DrawCommand because the DC requires the
			// vertex format of the mesh to be known
			m.Upload();

			g.mesh = CoreManagers::Get()->meshManager->Add(Move(m));
			g.dc.SetMesh(g.mesh);
			g.dc.SetMaterial(mov->sheets[sheetIndex].material);
		}

		void FlashMovieState::PrepareMetaGroup(FlashMovie *mov)
		{
			Mesh m(true);
			m.SetIndexAttributes(3, 2);
			m.SetVertexAttributes(VertexFormat::AttributeFlag_Color);
			m.SetMeshFlag(Mesh::MeshFlag_HasTranslucency);
					
			uint32 maxQuads = mov->maxMetaElementsInSingleFrame;
			m.PushVertexData(sizeof(FlashMovie::MetaVertex) * 4 * maxQuads, nullptr);
			m.PushIndexData(maxQuads * 6 * sizeof(uint16), nullptr);

			// We can initialize the indices now, since they are constants, regardless of the quad
			// data being rendered
			uint16 *indices = (uint16 *)m.GetIndexData();
			for(uint32 k = 0; k < maxQuads; k++) {
				uint16 base = k * 4;
				*indices++ = base;
				*indices++ = base+1;
				*indices++ = base+2;
				*indices++ = base;
				*indices++ = base+2;
				*indices++ = base+3;
			}

			// Must upload before adding this mesh to the DrawCommand because the DC requires the
			// vertex format of the mesh to be known
			m.Upload();

			metaGroup.mesh = CoreManagers::Get()->meshManager->Add(Move(m));
			metaGroup.dc.SetMesh(metaGroup.mesh);
			metaGroup.dc.SetMaterial(mov->metaMaterial);
		}

		void FlashMovieState::Draw(Renderer *renderer, const Matrix44 &m)
		{
			for(uint32 i = 0; i < groups.count; i++) {
				renderer->Draw(groups[i].dc, m);
			}
#if _DEBUG
			if(metaGroup.mesh != HANDLE_NONE) {
				renderer->Draw(metaGroup.dc, m);
			}
#endif
		}


	} // namespace Framework

} // namespace Maki

