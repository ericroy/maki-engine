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

	} // namespace Framework

} // namespace Maki

