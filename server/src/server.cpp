#include "../include/server.h"

#pragma warning(disable: 4996)

namespace Net
{
	Server::Server(const char* ip, int port) noexcept
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

	void Server::HandlerAccept(void)
	{
		struct sockaddr_in client_info;
		int client_info_lenght = sizeof(client_info);
		SOCKET client_socket = accept(server_socket, (sockaddr*)&client_info, &client_info_lenght);

		if (client_socket == INVALID_SOCKET)
		{
			std::cerr << "accept\n";
			std::cout << WSAGetLastError() << '\n';
		}

		std::unique_ptr<Client> client(new Client(client_socket, client_info));
		client_mutex.lock();
		clients.emplace_back(std::move(client));
		client_mutex.unlock();

		std::cout << "Client ip: " << std::to_string(client_info.sin_addr.s_addr) << " port:" << std::to_string(client_info.sin_port) << " connected\n";
	}

	void Server::HandlerData()
	{
	}

	Server::state Server::Start()
	{
		std::cout << "Server:\n";

		if (server_state == state::up)
		{
			Stop();
		}

		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			std::cerr << "Error: WSA\n";
			std::cout << WSAGetLastError() << '\n';
			return server_state = state::init_error;
		}
		
		server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

		if (server_socket == SOCKET_ERROR)
		{
			std::cerr << "Error: socket\n";
			std::cout << WSAGetLastError() << '\n';
			return server_state = state::socket_error;
		}

		char yes = '1';

		if ((setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(char)) == -1))
		{
			std::cerr << "Error: setsockopt\n";
			std::cout << WSAGetLastError() << '\n';
			return server_state = state::socket_error;
		}

		server_info.sin_family = AF_INET;
		server_info.sin_port = htons(port);
		server_info.sin_addr.s_addr = inet_addr(ip);
		ZeroMemory(server_info.sin_zero, 8);

		if (bind(server_socket, (const sockaddr*)&server_info, server_info_lenght) == -1)
		{
			std::cerr << "Error: bind\n";
			std::cout << WSAGetLastError() << '\n';
			return server_state = state::bind_error;
		}

		if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
		{
			std::cerr << "Error: listen\n";
			std::cout << WSAGetLastError() << '\n';
			return server_state = state::listen_error;
		}

		server_state = state::up;

		Core::Instance::thread_pool.QueueJob(std::bind(&Server::HandlerAccept, this));
		//Core::Instance::thread_pool.QueueJob(std::bind(&Server::HandlerData, this));

		/*std::vector<char> buffer;
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
		return true;*/

		return server_state;
	}

	void Server::JoinLoop()
	{
		Core::Instance::thread_pool.Join();
	}

	void Server::Stop()
	{
		Core::Instance::thread_pool.DropJobs();
		server_state = state::close;
		closesocket(server_socket);
	}

	bool Server::Client::SendData(std::string data) const
	{
		return false;
	}
	std::vector<uint8_t> Server::Client::LoadData() const
	{
		return std::vector<uint8_t>();
	}
	std::string Server::Client::GetHost() const
	{
		return std::string();
	}
	std::string Server::Client::GetPort() const
	{
		return std::string();
	}

	Server::Client::state Server::Client::Disconnect()
	{
		if (client_state == state::disconnected)
		{
			return client_state;
		}

		client_state = state::disconnected;

		if (shutdown(client_socket, SD_BOTH) == SOCKET_ERROR)
		{
			std::cerr << "Error: shutdown\n";
			std::cout << WSAGetLastError() << '\n';
			closesocket(client_socket);
		}

		return client_state;
	}


	Server::Client::~Client()
	{
		if (client_socket == INVALID_SOCKET)
		{
			return;
		}

		shutdown(client_socket, SD_BOTH);
		closesocket(client_socket);
	}
}