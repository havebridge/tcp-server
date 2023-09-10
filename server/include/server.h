#pragma once

#include <iostream>
#include <vector>

#include <WinSock2.h>


namespace Net
{
	class Server
	{
	private:
		WSAData wsa;
		SOCKET server_socket;
		struct sockaddr_in server_info;
		int server_info_lenght;
		const char* ip;
		int port;

	public:
		Server(const char* ip, int port);
		~Server();

	public:
		bool Start();
		void Stop();
	};
}