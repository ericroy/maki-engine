#pragma once
#include "core/MakiMacros.h"
#include "core/MakiResource.h"
#include "core/MakiSkeleton.h"

namespace maki {
	namespace core {

		class skeleton_t;

		class skeleton_animation_t : public resource_t {
			MAKI_NO_COPY(skeleton_animation_t);

		public:
			class state_t {
			public:
				state_t() = default;
				state_t(uint32_t length);
				void set_length(uint64_t length);

			public:
				array_t<uint32_t> current_key_frames;
				float current_frame = 0;
			};

		private:
			struct key_frame_t : public skeleton_t::joint_t {
				uint32_t frame = 0;
			};

		public:
			static float debug_rate_coeff;

		public:
			skeleton_animation_t() = default;
			~skeleton_animation_t();
			bool load(rid_t rid);
			void advance_state(float time_delta, state_t &state, array_t<skeleton_t::joint_t> &pose, bool loop, float rate_coeff);
			inline uint64_t frame_count() const { return frame_count_; }
			inline float frame_rate() const { return frame_rate_; }
			inline uint64_t bone_count() const { return data_.length(); }

		private:
			float frame_rate_ = 0.0f;
			uint64_t frame_count_ = 0;
			// Indexed as data[bone_index][key_frame_index]
			array_t<array_t<key_frame_t>> data_;
		};


	} // namespace core
} // namespace maki
