#pragma once
#include <functional>
#include "core/MakiPseudoSingleton.h"
#include "core/MakiTimer.h"

namespace maki {
	namespace core {
	
		class config_t;
		class asset_library_t;
		class renderer_t;
		class render_core_t;
		class window_t;
		class input_state_t;
		class timer_t;
		class time_source_t;

		
		

		class engine_t : public pseudo_singleton_t<engine_t>
		{
		public:
			static const uint32_t default_updates_per_second_ = 60;
			static const uint32_t default_max_skipped_frames_ = 6;

		public:
			engine_t(window_t *window, time_source_t *time_source, render_core_t *core, const asset_library_t *assets, const config_t *config);
			virtual ~engine_t();
			void tick();

		public:
			std::function<void(float)> frame_update_;
			std::function<void()> frame_draw_;
			const config_t *config_;
			const asset_library_t *assets_;
		
			timer_t update_timer_;
			timer_t render_timer_;
		
			input_state_t *input_state_;
			renderer_t *renderer_;
			window_t *window_;

		private:
			time_source_t *time_source_;
			int64_t next_update_;
			uint32_t micros_per_update_;
			uint32_t max_skipped_frames_;
		};

	} // namespace core
} // namespace maki
