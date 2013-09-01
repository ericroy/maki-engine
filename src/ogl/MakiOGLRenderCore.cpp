#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLRenderCore.h"
#include "ogl/MakiOGLGPUTypes.h"
#include "ogl/MakiOGLCommon.h"
#include <algorithm>

using namespace Maki::Core;

#if MAKI_USE_OGL_MUTEX
#define MAKI_ACQUIRE_OGL_MUTEX std::lock_guard<std::mutex> lock(mutex);
#else
#define MAKI_ACQUIRE_OGL_MUTEX
#endif

namespace Maki
{
	namespace OGL
	{

		GLvoid APIENTRY OGLDebugMessageHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *userParam)
		{
			Console::Info("GL DEBUG: %s", message);
		}

		OGLRenderCore::OGLRenderCore(Window *window, const Config *config)
			:	RenderCore(),
			window(window),
			mainThreadContext(nullptr),
			renderThreadContext(nullptr),
			frameBuffer(0),
			vao(0),
			defaultRenderTarget(0),
			defaultDepthStencil(0),
			currentRenderTarget(0),
			currentRenderTargetType(RenderState::RenderTarget_Default),
			currentDepthStencil(0),
			currentDepthStencilType(RenderState::DepthStencil_Default),
			depthWriteEnabled(true),
			blendEnabled(false),
			currentDepthTest(RenderState::DepthTest_Less),
			currentCullMode(RenderState::CullMode_Back),
			debugOutput(false)
		{
			int32 displayIndex = SDL_GetWindowDisplayIndex(window->window);
			int32 modeCount = SDL_GetNumDisplayModes(displayIndex);
			SDL_DisplayMode mode;
			bool found = false;
			for(int32 i = 0; i < modeCount; i++) {
				SDL_GetDisplayMode(displayIndex, i, &mode);
				bool resMatch = mode.w == window->width && mode.h == window->height;
				Console::Info("Supported mode: %dx%d @ %d Hz %s", mode.w, mode.h, mode.refresh_rate, resMatch ? "<<< Found!" : "");
				if(resMatch) {
					found = true;
				}
			}
			if(!found)
			{
				Console::Warning("Mode not supported: %dx%d", window->width, window->height);
			}
			
			vsync = config->GetBool("engine.vsync", false);

			int32 major = config->GetInt("ogl.major_version", 3);
			int32 minor = config->GetInt("ogl.minor_version", 1);

			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
			SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
			
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

			if(config->GetBool("ogl.require_hw_accel", false)) {
				Console::Info("Requiring OpenGL hardware acceleration");
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_ACCELERATED_VISUAL);
			}
			if(config->GetBool("ogl.debug_context", false)) {
				Console::Info("Requesting OpenGL debug context");
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
			}


			Console::Info("Creating OpenGL %d.%d contexts", major, minor);

			mainThreadContext = SDL_GL_CreateContext(window->window);
			if(mainThreadContext == nullptr) {
				Console::Error("Failed to create OpenGL main context: %s", SDL_GetError());
				SDL_ClearError();
			}

			SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
			
			renderThreadContext = SDL_GL_CreateContext(window->window);
			if(renderThreadContext == nullptr) {
				Console::Error("Failed to create OpenGL render context: %s", SDL_GetError());
				SDL_ClearError();
			}
			
			if(SDL_GL_MakeCurrent(window->window, mainThreadContext) != 0) {
				Console::Error("Failed to make OpenGL main context current: %s", SDL_GetError());
				SDL_ClearError();
			}

			DefineGLFunctions();

			debugOutput = config->GetBool("ogl.debug_messages", false);
			if(debugOutput && glDebugMessageCallback != nullptr) {
				Console::Info("Registering for OpenGL debug messages");
				// Register debug callback for main thread's context
				glDebugMessageCallback(OGLDebugMessageHandler, nullptr);
				glEnable(GL_DEBUG_OUTPUT);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			}
		}

		OGLRenderCore::~OGLRenderCore() {
			glDeleteVertexArrays(1, &vao);
			glDeleteFramebuffers(1, &frameBuffer);
			SDL_GL_DeleteContext(mainThreadContext);
			SDL_GL_DeleteContext(renderThreadContext);
			Console::Info("OpenGL renderer destroyed");
		}

		void OGLRenderCore::Init()
		{
			MAKI_ACQUIRE_OGL_MUTEX
			if(SDL_GL_MakeCurrent(window->window, renderThreadContext) != 0) {
				Console::Error("Failed to make ogl render context current: %s", SDL_GetError());
				SDL_ClearError();
			}

			SDL_GL_SetSwapInterval(vsync ? 1 : 0);

			if(debugOutput && glDebugMessageCallback != nullptr) {
				// Register debug callback for render thread's context
				glDebugMessageCallback(OGLDebugMessageHandler, nullptr);
				glEnable(GL_DEBUG_OUTPUT);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			}

			
			// Set initial state:
			windowWidth = 1;
			windowHeight = 1;
			glViewport(0, 0, windowWidth, windowHeight);

			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);
			glEnable(GL_DEPTH_TEST);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			
			glEnable(GL_BLEND);


			// Render a blank frame so we don't see a flash of white on startup
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			SDL_GL_SwapWindow(window->window);

			glGenFramebuffers(1, &frameBuffer);

			// Using one global VAO, because I read in some valve presentation that switching VAOs is typically slower than 
			// just making glVertexAttribPointer calls each time.  I should probably test this out...
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
		}

		void OGLRenderCore::Resized(uint32 newWidth, uint32 newHeight)
		{
		}

		void OGLRenderCore::Present()
		{
			MAKI_ACQUIRE_OGL_MUTEX
			SDL_GL_SwapWindow(window->window);
		}



		// Resource creation, deletion, modification:

		void *OGLRenderCore::UploadBuffer(void *buffer, VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic, bool lengthChanged)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			Buffer *b = (Buffer *)buffer;
			bool reuse = b != nullptr;

			if(reuse) {
				if(lengthChanged) {
					b->DeleteBuffers();
					glGenBuffers(2, b->vbos);
				}
			} else {
				b = new Buffer();
				memset(b, 0, sizeof(Buffer));
				glGenBuffers(2, b->vbos);
			}

			b->vertexCount = vertexCount;
			b->faceCount = faceCount;
			b->indicesPerFace = indicesPerFace;
			b->bytesPerIndex = bytesPerIndex;

			assert(indicesPerFace > 0 && indicesPerFace <= 3);
			b->geometryType = indicesPerFaceToGeometryType[indicesPerFace];
		
			assert(bytesPerIndex > 0 && bytesPerIndex <= 4 && bytesPerIndex != 3);
			b->indexDataType = bytesPerIndexToFormat[bytesPerIndex];

			glBindBuffer(GL_ARRAY_BUFFER, b->vbos[0]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->vbos[1]);

			int32 stride = vf->GetStride();
			if(reuse && !lengthChanged) {
				glBufferSubData(GL_ARRAY_BUFFER, 0, stride*vertexCount, vertexData);
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, bytesPerIndex*indicesPerFace*faceCount, indexData);
			} else {
				glBufferData(GL_ARRAY_BUFFER, stride*vertexCount, vertexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytesPerIndex*indicesPerFace*faceCount, indexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			}

			// Must finish here to ensure that vbos have actually received the data before the render thread tries to draw with them
			glFinish();
			
			return (void *)b;
		}

		void OGLRenderCore::FreeBuffer(void *buffer)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			if(buffer != nullptr) {
				Buffer *b = (Buffer *)buffer;
				b->DeleteBuffers();
				delete b;
			}
		}


		bool OGLRenderCore::CreateShader(GLenum shaderType, Shader *s)
		{
			GPUShader *gs = nullptr;
			if(shaderType == GL_VERTEX_SHADER) {
				gs = new GPUVertexShader();
			} else {
				gs = new GPUPixelShader();
			}

			gs->sh = glCreateShader(shaderType);
			if(MAKI_OGL_FAILED()) { goto failed; }

			int32 length = (int32)s->programDataBytes;
			glShaderSource(gs->sh, 1, (const GLchar **)&s->programData, &length);
			if(MAKI_OGL_FAILED()) { goto failed; }
			
			glCompileShader(gs->sh);
			if(MAKI_OGL_FAILED()) { goto failed; }

			GLint infoLogLength;
			glGetShaderiv(gs->sh, GL_INFO_LOG_LENGTH, &infoLogLength);
			if(infoLogLength > 1) {
				char *buffer = new char[infoLogLength+1];
				glGetShaderInfoLog(gs->sh, infoLogLength, NULL, buffer);
				Console::Info("Program info log:");
				Console::Info(buffer);
				delete[] buffer;
			}

			GLint compileStatus;
			glGetShaderiv(gs->sh, GL_COMPILE_STATUS, &compileStatus);
			if(MAKI_OGL_FAILED()) { goto failed; }
			if(compileStatus == GL_FALSE) {
				Console::Error("Failed to compile glsl %s shader", shaderType == GL_FRAGMENT_SHADER ? "pixel" : "vertex");
				goto failed;
			}

			s->handle = (intptr_t)gs;
			return true;
failed:
			SAFE_DELETE(gs);
			return false;
		}

		bool OGLRenderCore::CreateShaderProgram(ShaderProgram *s)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			if(!CreateShader(GL_VERTEX_SHADER, &s->vertexShader)) {
				MAKI_OGL_FAILED();
				return false;
			}
			if(!CreateShader(GL_FRAGMENT_SHADER, &s->pixelShader)) {
				MAKI_OGL_FAILED();
				return false;
			}

			GLuint program = glCreateProgram();
			glAttachShader(program, (GLuint)((GPUShader *)s->pixelShader.handle)->sh);
			glAttachShader(program, (GLuint)((GPUShader *)s->vertexShader.handle)->sh);
			for(uint32 i = 0; i < VertexFormat::AttributeCount; i++) {
				glBindAttribLocation(program, i, attributeName[i]);
			}
			glLinkProgram(program);
			if(MAKI_OGL_FAILED()) { goto failed; }
			
			GLint infoLogLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
			if(infoLogLength > 1) {
				char *buffer = new char[infoLogLength+1];
				glGetProgramInfoLog(program, infoLogLength, NULL, buffer);
				Console::Info("Program info log:");
				Console::Info(buffer);
				delete[] buffer;
			}

			GLint linkStatus;
			glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
			if(MAKI_OGL_FAILED()) { goto failed; }
			if(linkStatus == GL_FALSE) {
				Console::Error("Failed to link glsl program");
				goto failed;
			}
			assert(glIsProgram(program));


			GPUVertexShader *gvs = (GPUVertexShader *)s->vertexShader.handle;

			// Regarding the nastiness below:
			// Most of this info is supposed to be provided in the mshad files that describe the shader.  However,
			// since glsl shaders are not meant to be pre-compiled, we need to resolve this stuff at runtime.
			//
			// This is totally gross since it modifies the cpu-side shader object.

			s->vertexShader.frameUniformBufferLocation = glGetUniformBlockIndex(program, "enginePerFrame");
			if(s->vertexShader.frameUniformBufferLocation != -1) {
				memset(s->vertexShader.engineFrameUniformLocations, 0xff, sizeof(s->vertexShader.engineFrameUniformLocations));
				memset(s->pixelShader.engineFrameUniformLocations, 0xff, sizeof(s->pixelShader.engineFrameUniformLocations));

				GLint uniformBlockSize = 0;
				glGetActiveUniformBlockiv(program, s->vertexShader.frameUniformBufferLocation, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlockSize);
				s->vertexShader.engineFrameUniformBytes = uniformBlockSize;

				glGenBuffers(1, &gvs->uboPerFrame);
				glBindBuffer(GL_UNIFORM_BUFFER, gvs->uboPerFrame);
				glBufferData(GL_UNIFORM_BUFFER, s->vertexShader.engineFrameUniformBytes, 0, GL_STREAM_DRAW);
				glUniformBlockBinding(program, s->vertexShader.frameUniformBufferLocation, UniformBuffer_Frame);

				GLint activeUniformCount = 0;
				glGetActiveUniformBlockiv(program, s->vertexShader.frameUniformBufferLocation, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUniformCount);

				std::vector<GLint> indices(activeUniformCount, 0);
				std::vector<GLint> offsets(activeUniformCount, 0);
				std::vector<GLint> nameLengths(activeUniformCount, 0);

				glGetActiveUniformBlockiv(program, s->vertexShader.frameUniformBufferLocation, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data());
				glGetActiveUniformsiv(program, activeUniformCount, (GLuint *)indices.data(), GL_UNIFORM_OFFSET, offsets.data());
				glGetActiveUniformsiv(program, activeUniformCount, (GLuint *)indices.data(), GL_UNIFORM_NAME_LENGTH, nameLengths.data());

				GLint maxNameLength = *std::max_element(nameLengths.begin(), nameLengths.end());
				char *nameBuffer = new char[maxNameLength+1];

				for(int32 i = 0; i < activeUniformCount; i++) {
					GLint size = 0;
					GLenum type = 0;
					glGetActiveUniform(program, indices[i], maxNameLength+1, nullptr, &size, &type, nameBuffer);

					char *bracket = strchr(nameBuffer, '[');
					if(bracket != nullptr) {
						*bracket = 0;
					}

					Shader::FrameUniform uni = Shader::GetFrameUniformByName(nameBuffer);
					if(s->vertexShader.engineFrameUniformLocations[uni] == -1) {
						s->vertexShader.engineFrameUniformLocations[uni] = offsets[i];
						s->pixelShader.engineFrameUniformLocations[uni] = offsets[i];
					}
				}
				SAFE_DELETE(nameBuffer);
			}

			s->vertexShader.objectUniformBufferLocation = glGetUniformBlockIndex(program, "enginePerObject");
			if(s->vertexShader.objectUniformBufferLocation != -1) {
				memset(s->vertexShader.engineObjectUniformLocations, 0xff, sizeof(s->vertexShader.engineObjectUniformLocations));
				memset(s->pixelShader.engineObjectUniformLocations, 0xff, sizeof(s->pixelShader.engineObjectUniformLocations));

				GLint uniformBlockSize = 0;
				glGetActiveUniformBlockiv(program, s->vertexShader.objectUniformBufferLocation, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlockSize);
				s->vertexShader.engineObjectUniformBytes = uniformBlockSize;

				glGenBuffers(1, &gvs->uboPerObject);
				glBindBuffer(GL_UNIFORM_BUFFER, gvs->uboPerObject);
				glBufferData(GL_UNIFORM_BUFFER, s->vertexShader.engineObjectUniformBytes, 0, GL_STREAM_DRAW);
				glUniformBlockBinding(program, s->vertexShader.objectUniformBufferLocation, UniformBuffer_Object);
				
				GLint activeUniformCount = 0;
				glGetActiveUniformBlockiv(program, s->vertexShader.objectUniformBufferLocation, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUniformCount);
				
				std::vector<GLint> indices(activeUniformCount, 0);
				std::vector<GLint> offsets(activeUniformCount, 0);
				std::vector<GLint> nameLengths(activeUniformCount, 0);

				glGetActiveUniformBlockiv(program, s->vertexShader.objectUniformBufferLocation, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data());
				glGetActiveUniformsiv(program, activeUniformCount, (GLuint *)indices.data(), GL_UNIFORM_OFFSET, offsets.data());
				glGetActiveUniformsiv(program, activeUniformCount, (GLuint *)indices.data(), GL_UNIFORM_NAME_LENGTH, nameLengths.data());

				GLint maxNameLength = *std::max_element(nameLengths.begin(), nameLengths.end());
				char *nameBuffer = new char[maxNameLength+1];

				for(int32 i = 0; i < activeUniformCount; i++) {
					GLint size = 0;
					GLenum type = 0;
					glGetActiveUniform(program, indices[i], maxNameLength+1, nullptr, &size, &type, nameBuffer);

					char *bracket = strchr(nameBuffer, '[');
					if(bracket != nullptr) {
						*bracket = 0;
					}

					Shader::ObjectUniform uni = Shader::GetObjectUniformByName(nameBuffer);
					if(s->vertexShader.engineObjectUniformLocations[uni] == -1) {
						s->vertexShader.engineObjectUniformLocations[uni] = offsets[i];
						s->pixelShader.engineObjectUniformLocations[uni] = offsets[i];
					}
				}
				SAFE_DELETE(nameBuffer);
			}

			s->vertexShader.materialUniformBufferLocation = glGetUniformBlockIndex(program, "material");
			if(s->vertexShader.materialUniformBufferLocation != -1) {
				s->vertexShader.materialUniformLocations.clear();
				s->pixelShader.materialUniformLocations.clear();

				GLint uniformBlockSize = 0;
				glGetActiveUniformBlockiv(program, s->vertexShader.materialUniformBufferLocation, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlockSize);
				s->vertexShader.materialUniformBytes = uniformBlockSize;

				glGenBuffers(1, &gvs->uboMaterial);
				glBindBuffer(GL_UNIFORM_BUFFER, gvs->uboMaterial);
				glBufferData(GL_UNIFORM_BUFFER, s->vertexShader.materialUniformBytes, 0, GL_STREAM_DRAW);
				glUniformBlockBinding(program, s->vertexShader.materialUniformBufferLocation, UniformBuffer_Material);
				
				GLint activeUniformCount = 0;
				glGetActiveUniformBlockiv(program, s->vertexShader.materialUniformBufferLocation, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUniformCount);

				std::vector<GLint> indices(activeUniformCount, 0);
				std::vector<GLint> offsets(activeUniformCount, 0);
				std::vector<GLint> nameLengths(activeUniformCount, 0);

				glGetActiveUniformBlockiv(program, s->vertexShader.materialUniformBufferLocation, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data());
				glGetActiveUniformsiv(program, activeUniformCount, (GLuint *)indices.data(), GL_UNIFORM_OFFSET, offsets.data());
				glGetActiveUniformsiv(program, activeUniformCount, (GLuint *)indices.data(), GL_UNIFORM_NAME_LENGTH, nameLengths.data());

				GLint maxNameLength = *std::max_element(nameLengths.begin(), nameLengths.end());
				char *nameBuffer = new char[maxNameLength+1];

				for(int32 i = 0; i < activeUniformCount; i++) {
					GLint size = 0;
					GLenum type = 0;
					glGetActiveUniform(program, indices[i], maxNameLength+1, nullptr, &size, &type, nameBuffer);

					char *bracket = strchr(nameBuffer, '[');
					if(bracket != nullptr) {
						*bracket = 0;
					}

					int32 index = s->vertexShader.FindMaterialConstantLocation(nameBuffer);
					if(index == -1) {
						Shader::MaterialUniformLocation uni(offsets[i], nameBuffer);
						s->vertexShader.materialUniformLocations.push_back(uni);
						s->pixelShader.materialUniformLocations.push_back(uni);
					}
				}
				SAFE_DELETE(nameBuffer);
			}

			if(MAKI_OGL_FAILED()) { goto failed; }

			int32 largestBuffer = std::max(std::max(s->vertexShader.materialUniformBytes, s->vertexShader.engineObjectUniformBytes), s->vertexShader.engineFrameUniformBytes);
			gvs->scratchBuffer = (char *)Allocator::Malloc(largestBuffer, 16);

			// Lookup texture sampler locations
			// Arbitrarily, we'll decide to store the sampler locations in the vertex shader's array
			char buffer[32];
			for(uint32 i = 0; i < SHADOW_MAP_SLOT_INDEX_START; i++) {
				sprintf(buffer, "uSampler%d", i);
				GLint location = glGetUniformLocation(program, buffer);
				gvs->textureSamplerLocations[i] = location;
			}
			for(uint32 i = 0; i < Core::RenderState::MAX_LIGHTS; i++) {
				sprintf(buffer, "uShadowSampler[%d]", i);
				GLint location = glGetUniformLocation(program, buffer);
				gvs->textureSamplerLocations[SHADOW_MAP_SLOT_INDEX_START+i] = location;
			}

			s->handle = (intptr_t)program;
			return true;

failed:
			glDeleteProgram(program);
			return false;
		}

	

		bool OGLRenderCore::CreateEmptyTexture(Texture *t, uint8 channels)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			if(channels == 0 || channels > 4 || channels == 3) {
				Console::Error("Unsupported number of channels in image: %d", channels);
				goto failed;
			}

			GLuint tex = 0;
			glGenTextures(1, &tex);
			if(MAKI_OGL_FAILED()) { goto failed; }

			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, channels, t->width, t->height, 0, channelsToFormat[channels], GL_UNSIGNED_BYTE, nullptr);
			if(MAKI_OGL_FAILED()) { goto failed; }

			GPUTexture *gtex = new GPUTexture();
			gtex->tex = tex;
			t->handle = (intptr_t)gtex;
			return true;

failed:
			glDeleteTextures(1, &tex);
			return false;
		}

		bool OGLRenderCore::CreateRenderTarget(Texture *t)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			GLuint tex = 0;
			glGenTextures(1, &tex);
			if(MAKI_OGL_FAILED()) { goto failed; }

			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->width, t->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			if(MAKI_OGL_FAILED()) { goto failed; }

			GPUTexture *gtex = new GPUTexture();
			gtex->tex = tex;
			t->handle = (intptr_t)gtex;
			return true;
failed:
			glDeleteTextures(1, &tex);
			return false;
		}

		bool OGLRenderCore::CreateDepthTexture(Texture *t)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			GLuint tex = 0;
			glGenRenderbuffers(1, &tex);
			glBindRenderbuffer(GL_RENDERBUFFER, tex);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, t->width, t->height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			if(MAKI_OGL_FAILED()) { goto failed; }

			GPUTexture *gtex = new GPUTexture();
			gtex->tex = tex;
			t->handle = (intptr_t)gtex;
			return true;

failed:
			glDeleteTextures(1, &tex);
			return false;
		}

		bool OGLRenderCore::CreateTexture(Texture *t, char *data, uint32 dataLength)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			const void *dataOut = nullptr;
			unsigned long dataLengthOut = 0;
            uint32 format = 0;
			uint32 mipLevels = 0;
			int32 ret = MOJODDS_getTexture(data, dataLength, &dataOut, &dataLengthOut, &format, &t->width, &t->height, &mipLevels);
			if(ret == 0) {
				Console::Error("Failed to mojo-load dds file");
				goto failed;
			}

			GLuint tex = 0;
			glGenTextures(1, &tex);
			if(MAKI_OGL_FAILED()) { goto failed; }

			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if(MAKI_OGL_FAILED()) { goto failed; }

			switch(format)
			{
			case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, t->width, t->height, 0, dataLengthOut, dataOut);
				break;
			case GL_RGB:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->width, t->height, 0, GL_RGB, GL_UNSIGNED_BYTE, dataOut);
				break;
			case GL_RGBA:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->width, t->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataOut);
				break;
			case GL_LUMINANCE:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->width, t->height, 0, GL_RED, GL_UNSIGNED_BYTE, dataOut);
				break;
			default:
				Console::Error("Failed to load texture, not a supported internal pixel format");
				goto failed;
			}
			if(MAKI_OGL_FAILED()) { goto failed; }

			glGenerateMipmap(GL_TEXTURE_2D);
			if(MAKI_OGL_FAILED()) { goto failed; }

			GPUTexture *gtex = new GPUTexture();
			gtex->tex = tex;
			t->handle = (intptr_t)gtex;
			return true;

failed:
			glDeleteTextures(1, &tex);
			return false;
		}

		void OGLRenderCore::WriteToTexture(Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			GPUTexture *gtex = (GPUTexture *)t->handle;
			glBindTexture(GL_TEXTURE_2D, gtex->tex);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, srcPitch/channels);

			GLenum format = channelsToFormat[channels];

			// Find the data at point (srcX, srcY) in the source image
			char *p = srcData + (srcY*srcPitch) + (srcX*channels);

			glTexSubImage2D(GL_TEXTURE_2D, 0, dstX, dstY, (GLsizei)srcWidth, (GLsizei)srcHeight, format, GL_UNSIGNED_BYTE, p);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		}

		void OGLRenderCore::DeleteShaderProgram(ShaderProgram *s)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			Console::Info("Deleting shader: <rid %u> <variant %d>", s->rid, s->variant);

			GPUShader *gvs = (GPUShader *)s->vertexShader.handle;
			SAFE_DELETE(gvs);
			s->vertexShader.handle = (intptr_t)nullptr;

			GPUShader *gps = (GPUShader *)s->pixelShader.handle;
			SAFE_DELETE(gps);
			s->pixelShader.handle = (intptr_t)nullptr;
		}

		void OGLRenderCore::DeleteTexture(Texture *t)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			GPUTexture *gtex = (GPUTexture *)t->handle;
			SAFE_DELETE(gtex);
			t->handle = (intptr_t)nullptr;
		}


	} // namespace OGL

} // namespace Maki
