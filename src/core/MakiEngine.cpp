#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiWindow.h"
#include "core/MakiTimeSource.h"
#include "core/MakiTimer.h"
#include "core/MakiInputState.h"
#include "core/MakiConfig.h"

namespace maki {
	namespace core {

		engine_t::engine_t(window_t *window, time_source_t *time_source, render_core_t *core, const asset_library_t *assets, const config_t *config)
			: pseudo_singleton_t<engine_t>(),
			window(window),
			config(config),
			assets(assets),
			renderer(nullptr),
			update_timer(time_source),
			render_timer(time_source),
			input_state(nullptr),
			time_source_(time_source) {

			input_state = new input_state_t();
			renderer = new renderer_t(window, core, config);
		
			micros_per_update_ = 1000000u / config->get_uint("engine.updates_per_second", default_updates_per_second);
			max_skipped_frames_ = config->get_uint("engine.max_skipped_frames", default_max_skipped_frames);
			next_update_ = time_source->get_time_micro() + micros_per_update_;
		}

		engine_t::~engine_t() {
			MAKI_SAFE_DELETE(renderer);
			MAKI_SAFE_DELETE(input_state);
		}

		void engine_t::tick() {
			// While we are overdue for the next update...
			int64_t now = 0;
			uint32_t skipped_frames = 0;
			while((now = time_source_->get_time_micro()) > next_update_ && skipped_frames < max_skipped_frames_) {
				update_timer.tick();
				
				window->poll_input(input_state);
				if(frame_update != nullptr) {
					frame_update(micros_per_update_ / 1e6f);
				}

				next_update_ = now + micros_per_update_;
				skipped_frames++;
			}

			render_timer.tick();
			//console_t::info("R:%0.02f  U:%0.02f", render_timer.average_fps, update_timer.average_fps);

			if(frame_draw != nullptr)
				frame_draw();
		}

	} // namespace core
} // namespace maki
