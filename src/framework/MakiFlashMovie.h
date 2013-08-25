#pragma once
#include "framework/framework_stdafx.h"
#include "core/MakiDocument.h"
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
				int32 libraryIndex;

				int32 drawCommandIndex;
			};

			struct SpriteSheet
			{
				SpriteSheet() : material(HANDLE_NONE), textureRid(RID_NONE), maxElementsInSingleFrame(0) {}
				~SpriteSheet() { MaterialManager::Free(material); }

				Rid textureRid;
				Vector2 textureSize;
				Handle material;
				uint32 maxElementsInSingleFrame;
			};

			struct SpriteSequence
			{
				SpriteSequence(uint32 sheetIndex, Document::Node *libItemNode)
					: sheetIndex(sheetIndex)
				{
					strncpy(name, libItemNode->ResolveValue("name.#0"), sizeof(name));
					strncpy(type, libItemNode->ResolveValue("type.#0"), sizeof(type));

					Document::Node *framesNode = libItemNode->Resolve("frames");
					spriteRects.SetSize(framesNode->count);
					spriteRects.Zero();

					for(uint32 i = 0; i < framesNode->count; i++) {
						float buffer[4];
						framesNode->children[i]->ResolveAsVectorN("", 4, buffer);
						Rect &r = spriteRects[i];
						r.left = buffer[0];
						r.top = buffer[1];
						r.right = r.left + buffer[2];
						r.bottom = r.top + buffer[3];
					}
				}

				~SpriteSequence() {}
				
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
				Curve() : active(false) {}
				~Curve() {}

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

						Document::Node *propertiesNode = tweenNode->Resolve("properties");
						for(uint32 i = 0; i < propertiesNode->count; i++) {
							Document::Node *propertyNode = propertiesNode->children[i];
							TweenProperty prop = GetTweenPropertyByName(propertyNode->value);
							if(prop == TweenProperty_None) {
								Console::Warning("Warning, unrecognized tween property: %s", propertyNode->value);
								continue;
							}


							new(&curves[prop]) Curve();
							Curve &c = curves[prop];
							c.active = true;
							c.controlPoints.SetSize(propertyNode->count);
							c.controlPoints.Zero();

							for(uint32 j = 0; j < propertyNode->count; j++) {
								Document::Node *cpNode = propertyNode->children[j];
								ControlPoint &cp = c.controlPoints[j];

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
					elements.SetSize(elementsNode->count);
					elements.Zero();
					for(uint32 i = 0; i < elementsNode->count; i++) {
						Document::Node *elemNode = elementsNode->children[i];
						Element &e = elements[i];

						float buffer[6];
						elemNode->ResolveAsVectorN("matrix", 6, buffer);

						e.zIndex = elemNode->ResolveAsInt("z_index");
						elemNode->ResolveAsVectorN("size", 2, e.size.vals);
						//elemNode->ResolveAsVectorN("transform", 2, e.trans.vals);
						
						e.m.SetIdentity();
						e.m.cols[0][0] = buffer[0];
						e.m.cols[1][0] = buffer[1];
						e.m.cols[0][1] = buffer[2];
						e.m.cols[1][1] = buffer[3];
						e.m.cols[3][0] = buffer[4];
						e.m.cols[3][1] = buffer[5];

						e.libraryIndex = elemNode->ResolveAsInt("library_index");
						e.drawCommandIndex = -1;
					}
				}

				~KeyFrame()
				{
					for(uint32 i = 0; i < TweenPropertyCount; i++) {
						if(curves[i].active) {
							curves[i].~Curve();
						}
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

				Array<Element> elements;
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
				
				~Layer()
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

			struct ElementDescriptor
			{
				ElementDescriptor(const Element &e)
					:	size(e.size), libraryIndex(e.libraryIndex)
				{}

				inline bool operator==(const ElementDescriptor &other) const
				{
					return libraryIndex == other.libraryIndex && size.x == other.size.x && size.y == other.size.y;
				}

				// Key
				Vector2 size;
				int32 libraryIndex;
			
				// Value
				int32 drawCommandIndex;
			};

		public:
			FlashMovie()
				: Resource(), materialRid(RID_NONE), maxFrameCount(0)
			{
			}

		private:
			FlashMovie(const FlashMovie &other) {}

		public:
			bool Load(Rid movieRid, Rid materialRid)
			{
				Document doc;
				if(!doc.Load(movieRid)) {
					Console::Error("Failed to load flash movie <rid %u>", movieRid);
					return false;
				}

				maxFrameCount = doc.root->ResolveAsUInt("max_frame_count");

				Document::Node *layersNode = doc.root->Resolve("layers");
				layers.SetSize(layersNode->count);
				for(uint32 i = 0; i < layersNode->count; i++) {
					Document::Node *layerNode = layersNode->children[i];
					new(&layers[i]) Layer(layerNode);
				}

				Document::Node *sheetsNode = doc.root->Resolve("sprite_sheets");
				sheets.SetSize(sheetsNode->count);
				for(uint32 i = 0; i < sheetsNode->count; i++) {
					new(&sheets[i]) SpriteSheet();
					SpriteSheet &sheet = sheets[i];
					
					sheetsNode->children[i]->ResolveAsVectorN("size", 2, sheet.textureSize.vals);

					// TODO: Create a material for this sprite sheet
					const char *path = sheetsNode->children[i]->ResolveValue("path.#0");
					sheets[i].textureRid = Engine::Get()->assets->PathToRid(path);
					if(sheets[i].textureRid == RID_NONE) {
						Console::Error("Failed to find lookup RID for path: %s", path);
						return false;
					}
				}

				Document::Node *libraryNode = doc.root->Resolve("library");
				library.SetSize(libraryNode->count);
				for(uint32 i = 0; i < libraryNode->count; i++) {
					Document::Node *libItemNode = libraryNode->children[i];

					// TODO: make sheet index dynamic someday when export script supports multiple sheets
					new(&library[i]) SpriteSequence(0, libItemNode);
				}
								
				// Create a material for each sheet
				Handle material = CoreManagers::Get()->materialManager->Load(materialRid);
				if(material == HANDLE_NONE) {
					return false;
				}
				for(uint32 i = 0; i < sheets.count; i++) {
					SpriteSheet &sheet = sheets[i];
					MaterialManager::AddRef(material);
					sheet.material = MaterialManager::DuplicateIfShared(material);
					MaterialManager::Get(sheet.material)->SetTextures(1, &sheet.textureRid);
				}
				MaterialManager::Free(material);
				
				// Determine the maximum number of elements from each sprite sheet that are drawn in a single frame
				// This will help the FlashMovieState (that uses this FlashMovie) to know how big to make it's meshes.

				for(uint32 ki = 0; ki < maxFrameCount; ki++) {
					std::vector<uint32> sheetFrameMaximums(sheets.count, 0);
					for(uint32 li = 0; li < layers.count; li++) {
						Layer &layer = layers[li];
						if(ki < layer.keyFrames.count) {
							KeyFrame &kf = layer.keyFrames[ki];
							for(uint32 ei = 0; ei < kf.elements.count; ei++) {
								Element e = kf.elements[ei];
								sheetFrameMaximums[library[e.libraryIndex].sheetIndex]++;
							}
						}
					}
					for(uint32 i = 0; i < sheets.count; i++) {
						sheets[i].maxElementsInSingleFrame = std::max<uint32>(sheets[i].maxElementsInSingleFrame, sheetFrameMaximums[i]);
					}
				}

				this->materialRid = materialRid;
				this->rid = rid;
				return true;
			}

			virtual ~FlashMovie()
			{
				for(uint32 i = 0; i < layers.count; i++) {
					layers[i].~Layer();
				}
				for(uint32 i = 0; i < library.count; i++) {
					library[i].~SpriteSequence();
				}
				for(uint32 i = 0; i < sheets.count; i++) {
					sheets[i].~SpriteSheet();
				}
			}

		public:
			Rid materialRid;
			Array<Layer> layers;
			Array<SpriteSheet> sheets;
			Array<SpriteSequence> library;

			// The largest number of frames found in any layer
			uint32 maxFrameCount;
		};

		
	} // namespace Framework

} // namespace Maki