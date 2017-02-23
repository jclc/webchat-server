#include "version.h"
#include <iostream>
#include <unistd.h>
#include <csignal>
#include "optparse/optparse.h"
#include "server.h"
//#include <json.hpp>

// Forward declarations
void interruptHandler(int signal);
void printHelp();
void printVersion();

chat::Server* server = nullptr;

int main(int argc, char* argv[]) {

	// Initialise options
	int listenPort = 80;

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
			exit(EXIT_SUCCESS);
		case 'V':
			printVersion();
			exit(EXIT_SUCCESS);
		case 'p':
			listenPort = atoi(options.optarg);
			if (listenPort <= 0) {
				std::cerr << "Error: Invalid port " << listenPort << std::endl;
				exit(EXIT_FAILURE);
			}
			continue;
		case '?':
			std::cerr << options.errmsg << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	// Register interrupt signal handler
	signal(SIGINT, interruptHandler);

	// Create the server
	server = new chat::Server();
	int ret = server->run(listenPort);

	delete server;
	return ret;
}

/// Signal handler function for gracefully terminating the program
void interruptHandler(int signal) {
	if (signal == SIGINT) {
		std::cout << std::endl << "Interrupt signal received, terminating" << std::endl;
		if (server) {
			delete server;
		}
		exit(EXIT_SUCCESS);
	}
}

void printHelp() {
	std::cout
	<< "webchat-server [options]" << std::endl
	<< "  Options:" << std::endl
	<< "      -h | --help" << std::endl
	<< "        Display this help message" << std::endl
	<< "      -V | --version" << std::endl
	<< "        Display program version" << std::endl
	<< "      -p | --port <port number>" << std::endl
	<< "        Which port to listen" << std::endl;
}

void printVersion() {
	std::cout << PROGRAM_VERSION << std::endl;
}
