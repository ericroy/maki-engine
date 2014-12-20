#include "core/core_stdafx.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiWindow.h"
#include "core/MakiTimeSource.h"
#include "core/MakiTimer.h"
#include "core/MakiInputState.h"
#include "core/MakiConfig.h"

namespace maki
{
	namespace core
	{

		engine_t::engine_t(window_t *window, time_source_t *time_source, render_core_t *core, const asset_library_t *assets, const config_t *config)
			: pseudo_singleton_t<engine_t>(),
			window_(window),
			config_(config),
			assets_(assets),
			renderer_(nullptr),
			time_source_(time_source),
			update_timer_(time_source),
			render_timer_(time_source),
			input_state_(nullptr)
		{
			input_state_ = new input_state_t();
			renderer_ = new renderer_t(window, core, config);
		
			micros_per_update_ = 1000000u / config->get_uint("engine.updates_per_second", default_updates_per_second_);
			max_skipped_frames_ = config->get_uint("engine.max_skipped_frames", default_max_skipped_frames_);

			next_update_ = time_source->GetTimeMicro() + micros_per_update_;
		}

		engine_t::~engine_t()
		{
			SAFE_DELETE(renderer_);
			SAFE_DELETE(input_state);
		}

		void engine_t::tick()
		{
			// While we are overdue for the next update...
			int64 now = 0;
			uint32 skippedFrames = 0;
			while((now = time_source_->GetTimeMicro()) > next_update_ && skippedFrames < max_skipped_frames_) {
				update_timer_.tick();
				
				window_->poll_input(input_state_);
				if(frame_update_ != nullptr) {
					frame_update_(micros_per_update_ / 1e6f);
				}

				next_update_ = now + micros_per_update_;
				skippedFrames++;
			}

			render_timer_.tick();
			//console_t::info("R:%0.02f  U:%0.02f", render_timer.averageFps, update_timer.averageFps);

			if(frame_draw_ != nullptr) {
				frame_draw_();
			}
		}

	} // namespace core

} // namespace maki
