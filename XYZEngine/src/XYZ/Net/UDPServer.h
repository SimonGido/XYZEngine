#pragma once
#include "Core.h"

#include "UDPConnection.h"
#include "UDPConnectionManager.h"

#include "XYZ/Utils/DataStructures/Queue.h"

namespace XYZ {
	
	class UDPServer : public std::enable_shared_from_this<UDPServer>
	{
	public:
		UDPServer(asio::io_context& asioContext, uint16_t port);

		void Start();

		void Stop();

	private:

		void requestNextPacket();

		void receive(asio::error_code error, size_t length);
	


		void acceptConnection();

	private:
		char m_Buffer[65536]{ 0 };

		asio::io_context&	  m_Context;
		asio::ip::udp::socket m_Socket;
		

		Queue<std::string>	  m_Messages;

		
		UDPConnectionManager  m_ConnManager;
	};
}