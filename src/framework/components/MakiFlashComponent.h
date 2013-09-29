#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"
#include "framework/MakiFlashMovieState.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			class Flash : public Component
			{
			public:
				static const Type TYPE = Type_Flash;
				static const TypeFlag DEPENDENCIES = 0;

			public:
				Flash();
				virtual ~Flash();
				bool Init(Document::Node *props);
				bool Init(HandleOrRid movieId);
				Flash *Clone(bool prototype);

			public:
				Handle movie;
				FlashMovieState state;
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki