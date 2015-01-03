#include "framework/framework_stdafx.h"
#include "framework/components/MakiMetaComponent.h"
#include "framework/MakiComponentPool.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			
			Meta::Meta()
				: Component(TYPE, DEPENDENCIES)
			{
			}
			
			Meta::~Meta()
			{
			}
			
			bool Meta::Init(Document::Node *props)
			{
				for(uint32 i = 0; i < props->count; i++) {
					Document::Node *n = props->children[i];
					if(n->count != 1) {
						Console::Error("Meta variable must have exactly one value: %s", n->value);
						return false;
					}
					const char *name = n->value;
					const char *value = n->children[0]->value;

					Property prop;
					prop.SetName(name);
					
					// Load value.  If it can be converted to a float, then assume that it is, otherwise treat it as a string
					char *end;
					double f = strtod(value, &end);
					if(*end == '\0') {
						prop.SetValue((float)f);
					} else {
						prop.SetValue(value);
					}
					properties.push_back(prop);
				}
				return true;
			}

			Meta *Meta::Clone(bool prototype)
			{
				Meta *c = ComponentPool<Meta>::Get()->Create();
				// Copy by value
				c->properties = properties;
				return c;
			}


		} // namspace Components

	} // namespace Framework

} // namespace Maki
