#include <functional>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <sqlite3.h>
#include "server.hpp"

namespace chat {

Server::Server() {
//	this->m_server = new websocketpp::server<websocketpp::config::asio>();
	typedef websocketpp::server<websocketpp::config::asio> t_server;
	using websocketpp::lib::placeholders::_1;
	using websocketpp::lib::placeholders::_2;
	using websocketpp::lib::bind;

	m_isListening = false;
	db_users = nullptr;
	db_chatrooms = nullptr;
	m_server.init_asio();
	m_server.set_open_handler(bind(&Server::onConnectionOpen,this,_1));
	m_server.set_close_handler(bind(&Server::onConnectionClose,this,_1));
	m_server.set_message_handler(bind(&Server::onMessage,this,_1,_2));
}

Server::~Server() {
	std::cout << "Server closed" << std::endl;
	if (m_isListening)
		m_server.stop_listening();
	if (db_users)
		sqlite3_close(db_users);
	if (db_chatrooms)
		sqlite3_close(db_chatrooms);
//	delete this->m_server;
}

int Server::run(int port) {
	if (!initDatabases()) {
		return EXIT_FAILURE;
	}
	std::cout << "Starting server on port " << port << std::endl;
	try {
		m_server.listen(port);
		m_isListening = true;
		m_server.start_accept();
		m_server.run();
	} catch (websocketpp::exception &e) {
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	} catch (...) {
		std::cerr << "Unknown fatal error" << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

bool Server::initDatabases() {
	// We don't need to store connected users on the disk, so we
	// open the database in memory only
	int rc = sqlite3_open(":memory:", &db_users);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to open users databse: " << sqlite3_errmsg(db_users) << std::endl;
		sqlite3_close(db_users);
		db_users = nullptr;
		return false;
	}
	rc = sqlite3_open("db/chatrooms.db", &db_chatrooms);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to open users databse: " << sqlite3_errmsg(db_users) << std::endl;
		sqlite3_close(db_users);
		sqlite3_close(db_chatrooms);
		db_users = nullptr;
		db_chatrooms = nullptr;
		return false;
	}
	return true;
}

void Server::onConnectionOpen(connection_hdl hdl) {
	std::cout << "New connection: " << hdl.lock().get() << std::endl;
}

void Server::onConnectionClose(connection_hdl hdl) {
	std::cout << "Connection closed: " << hdl.lock().get() << std::endl;
}

void Server::onMessage(connection_hdl hdl, message_ptr msg) {
	std::cout << "Received message: " << msg->get_payload() << std::endl;
}

} // namespace
