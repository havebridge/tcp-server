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
		std::cout << "Client ip: " << client->GetHost() << " port:" << client->GetPort() << " connected\n";
		clients.emplace_back(std::move(client));
		client_mutex.unlock();
	}

	void Server::HandlerData()
	{
		{
			std::lock_guard<std::mutex> client_lock(client_mutex);

			for (auto it = clients.begin(), end = clients.end(); it != end; ++it)
			{
				auto& client = *it;

				if (client)
				{
					std::vector<uint8_t> data = client->LoadData();

					if (!data.empty())
					{
						client->client_mutex.lock();
						Core::Instance::thread_pool.QueueJob([this, data = std::move(data), &client]{ Data(std::move(client), std::move(data)); });
						client->client_mutex.unlock();
					}
				}
			}
		}
	}

	void Server::Data(std::unique_ptr<Client> client, std::vector<uint8_t> data)
	{
		std::cout << "Data\n";
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
		Core::Instance::thread_pool.QueueJob(std::bind(&Server::HandlerData, this));

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

	void Server::SendData(std::string data)
	{
		for (auto& client : clients)
		{
			client->SendData(data);
		}
	}

	void Server::Stop()
	{
		Core::Instance::thread_pool.DropJobs();
		server_state = state::close;
		closesocket(server_socket);
	}

	bool Server::Client::SendData(std::string data) const
	{
		if (client_state == state::disconnected)
		{
			std::cerr << "Error SendData: The client is not connected\n";
			return false;
		}

		int data_lenght = data.size();

		if (send(client_socket, (char*)&data_lenght, sizeof(int), 0) < 0)
		{
			std::cerr << "Error SendData: the send size\n";
			return false;
		}

		if (send(client_socket, data.c_str(), data_lenght, 0) < 0)
		{
			std::cerr << "Error SendData: the send data\n";
			return false;
		}

		std::cout << "SendData: The size " << data_lenght << " and the data " << data.data() << " is sended successfully\n";

		return true;
	}

	std::vector<uint8_t> Server::Client::LoadData() const
	{
		if (client_state == state::disconnected)
		{
			std::cerr << "Error SendData: The client is not connected\n";
			return std::vector<uint8_t>();
		}

		std::vector<uint8_t> recieved_buffer;
		int recieved_buffer_lenght = 0;

		if (recv(client_socket, (char*)&recieved_buffer_lenght, sizeof(int), 0) < 0)
		{
			std::cerr << "Error LoadData: the recv size\n";
			return std::vector<uint8_t>();
		}

		recieved_buffer.resize(recieved_buffer_lenght, 0x00);

		if (recv(client_socket, (char*)&recieved_buffer, recieved_buffer_lenght, 0) < 0)
		{
			std::cerr << "Error LoadData: the recv buffer\n";
			return std::vector<uint8_t>();
		}

		std::cout << "LoadData: The size " << recieved_buffer_lenght << " and the data " << recieved_buffer.data() << " is recieved successfully\n";

		return recieved_buffer;
	}


	std::string Server::Client::GetHost() const
	{
		uint32_t ip = client_info.sin_addr.s_addr;

		return std::string() + std::to_string(int(reinterpret_cast<char*>(&ip)[0])) + '.' +
			std::to_string(int(reinterpret_cast<char*>(&ip)[1])) + '.' +
			std::to_string(int(reinterpret_cast<char*>(&ip)[2])) + '.' +
			std::to_string(int(reinterpret_cast<char*>(&ip)[3]));
	}

	std::string Server::Client::GetPort() const
	{
		return std::to_string(client_info.sin_port);
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

		client_socket = INVALID_SOCKET;

		shutdown(client_socket, SD_BOTH);
		closesocket(client_socket);
	}
}