#pragma once

#include <iostream>
#include <string>

#include <WinSock2.h>

#include "../../ClientCore.h"
#include "../../ThreadPool.h"

namespace Net
{
	class Client : public CLCore::ClientCore
	{
	private:
		WSAData wsa;
		SOCKET client_socket;
		struct sockaddr_in client_info;
		int client_info_lenght;
		const char* ip;
		int port;

		typedef CLCore::ClientState state;
		state client_state = state::disconnected;

	public:
		Client() noexcept;
		~Client();

	public:
		state Connect(const char* ip, int port);

	public:
		virtual bool SendData(std::string data) const override;
		virtual std::vector<uint8_t> LoadData() const override;
		virtual std::string GetHost() const override;
		virtual std::string GetPort() const override;
		virtual state Disconnect() override;
	};
}