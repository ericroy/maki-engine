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
			public:
				class Property
				{
				public:
					enum Type
					{
						Type_String = 0,
						Type_Number,
					};

				public:
					Property() : type(Type_String) { name[0] = value.string[0] = 0; }
					
					inline void SetName(const char *name)
					{
						if(strlen(name) >= sizeof(this->name)) {
							Console::Warning("Meta variable name is too long: %s", name);
						}
						strncpy(this->name, name, sizeof(this->name)-1);
					}

					inline void SetValue(float v) { value.number = v; type = Type_Number; }

					inline void SetValue(const char *v)
					{
						if(strlen(v) >= sizeof(value.string)) {
							Console::Warning("Meta property value exceeded buffer size: %s", name);
							return;
						}
						strncpy(value.string, v, sizeof(value.string)-1);
						type = Type_String;
					}

				public:
					Type type;
					char name[16];
					union {
						char string[16];
						float number;
					} value;
				};

				template<class T> struct PropertyTraits {};

				static const Type TYPE = Type_Meta;
				static const TypeFlag DEPENDENCIES = 0;

			public:
				Meta();
				virtual ~Meta();
				bool Init(Document::Node *props);
				Meta *Clone(bool prototype);

				template<class T>
				inline bool GetProperty(const char *name, T *valueOut = nullptr) {
					Property *prop = FindProperty(name);
					if(prop != nullptr) {
						if(prop->type == PropertyTraits<T>::PROPERTY_TYPE) {
							if(valueOut != nullptr) {
								*valueOut = PropertyTraits<T>::ToValueType(*reinterpret_cast<typename PropertyTraits<T>::ELEMENT_TYPE *>(&prop->value));
							}
							return true;
						}
					}
					return false;
				}

				template<class T>
				inline void SetProperty(const char *name, T value) {
					Property *prop = FindProperty(name);
					if(prop != nullptr) {
						prop->SetValue(value);
					} else {
						Property p;
						p.SetName(name);
						p.SetValue(value);
						properties.push_back(p);
					}
				}

				inline void DeleteProperty(const char *name) {
					const uint32 count = properties.size();
					for(uint32 i = 0; i < count; i++) {
						if(strcmp(properties[i].name, name) == 0) {
							properties.erase(properties.begin() + i);
							return;
						}
					}
				}

				inline Property *FindProperty(const char *name) {
					const uint32 count = properties.size();
					for(uint32 i = 0; i < count; i++) {
						if(strcmp(properties[i].name, name) == 0) {
							return &properties[i];
						}
					}
					return nullptr;
				}

			public:
				std::vector<Property> properties;
			};


			template<> struct Meta::PropertyTraits<float>
			{
				static const Property::Type PROPERTY_TYPE = Property::Type_Number;
				typedef float ELEMENT_TYPE;
				typedef float VALUE_TYPE;
				static VALUE_TYPE ToValueType(ELEMENT_TYPE &p) { return p; }
			};

			template<> struct Meta::PropertyTraits<const char *>
			{
				static const Property::Type PROPERTY_TYPE = Property::Type_String;
				typedef char ELEMENT_TYPE;
				typedef const char *VALUE_TYPE;
				static VALUE_TYPE ToValueType(ELEMENT_TYPE &p) { return &p; }
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki