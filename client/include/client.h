#pragma once

#include <iostream>
#include <string>

#include <WinSock2.h>

namespace Net
{
	class Client
	{
	private:
		WSAData wsa;
		SOCKET client_socket;
		struct sockaddr_in client_info;
		int client_info_lenght;
		const char* ip;
		int port;

	public:
		Client() noexcept;
		~Client();

	public:
		bool Connect(const char* ip, int port);
	};
}