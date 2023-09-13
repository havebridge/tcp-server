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
		state server_state;

		struct Client;
		std::list<std::unique_ptr<Client>> clients;
		std::mutex client_mutex;

	public:
		struct Client : public CLCore::ClientCore
		{
			typedef CLCore::ClientState client_state;

			friend class Server;

			SOCKET client_socket;
			struct sockaddr_in;
			std::mutex client_mutex;

			Client(SOCKET, struct sockaddr_in);

			virtual bool SendData(std::string data) const override;
			virtual std::vector<uint8_t> LoadData() const override;
			virtual std::string GetHost() const override;
			virtual std::string GetPort() const override;
			virtual client_state Disconnect() override;
			virtual ~Client() override;
		};

	private:
		void HandlerAccept();
		void HandlerData();

	public:
		Server(const char* ip, int port);
		~Server();

	public:
		state Start();
		void Stop();

		void JoinLoop();
	};
}