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

			libItemNode->ResolveAsVectorN("pos", 2, pos.vals);
			pos /= PPU;

			Document::Node *framesNode = libItemNode->Resolve("frames");
			cells.SetSize(framesNode->count);
			cells.Zero();

			for(uint32 i = 0; i < framesNode->count; i++) {
				Document::Node *frameNode = framesNode->children[i];

				float buffer[4];
				frameNode->ResolveAsVectorN("tex_rect", 4, buffer);
				
				Rect *r = &cells[i].texRect;
				r->left = buffer[0];
				r->top = buffer[1];
				r->right = r->left + buffer[2];
				r->bottom = r->top + buffer[3];

				frameNode->ResolveAsVectorN("stage_rect", 4, buffer);
				
				r = &cells[i].stageRect;
				r->left = buffer[0] / PPU;
				r->top = buffer[1] / PPU;
				r->right = r->left + buffer[2] / PPU;
				r->bottom = r->top + buffer[3] / PPU;
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

				float buffer[6];
				elemNode->ResolveAsVectorN("matrix", 6, buffer);
				e.m.SetIdentity();
				e.m.cols[0][0] = buffer[0];
				e.m.cols[1][0] = buffer[1];
				e.m.cols[0][1] = buffer[2];
				e.m.cols[1][1] = buffer[3];
				e.m.cols[3][0] = buffer[4] / PPU;
				// Negation here because flash has origin in top left, and we have origin in bottom left
				e.m.cols[3][1] = -buffer[5] / PPU;

				e.libraryIndex = elemNode->ResolveAsInt("library_index.#0");
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
				// TODO: make sheet index dynamic someday when export script supports multiple sheets
				new(&library[i]) SpriteSequence(0, libraryNode->children[i]);
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
							if(e.libraryIndex >= 0) {
								sheetFrameMaximums[library[e.libraryIndex].sheetIndex]++;
							}
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

		static Vector2 unitQuadCoeffs[4] = {
			Vector2(0.0f, 0.0f),
			Vector2(0.0f, -1.0f),
			Vector2(1.0f, -1.0f),
			Vector2(1.0f, 0.0f)
		};

		static Vector2 unitQuadTexRectCoeff[4] = {
			Vector2(0.0f, 0.0f),
			Vector2(0.0f, 1.0f),
			Vector2(1.0f, 1.0f),
			Vector2(1.0f, 0.0f)
		};

		void FlashMovie::AdvanceState(float timeDelta, float rateCoeff, bool loop, FlashMovieState &state)
		{
			if(state.finished) {
				return;
			}

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
			state.playhead += timeDelta * rateCoeff * frameRate;
			if(state.playhead >= maxFrameCount) {
				if(!loop) {
					state.finished = true;
					return;
				}
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
					kf = &layer.keyFrames[kfi];
				}
				state.currentKeyFrames[li] = kfi;

				// If playhead is still in range of this layer's frame span, then we need to update this layer
				if(kfi < layer.keyFrames.count) {
					kf = &layer.keyFrames[kfi];
					for(uint32 ei = 0; ei < kf->elements.count; ei++) {
						Element &e = kf->elements[ei];

						if(e.libraryIndex < 0) {
							// A 'meta-element' that doesn't have a sprite representation.
							// Probably a hitbox or a helper or something like that.
							continue;
						}
						
						SpriteSequence &seq = library[e.libraryIndex];
						FlashMovieState::ElementGroup &group = state.groups[seq.sheetIndex];
						assert(group.activeElementCount < sheets[seq.sheetIndex].maxElementsInSingleFrame);
						
						uint32 seqFrame = 0;
						if(seq.cells.count > 1) {
							// TODO: Not just a simple graphic, must decide what frame we should show
						}
						const SpriteSequence::Cell &cell = seq.cells[seqFrame];
						const Vector2 &sheetSize = sheets[seq.sheetIndex].textureSize;

						// Get mesh data as an array of vertices
						// Offset the pointer to the quad we want to set
						Vertex *v = (Vertex *)MeshManager::Get(group.mesh)->GetVertexData() + group.activeElementCount * 4;

						for(uint32 i = 0; i < 4; i++) {
							// Position corner of the quad
							v->pos.x = unitQuadCoeffs[i].x * cell.stageRect.GetWidth() + cell.stageRect.left;
							v->pos.y = unitQuadCoeffs[i].y * cell.stageRect.GetHeight() - cell.stageRect.top;
							v->pos.z = 0.0f;
							v->pos = e.m * v->pos;

							// Decide on uv coords for this corner, such that the quad will show
							// the appropriate part of the spritesheet.
							v->uv.x = (cell.texRect.left + unitQuadTexRectCoeff[i].x * cell.texRect.GetWidth()) / sheetSize.x;
							v->uv.y = (cell.texRect.top + unitQuadTexRectCoeff[i].y * cell.texRect.GetHeight()) / sheetSize.y;

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

