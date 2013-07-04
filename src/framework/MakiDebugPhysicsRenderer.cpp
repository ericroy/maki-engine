#include "framework/framework_stdafx.h"
#include "framework/MakiDebugPhysicsRenderer.h"

namespace Maki
{
	namespace Framework
	{
	
		struct Point
		{
			float pos[3];
			uint8 color[4];
		};

		DebugPhysicsRenderer::DebugPhysicsRenderer(Rid matRid)
			: debugMode(btIDebugDraw::DBG_DrawWireframe),
			lineIndex(0),
			material(HANDLE_NONE)
		{
			CoreManagers *oldRes = CoreManagers::SetCurrent(&res);

			res.vertexFormatManager = new VertexFormatManager(1);
			res.shaderProgramManager = new ShaderProgramManager(3);
			res.materialManager = new MaterialManager(1);
			res.meshManager = new MeshManager(1);

			material = res.materialManager->Load(matRid);
			cmd.SetMaterial(material);

			Point verts[2 * MAX_LINES];
			memset(verts, 0, sizeof(verts));

			uint16 indices[2 * MAX_LINES];
			for(uint32 i = 0; i < MAX_LINES*2; ++i) {
				indices[i] = i;
			}

			Mesh m(true);
			m.SetVertexAttributes(VertexFormat::AttributeFlag_Color);
			m.SetIndexAttributes(2, 2);
			m.PushVertexData(sizeof(verts), (char *)verts);
			m.PushIndexData(sizeof(indices), (char *)indices);
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

		void DebugPhysicsRenderer::Draw(btDynamicsWorld *world)
		{
			Engine *eng = Engine::Get();
			lineIndex = 0;
			
			Mesh *m = MeshManager::Get(lines);
			Point *verts = (Point *)m->GetVertexData();
			memset(verts, 0, sizeof(verts));

			world->setDebugDrawer(this);
			world->debugDrawWorld();
						
			m->Upload();
			Engine::Get()->renderer->Draw(cmd, Matrix44::Identity);
		}

		void DebugPhysicsRenderer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
		{
			if(lineIndex >= MAX_LINES) {
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