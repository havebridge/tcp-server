#include "../include/client.h"

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	Net::Client client;

	if (client.Connect("127.0.0.1", 8888))
	{
		std::cout << "Good\n";
	}

	return 0;
}