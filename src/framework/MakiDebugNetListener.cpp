#include "framework/framework_stdafx.h"
#include "framework/MakiDebugNetListener.h"

#if defined(_WIN32) || defined(_WIN64)

#include <winsock2.h>
#include <Ws2tcpip.h>


namespace Maki
{
	namespace Framework
	{

		struct DebugNetListener::Context {
	#if _DEBUG
			WSADATA wsaData;
			SOCKET recvSocket;
			sockaddr_in recvAddr;
			sockaddr_in senderAddr;
	#endif
		};
	
		DebugNetListener::DebugNetListener()
		:	context(nullptr) {

	#if _DEBUG
			context = new Context();
			memset(context, 0, sizeof(context));
			context->recvSocket = INVALID_SOCKET;

			int ret = 0;

			// Initialize Winsock
			ret = WSAStartup(MAKEWORD(2, 2), &context->wsaData);
			if(ret != NO_ERROR) {
				Console::Error("WSAStartup failed with error %d", ret);
				return;
			}

			// Create a receiver socket to receive datagrams
			context->recvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if(context->recvSocket == INVALID_SOCKET) {
				Console::Error("socket failed with error %d", WSAGetLastError());
				return;
			}

			// Make the socket non-blocking
			unsigned long mode = 1;
			ioctlsocket(context->recvSocket, FIONBIO, &mode);

			// Bind the socket to any address and the specified port.
			context->recvAddr.sin_family = AF_INET;
			context->recvAddr.sin_port = htons(PORT);
			context->recvAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			ret = bind(context->recvSocket, (SOCKADDR *)&context->recvAddr, sizeof(context->recvAddr));
			if(ret != 0) {
				Console::Error("bind failed with error %d", WSAGetLastError());
				return;
			}
	#endif
		}
	
		DebugNetListener::~DebugNetListener() {
	#if _DEBUG
			int ret = closesocket(context->recvSocket);
			if(ret == SOCKET_ERROR) {
				Console::Error("closesocket failed with error %d", WSAGetLastError());
			}
			WSACleanup();
			SAFE_DELETE(context);
	#endif
		}

		void DebugNetListener::FireNotification(Rid rid) {
			std::map< Rid, std::function<void(Rid)> >::iterator iter = listeners.find(rid);
			if(iter != listeners.end()) {
				Console::Info("Notifying registered listener about changes to <rid %d>", rid);
				iter->second(rid);
			}
		}

		void DebugNetListener::Tick() {
	#if _DEBUG
			int32 senderAddrSize = sizeof(context->senderAddr);
			int bytes = recvfrom(context->recvSocket, recvBuf, MAX_BUFFER_LENGTH, 0, (SOCKADDR *)&context->senderAddr, &senderAddrSize);
			if(bytes == SOCKET_ERROR) {
				int err = WSAGetLastError();
				if(err != WSAEWOULDBLOCK) {
					Console::Error("recvfrom failed with error %d", err);
				}
			} else {
				assert(bytes > 4);
			
				recvBuf[bytes] = 0;
				char *p = recvBuf;
				uint32 packetType = *(uint32 *)p;
				p += 4;
				bytes -= 4;

				Rid rid = RID_NONE;
				switch(packetType)
				{
				case 0:
					Console::Info("Got packet type %u (path='%s')", packetType, p);
					rid = Engine::Get()->assets->PathToRid(p);
					ResourceProvider::Get()->ReloadAsset(rid);
					FireNotification(rid);
					break;
				default:
					Console::Warning("Got unrecognized packet type: %d", packetType);
				}
			}
	#endif
		}

	} // namespace Framework

} // namespace Maki

#endif