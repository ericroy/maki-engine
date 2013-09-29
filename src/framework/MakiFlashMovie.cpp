#include "framework/framework_stdafx.h"
#include "framework/MakiFlashMovie.h"


namespace Maki
{
	namespace Framework
	{
		Rid FlashMovie::materialRid = RID_NONE;
		Rid FlashMovie::metaMaterialRid = RID_NONE;


		const char *FlashMovie::easingMethodNames[FlashMovie::EasingMethodCount] = {
			"",
			"quadratic",
		};

		const char *FlashMovie::tweenPropertyNames[FlashMovie::TweenPropertyCount] = {
			"motion_x",
			"motion_y",
			"rotation_z",
			"scale_x",
			"scale_y",
			"skew_x",
			"skew_y",
			"alpha_amount",
			"advclr_r_pct",
			"advclr_r_offset",
			"advclr_g_pct",
			"advclr_g_offset",
			"advclr_b_pct",
			"advclr_b_offset",
			"advclr_a_pct",
			"advclr_a_offset",
		};

		const char *FlashMovie::metaTypeNames[MetaTypeCount] = {
			"collision",
			"damage",
			"sensor",
			"helper"
		};

		uint32 FlashMovie::metaTypeColors[MetaTypeCount] = {
			0x44ff4444,
			0x444444ff,
			0x44ffff44,
			0x4444ffff,
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

		FlashMovie::MetaType FlashMovie::GetMetaTypeByName(const char *metaTypeName)
		{
			for(uint32 i = 0; i < MetaTypeCount; i++) {
				if(strcmp(metaTypeName, metaTypeNames[i]) == 0) {
					return (MetaType)i;
				}
			}
			return MetaType_None;
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
			strncpy(name, libItemNode->ResolveValue("name.#0"), sizeof(name)-1);
			strncpy(type, libItemNode->ResolveValue("type.#0"), sizeof(type)-1);

			libItemNode->ResolveAsVectorN("pos", 2, pos.vals);
			pos /= PPU;

			Document::Node *framesNode = libItemNode->Resolve("frames");
			cells.SetSize(framesNode->count);
			cells.Zero();

			for(uint32 i = 0; i < framesNode->count; i++) {
				Document::Node *frameNode = framesNode->children[i];

				float buffer[4];
				frameNode->ResolveAsVectorN("tex_rect", 4, buffer);
				
				Rect &r = cells[i].texRect;
				r.left = buffer[0];
				r.top = buffer[1];
				r.right = r.left + buffer[2];
				r.bottom = r.top + buffer[3];

				frameNode->ResolveAsVectorN("stage_pos", 2, cells[i].stagePos.vals);
				cells[i].stagePos /= PPU;
			}
		}

		FlashMovie::SpriteSequence::~SpriteSequence()
		{
		}




		FlashMovie::Curve::Curve()
		{
		}

		FlashMovie::Curve::~Curve()
		{
		}




		FlashMovie::KeyFrame::KeyFrame(Document::Node *node)
			:	frameStart(0),
				frameDuration(1),
				tween(false),
				timeScale(0),
				timeDuration(0),
				easing(EasingMethod_None),
				easeStrength(0),
				curveFlags(0)
		{
			frameStart = node->ResolveAsUInt("frame_start.#0");
			frameDuration = node->ResolveAsUInt("frame_duration.#0");

			Document::Node *tweenNode = node->Resolve("tween");
			if(tweenNode != nullptr) {
				tween = true;
				timeScale = tweenNode->ResolveAsFloat("time_scale.#0") / 1000.0f;
				timeDuration = tweenNode->ResolveAsFloat("time_duration.#0") / 1000.0f;
				easing = GetEasingMethodByName(tweenNode->ResolveValue("easing.#0"));
				easeStrength = tweenNode->ResolveAsFloat("ease_strength.#0") / 100.0f;

				Document::Node *curvesNode = tweenNode->Resolve("curves");
				for(uint32 i = 0; i < curvesNode->count; i++) {
					Document::Node *curveNode = curvesNode->children[i];
					TweenProperty prop = GetTweenPropertyByName(curveNode->value);
					if(prop == TweenProperty_None) {
						Console::Warning("Warning, unrecognized tween property: %s", curveNode->value);
						continue;
					}

					curveFlags |= (1u << prop);
					new(&curves[prop]) Curve();
					Curve &c = curves[prop];
					c.controlPoints.SetSize(curveNode->count);
					c.controlPoints.Zero();

					for(uint32 j = 0; j < curveNode->count; j++) {
						Document::Node *cpNode = curveNode->children[j];
						ControlPoint &cp = c.controlPoints[j];

						float buffer[6];
						cpNode->ResolveAsVectorN("", 6, buffer);
						cp.prev.x = buffer[0] / 1000.0f;
						cp.prev.y = buffer[1];
						cp.anchor.x = buffer[2] / 1000.0f;
						cp.anchor.y = buffer[3];
						cp.next.x = buffer[4] / 1000.0f;
						cp.next.y = buffer[5];
					}

					// If a handle has the same position as the anchor, then it shouldn't effect the spline at all.
					// In order to prevent its effects, we'll move it so it falls 1/3 of the way along the linear line between
					// this anchor and the next (or previous) anchor. Eg:
					//
					// 0------------o------------o------------0
					// ^a.anchor    ^a.next      ^b.prev      ^b.anchor
					//
					// A spline that has it's handles distributed evenly like this has the property that, as the parameter
					// varies constantly, the position will vary constantly as well.  In other words, there is no easing effect.
					for(uint32 j = 0; j < c.controlPoints.count; j++) {
						ControlPoint &cp = c.controlPoints[j];
						if(j > 0 && (cp.prev - cp.anchor).LengthSquared() < 1e-6f) {
							cp.prev = cp.anchor + (c.controlPoints[j-1].anchor - cp.anchor) / 3.0f;
						}
						if(j < c.controlPoints.count-1 && (cp.next - cp.anchor).LengthSquared() < 1e-6f) {
							cp.next = cp.anchor + (c.controlPoints[j+1].anchor - cp.anchor) / 3.0f;
						}
					}
				}
			}

			Document::Node *elementsNode = node->Resolve("elements");
			elements.SetSize(elementsNode->count);
			elements.Zero();
			for(uint32 i = 0; i < elementsNode->count; i++) {
				Document::Node *elemNode = elementsNode->children[i];
				Element &e = elements[i];

				elemNode->ResolveAsVectorN("trans_point", 2, e.transPoint.vals);
				e.transPoint /= PPU;

				elemNode->ResolveAsVectorN("size", 2, e.size.vals);
				e.size /= PPU;

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

				Document::Node *libItemNode = elemNode->Resolve("lib_item");
				e.libraryIndex = libItemNode->children[0]->ValueAsInt(0);
				e.cellIndex = libItemNode->children[1]->ValueAsInt(0);
			}
		}

		FlashMovie::KeyFrame::~KeyFrame()
		{
			for(uint32 i = 0; i < TweenPropertyCount; i++) {
				if(curveFlags & (1u << i)) {
					curves[i].~Curve();
				}
			}
		}





		FlashMovie::Layer::Layer(Document::Node *node)
		{
			name = node->ResolveValue("name.#0");
			
			metaType = MetaType_None;
			size_t split = name.find_first_of(':', 0);
			if(split != name.npos && name.substr(0, split) == "meta") {
				metaType = GetMetaTypeByName(name.substr(split+1, name.npos).c_str());
			}

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
			: Resource(), metaMaterial(HANDLE_NONE), maxFrameCount(0), maxMetaElementsInSingleFrame(0)
		{
		}

		bool FlashMovie::Load(Rid movieRid)
		{
			Document doc;
			if(!doc.Load(movieRid)) {
				Console::Error("Failed to load flash movie <rid %u>", movieRid);
				goto failed;
			}

			Handle metaMaterial = CoreManagers::Get()->materialManager->Load(metaMaterialRid);
			if(metaMaterial == HANDLE_NONE) {
				Console::Error("Failed to load flash meta material <rid %u>", metaMaterialRid);
				goto failed;
			}

			Handle material = CoreManagers::Get()->materialManager->Load(materialRid);
			if(material == HANDLE_NONE) {
				Console::Error("Failed to load flash material <rid %u>", materialRid);
				goto failed;
			}

			frameRate = doc.root->ResolveAsUInt("frame_rate.#0");
			maxFrameCount = doc.root->ResolveAsUInt("max_frame_count.#0");

			Document::Node *tracksNode = doc.root->Resolve("tracks");
			tracks.SetSize(tracksNode->count);
			for(uint32 i = 0; i < tracksNode->count; i++) {
				Document::Node *trackNode = tracksNode->children[i];
				memset(&tracks[i], 0, sizeof(Track));
				strncpy(tracks[i].name, trackNode->value, sizeof(tracks[i].name)-1);
				tracks[i].firstFrame = trackNode->ResolveAsInt("#0")-1;
				tracks[i].lastFrame = trackNode->ResolveAsInt("#1")-1;
			}

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
					goto failed;
				}
			}

			Document::Node *libraryNode = doc.root->Resolve("library");
			library.SetSize(libraryNode->count);
			for(uint32 i = 0; i < libraryNode->count; i++) {
				// TODO: make sheet index dynamic someday when export script supports multiple sheets
				new(&library[i]) SpriteSequence(0, libraryNode->children[i]);
			}
								
			// Create a material for each sheet
			for(uint32 i = 0; i < sheets.count; i++) {
				SpriteSheet &sheet = sheets[i];
				MaterialManager::AddRef(material);
				sheet.material = MaterialManager::DuplicateIfShared(material);
				MaterialManager::Get(sheet.material)->SetTextures(1, &sheet.textureRid);
			}

			// Done with the base material handle now
			MaterialManager::Free(material);
				
			// Determine the maximum number of elements from each sprite sheet that are drawn in a single frame
			// This will help the FlashMovieState (that uses this FlashMovie) to know how big to make it's meshes.
			for(uint32 ki = 0; ki < maxFrameCount; ki++) {
				uint32 maxMetaElements = 0;
				std::vector<uint32> sheetFrameMaximums(sheets.count, 0);

				for(uint32 li = 0; li < layers.count; li++) {
					Layer &layer = layers[li];
					if(ki < layer.keyFrames.count) {
						KeyFrame &kf = layer.keyFrames[ki];
						for(uint32 ei = 0; ei < kf.elements.count; ei++) {
							Element e = kf.elements[ei];
							if(layer.metaType != MetaType_None) {
								maxMetaElements++;
							} else if(e.libraryIndex >= 0) {
								sheetFrameMaximums[library[e.libraryIndex].sheetIndex]++;
							}
						}
					}
				}
				for(uint32 i = 0; i < sheets.count; i++) {
					sheets[i].maxElementsInSingleFrame = std::max<uint32>(sheets[i].maxElementsInSingleFrame, sheetFrameMaximums[i]);
				}
				maxMetaElementsInSingleFrame = std::max(maxMetaElementsInSingleFrame, maxMetaElements);
			}

			this->metaMaterial = metaMaterial;
			this->rid = rid;
			return true;

failed:
			MaterialManager::Free(material);
			MaterialManager::Free(metaMaterial);
			return false;
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

		void FlashMovie::AdvanceState(float timeDelta, FlashMovieState &state, float rateCoeff)
		{
			if(state.finished || state.trackIndex < 0) {
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
			state.metaGroup.activeElementCount = 0;

			// Handle animation track changes
			const Track &track = tracks[state.trackIndex];
			if(state.trackChanged) {
				state.trackChanged = false;
				state.playhead = (float)track.firstFrame;
				state.currentKeyFrames.Zero();
			} else {
				// Advance the playhead.
				// This is the floating point frame position of the overall movie
				state.playhead += timeDelta * rateCoeff * frameRate;
				if(state.playhead >= track.lastFrame) {
					if(!track.loop) {
						state.finished = true;
						return;
					}
					state.playhead = (float)std::fmod(state.playhead, track.lastFrame) + track.firstFrame;
					state.currentKeyFrames.Zero();
				}
			}

			
			for(uint32 li = 0; li < layers.count; li++) {
				Layer &layer = layers[li];
				bool isMeta = layer.metaType != MetaType_None;
				uint32 layerColor = isMeta ? metaTypeColors[layer.metaType] : 0xffffffff;
				
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
					
					// Find playhead position within this keyframe
					float t = state.playhead - kf->frameStart;

					// Normalize param to [0, 1] range
					t /= kf->timeDuration;
							
					// Ease it
					float easedT = t;
					switch(kf->easing) {
					case EasingMethod_None:
						break;
					case EasingMethod_Quadratic:
						if(kf->easeStrength > 0.0f) {
							// Ease out
							easedT = -(t-1.0f)*(t-1.0f) + 1.0f;
						} else {
							// Ease in
							easedT = t*t;
						}
						break;
					default:
						assert(false && "Unhandled easing method");
					}

					// Adjust "strength" of ease by doing linear combination of linear and eased value
					float absStrength = fabs(kf->easeStrength);
					t = (1.0f - absStrength) * t + absStrength * easedT;

					// Convert param back to full keyframe range
					t *= kf->timeDuration;


					for(uint32 ei = 0; ei < kf->elements.count; ei++) {
						Element &e = kf->elements[ei];

						uint8 r = 0xff, g = 0xff, b = 0xff, a = 0xff;
						uint8 rAdd = 0x0, gAdd = 0x0, bAdd = 0x0, aAdd = 0x0;
						Matrix44 tweenRotScale(true);
						Vector2 tweenTrans(0.0f);

						if(kf->tween) {
							
							if(kf->curveFlags & TweenPropertyFlag_MotionX) {
								tweenTrans.x = kf->curves[TweenProperty_MotionX].Evaluate(t) / PPU;
							}
							if(kf->curveFlags & TweenPropertyFlag_MotionY) {
								tweenTrans.y = -kf->curves[TweenProperty_MotionY].Evaluate(t) / PPU;
							}

							if(kf->curveFlags & TweenPropertyFlag_RotationZ) {
								float rads = -kf->curves[TweenProperty_RotationZ].Evaluate(t) * MAKI_PI / 180.0f;
								Matrix44::RotationZ(rads, tweenRotScale);
							}

							if(kf->curveFlags & (TweenPropertyFlag_ScaleX|TweenPropertyFlag_ScaleY)) {
								Matrix44 scaleMat(true);
								Vector4 scale(1.0f);
								if(kf->curveFlags & TweenPropertyFlag_ScaleX) {
									scale.x = kf->curves[TweenProperty_ScaleX].Evaluate(t) / 100.0f;
								}
								if(kf->curveFlags & TweenPropertyFlag_ScaleY) {
									scale.y = kf->curves[TweenProperty_ScaleY].Evaluate(t) / 100.0f;
								}
								Matrix44::Scale(scale, scaleMat);
								tweenRotScale = tweenRotScale * scaleMat;
							}

							// Meta layers will never have color tweens
							if(!isMeta) {

								// Alpha effect, and advanced colour effects are mutally exclusive
								if(kf->curveFlags & TweenPropertyFlag_Alpha) {
									a = (uint8)(kf->curves[TweenProperty_Alpha].Evaluate(t) / 100.0f * 255.0f);
								} else if(kf->curveFlags & (
									TweenPropertyFlag_AdvancedRedPercent|TweenPropertyFlag_AdvancedRedOffset|
									TweenPropertyFlag_AdvancedGreenPercent|TweenPropertyFlag_AdvancedGreenOffset|
									TweenPropertyFlag_AdvancedBluePercent|TweenPropertyFlag_AdvancedBlueOffset|
									TweenPropertyFlag_AdvancedAlphaPercent|TweenPropertyFlag_AdvancedAlphaOffset)) {

									if(kf->curveFlags & TweenPropertyFlag_AdvancedRedPercent) {
										r = (uint8)Clamp(r * kf->curves[TweenProperty_AdvancedRedPercent].Evaluate(t) / 100.0f, 0.0f, 255.0f);
									}
									if(kf->curveFlags & TweenPropertyFlag_AdvancedGreenPercent) {
										g = (uint8)Clamp(g * kf->curves[TweenProperty_AdvancedGreenPercent].Evaluate(t) / 100.0f, 0.0f, 255.0f);
									}
									if(kf->curveFlags & TweenPropertyFlag_AdvancedBluePercent) {
										b = (uint8)Clamp(b * kf->curves[TweenProperty_AdvancedBluePercent].Evaluate(t) / 100.0f, 0.0f, 255.0f);
									}
									if(kf->curveFlags & TweenPropertyFlag_AdvancedAlphaPercent) {
										a = (uint8)Clamp(a * kf->curves[TweenProperty_AdvancedAlphaPercent].Evaluate(t) / 100.0f, 0.0f, 255.0f);
									}
								
									if(kf->curveFlags & TweenPropertyFlag_AdvancedRedOffset) {
										rAdd = (uint8)kf->curves[TweenProperty_AdvancedRedOffset].Evaluate(t);
									}
									if(kf->curveFlags & TweenPropertyFlag_AdvancedGreenOffset) {
										gAdd = (uint8)kf->curves[TweenProperty_AdvancedGreenOffset].Evaluate(t);
									}
									if(kf->curveFlags & TweenPropertyFlag_AdvancedBlueOffset) {
										bAdd = (uint8)kf->curves[TweenProperty_AdvancedBlueOffset].Evaluate(t);
									}
									if(kf->curveFlags & TweenPropertyFlag_AdvancedAlphaOffset) {
										aAdd = (uint8)kf->curves[TweenProperty_AdvancedAlphaOffset].Evaluate(t);
									}
								}

							}
						}

						if(isMeta) {

							// A 'meta-element' that doesn't have a sprite representation.
							// Probably a hitbox or a helper or something like that.

							// Get mesh data as an array of vertices
							// Offset the pointer to the quad we want to set
							MetaVertex *v = (MetaVertex *)MeshManager::Get(state.metaGroup.mesh)->GetVertexData() + state.metaGroup.activeElementCount * 4;

							for(uint32 i = 0; i < 4; i++) {
								// Position corner of the quad
								v->pos.x = unitQuadCoeffs[i].x * e.size.x + (0.0f - e.transPoint.x);
								v->pos.y = unitQuadCoeffs[i].y * e.size.y - (0.0f - e.transPoint.y);
								v->pos.z = 0.0f;
								v->pos = tweenRotScale * v->pos;
							
								v->pos.x += e.transPoint.x;
								v->pos.y += -e.transPoint.y;

								v->pos = e.m * v->pos;
								v->pos.x += tweenTrans.x;
								v->pos.y += tweenTrans.y;

								*(uint32 *)v->color = layerColor;
								v++;
							}

							// This group now has one more quad in it that will need to be drawn
							state.metaGroup.activeElementCount++;

						} else {

							// This shouldn't normally happen, but if you have an unrecognized meta layer in the document, it might.
							if(e.libraryIndex < 0) {
								continue;
							}

							SpriteSequence &seq = library[e.libraryIndex];
							FlashMovieState::ElementGroup &group = state.groups[seq.sheetIndex];
							assert(group.activeElementCount < sheets[seq.sheetIndex].maxElementsInSingleFrame);
						
							const SpriteSequence::Cell &cell = seq.cells[e.cellIndex];
							const Vector2 &sheetSize = sheets[seq.sheetIndex].textureSize;

							// Get mesh data as an array of vertices
							// Offset the pointer to the quad we want to set
							Vertex *v = (Vertex *)MeshManager::Get(group.mesh)->GetVertexData() + group.activeElementCount * 4;

							for(uint32 i = 0; i < 4; i++) {
								// Position corner of the quad
								v->pos.x = unitQuadCoeffs[i].x * cell.texRect.GetWidth() / PPU + (cell.stagePos.x - e.transPoint.x);
								v->pos.y = unitQuadCoeffs[i].y * cell.texRect.GetHeight() / PPU - (cell.stagePos.y - e.transPoint.y);
								v->pos.z = 0.0f;
								v->pos = tweenRotScale * v->pos;
							
								v->pos.x += e.transPoint.x;
								v->pos.y += -e.transPoint.y;

								v->pos = e.m * v->pos;
								v->pos.x += tweenTrans.x;
								v->pos.y += tweenTrans.y;

								v->color[0] = r;
								v->color[1] = g;
								v->color[2] = b;
								v->color[3] = a;

								v->colorAdd[0] = rAdd;
								v->colorAdd[1] = gAdd;
								v->colorAdd[2] = bAdd;
								v->colorAdd[3] = aAdd;

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
			}

			for(uint32 i = 0; i < state.groups.count; i++) {
				MeshManager::Get(state.groups[i].mesh)->Upload();
			}
			if(state.metaGroup.mesh != HANDLE_NONE) {
				MeshManager::Get(state.metaGroup.mesh)->Upload();
			}
		}

		int32 FlashMovie::GetTrackIndex(const char *trackName)
		{
			for(uint32 i = 0; i < tracks.count; i++) {
				if(strcmp(tracks[i].name, trackName) == 0) {
					return (int32)i;
				}
			}
			return -1;
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

