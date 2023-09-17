#pragma once

#include <iostream>
#include <vector>
#include <list>

#include <WinSock2.h>

#include "../../ClientCore.h"
#include "../../ThreadPool.h"

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

		typedef CLCore::ServerState state;
		state server_state = state::close;

		struct Client;
		std::list<std::unique_ptr<Client>> clients;
		std::mutex client_mutex;

	public:
		struct Client : public CLCore::ClientCore
		{
			typedef CLCore::ClientState state;

			friend class Server;

			SOCKET client_socket;
			struct sockaddr_in client_info;
			std::mutex client_mutex;

			state client_state = state::connected;

			Client(SOCKET client_socket, struct sockaddr_in client_info)
				:
				client_socket(client_socket),
				client_info(client_info) {}

			virtual bool SendData(std::string data) const override;
			virtual std::vector<uint8_t> LoadData() const override;
			virtual std::string GetHost() const override;
			virtual std::string GetPort() const override;
			virtual state Disconnect() override;
			virtual ~Client() override;
		};

	private:
		void HandlerAccept();
		void HandlerData();
		void Data(std::unique_ptr<Client> client, std::vector<uint8_t> data);

	public:
		Server(const char* ip, int port) noexcept;
		~Server();

	public:
		state Start();
		void Stop();

		void JoinLoop();

		void SendData(std::string);
	};
}