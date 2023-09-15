#include "../include/client.h"

#pragma warning(disable: 4996)

namespace Net
{
	Client::Client() noexcept
		:
		wsa{ 0 },
		client_socket(INVALID_SOCKET),
		client_info{ 0 },
		client_info_lenght(sizeof(client_info)) {}

	Client::~Client()
	{
		shutdown(client_socket, SD_BOTH);
		closesocket(client_socket);
		WSACleanup();
	}


	bool Client::Connect(const char* ip, int port)
	{
		std::cout << "Client:\n";

		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			std::cerr << "Error: WSA\n";
			std::cout << WSAGetLastError() << '\n';
			return false;
		}

		if ((client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET)
		{
			std::cerr << "Error: socket\n";
			std::cout << WSAGetLastError() << '\n';
			return false;
		}

		client_info.sin_family = AF_INET;
		client_info.sin_port = htons(port);
		client_info.sin_addr.s_addr = inet_addr(ip);

		ZeroMemory(client_info.sin_zero, 8);

		if (connect(client_socket, (const sockaddr*)&client_info, client_info_lenght) == SOCKET_ERROR)
		{
			std::cerr << "Error: connect\n";
			std::cout << WSAGetLastError() << '\n';
			return false;
		}

		/*std::string message = "Hello\n";
		int size = message.size();

		if (send(client_socket, (char*)&size, sizeof(int), 0) < 0)
		{
			std::cerr << "send\n";
			return false;
		}

		if (send(client_socket, message.c_str(), size, 0) < 0)
		{
			std::cerr << "send\n";
			return false;
		}

		std::cout << "The message size " << size << '\n';
		std::cout << "The message " << message << '\n';

		*/

		return true;
	}
}