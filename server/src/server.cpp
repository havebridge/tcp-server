#include "../include/server.h"

#pragma warning(disable: 4996)

namespace Net
{
	Server::Server(const char* ip, int port)
		:
		wsa{ 0 },
		server_socket(INVALID_SOCKET),
		server_info{ 0 },
		server_info_lenght(sizeof(server_info)),
		ip(ip),
		port(port) {}

	Server::~Server()
	{
		closesocket(server_socket);
		WSACleanup();
	}
		

	bool Server::Start()
	{
		std::cout << "Server:\n";

		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			std::cerr << "WSA\n";
			std::cout << WSAGetLastError() << '\n';
			return false;
		}
		
		server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

		if (server_socket == SOCKET_ERROR)
		{
			std::cerr << "socket\n";
			std::cout << WSAGetLastError() << '\n';
			return false;
		}

		char yes = '1';

		if ((setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(char)) == -1))
		{
			std::cerr << "setsockopt\n";
			std::cout << WSAGetLastError() << '\n';
			return false;
		}

		server_info.sin_family = AF_INET;
		server_info.sin_port = htons(port);
		server_info.sin_addr.s_addr = inet_addr(ip);
		ZeroMemory(server_info.sin_zero, 8);

		if (bind(server_socket, (const sockaddr*)&server_info, server_info_lenght) == -1)
		{
			std::cerr << "bind\n";
			std::cout << WSAGetLastError() << '\n';
			return false;
		}

		if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
		{
			std::cerr << "listen\n";
			std::cout << WSAGetLastError() << '\n';
			return false;
		}

		struct sockaddr_in client_info;
		int client_info_lenght = sizeof(client_info);
		SOCKET client_socket = accept(server_socket, (sockaddr*)&client_info, &client_info_lenght);

		if (client_socket == SOCKET_ERROR) 
		{
			std::cerr << "accept\n";
			std::cout << WSAGetLastError() << '\n';
			return false;
		}

		std::cout << "server started\n";
		std::cout << "Connected with client\n";
		std::cout << "IP: " << client_info.sin_addr.s_addr << '\n';
		std::cout << "PORT: " << client_info.sin_port << '\n';

		std::vector<char> buffer;
		std::string message;
		int size = 0;

		if (recv(client_socket, (char*)&size, sizeof(int), 0) < 0)
		{
			std::cerr << "recv\n";
			return false;
		}

		buffer.resize(size);

		if (recv(client_socket, buffer.data(), size, 0) < 0)
		{
			std::cerr << "recv\n";
			return false;
		}


		for (const auto& t : buffer)
		{
			message += t;
		}

		std::cout << "The message size " << size << '\n';
		std::cout << "The message " << message << '\n';
		return true;
	}

}