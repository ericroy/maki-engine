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
			currentDepthStencil(0)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, config->GetInt("engine.ogl_major_version", 2));
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, config->GetInt("engine.ogl_minor_version", 1));
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

#if _DEBUG
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

			vsync = config->GetBool("engine.vertical_sync", true);
			// -1 allows late swaps to happen immediately
			SDL_GL_SetSwapInterval(vsync ? -1 : 0);

			mainThreadContext = SDL_GL_CreateContext(window->window);

			SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
			renderThreadContext = SDL_GL_CreateContext(window->window);

			SDL_GL_MakeCurrent(window->window, mainThreadContext);
		}

		OGLRenderCore::~OGLRenderCore() {
			if(mainThreadContext != nullptr) {
				if(frameBuffer != 0) {
					glDeleteFramebuffers(1, &frameBuffer);
				}			
				SDL_GL_DeleteContext(mainThreadContext);
			}
			if(renderThreadContext != nullptr) {
				SDL_GL_DeleteContext(renderThreadContext);
			}			
			Console::Info("OpenGL renderer destroyed");
		}

		void OGLRenderCore::Init()
		{
			std::lock_guard<std::mutex> lock(mutex);

			SDL_GL_MakeCurrent(window->window, renderThreadContext);

			DefineGLFunctions();
			MAKI_OGL_FAILED();

#if _DEBUG
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
			glDebugMessageCallbackARB(OGLDebugMessageHandler, nullptr);
#endif

			Console::Info("GL Version: %s", (const char *)glGetString(GL_VERSION));

			glGenFramebuffers(1, &frameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

			// Set initial state:
			windowWidth = 1;
			windowHeight = 1;
			Resized(windowWidth, windowHeight);

			currentRenderTarget = defaultRenderTarget;
			currentDepthStencil = defaultDepthStencil;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currentRenderTarget, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, currentDepthStencil);
			MAKI_OGL_FAILED();

			glViewport(0, 0, window->width, window->height);
			MAKI_OGL_FAILED();

			// Render a blank frame so we don't see a flash of white on startup
			glClearColor(1, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			MAKI_OGL_FAILED();

			SDL_GL_SwapWindow(window->window);
			MAKI_OGL_FAILED();
		}

		void OGLRenderCore::Resized(uint32 newWidth, uint32 newHeight)
		{
			if(defaultRenderTarget != 0) {
				glDeleteTextures(1, &defaultRenderTarget);
			}
			if(defaultDepthStencil != 0) {
				glDeleteTextures(1, &defaultDepthStencil);
			}

			glGenTextures(1, &defaultRenderTarget);
			glBindTexture(GL_TEXTURE_2D, defaultRenderTarget);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);
			MAKI_OGL_FAILED();

			glGenRenderbuffers(1, &defaultDepthStencil);
			glBindRenderbuffer(GL_RENDERBUFFER, defaultDepthStencil);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, newWidth, newHeight);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			MAKI_OGL_FAILED();
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

			glGenBuffersARB(2, b->vbos);
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

			// Create vertex buffer
			glBindBufferARB(GL_ARRAY_BUFFER, b->vbos[0]);
			glBufferDataARB(GL_ARRAY_BUFFER, stride*vertexCount, vertexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			if(MAKI_OGL_FAILED()) { goto failed; }

			// Create index buffer
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, b->vbos[1]);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, bytesPerIndex*indicesPerFace*faceCount, indexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
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

		bool OGLRenderCore::CreatePixelShader(Shader *ps)
		{
			GPUPixelShader *gps = new GPUPixelShader();

			const MOJOSHADER_parseData *parseData = MOJOSHADER_parse(MOJOSHADER_PROFILE_GLSL120, (unsigned char *)ps->programData, ps->programDataBytes, nullptr, 0, nullptr, 0, nullptr, nullptr, nullptr);
			if(parseData->error_count > 0) {
				for(int32 i = 0; i < parseData->error_count; i++) {
					Console::Error("MOJOSHADER: %s", parseData->errors[i].error);
				}
				goto failed;
			}

			gps->ps = glCreateShader(GL_FRAGMENT_SHADER);
			if(MAKI_OGL_FAILED()) { goto failed;}

			int32 length = (int32)parseData->output_count;
			glShaderSourceARB(gps->ps, 1, (const GLchar **)&parseData->output, &length);
			MOJOSHADER_freeParseData(parseData);
			parseData = nullptr;
			if(MAKI_OGL_FAILED()) { goto failed; }			
			
			glCompileShaderARB(gps->ps);
			if(MAKI_OGL_FAILED()) { goto failed; }

			ps->handle = (intptr_t)gps;
			return true;

failed:
			
			if(parseData != nullptr) {
				MOJOSHADER_freeParseData(parseData);
			}
			if(gps != nullptr && gps->ps != 0) {
				glDeleteShader(gps->ps);
			}
			SAFE_DELETE(gps);
			return false;
		}

		bool OGLRenderCore::CreateVertexShader(Shader *vs)
		{
			GPUVertexShader *gvs = new GPUVertexShader();

			const MOJOSHADER_parseData *parseData = MOJOSHADER_parse(MOJOSHADER_PROFILE_GLSL120, (unsigned char *)vs->programData, vs->programDataBytes, nullptr, 0, nullptr, 0, nullptr, nullptr, nullptr);
			if(parseData->error_count > 0) {
				for(int32 i = 0; i < parseData->error_count; i++) {
					Console::Error("MOJOSHADER: %s", parseData->errors[i].error);
				}
				goto failed;
			}

			gvs->vs = glCreateShader(GL_VERTEX_SHADER);
			if(MAKI_OGL_FAILED()) { goto failed; }

			int32 length = (int32)vs->programDataBytes;
			glShaderSourceARB(gvs->vs, 1, (const GLchar **)&vs->programData, &length);
			MOJOSHADER_freeParseData(parseData);
			parseData = nullptr;
			if(MAKI_OGL_FAILED()) { goto failed; }
			
			glCompileShaderARB(gvs->vs);
			if(MAKI_OGL_FAILED()) { goto failed; }

			vs->handle = (intptr_t)gvs;
			return true;

failed:
			if(parseData != nullptr) {
				MOJOSHADER_freeParseData(parseData);
			}
			if(gvs != nullptr && gvs->vs != 0) {
				glDeleteShader(gvs->vs);
			}
			SAFE_DELETE(gvs);
			return false;
		}

		bool OGLRenderCore::CreateShaderProgram(ShaderProgram *s)
		{
			std::lock_guard<std::mutex> lock(mutex);

			if(!CreateVertexShader(&s->vertexShader)) {
				return false;
			}

			if(!CreatePixelShader(&s->pixelShader)) {
				return false;
			}

			GLuint program = glCreateProgram();
			if(MAKI_OGL_FAILED()) { goto failed; }

			glAttachShader(program, (GLuint)s->vertexShader.handle);
			if(MAKI_OGL_FAILED()) { goto failed; }

			glAttachShader(program, (GLuint)s->pixelShader.handle);
			if(MAKI_OGL_FAILED()) { goto failed; }

			glLinkProgramARB(program);
			if(MAKI_OGL_FAILED()) { goto failed; }

			s->handle = (intptr_t)program;
			return true;

failed:
			if(program != 0) {
				glDeleteProgram(program);
			}
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
			if(tex != 0) {
				glDeleteTextures(1, &tex);
			}
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
			if(tex != 0) {
				glDeleteTextures(1, &tex);
			}
			return false;
		}

		bool OGLRenderCore::CreateDepthTexture(Texture *t)
		{
			std::lock_guard<std::mutex> lock(mutex);

			GLuint tex = 0;
			glGenTextures(1, &tex);
			if(MAKI_OGL_FAILED()) { goto failed; }

			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_TEXTURE_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_TEXTURE_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, t->width, t->height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
			if(MAKI_OGL_FAILED()) { goto failed; }

			GPUTexture *gtex = new GPUTexture();
			gtex->tex = tex;
			t->handle = (intptr_t)gtex;
			return true;

failed:
			if(tex != 0) {
				glDeleteTextures(1, &tex);
			}
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
			glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, format, t->width, t->height, 0, dataLengthOut, dataOut);
			if(MAKI_OGL_FAILED()) { goto failed; }

			GPUTexture *gtex = new GPUTexture();
			gtex->tex = tex;
			t->handle = (intptr_t)gtex;
			return true;

failed:
			if(tex != 0) {
				glDeleteTextures(1, &tex);
			}
			return false;
		}

		void OGLRenderCore::WriteToTexture(Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData)
		{
			std::lock_guard<std::mutex> lock(mutex);

			GPUTexture *gtex = (GPUTexture *)t->handle;
			glBindTexture(GL_TEXTURE_2D, gtex->tex);
			MAKI_OGL_FAILED();

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			MAKI_OGL_FAILED();

			glPixelStorei(GL_UNPACK_ROW_LENGTH, srcPitch/channels);
			MAKI_OGL_FAILED();

			GLenum format = channelsToFormat[channels];

			// Find the data at point (srcX, srcY) in the source image
			char *p = srcData + (srcY*srcPitch) + (srcX*channels);

			glTexSubImage2D(GL_TEXTURE_2D, 0, dstX, dstY, (GLsizei)srcWidth, (GLsizei)srcHeight, format, GL_UNSIGNED_BYTE, p);
			MAKI_OGL_FAILED();

			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			MAKI_OGL_FAILED();
		}

		void OGLRenderCore::DeleteShaderProgram(ShaderProgram *s)
		{
			std::lock_guard<std::mutex> lock(mutex);

			GPUVertexShader *gvs = (GPUVertexShader *)s->vertexShader.handle;
			SAFE_DELETE(gvs);
			s->vertexShader.handle = (intptr_t)nullptr;

			GPUPixelShader *gps = (GPUPixelShader *)s->pixelShader.handle;
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
