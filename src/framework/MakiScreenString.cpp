#include "framework/framework_stdafx.h"
#include "framework/MakiScreenString.h"


namespace Maki
{

	ScreenString::ScreenString(Handle font, const char *str)
	:	font(HANDLE_NONE),
		mesh(HANDLE_NONE),
		str(str)
	{
		FontManager::AddRef(font);
		this->font = font;
		Font *f = FontManager::Get(font);
		
		// Add all the letters as geometry of a new mesh
		// Move the mesh into the manager and get a handle
		Mesh m;
		f->RenderAsMesh(str, &m);
		mesh = ResourceProvider::Get()->meshManager->Add(std::move(m));
		
		drawCommand.SetMaterial(f->material);
		drawCommand.SetMesh(mesh);
	}
	
	ScreenString::~ScreenString() {
		FontManager::Free(font);
		MeshManager::Free(mesh);
	}

	void ScreenString::Draw(Renderer *renderer) {
		renderer->Draw(drawCommand, GetWorldMatrix());
	}

	void ScreenString::SetString(const char *str) {
		this->str = str;
		Font *f = FontManager::Get(font);
		Mesh *m = MeshManager::Get(mesh);
		m->ClearData();
		f->RenderAsMesh(str, m);
	}

} // namespace Maki