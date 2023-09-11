#pragma once

#include <cstdint>
#include <vector>
#include <string>


namespace Core
{
	enum class ClientState : uint8_t
	{
		disconnected = 0,
		socket_error,
		connect_error,
		connected
	};

	enum class ServerState : uint8_t
	{
		close = 0,
		socket_error,
		bind_error,
		listen_error,
		accept_error,
		up
	};

	class ClientCore
	{
	public:
		virtual bool SendData(std::string data) const = 0;
		virtual std::vector<uint8_t> LoadData() const = 0;
		virtual std::string GetHost() const = 0;
		virtual std::string GetPort() const = 0;
		virtual ClientState Disconnect() = 0;
		virtual ~ClientCore() {};
	};
}