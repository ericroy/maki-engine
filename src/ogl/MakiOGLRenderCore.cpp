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
			defaultRenderTarget(0),
			defaultDepthStencil(0),
			currentRenderTarget(0),
			currentRenderTargetType(RenderState::RenderTarget_Default),
			currentDepthStencil(0),
			currentDepthStencilType(RenderState::DepthStencil_Default)
		{
			

			vsync = config->GetBool("engine.vertical_sync", true);
			// -1 allows late swaps to happen immediately
			SDL_GL_SetSwapInterval(vsync ? -1 : 0);

			int32 major = config->GetInt("engine.ogl_major_version", 3);
			int32 minor = config->GetInt("engine.ogl_minor_version", 1);

			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

			if(config->GetInt("engine.ogl_forward_compatible", true)) {
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
			}
#if _DEBUG
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

			Console::Info("Creating OpenGL %d.%d contexts", major, minor);

			mainThreadContext = SDL_GL_CreateContext(window->window);
			if(mainThreadContext == nullptr) {
				Console::Error("Failed to create ogl main context: %s", SDL_GetError());
				SDL_ClearError();
			}

			SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
			
			renderThreadContext = SDL_GL_CreateContext(window->window);
			if(renderThreadContext == nullptr) {
				Console::Error("Failed to create ogl render context: %s", SDL_GetError());
				SDL_ClearError();
			}
			
			if(SDL_GL_MakeCurrent(window->window, mainThreadContext) != 0) {
				Console::Error("Failed to make ogl main context current: %s", SDL_GetError());
				SDL_ClearError();
			}

			DefineGLFunctions();

#if _DEBUG
			// Register debug callback for main thread's context
			glDebugMessageCallback(OGLDebugMessageHandler, nullptr);
#endif
		}

		OGLRenderCore::~OGLRenderCore() {
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

#if _DEBUG
			// Register debug callback for render thread's context
			glDebugMessageCallback(OGLDebugMessageHandler, nullptr);
#endif
			
			// Set initial state:
			windowWidth = 1;
			windowHeight = 1;
			glViewport(0, 0, windowWidth, windowHeight);

			// Render a blank frame so we don't see a flash of white on startup
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			SDL_GL_SwapWindow(window->window);

			glGenFramebuffers(1, &frameBuffer);
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

		void *OGLRenderCore::UploadBuffer(void *buffer, VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic)
		{
			std::lock_guard<std::mutex> lock(mutex);

			Buffer *b = (Buffer *)buffer;
			if(b == nullptr) {
				b = new Buffer();
				memset(b, 0, sizeof(Buffer));
				buffer = b;
			} else {
				b->DeleteBuffers();
			}

			glGenBuffers(2, b->vbos);
			if(MAKI_OGL_FAILED()) { goto failed; }

			b->vertexCount = vertexCount;
			b->faceCount = faceCount;
			b->indicesPerFace = indicesPerFace;
			b->bytesPerIndex = bytesPerIndex;

			assert(indicesPerFace > 0 && indicesPerFace <= 3);
			b->geometryType = indicesPerFaceToGeometryType[indicesPerFace];
		
			assert(bytesPerIndex > 0 && bytesPerIndex <= 4 && bytesPerIndex != 3);
			b->indexDataType = bytesPerIndexToFormat[bytesPerIndex];

			int32 stride = vf->GetStride();

			// Setup the vertex buffer
			glBindBuffer(GL_ARRAY_BUFFER, b->vbos[0]);
			glBufferData(GL_ARRAY_BUFFER, stride*vertexCount, vertexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			if(MAKI_OGL_FAILED()) { goto failed; }

			// Setup the index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->vbos[1]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytesPerIndex*indicesPerFace*faceCount, indexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			if(MAKI_OGL_FAILED()) { goto failed; }

			return buffer;
failed:
			if(b != nullptr) {
				b->DeleteBuffers();
				delete b;
			}
			return nullptr;
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
			}
			if(s->objectUniformBufferLocation != -1) {
				glGenBuffers(1, &gs->uboPerObject);
				glBindBuffer(GL_UNIFORM_BUFFER, gs->uboPerObject);
				glBufferData(GL_UNIFORM_BUFFER, s->engineObjectUniformBytes, 0, GL_STREAM_DRAW);
			}
			if(s->materialUniformBufferLocation != -1) {
				glGenBuffers(1, &gs->uboMaterial);
				glBindBuffer(GL_UNIFORM_BUFFER, gs->uboMaterial);
				glBufferData(GL_UNIFORM_BUFFER, s->materialUniformBytes, 0, GL_STREAM_DRAW);
			}

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

			for(uint32 i = 0; i < VertexFormat::AttributeCount; i++) {
				glBindAttribLocation(program, i, attributeToSemanicName[i]);
			}

			glAttachShader(program, (GLuint)((GPUShader *)s->pixelShader.handle)->sh);
			glAttachShader(program, (GLuint)((GPUShader *)s->vertexShader.handle)->sh);
			if(MAKI_OGL_FAILED()) { goto failed; }

			if(MAKI_OGL_FAILED()) { goto failed; }

			glLinkProgram(program);
			if(MAKI_OGL_FAILED()) { goto failed; }

			for(uint32 i = 0; i < VertexFormat::AttributeCount; i++) {
				GLint location = glGetAttribLocation(program, attributeToSemanicName[i]);
				Console::Info("Attr semantic %s bound at %d  <rid %u>", attributeToSemanicName[i], location, s->rid);
			}


			GLuint enginePerFrame = glGetUniformBlockIndex(program, "enginePerFrame");
			GLuint enginePerObject = glGetUniformBlockIndex(program, "enginePerObject");
			GLuint material = glGetUniformBlockIndex(program, "material");
			MAKI_OGL_FAILED();

			GLint linkStatus;
			glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
			if(MAKI_OGL_FAILED()) { goto failed; }
			if(linkStatus == GL_FALSE) {
				Console::Error("Failed to link glsl program");
				goto failed;
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


			//glBindTexture(GL_TEXTURE_2D, tex);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_TEXTURE_BORDER);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_TEXTURE_BORDER);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, t->width, t->height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
			//if(MAKI_OGL_FAILED()) { goto failed; }

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

			const void *dataOut;
			unsigned long dataLengthOut;
            uint32 format;
			uint32 mipLevels;
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
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

			// TODO:
			// Need to determine internal format here - probably need to borrow code from directx dds loader
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, t->width, t->height, 0, dataLengthOut, dataOut);
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
