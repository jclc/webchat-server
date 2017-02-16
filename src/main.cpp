#include "version.h"
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <uWS.h>
#include "optparse/optparse.h"
#include "database/database.h"

// Function forward declarations
void signalHandler(int signal);
void printHelp();
void printVersion();

int main(int argc, char *argv[]) {
	// Initialise options
	int listenPort = 80;

	Database db;

	// Parse CLI arguments
	struct optparse options;
	optparse_init(&options, argv);
	struct optparse_long longopts[] = {
		{"help", 'h', OPTPARSE_NONE},
		{"version", 'V', OPTPARSE_NONE},
		{"port", 'p', OPTPARSE_REQUIRED},
		{0}
	};
	for (int option = 0; (option = optparse_long(&options, longopts, NULL)) != -1; ) {
		switch (option) {
		case 'h':
			printHelp();
			exit(0);
		case 'V':
			printVersion();
			exit(0);
		case 'p':
			listenPort = atoi(options.optarg);
			if (listenPort <= 0) {
				std::cerr << "Error: Invalid port " << listenPort << std::endl;
				exit(1);
			}
			continue;
		case '?':
			std::cerr << options.errmsg << std::endl;
			exit(1);
		}
	}

	signal(SIGINT, signalHandler);
	uWS::Hub hub;
	hub.onMessage([](uWS::WebSocket<uWS::SERVER> ws, char *message, size_t length, uWS::OpCode opCode) {
		std::cout << "Received message:" << std::endl << message;
		std::cout << "Length: " << length << ", OpCode: " << opCode << std::endl;
		ws.send(message, length, opCode);
	});

	bool success;

	success = hub.listen(listenPort);

	if (!success) {
		std::cerr << "Error: Port rejected" << std::endl;
		exit(2);
	}

	std::cout << "Starting server on port " << listenPort << std::endl;
	hub.run();
	return 0;
}

// Signal handler function for gracefully terminating the program
void signalHandler(int signal) {
	if (signal == SIGINT) {
		std::cout << std::endl << "Interrupt signal received, terminating" << std::endl;
		exit(0);
	}
}

void printHelp() {
	std::cout
	<< "webchat-server [options]" << std::endl
	<< "  Options:" << std::endl
	<< "  -h | --help" << std::endl
	<< "    Display this help message" << std::endl
	<< "  -V | --version" << std::endl
	<< "    Display program version" << std::endl
	<< "  -p | --port <port number>" << std::endl
	<< "    Which listen port to use" << std::endl;
}

void printVersion() {
	std::cout << PROGRAM_VERSION << std::endl;
}
