#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			class Meta : public Component
			{
				class Property
				{
				public:
					enum Type
					{
						Type_String = 0,
						Type_Number = 1
					};

				public:
					Property() : type(Type_String) { key[0] = stringValue[0] = 0; }
					
					inline void Set(float f)
					{
						floatValue = f;
						type = Type_Number;
					}

					inline void Set(const char *s)
					{
#if _DEBUG
						if(strlen(s) >= sizeof(stringValue)) {
							Console::Warning("Meta property value exceeded buffer size: %s", key);
							return;
						}
#endif
						strncpy(stringValue, s, sizeof(stringValue));
						type = Type_String;
					}

				public:
					Type type;
					char key[16];
					union {
						char stringValue[16];
						float floatValue;
					};
				};

			public:
				static const Type TYPE = Type_Meta;
				static const TypeFlag DEPENDENCIES = 0;

			public:
				Meta();
				virtual ~Meta();
				bool Init(Document::Node *props);
				Meta *Clone(bool prototype);

				inline const char *GetString(const char *name, const char *defaultValue = nullptr) const {
					const uint32 count = properties.size();
					for(uint32 i = 0; i < count; i++) {
						if(strcmp(properties[i].key, name) == 0) {
							if(properties[i].type == Property::Type_String) {
								return properties[i].stringValue;
							}
							return defaultValue;
						}
					}
					return defaultValue;
				}

				static_assert(std::numeric_limits<float>::has_quiet_NaN, "Doesn't have QNAN");
				inline float GetNumber(const char *name, float defaultValue = std::numeric_limits<float>::quiet_NaN()) const {
					const uint32 count = properties.size();
					for(uint32 i = 0; i < count; i++) {
						if(strcmp(properties[i].key, name) == 0) {
							if(properties[i].type == Property::Type_Number) {
								return properties[i].floatValue;
							}
							return defaultValue;
						}
					}
					return defaultValue;
				}

			public:
				std::vector<Property> properties;
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki