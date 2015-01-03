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
			enum TweenProperty
			{
				TweenProperty_None = -1,
				TweenProperty_MotionX = 0,
				TweenProperty_MotionY,
				TweenProperty_RotationZ,
				TweenProperty_ScaleX,
				TweenProperty_ScaleY,
				TweenProperty_SkewX,
				TweenProperty_SkewY,
				TweenProperty_Alpha,
				TweenProperty_AdvancedRedPercent,
				TweenProperty_AdvancedRedOffset,
				TweenProperty_AdvancedGreenPercent,
				TweenProperty_AdvancedGreenOffset,
				TweenProperty_AdvancedBluePercent,
				TweenProperty_AdvancedBlueOffset,
				TweenProperty_AdvancedAlphaPercent,
				TweenProperty_AdvancedAlphaOffset,
				TweenPropertyCount
			};
			enum TweenPropertyFlag
			{
				TweenPropertyFlag_MotionX = 1u << TweenProperty_MotionX,
				TweenPropertyFlag_MotionY = 1u << TweenProperty_MotionY,
				TweenPropertyFlag_RotationZ = 1u << TweenProperty_RotationZ,
				TweenPropertyFlag_ScaleX = 1u << TweenProperty_ScaleX,
				TweenPropertyFlag_ScaleY = 1u << TweenProperty_ScaleY,
				TweenPropertyFlag_SkewX = 1u << TweenProperty_SkewX,
				TweenPropertyFlag_SkewY = 1u << TweenProperty_SkewY,
				TweenPropertyFlag_Alpha = 1u << TweenProperty_Alpha,
				TweenPropertyFlag_AdvancedRedPercent = 1u << TweenProperty_AdvancedRedPercent,
				TweenPropertyFlag_AdvancedRedOffset = 1u << TweenProperty_AdvancedRedOffset,
				TweenPropertyFlag_AdvancedGreenPercent = 1u << TweenProperty_AdvancedGreenPercent,
				TweenPropertyFlag_AdvancedGreenOffset = 1u << TweenProperty_AdvancedGreenOffset,
				TweenPropertyFlag_AdvancedBluePercent = 1u << TweenProperty_AdvancedBluePercent,
				TweenPropertyFlag_AdvancedBlueOffset = 1u << TweenProperty_AdvancedBlueOffset,
				TweenPropertyFlag_AdvancedAlphaPercent = 1u << TweenProperty_AdvancedAlphaPercent,
				TweenPropertyFlag_AdvancedAlphaOffset = 1u << TweenProperty_AdvancedAlphaOffset,
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

			enum MetaType
			{
				MetaType_None = -1,
				MetaType_Collision = 0,
				MetaType_Damage,
				MetaType_Sensor,
				MetaType_Helper,
				MetaTypeCount
			};
			static const char *metaTypeNames[MetaTypeCount];
			static uint32 metaTypeColors[MetaTypeCount];
			static MetaType GetMetaTypeByName(const char *metaTypeName);

			struct Track
			{
				char name[16];
				uint32 nameHash;
				uint32 firstFrame;
				uint32 lastFrame;
				bool loop;
			};

			struct Element
			{
				Matrix44 m;
				Vector2 transPoint;
				Vector2 size;
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
					return controlPoints[controlPoints.count-1].anchor.y;
				}

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
				uint32 curveFlags;
				Curve curves[TweenPropertyCount];
			};

			struct Layer
			{
				Layer(Document::Node *node);				
				~Layer();

				Array<KeyFrame> keyFrames;
				MetaType metaType;
				std::string name;
			};

		public:
			struct Vertex
			{
				Vector3 pos;
				uint8 color[4];
				uint8 colorAdd[4];
				Vector2 uv;
			};

			struct MetaVertex
			{
				Vector3 pos;
				uint8 color[4];
			};

		public:
			// Materials that all flash movies will use to render their elements
			static Rid materialRid;
			static Rid metaMaterialRid;

		public:
			FlashMovie();

		private:
			FlashMovie(const FlashMovie &other) {}

		public:
			virtual ~FlashMovie();
			bool Load(Rid movieRid);
			void AdvanceState(float timeDelta, FlashMovieState &state, float rateCoeff);
			int32 GetTrackIndexByName(const char *trackName);
			int32 GetTrackIndexByNameHash(uint32 hash);

		public:
			Handle metaMaterial;
			Array<Track> tracks;
			Array<Layer> layers;
			Array<SpriteSheet> sheets;
			Array<SpriteSequence> library;
			uint32 frameRate;
			Vector2 size;
			// The largest number of frames found in any layer
			uint32 maxFrameCount;
			uint32 maxMetaElementsInSingleFrame;
		};
		
	} // namespace Framework

} // namespace Maki