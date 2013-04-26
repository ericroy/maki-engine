#include "core/core_stdafx.h"
#include "core/MakiShader.h"
#include "core/MakiEngine.h"
#include "core/MakiBase64.h"
#include <sstream>

namespace Maki
{

	const char *Shader::frameUniformNames[FrameUniformCount] = {
		"uView",
		"uProjection",
		"uViewProjection",
		"uLightPositions",
		"uLightDirections",
		"uLightViewProj",
		"uLightProperties",
		"uShadowMapProperties",
		"uLightSplitRegions",
		"uCameraSplitDistances",
		"uGlobalAmbientColor",
		"uCameraWithHeightNearFar",
	};
	
	const char *Shader::objectUniformNames[ObjectUniformCount] = {
		"uModel",
		"uModelView",
		"uModelViewProjection"
	};

	Shader::FrameUniform Shader::GetFrameUniformByName(const char *name)
	{
		for(int32 i = 0; i < FrameUniformCount; i++) {
			if(strcmp(frameUniformNames[i], name) == 0) {
				return (FrameUniform)i;
			}
		}
		return FrameUniform_None;
	}

	Shader::ObjectUniform Shader::GetObjectUniformByName(const char *name)
	{
		for(int32 i = 0; i < ObjectUniformCount; i++) {
			if(strcmp(objectUniformNames[i], name) == 0) {
				return (ObjectUniform)i;
			}
		}
		return ObjectUniform_None;
	}



	Shader::Shader()
		: handle(0),
		programData(nullptr),
		programDataBytes(0),
		engineFrameUniformBytes(0),
		engineObjectUniformBytes(0),
		materialUniformBytes(0),
		frameUniformBufferLocation(-1),
		objectUniformBufferLocation(-1),
		materialUniformBufferLocation(-1)
	{
		memset(engineFrameUniformLocations, -1, sizeof(engineFrameUniformLocations));
		memset(engineObjectUniformLocations, -1, sizeof(engineObjectUniformLocations));
	}

	Shader::~Shader()
	{
		SAFE_FREE(programData);
	}

	bool Shader::Init(Document::Node *shaderNode, const char *dataKey, const char *metaKey)
	{
		Document::Node *dataNode = shaderNode->Resolve(dataKey);
		if(dataNode == nullptr || dataNode->count != 1) {
			return false;
		}

		char *encodedData = dataNode->children[0]->value;
		if(encodedData == nullptr) {
			Console::Error("Failed to parse shader, no base64 encoded program found in 'data' node");
			return false;
		}

		std::stringstream in(encodedData);
		std::stringstream out(std::ios::out | std::ios::binary);
		if(!Base64::Decode(in, out)) {
			Console::Error("Failed to base64 decode shader program");
			return false;
		}
		std::string data = out.str();
		programDataBytes = data.length();

		SAFE_FREE(programData);
		programData = (char *)Allocator::Malloc(programDataBytes);
		memcpy(programData, data.c_str(), programDataBytes);
		

		Document::Node *metaNode = shaderNode->Resolve(metaKey);
		if(metaNode == nullptr) {
			return true;
		}

		Document::Node *enginePerFrame = metaNode->Resolve("enginePerFrame");
		if(enginePerFrame != nullptr) {
			if(!enginePerFrame->ResolveAsInt("slot.#0", &frameUniformBufferLocation)) {
				return false;
			}

			Document::Node *uniforms = enginePerFrame->Resolve("uniforms");
			if(!uniforms) {
				return false;
			}

			for(uint32 i = 0; i < uniforms->count; i++) {
				Document::Node *uni = uniforms->children[i];
				assert(uni->count == 2 && "uniform must have offset and size");

				FrameUniform c = GetFrameUniformByName(uni->value);
				if(c == FrameUniform_None) {
					return false;
				}
				uint32 offset = uni->children[0]->ValueAsUInt();
				uint32 size = uni->children[1]->ValueAsUInt();
				engineFrameUniformLocations[c] = (int32)offset;				
				engineFrameUniformBytes = std::max(offset+size, engineFrameUniformBytes);
			}		
		}

		Document::Node *enginePerObject = metaNode->Resolve("enginePerObject");
		if(enginePerObject != nullptr) {
			if(!enginePerObject->ResolveAsInt("slot.#0", &objectUniformBufferLocation)) {
				return false;
			}

			Document::Node *uniforms = enginePerObject->Resolve("uniforms");
			if(!uniforms) {
				return false;
			}

			for(uint32 i = 0; i < uniforms->count; i++) {
				Document::Node *uni = uniforms->children[i];
				assert(uni->count == 2 && "uniform must have offset and size");

				ObjectUniform c = GetObjectUniformByName(uni->value);
				if(c == ObjectUniform_None) {
					return false;
				}
				uint32 offset = uni->children[0]->ValueAsUInt();
				uint32 size = uni->children[1]->ValueAsUInt();
				engineObjectUniformLocations[c] = (int32)offset;				
				engineObjectUniformBytes = std::max(offset+size, engineObjectUniformBytes);
			}		
		}

		Document::Node *material = metaNode->Resolve("material");
		if(material != nullptr) {
			if(!material->ResolveAsInt("slot.#0", &materialUniformBufferLocation)) {
				return false;
			}

			Document::Node *uniforms = material->Resolve("uniforms");
			if(!uniforms) {
				return false;
			}

			for(uint32 i = 0; i < uniforms->count; i++) {
				Document::Node *uni = uniforms->children[i];
				assert(uni->count == 2 && "uniform must have offset and size");

				uint32 offset = uni->children[0]->ValueAsUInt();
				uint32 size = uni->children[1]->ValueAsUInt();
				
				materialUniformLocations.push_back(MaterialUniformLocation((int32)offset, uni->value));
				materialUniformBytes = std::max(offset+size, materialUniformBytes);
			}		
		}

		return true;
	}

	int32 Shader::FindMaterialConstantLocation(const char *name)
	{
		const uint32 count = materialUniformLocations.size();
		for(uint32 i = 0; i < count; i++) {
			if(strcmp(name, materialUniformLocations[i].name) == 0) {
				return materialUniformLocations[i].location;
			}
		}
		return -1;
	}


} // namespace Maki
