#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"
#include "core/MakiSkeleton.h"

namespace maki
{
	namespace core
	{
		class skeleton_t;

		

		class skeleton_animation_t : public resource_t
		{
		public:
			struct state_t
			{
			public:
				state_t(uint32 size);
				state_t();
				void set_size(uint32 size);

			public:
				array_t<uint32> current_key_frames_;
				float current_frame_;
			};

		private:
			struct key_frame_t : public skeleton_t::joint_t
			{
				uint32 frame_;
			};

		public:
			static float debug_rate_coeff_;

		public:
			skeleton_animation_t();
			skeleton_animation_t(const move_token_t<skeleton_animation_t> &) { assert(false && "skeleton_animation_t move construction not allowed"); }
			skeleton_animation_t(const skeleton_animation_t &) { assert(false && "skeleton_animation_t copy construction not allowed"); }
			~skeleton_animation_t();
			bool load(rid_t rid);
			void advance_state(float time_delta, state_t &state, array_t<skeleton_t::joint_t> &pose, bool loop, float rate_coeff);
			inline uint32 get_bone_count() const;

		public:
			float frame_rate_;
			uint32 frame_count_;

			// Indexed as data[bone_index][key_frame_index]

			array_t< array_t<key_frame_t> > data_;

		};

		uint32 skeleton_animation_t::get_bone_count() const
		{
			return data_.count_;
		}

		
		

	} // namespace core

} // namespace maki
