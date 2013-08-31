#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	namespace Framework
	{
		class FlashMovie;

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

		private:
			void PrepareGroup(ElementGroup &g, FlashMovie *mov, uint32 sheetIndex);
			void PrepareMetaGroup(FlashMovie *mov);

		public:
			float playhead;
			bool finished;
			Array<ElementGroup> groups;
			ElementGroup metaGroup;
			Array<uint32> currentKeyFrames;

		private:
			Handle movie;
		};

		
	} // namespace Framework

} // namespace Maki