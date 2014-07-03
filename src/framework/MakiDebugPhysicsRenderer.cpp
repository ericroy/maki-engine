#include "framework/framework_stdafx.h"
#include "framework/MakiDebugPhysicsRenderer.h"

#if 0
namespace Maki
{
	namespace Framework
	{
	
		struct Point
		{
			float pos[3];
			uint8 color[4];
		};

		DebugPhysicsRenderer::DebugPhysicsRenderer(Config *config, Rid matRid)
			: debugMode(btIDebugDraw::DBG_DrawWireframe),
			lineIndex(0),
			maxLines(0),
			material(HANDLE_NONE)
		{
			CoreManagers *oldRes = CoreManagers::SetCurrent(&res);

			res.vertexFormatManager = new VertexFormatManager(1);
			res.shaderProgramManager = new ShaderProgramManager(3);
			res.materialManager = new MaterialManager(1);
			res.meshManager = new MeshManager(1);

			material = res.materialManager->Load(matRid);
			cmd.SetMaterial(material);

			Mesh m(true);
			m.SetVertexAttributes(VertexFormat::AttributeFlag_Color);
			m.SetIndexAttributes(2, 2);

			maxLines = config->GetUint("engine.debug_physics_max_lines", DEFAULT_MAX_LINES);

			Point p[2] = {{ 0.0f, 0.0f, 0.0f, 0, 0, 0, 0 }, { 0.0f, 0.0f, 0.0f, 0, 0, 0, 0 }};
			for(uint32 i = 0; i < maxLines; ++i) {	
				m.PushVertexData(sizeof(p), (char *)p);

				uint16 line[2] = { i*2, i*2+1 };
				m.PushIndexData(sizeof(line), (char *)line);
			}

			m.Upload();
			lines = res.meshManager->Add(Move(m));

			cmd.SetMesh(lines);

			CoreManagers::SetCurrent(oldRes);
		}

		DebugPhysicsRenderer::~DebugPhysicsRenderer()
		{
			MaterialManager::Free(material);
			MeshManager::Free(lines);
			cmd.Clear();
		}

		void DebugPhysicsRenderer::Update(btDynamicsWorld *world)
		{
			lineIndex = 0;
			
			Mesh *m = MeshManager::Get(lines);
			char *verts = m->GetVertexData();
			memset(verts, 0, m->GetVertexStride() * m->GetVertexCount());

			world->setDebugDrawer(this);
			world->debugDrawWorld();
						
			m->Upload();
		}

		void DebugPhysicsRenderer::Draw()
		{
			Engine::Get()->renderer->Draw(cmd, Matrix44::Identity);
		}

		void DebugPhysicsRenderer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
		{
			if(lineIndex >= maxLines) {
				Console::Warning("Debug physics rendering drew too many lines, increase hopper size");
				return;
			}
	
			Mesh *m = MeshManager::Get(lines);
			Point *verts = (Point *)m->GetVertexData();

			Point *p = &verts[2*lineIndex];
			p->pos[0] = from[0];
			p->pos[1] = from[1];
			p->pos[2] = from[2];
			p->color[0] = (uint8)(color[0]*255);
			p->color[1] = (uint8)(color[1]*255);
			p->color[2] = (uint8)(color[2]*255);
			p->color[3] = 255;
			p++;

			p->pos[0] = to[0];
			p->pos[1] = to[1];
			p->pos[2] = to[2];
			p->color[0] = (uint8)(color[0]*255);
			p->color[1] = (uint8)(color[1]*255);
			p->color[2] = (uint8)(color[2]*255);
			p->color[3] = 255;

			lineIndex++;
		}

		void DebugPhysicsRenderer::drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int32 lifeTime, const btVector3 &color)
		{
		}

		void DebugPhysicsRenderer::reportErrorWarning(const char  *warningString)
		{
			Console::Warning("Bullet Warning: %s", warningString);
		}

		void DebugPhysicsRenderer::draw3dText(const btVector3 &location, const char *textString)
		{
		}

		void DebugPhysicsRenderer::setDebugMode(int32 debugMode)
		{
			this->debugMode = debugMode;
		}

		int DebugPhysicsRenderer::getDebugMode() const
		{
			return debugMode;
		}

	} // namespace Framework

} // namespace Maki

#endif
