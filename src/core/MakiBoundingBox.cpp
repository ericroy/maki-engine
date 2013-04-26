#include "core/core_stdafx.h"
#include "core/MakiBoundingBox.h"

namespace Maki
{

	const Vector4 BoundingBox::cornerCoeffs[8] = {
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
		Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
		Vector4(1.0f, -1.0f, 1.0f, 1.0f),

		Vector4(1.0f, 1.0f, -1.0f, 1.0f),
		Vector4(-1.0f, 1.0f, -1.0f, 1.0f),
		Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
		Vector4(1.0f, -1.0f, -1.0f, 1.0f),
	};

} // namespace Maki