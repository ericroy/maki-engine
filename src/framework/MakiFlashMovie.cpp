#include "framework/framework_stdafx.h"
#include "framework/MakiFlashMovie.h"


namespace Maki
{
	namespace Framework
	{

		const char *FlashMovie::easingMethodNames[FlashMovie::EasingMethodCount] = {
			"",
			"quadratic",
		};

		const char *FlashMovie::tweenPropertyNames[FlashMovie::TweenPropertyCount] = {
			"motion_x",
			"motion_y",
			"scale_x",
			"scale_y",
		};

		FlashMovie::TweenProperty FlashMovie::GetTweenPropertyByName(const char *tweenPropertyName)
		{
			for(uint32 i = 0; i < TweenPropertyCount; i++) {
				if(strcmp(tweenPropertyName, tweenPropertyNames[i]) == 0) {
					return (TweenProperty)i;
				}
			}
			return TweenProperty_None;
		}

		FlashMovie::EasingMethod FlashMovie::GetEasingMethodByName(const char *methodName)
		{
			for(uint32 i = 0; i < EasingMethodCount; i++) {
				if(strcmp(methodName, easingMethodNames[i]) == 0) {
					return (EasingMethod)i;
				}
			}
			return EasingMethod_None;
		}




		FlashMovie::SpriteSheet::SpriteSheet()
			:	material(HANDLE_NONE),
				textureRid(RID_NONE),
				maxElementsInSingleFrame(0)
		{
		}

		FlashMovie::SpriteSheet::~SpriteSheet()
		{
			MaterialManager::Free(material);
		}




		FlashMovie::SpriteSequence::SpriteSequence(uint32 sheetIndex, Document::Node *libItemNode)
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

		FlashMovie::SpriteSequence::~SpriteSequence()
		{
		}




		FlashMovie::Curve::Curve()
			:	active(false)
		{
		}

		FlashMovie::Curve::~Curve()
		{
		}




		FlashMovie::KeyFrame::KeyFrame(Document::Node *node)
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

				e.zIndex = elemNode->ResolveAsInt("z_index");
				
				elemNode->ResolveAsVectorN("size", 2, e.size.vals);
				e.size /= PPU;

				elemNode->ResolveAsVectorN("local_offset", 2, e.localOffset.vals);
				e.localOffset /= PPU;

				float buffer[6];
				elemNode->ResolveAsVectorN("matrix", 6, buffer);
				e.m.SetIdentity();
				e.m.cols[0][0] = buffer[0];
				e.m.cols[1][0] = buffer[1];
				e.m.cols[0][1] = buffer[2];
				e.m.cols[1][1] = buffer[3];
				e.m.cols[3][0] = buffer[4] / PPU;
				e.m.cols[3][1] = buffer[5] / PPU;

				e.libraryIndex = elemNode->ResolveAsInt("library_index");
			}
		}

		FlashMovie::KeyFrame::~KeyFrame()
		{
			for(uint32 i = 0; i < TweenPropertyCount; i++) {
				if(curves[i].active) {
					curves[i].~Curve();
				}
			}
		}





		FlashMovie::Layer::Layer(Document::Node *node)
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
				
		FlashMovie::Layer::~Layer()
		{
			for(uint32 i = 0; i < keyFrames.count; i++) {
				keyFrames[i].~KeyFrame();
			}
		}



		FlashMovie::ElementDescriptor::ElementDescriptor(const Element &e)
		:	size(e.size), libraryIndex(e.libraryIndex)
		{
		}




		FlashMovie::FlashMovie()
			: Resource(), materialRid(RID_NONE), maxFrameCount(0)
		{
		}

		bool FlashMovie::Load(Rid movieRid, Rid materialRid)
		{
			Document doc;
			if(!doc.Load(movieRid)) {
				Console::Error("Failed to load flash movie <rid %u>", movieRid);
				return false;
			}

			frameRate = doc.root->ResolveAsUInt("frame_rate.#0");
			maxFrameCount = doc.root->ResolveAsUInt("max_frame_count.#0");

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

		static Vector2 unitQuad[4] = {
			Vector2(-0.5f, -0.5f),
			Vector2(0.5f, -0.5f),
			Vector2(0.5f, 0.5f),
			Vector2(-0.5f, 0.5f)
		};

		static Vector2 unitTexCoords[4] = {
			Vector2(0.0f, 0.0f),
			Vector2(1.0f, 0.0f),
			Vector2(1.0f, 1.0f),
			Vector2(0.0f, 1.0f)
		};

		void FlashMovie::AdvanceState(float timeDelta, float rateCoeff, bool loop, FlashMovieState &state)
		{
			// Set each element group to have no active elements.
			// We will activate elements below, depending on how many we need to represent the current frame.
			for(uint32 i = 0; i < state.groups.count; i++) {
				state.groups[i].activeElementCount = 0;
				Mesh *m = MeshManager::Get(state.groups[i].mesh);
				char *p = m->GetVertexData();
				memset(p, 0, m->GetVertexCount() * m->GetVertexStride());
			}

			// Advance the playhead.
			// This is the floating point frame position of the overall movie
			state.playhead += timeDelta * frameRate;
			if(state.playhead >= maxFrameCount) {
				state.playhead = (float)std::fmod(state.playhead, maxFrameCount);
				state.currentKeyFrames.Zero();	
			}

			
			for(uint32 li = 0; li < layers.count; li++) {
				Layer &layer = layers[li];
				
				// Advance the key frame index for this layer
				uint32 kfi = state.currentKeyFrames[li];
				KeyFrame *kf = &layer.keyFrames[kfi];
				while(state.playhead >= kf->frameStart + kf->frameDuration) {
					kfi++;
					if(kfi >= layer.keyFrames.count) {
						break;
					}
					kf = &layer.keyFrames[++kfi];
				}
				state.currentKeyFrames[li] = kfi;

				// If playhead is still in range of this layer's frame span, then we need to update this layer
				if(kfi < layer.keyFrames.count) {
					kf = &layer.keyFrames[kfi];
					for(uint32 ei = 0; ei < kf->elements.count; ei++) {
						Element &e = kf->elements[ei];
						
						SpriteSequence &seq = library[e.libraryIndex];
						FlashMovieState::ElementGroup &group = state.groups[seq.sheetIndex];
						Mesh *m = MeshManager::Get(group.mesh);
						
						const Vector2 &sheetSize = sheets[seq.sheetIndex].textureSize;

						uint32 seqFrame = 0;
						if(seq.spriteRects.count > 1) {
							// TODO: Not just a simple graphic, must decide what frame we should show
						}
						const Rect &region = seq.spriteRects[seqFrame];
						

						// Set the geometry for this quad.
						struct V
						{
							Vector3 pos;
							Vector2 uv;
						} *v = (V *)m->GetVertexData();
						v += group.activeElementCount * 4;
						for(uint32 i = 0; i < 4; i++) {
							// Position corner of the quad
							v->pos.x = unitQuad[i].x * e.size.x + e.localOffset.x;
							v->pos.y = unitQuad[i].y * e.size.y + e.localOffset.y;
							v->pos.z = 0.0f;

							// Transform corner by element's matrix
							v->pos = e.m * v->pos;

							// Decide on uv coords for this corner, such that the quad will show
							// the appropriate part of the spritesheet.
							v->uv.x = (region.left + unitTexCoords[i].x * region.GetWidth()) / sheetSize.x;
							v->uv.y = 1.0f - (region.bottom + unitTexCoords[i].y * region.GetHeight()) / sheetSize.y;

							v++;
						}

						// This group now has one more quad in it that will need to be drawn
						group.activeElementCount++;						
					}
				}
			}

			for(uint32 i = 0; i < state.groups.count; i++) {
				MeshManager::Get(state.groups[i].mesh)->Upload();
			}
		}

		FlashMovie::~FlashMovie()
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

	} // namespace Framework

} // namespace Maki

