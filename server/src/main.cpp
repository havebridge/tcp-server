#include "../include/server.h"

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	Net::Server server("127.0.0.1", 8888);

	if (server.Start() == CLCore::ServerState::up)
	{
		server.JoinLoop();
	}

	server.Stop();

	return 0;
}