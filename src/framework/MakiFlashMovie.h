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
				int32 libraryIndex;
				int32 cellIndex;
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
				struct Cell
				{
					Rect texRect;
					Vector2 stagePos;
				};

				SpriteSequence(uint32 sheetIndex, Document::Node *libItemNode);
				~SpriteSequence();
				
				char name[32];
				char type[32];
				Vector2 pos;
				uint32 sheetIndex;
				Array<Cell> cells;
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

				inline float Evaluate(float playhead)
				{
					for(uint32 cpi = 0; cpi < controlPoints.count-1; cpi++) {
						ControlPoint &a = controlPoints[cpi];
						ControlPoint &b = controlPoints[cpi+1];
						if(playhead >= a.anchor.x && playhead <= b.anchor.x) {
							float cy = 3 * (a.next.y - a.anchor.y);
							float by = 3 * (b.prev.y - a.next.y) - cy;
							float ay = b.anchor.y - a.anchor.y - cy - by;
							
							// Convert playhead position to parameter t in range [0, 1]
							float t = (playhead - a.anchor.x) / (b.anchor.x - a.anchor.x);
							
							// Evaluate y coordinate of spline
							return ay * t*t*t + by * t*t + cy * t + a.anchor.y;
						}
					}
					return controlPoints[0].anchor.y;
				}

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

				// The items on the stage for this frame
				Array<Element> elements;				


				// Everything below here is only available if tween == true
				bool tween;
				float timeScale;
				float timeDuration;
				EasingMethod easing;
				float easeStrength;
				Curve curves[TweenPropertyCount];
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

		public:
			struct Vertex
			{
				Vector3 pos;
				Vector2 uv;
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