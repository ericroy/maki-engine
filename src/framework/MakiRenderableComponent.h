#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{
			class RenderSystem;
		}

		class RenderableComponent : public Component
		{
			friend class Systems::RenderSystem;
			
		public:
			enum Flag
			{
				Flag_Visible = 1<<0,
				Flag_CastShadow = 1<<1,
				Flag_Physics = 1<<2,
			};

			static const uint32 DEFAULT_FLAGS = Flag_Visible;

		public:
			RenderableComponent(Type type, uint64 dependencies)
				: Component(type, dependencies),
				renderFlags(DEFAULT_FLAGS),
				scaleMatrix(true),
				meshScale(1.0f)
			{
			}

			virtual ~RenderableComponent()
			{
			}
			
			inline void SetMeshScale(float scale)
			{
				scaleMatrix.SetIdentity();
				Matrix44::Scale(scale, scale, scale, scaleMatrix);
				meshScale = scale;
			}

		public:
			uint32 renderFlags;
			BoundingBox bounds;
		
		protected:
			float meshScale;
			Matrix44 scaleMatrix;
		};

	} // namespace Framework

} // namespace Maki