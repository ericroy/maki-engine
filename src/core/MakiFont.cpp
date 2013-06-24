#include "core/core_stdafx.h"
#include "core/MakiFont.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiMesh.h"
#include "core/MakiTextureSet.h"
#include "core/MakiTextureSetManager.h"

namespace Maki
{
	namespace Core
	{

		inline uint32 NextPowerOfTwo(uint32 v)
		{
			v--;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			v++;
			return v;
		}


		Font::Font()
			: Resource(), material(HANDLE_NONE), pixelSize(0), textureWidth(0), textureHeight(0)
		{
		}

		Font::~Font()
		{
			MaterialManager::Free(material);
		}

		bool Font::operator==(const Font &other) const
		{
			return rid == other.rid && pixelSize == other.pixelSize;
		}

		bool Font::Load(Rid shaderProgramRid, Rid fontRid, uint32 pixelSize)
		{
			ResourceProvider *res = ResourceProvider::Get();
			Engine *eng = Engine::Get();

			char *fontData = eng->assets->AllocRead(fontRid);
			if(fontData == nullptr) {
				Console::Error("Failed to load font <rid %d>", fontRid);
				return false;
			}

			textureWidth = 512;
			textureHeight = NextPowerOfTwo(pixelSize);
			uint8 *pixels = (uint8 *)Allocator::Realloc(nullptr, textureWidth * textureHeight);

			int ret;
			while((ret = stbtt_BakeFontBitmap((const uint8 *)fontData, 0, (float)pixelSize, pixels, textureWidth, textureHeight, MIN_CHAR_CODE, CHAR_CODE_COUNT, bakedChars)) <= 0) {
				textureHeight *= 2;
				pixels = (uint8 *)Allocator::Realloc(pixels, textureWidth * textureHeight);
			}

			Handle glyphAtlas = res->textureManager->AllocTexture(Texture::TextureType_Regular, textureWidth, textureHeight, 1);
			Texture *tex = TextureManager::Get(glyphAtlas);
			eng->renderer->WriteToTexture(tex, 0, 0, 0, 0, textureWidth, textureHeight, textureWidth, 1, (char *)pixels);
		
			Allocator::Free(pixels);
			SAFE_FREE(fontData);

			Material mat;
			mat.SetShaderProgram(shaderProgramRid);

			TextureSet ts;
			// The texture set will take ownership of the glyphAtlas handle
			ts.textures[ts.textureCount++] = glyphAtlas;

			// The material will take ownership of the new texture set handle
			mat.textureSet = res->textureSetManager->Add(Move(ts));

			// And finally, we will accept ownerhip of the material handle
			material = res->materialManager->Add(Move(mat));


			this->pixelSize = pixelSize;
			this->rid = fontRid;
			return true;
		}

		void Font::RenderAsMesh(const char *s, Mesh *m)
		{
			m->SetVertexAttributes(VertexFormat::AttributeFlag_TexCoord);
			m->SetIndexAttributes(3, 2);
			m->SetMeshFlag(Mesh::MeshFlag_HasTranslucency);

			float penX = 0.0f;
			float penY = 0.0f;
		
			struct V {
				float pos[3];
				float uv[2];
			};

			stbtt_aligned_quad q;

			const uint32 len = strlen(s);
			for(uint32 i = 0; i < len; i++) {
				if(s[i] < MIN_CHAR_CODE || s[i] > MAX_CHAR_CODE) {
					continue;
				}
			
				stbtt_GetBakedQuad(bakedChars, textureWidth, textureHeight, s[i] - MIN_CHAR_CODE, &penX, &penY, &q, 1);

				V v[4] = {
					{q.x0, q.y0, 0, q.s0, q.t0},
					{q.x0, q.y1, 0, q.s0, q.t1},
					{q.x1, q.y1, 0, q.s1, q.t1},
					{q.x1, q.y0, 0, q.s1, q.t0},
				};
				m->PushVertexData(sizeof(v), (char *)v);

				uint16 base = i*4;
				uint16 f[6] = {base, base+1, base+2, base, base+2, base+3};
				m->PushIndexData(sizeof(f), (char *)f);
			}

			m->Upload();
		}


	} // namespace Core

} // namespace Maki