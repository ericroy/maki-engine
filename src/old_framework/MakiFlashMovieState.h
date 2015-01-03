#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	namespace Framework
	{
		class FlashMovie;

		class FlashMovieState
		{
			friend class FlashMovie;

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
			FlashMovieState();
			virtual ~FlashMovieState();
			bool Init(Handle movie);
			inline bool IsFinished() const { return finished; }
			inline int32 GetTrackIndex() const { return trackIndex; }
			inline void PlayTrack(int32 index)
			{
				trackChanged = index != trackIndex;
				finished = false;
				trackIndex = index;
			}

		private:
			void PrepareGroup(ElementGroup &g, FlashMovie *mov, uint32 sheetIndex);
			void PrepareMetaGroup(FlashMovie *mov);

		public:
			Array<ElementGroup> groups;
			ElementGroup metaGroup;
			Array<uint32> currentKeyFrames;

		private:
			Handle movie;
			float playhead;
			bool finished;
			bool trackChanged;
			int32 trackIndex;
		};

		
	} // namespace Framework

} // namespace Maki