#include "../include/client.h"

void job(void)
{
	std::cout << "55" << '\n';
}


int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	Net::Client client1;

	if (client1.Connect("127.0.0.1", 8888) == CLCore::ClientState::connected)
	{
		std::cout << "Good\n";
	}
	Net::Client client2;

	if (client2.Connect("127.0.0.1", 8888) == CLCore::ClientState::connected)
	{
		std::cout << "Good\n";
	}
	Net::Client client3;

	if (client3.Connect("127.0.0.1", 8888) == CLCore::ClientState::connected)
	{
		std::cout << "Good\n";
	}

	return 0;
}