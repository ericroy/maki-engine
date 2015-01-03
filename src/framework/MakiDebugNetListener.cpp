#include "framework/framework_stdafx.h"
#include "framework/MakiDebugNetListener.h"
#include "core/MakiConsole.h"

#if defined(_WIN32) || defined(_WIN64)

#include <winsock2.h>
#include <Ws2tcpip.h>

using namespace maki::core;

namespace maki
{
	namespace framework
	{

		struct debug_net_listener_t::context_t
		{
#if _DEBUG
			WSADATA wsa_data_;
			SOCKET recv_socket_;
			sockaddr_in recv_addr_;
			sockaddr_in sender_addr_;
#endif
		};
	
		debug_net_listener_t::debug_net_listener_t()
			: context_(nullptr)
		{

#if _DEBUG
			context_ = new context_t();
			memset(context_, 0, sizeof(context_));
			context_->recv_socket_ = INVALID_SOCKET;

			int ret = 0;

			// Initialize Winsock
			ret = WSAStartup(MAKEWORD(2, 2), &context_->wsa_data_);
			if(ret != NO_ERROR) {
				console_t::error("WSAStartup failed with error %d", ret);
				return;
			}

			// Create a receiver socket to receive datagrams
			context_->recv_socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if(context_->recv_socket_ == INVALID_SOCKET) {
				console_t::error("socket failed with error %d", WSAGetLastError());
				return;
			}

			// Make the socket non-blocking
			unsigned long mode = 1;
			ioctlsocket(context_->recv_socket_, FIONBIO, &mode);

			// Bind the socket to any address and the specified port.
			context_->recv_addr_.sin_family = AF_INET;
			context_->recv_addr_.sin_port = htons(port_);
			context_->recv_addr_.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			ret = bind(context_->recv_socket_, (SOCKADDR *)&context_->recv_addr_, sizeof(context_->recv_addr_));
			if(ret != 0) {
				console_t::error("bind failed with error %d", WSAGetLastError());
				return;
			}
#endif
		}
	
		debug_net_listener_t::~debug_net_listener_t()
		{
#if _DEBUG
			int ret = closesocket(context_->recv_socket_);
			if(ret == SOCKET_ERROR) {
				console_t::error("closesocket failed with error %d", WSAGetLastError());
			}
			WSACleanup();
			MAKI_SAFE_DELETE(context_);
#endif
		}

		void debug_net_listener_t::fire_notification(rid_t rid)
		{
			if(generic_listener_ != nullptr) {
				generic_listener_(rid);
			}
			std::map< rid_t, std::function<void(rid_t)> >::iterator iter = listeners_.find(rid);
			if(iter != listeners_.end()) {
				console_t::info("Notifying registered listener about changes to <rid %d>", rid);
				iter->second(rid);
			}
		}

		void debug_net_listener_t::tick()
		{
#if _DEBUG
			int32 sender_addr_size = sizeof(context_->sender_addr_);
			int bytes = recvfrom(context_->recv_socket_, recv_buffer_, max_buffer_length_, 0, (SOCKADDR *)&context_->sender_addr_, &sender_addr_size);
			if(bytes == SOCKET_ERROR) {
				int err = WSAGetLastError();
				if(err != WSAEWOULDBLOCK) {
					console_t::error("recvfrom failed with error %d", err);
				}
			} else {
				assert(bytes > 4);
			
				recv_buffer_[bytes] = 0;
				char *p = recv_buffer_;
				uint32 packet_type = *(uint32 *)p;
				p += 4;
				bytes -= 4;

				rid_t rid = RID_NONE;
				switch(packet_type)
				{
				case 0:
					console_t::info("Got packet type %u (path='%s')", packet_type, p);
					rid = engine_t::get()->assets_->full_path_to_rid(p);
					if(rid != RID_NONE) {
						fire_notification(rid);
					} else {
						console_t::warning("Could not convert path to rid: %s", p);
					}
					break;
				default:
					console_t::warning("Got unrecognized packet type: %d", packet_type);
				}
			}
#endif
		}

	} // namespace framework

} // namespace maki

#endif