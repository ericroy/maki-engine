#include "framework/framework_stdafx.h"
#include "framework/MakiDebugPhysicsRenderer.h"

namespace Maki
{
	struct Point
	{
		float pos[3];
		uint8 color[4];
	};

	DebugPhysicsRenderer::DebugPhysicsRenderer(Rid matRid)
		: debugMode(0),
		lineIndex(0),
		material(HANDLE_NONE)
	{
		auto *oldRes = ResourceProvider::SetCurrent(&res);

		res.vertexFormatManager = new VertexFormatManager(1);
		res.shaderProgramManager = new ShaderProgramManager(3);
		res.materialManager = new MaterialManager(1);
		res.meshManager = new MeshManager(4096);

		material = res.materialManager->Load(matRid);

		Point verts[2];
		uint16 indices[2] = {0, 1};

		// Precache a bunch of line meshes
		for(uint32 i = 0; i < MAX_LINES; i++) {
			Mesh m(true);
			m.SetVertexAttributes(VertexFormat::AttributeFlag_Color);
			m.SetIndexAttributes(2, 2);
			m.PushVertexData(sizeof(Point)*2, (char *)verts);
			m.PushIndexData(sizeof(uint16)*2, (char *)indices);
			m.Upload();
			Handle h = res.meshManager->Add(std::move(m));
			lines.push_back(h);
		}

		ResourceProvider::SetCurrent(oldRes);
	}

	DebugPhysicsRenderer::~DebugPhysicsRenderer()
	{
		MaterialManager::Free(material);
		MeshManager::Free(lines.size(), lines.data());
	}

	void DebugPhysicsRenderer::Draw(btDynamicsWorld *world)
	{
		auto eng = Engine::Get();
		lineIndex = 0;
		world->setDebugDrawer(this);
		world->debugDrawWorld();
	}

	void DebugPhysicsRenderer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
	{
		if(lineIndex < MAX_LINES-1) {
			DrawCommand cmd;
			cmd.SetMaterial(material);
			
			Handle h = lines[lineIndex++];
			Mesh *m = MeshManager::Get(h);
			Point *verts = (Point *)m->GetVertexData();
			verts[0].pos[0] = from[0];
			verts[0].pos[1] = from[1];
			verts[0].pos[2] = from[2];
			verts[0].color[0] = (uint8)(color[0]*255);
			verts[0].color[1] = (uint8)(color[1]*255);
			verts[0].color[2] = (uint8)(color[2]*255);
			verts[0].color[3] = 255;
			verts[1].pos[0] = to[0];
			verts[1].pos[1] = to[1];
			verts[1].pos[2] = to[2];
			verts[1].color[0] = (uint8)(color[0]*255);
			verts[1].color[1] = (uint8)(color[1]*255);
			verts[1].color[2] = (uint8)(color[2]*255);
			verts[1].color[3] = 255;
			m->Upload();
			cmd.SetMesh(h);
			Engine::Get()->renderer->Draw(cmd, Matrix44::Identity);
		} else {
			Console::Warning("Hopper empty");
		}
	}

	void DebugPhysicsRenderer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
	{
	}

	void DebugPhysicsRenderer::reportErrorWarning(const char* warningString)
	{
	}

	void DebugPhysicsRenderer::draw3dText(const btVector3& location, const char* textString)
	{
	}

	void DebugPhysicsRenderer::setDebugMode(int debugMode)
	{
		this->debugMode = debugMode;
	}

	int DebugPhysicsRenderer::getDebugMode() const
	{
		return debugMode;
	}

} // namespace Maki