#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	namespace Framework
	{

		class FlashMovieState
		{
		public:
			// Element group represents a mesh holding a series of quads.
			// Each quad in the mesh uses the same spritesheet texture.
			struct ElementGroup
			{
				ElementGroup();
				~ElementGroup();

				Handle mesh;
				DrawCommand dc;
				uint32 activeElementCount;
			};

		public:
			FlashMovieState(Handle movie);
			virtual ~FlashMovieState();
			void Draw(Renderer *renderer, const Matrix44 &m);

		public:
			Array<ElementGroup> groups;
			float playhead;
			Array<uint32> currentKeyFrames;

		private:
			Handle movie;
		};

		
	} // namespace Framework

} // namespace Maki