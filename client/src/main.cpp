#include "../include/client.h"

#include "../../ThreadPool.h"

void job(void)
{
	std::cout << "55" << '\n';
}


int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	//Net::Client client;

	/*if (client.Connect("127.0.0.1", 8888))
	{
		std::cout << "Good\n";
	}*/

	Core::ThreadPool thread_pool;

	thread_pool.QueueJob(std::bind(&job));

	thread_pool.Join();

	return 0;
}