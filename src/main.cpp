#include "version.h"
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "optparse/optparse.h"
#include "database.h"

// Function forward declarations
void interruptHandler(int signal);
void printHelp();
void printVersion();

std::vector<chat::Database*> db_vector;

int main(int argc, char *argv[]) {
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

	chat::Database* db = new chat::Database();
	db_vector.insert(db_vector.end(), db);

	db->openDb("hello.db");

	// Create the server
	websocketpp::server<websocketpp::config::asio> server;
	server.init_asio();

	std::cout << "Starting server on port " << listenPort << std::endl;
	try {
		server.listen(listenPort);
		server.start_accept();
		server.run();
	} catch (websocketpp::exception &e) {
		std::cerr << "Fatal error: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	} catch (...) {
		std::cerr << "Fatal error" << std::endl;
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

// Signal handler function for gracefully terminating the program
void interruptHandler(int signal) {
	if (signal == SIGINT) {
		std::cout << std::endl << "Interrupt signal received, terminating" << std::endl;
		for (chat::Database* db : db_vector) {
			delete db;
		}
		exit(EXIT_SUCCESS);
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
