#pragma once
#include "framework/framework_stdafx.h"
#include "core/MakiDocument.h"

namespace Maki
{
	namespace Framework
	{
		
		class FlashMovie
		{
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

			struct ControlPoint
			{
				Vector2 prev;
				Vector2 anchor;
				Vector2 next;
			};

			struct Curve
			{
				Curve() : active(false) {}

				bool active;
				Array<ControlPoint> controlPoints;
			};

			struct KeyFrame
			{
				KeyFrame(Document::Node *node)
					:	frameStart(0),
						frameDuration(1),
						timeScale(0),
						timeDuration(0),
						easing(EasingMethod_None),
						easeStrength(0)
				{
					frameStart = node->ResolveAsUInt("frame_start.#0");
					frameDuration = node->ResolveAsUInt("frame_duration.#0");

					Document::Node *tweenNode = node->Resolve("tween");
					if(tweenNode != nullptr) {

						timeScale = tweenNode->ResolveAsUInt("time_scale.#0");
						timeDuration = tweenNode->ResolveAsUInt("frame_duration.#0");
						easing = GetEasingMethodByName(tweenNode->ResolveValue("easing.#0"));
						easeStrength = tweenNode->ResolveAsFloat("ease_strength.#0");

						Document::Node *propertiesNode = node->Resolve("properties");
						for(uint32 i = 0; i < propertiesNode->count; i++) {
							Document::Node *propertyNode = propertiesNode->children[i];
							TweenProperty prop = GetTweenPropertyByName(propertyNode->value);
							if(prop == TweenProperty_None) {
								Console::Warning("Warning, unrecognized tween property: %s", propertyNode->value);
								continue;
							}

							Curve &c = curves[prop];
							c.active = true;
							c.controlPoints.SetSize(propertyNode->count);
							c.controlPoints.Zero();

							for(uint32 j = 0; j < propertyNode->count; j++) {
								Document::Node *cpNode = propertyNode->children[j];
								ControlPoint &cp = c.controlPoints.data[j];

								float buffer[6];
								cpNode->ResolveAsVectorN("", 6, buffer);
								cp.prev.x = buffer[0];
								cp.prev.y = buffer[1];
								cp.anchor.x = buffer[2];
								cp.anchor.y = buffer[3];
								cp.next.x = buffer[4];
								cp.next.y = buffer[5];
							}
						}
					}

					Document::Node *elementsNode = node->Resolve("elements");
					for(uint32 i = 0; i < elementsNode->count; i++) {
						Document::Node *elemNode = elementsNode->children[i];


					}
				}

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
			};

			struct Layer
			{
				Layer(Document::Node *node)
				{
					name = node->ResolveValue("name.#0");
					visible = node->ResolveAsBool("visible.#0");
					outline = node->ResolveAsBool("outline.#0");
					Document::Node *framesNode = node->Resolve("key_frames");
					keyFrames.SetSize(framesNode->count);
					keyFrames.Zero();
					for(uint32 i = 0; i < framesNode->count; i++) {
						new(&keyFrames[i]) KeyFrame(framesNode->children[i]);
					}
				}
				
				virtual ~Layer()
				{
					for(uint32 i = 0; i < keyFrames.count; i++) {
						keyFrames[i].~KeyFrame();
					}
				}

				Array<KeyFrame> keyFrames;
				bool visible;
				bool outline;
				std::string name;
			};

		public:
			FlashMovie() {}
			virtual ~FlashMovie() {}

		public:

		};

		
	} // namespace Framework

} // namespace Maki