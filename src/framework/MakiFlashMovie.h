#pragma once
#include "framework/framework_stdafx.h"
#include "core/MakiDocument.h"
#include "framework/MakiFlashMovieState.h"
#include <set>

namespace Maki
{
	namespace Framework
	{
		
		class FlashMovie : public Resource
		{
			// Number of pixels per world unit
			static const uint32 PPU = 150;

			enum TweenProperty
			{
				TweenProperty_None = -1,
				TweenProperty_MotionX = 0,
				TweenProperty_MotionY,
				TweenProperty_ScaleX,
				TweenProperty_ScaleY,
				TweenPropertyCount
			};
			static const char *tweenPropertyNames[TweenPropertyCount];
			static TweenProperty GetTweenPropertyByName(const char *tweenPropertyName);

			enum EasingMethod
			{
				EasingMethod_None = 0,
				EasingMethod_Quadratic,
				EasingMethodCount
			};
			static const char *easingMethodNames[EasingMethodCount];
			static EasingMethod GetEasingMethodByName(const char *methodName);

			struct Element
			{
				Matrix44 m;
				int32 zIndex;
				Vector2 size;
				Vector2 localOffset;
				int32 libraryIndex;
			};

			struct SpriteSheet
			{
				SpriteSheet();
				~SpriteSheet();

				Rid textureRid;
				Vector2 textureSize;
				Handle material;
				uint32 maxElementsInSingleFrame;
			};

			struct SpriteSequence
			{
				SpriteSequence(uint32 sheetIndex, Document::Node *libItemNode);
				~SpriteSequence();
				
				char name[32];
				char type[32];
				uint32 sheetIndex;
				Array<Rect> spriteRects;
			};

			struct ControlPoint
			{
				Vector2 prev;
				Vector2 anchor;
				Vector2 next;
			};

			struct Curve
			{
				Curve();
				~Curve();

				bool active;
				Array<ControlPoint> controlPoints;
			};

			struct KeyFrame
			{
				KeyFrame(Document::Node *node);
				~KeyFrame();

				// The frame index where this multi-frame sequence starts
				uint32 frameStart;

				// The duration of this sequence, in frames
				uint32 frameDuration;

				// Units are ms
				uint32 timeScale;
				uint32 timeDuration;

				EasingMethod easing;
				float easeStrength;
				Curve curves[TweenPropertyCount];

				Array<Element> elements;
			};

			struct Layer
			{
				Layer(Document::Node *node);				
				~Layer();

				Array<KeyFrame> keyFrames;
				bool visible;
				bool outline;
				std::string name;
			};

			struct ElementDescriptor
			{
				ElementDescriptor(const Element &e);
				inline bool operator==(const ElementDescriptor &other) const { return libraryIndex == other.libraryIndex && size.x == other.size.x && size.y == other.size.y; }

				// Key
				Vector2 size;
				int32 libraryIndex;
			
				// Value
				int32 drawCommandIndex;
			};

		public:
			FlashMovie();

		private:
			FlashMovie(const FlashMovie &other) {}

		public:
			virtual ~FlashMovie();
			bool Load(Rid movieRid, Rid materialRid);
			void AdvanceState(float timeDelta, float rateCoeff, bool loop, FlashMovieState &state);
			
		public:
			Rid materialRid;
			Array<Layer> layers;
			Array<SpriteSheet> sheets;
			Array<SpriteSequence> library;
			uint32 frameRate;
			// The largest number of frames found in any layer
			uint32 maxFrameCount;
		};
		
	} // namespace Framework

} // namespace Maki