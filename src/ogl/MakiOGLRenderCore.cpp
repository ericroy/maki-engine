#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLRenderCore.h"
#include "ogl/MakiOGLGPUTypes.h"
#include "ogl/MakiOGLCommon.h"

using namespace Maki::Core;

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
			if(debugOutput) {
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
			std::lock_guard<std::mutex> lock(mutex);
			if(SDL_GL_MakeCurrent(window->window, renderThreadContext) != 0) {
				Console::Error("Failed to make ogl render context current: %s", SDL_GetError());
				SDL_ClearError();
			}

			SDL_GL_SetSwapInterval(vsync ? 1 : 0);

			if(debugOutput) {
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
			std::lock_guard<std::mutex> lock(mutex);
			SDL_GL_SwapWindow(window->window);
		}



		// Resource creation, deletion, modification:

		void *OGLRenderCore::UploadBuffer(void *buffer, VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic, bool lengthChanged)
		{
			std::lock_guard<std::mutex> lock(mutex);

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
			std::lock_guard<std::mutex> lock(mutex);

			if(buffer != nullptr) {
				Buffer *b = (Buffer *)buffer;
				b->DeleteBuffers();
				delete b;
			}
		}


		bool OGLRenderCore::CreateShader(GLenum shaderType, Shader *s)
		{
			GPUShader *gs = new GPUShader();

			gs->sh = glCreateShader(shaderType);
			if(MAKI_OGL_FAILED()) { goto failed; }

			int32 length = (int32)s->programDataBytes;
			glShaderSource(gs->sh, 1, (const GLchar **)&s->programData, &length);
			if(MAKI_OGL_FAILED()) { goto failed; }
			
			glCompileShader(gs->sh);
			if(MAKI_OGL_FAILED()) { goto failed; }

			GLint infoLogLength;
			glGetShaderiv(gs->sh, GL_INFO_LOG_LENGTH, &infoLogLength);
			if(infoLogLength > 0) {
				char *buffer = new char[infoLogLength+1];
				glGetShaderInfoLog(gs->sh, infoLogLength, NULL, buffer);
				bool empty = true;
				char *p = buffer;
				while(*p != 0) {
					if(*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') {
						empty = false;
						break;
					}
					p++;
				}
				if(!empty) {
					Console::Info("Program info log:");
					Console::Info(buffer);
				}
				delete[] buffer;
			}

			GLint compileStatus;
			glGetShaderiv(gs->sh, GL_COMPILE_STATUS, &compileStatus);
			if(MAKI_OGL_FAILED()) { goto failed; }
			if(compileStatus == GL_FALSE) {
				Console::Error("Failed to compile glsl %s shader", shaderType == GL_FRAGMENT_SHADER ? "pixel" : "vertex");
				goto failed;
			}

			if(s->frameUniformBufferLocation != -1) {
				glGenBuffers(1, &gs->uboPerFrame);
				glBindBuffer(GL_UNIFORM_BUFFER, gs->uboPerFrame);
				glBufferData(GL_UNIFORM_BUFFER, s->engineFrameUniformBytes, 0, GL_STREAM_DRAW);
				glBindBufferBase(GL_UNIFORM_BUFFER, s->frameUniformBufferLocation, gs->uboPerFrame);
			}
			if(s->objectUniformBufferLocation != -1) {
				glGenBuffers(1, &gs->uboPerObject);
				glBindBuffer(GL_UNIFORM_BUFFER, gs->uboPerObject);
				glBufferData(GL_UNIFORM_BUFFER, s->engineObjectUniformBytes, 0, GL_STREAM_DRAW);
				glBindBufferBase(GL_UNIFORM_BUFFER, s->objectUniformBufferLocation, gs->uboPerObject);
			}
			if(s->materialUniformBufferLocation != -1) {
				glGenBuffers(1, &gs->uboMaterial);
				glBindBuffer(GL_UNIFORM_BUFFER, gs->uboMaterial);
				glBufferData(GL_UNIFORM_BUFFER, s->materialUniformBytes, 0, GL_STREAM_DRAW);
				glBindBufferBase(GL_UNIFORM_BUFFER, s->materialUniformBufferLocation, gs->uboMaterial);
			}
			if(MAKI_OGL_FAILED()) { goto failed; }

			int32 largestBuffer = std::max(std::max(s->materialUniformBytes, s->engineObjectUniformBytes), s->engineFrameUniformBytes);
			gs->scratchBuffer = (char *)Allocator::Malloc(largestBuffer, 16);

			s->handle = (intptr_t)gs;
			return true;
failed:
			SAFE_DELETE(gs);
			return false;
		}

		bool OGLRenderCore::CreateShaderProgram(ShaderProgram *s)
		{
			std::lock_guard<std::mutex> lock(mutex);

			if(!CreateShader(GL_VERTEX_SHADER, &s->vertexShader)) {
				MAKI_OGL_FAILED();
				return false;
			}
			if(!CreateShader(GL_FRAGMENT_SHADER, &s->pixelShader)) {
				MAKI_OGL_FAILED();
				return false;
			}

			GLuint program = glCreateProgram();
			if(MAKI_OGL_FAILED()) { goto failed; }

			glAttachShader(program, (GLuint)((GPUShader *)s->pixelShader.handle)->sh);
			glAttachShader(program, (GLuint)((GPUShader *)s->vertexShader.handle)->sh);
			if(MAKI_OGL_FAILED()) { goto failed; }

			for(uint32 i = 0; i < VertexFormat::AttributeCount; i++) {
				glBindAttribLocation(program, i, attributeName[i]);
				if(MAKI_OGL_FAILED()) { goto failed; }
			}

			glLinkProgram(program);
			if(MAKI_OGL_FAILED()) { goto failed; }
			
			GLint infoLogLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
			if(infoLogLength > 0) {
				char *buffer = new char[infoLogLength+1];
				glGetProgramInfoLog(program, infoLogLength, NULL, buffer);
				bool empty = true;
				char *p = buffer;
				while(*p != 0) {
					if(*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') {
						empty = false;
						break;
					}
					p++;
				}
				if(!empty) {
					Console::Info("Program info log:");
					Console::Info(buffer);
				}
				delete[] buffer;
			}

			GLint linkStatus;
			glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
			if(MAKI_OGL_FAILED()) { goto failed; }
			if(linkStatus == GL_FALSE) {
				Console::Error("Failed to link glsl program");
				goto failed;
			}

			// Lookup texture sampler locations
			// Arbitrarily, we'll decide to store the sampler locations in the vertex shader's array
			glUseProgram(program);
			GPUShader *gvs = (GPUShader *)s->vertexShader.handle;
			char buffer[32];
			for(uint32 i = 0; i < SHADOW_MAP_SLOT_INDEX_START; i++) {
				sprintf_s(buffer, "uSampler%d", i);
				GLint location = glGetUniformLocation(program, buffer);
				if(location >= 0) {
					glUniform1i(location, i);
				}
				gvs->textureSamplerLocations[i] = location;
			}
			for(uint32 i = 0; i < Core::RenderState::MAX_LIGHTS; i++) {
				sprintf_s(buffer, "uShadowSampler%d", i);
				GLint location = glGetUniformLocation(program, buffer);
				if(location >= 0) {
					glUniform1i(location, i);
				}
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
			std::lock_guard<std::mutex> lock(mutex);

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
			std::lock_guard<std::mutex> lock(mutex);

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
			std::lock_guard<std::mutex> lock(mutex);

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
			std::lock_guard<std::mutex> lock(mutex);

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

			// TODO:
			// Need to determine internal format here - probably need to borrow code from directx dds loader
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
			std::lock_guard<std::mutex> lock(mutex);

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
			std::lock_guard<std::mutex> lock(mutex);

			GPUShader *gvs = (GPUShader *)s->vertexShader.handle;
			SAFE_DELETE(gvs);
			s->vertexShader.handle = (intptr_t)nullptr;

			GPUShader *gps = (GPUShader *)s->pixelShader.handle;
			SAFE_DELETE(gps);
			s->pixelShader.handle = (intptr_t)nullptr;
		}

		void OGLRenderCore::DeleteTexture(Texture *t)
		{
			std::lock_guard<std::mutex> lock(mutex);

			GPUTexture *gtex = (GPUTexture *)t->handle;
			SAFE_DELETE(gtex);
			t->handle = (intptr_t)nullptr;
		}


	} // namespace OGL

} // namespace Maki
