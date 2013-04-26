#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"

namespace Maki
{

	class ScreenString : public Entity
	{
	public:
		ScreenString(Handle font, const char *str);
		virtual ~ScreenString();
		void Draw(Renderer *batcher);
		void SetString(const char *str);

	private:
		DrawCommand drawCommand;
		Handle font;
		Handle mesh;
		std::string str;
	};

} // namespace Maki