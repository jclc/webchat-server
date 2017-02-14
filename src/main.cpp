#include <iostream>
#include <unistd.h>
#include <csignal>
#include <uWS.h>

int listenPort = 3000;

// Signal handler function for gracefully terminating the program
void signalHandler(int signal) {
	if (signal == SIGINT) {
		std::cout << std::endl << "Interrupt signal received, terminating" << std::endl;
		exit(signal);
	}
}

int main(int argc, char *argv[])
{
	signal(SIGINT, signalHandler);
	uWS::Hub hub;
	hub.onMessage([](uWS::WebSocket<uWS::SERVER> ws, char *message, size_t length, uWS::OpCode opCode) {
			ws.send(message, length, opCode);
	});

	hub.listen(listenPort);
	std::cout << "Starting server"
	hub.run();
	return 0;
}
