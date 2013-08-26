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
				playhead(0.0f)
		{
			// Hold a reference to the movie
			FlashMovieManager::AddRef(movie);
			this->movie = movie;

			FlashMovie *mov = FlashMovieManager::Get(movie);
			groups.SetSize(mov->sheets.count);
			for(uint32 i = 0; i < groups.count; i++) {
				new(&groups[i]) ElementGroup();
				ElementGroup &g = groups[i];

				Mesh m(true);
				m.SetIndexAttributes(3, 2);
				m.SetVertexAttributes(VertexFormat::AttributeFlag_TexCoord);
					
				uint32 maxElems = mov->sheets[i].maxElementsInSingleFrame;
					
				// Vertex format: 3 floats for position, 2 floats for uv coords
				m.PushVertexData(maxElems * 4 * (3 * sizeof(float) + 2 * sizeof(float)), nullptr);
				
				// We can initialize the indices now, since they are constants, regardless of the quad
				// data being rendered
				m.PushIndexData(maxElems * 6, nullptr);
				uint16 *indices = (uint16 *)m.GetIndexData();
				for(uint32 k = 0; k < maxElems; k++) {
					uint16 base = k * 6;
					*indices++ = base;
					*indices++ = base+1;
					*indices++ = base+2;
					*indices++ = base;
					*indices++ = base+2;
					*indices++ = base+3;
				}

				// Must upload so that the mesh gets a vertex format
				m.Upload();

				g.mesh = CoreManagers::Get()->meshManager->Add(Move(m));
				
				g.dc.SetMesh(g.mesh);
				g.dc.SetMaterial(mov->sheets[i].material);
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

		void FlashMovieState::Draw(Renderer *renderer, const Matrix44 &m)
		{
			for(uint32 i = 0; i < groups.count; i++) {
				renderer->Draw(groups[i].dc, m);
			}
		}


	} // namespace Framework

} // namespace Maki

