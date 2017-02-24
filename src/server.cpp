#include <functional>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <sqlite3.h>
#include <string>
#include <mutex>
#include "server.hpp"
#include "chatroom.hpp"
#include "json.hpp"

namespace chat {

using namespace nlohmann; // for json (jesus, who names namespaces after themselves?)
typedef websocketpp::server<websocketpp::config::asio> t_server;
typedef websocketpp::frame::opcode::value opcode;
typedef websocketpp::lib::error_code error;

Server::Server() {
	using std::placeholders::_1;
	using std::placeholders::_2;
	using std::bind;

	m_isListening = false;
	db_chatrooms = nullptr;
	m_server.init_asio();
	m_server.set_open_handler(bind(&Server::onConnectionOpen,this,_1));
	m_server.set_close_handler(bind(&Server::onConnectionClose,this,_1));
	m_server.set_message_handler(bind(&Server::onMessage,this,_1,_2));
}

Server::~Server() {
	if (m_isListening)
		m_server.stop_listening();
	if (db_chatrooms)
		sqlite3_close(db_chatrooms);
	for (auto chatroom : m_chatrooms) {
		delete chatroom;
	}
	std::cout << "Server closed" << std::endl;
}

int Server::run(const int port) {
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
		std::cerr << "WebSocket error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

bool Server::initDatabases() {
	int rc = sqlite3_open("db/chatrooms.db", &db_chatrooms);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to open chatroom databse: " << sqlite3_errmsg(db_chatrooms) << std::endl;
		sqlite3_close(db_chatrooms);
		db_chatrooms = nullptr;
		return false;
	}
	return true;
}

void Server::onConnectionOpen(connection_hdl hdl) {
	std::cout << "New connection: " << hdl.lock().get() << std::endl;
	std::lock_guard<std::mutex> lock(m_lock);
	// Give an empty nickname at the start
	m_nicknames[hdl] = "";
}

void Server::onConnectionClose(connection_hdl hdl) {
	std::cout << "Connection closed: " << hdl.lock().get() << std::endl;
	std::lock_guard<std::mutex> lock(m_lock);
	m_nicknames.erase(hdl);
}

/**
 * This is the initial message handler, clients will use this before they
 * are assigned to a chatroom.
 */
void Server::onMessage(connection_hdl hdl, message_ptr msg) {
	std::cout << "Received message: " << msg->get_payload() << std::endl;
	json message;
	try {
		message = json::parse(msg->get_payload());
	} catch (...) {
		// Client sent us invalid JSON. Unacceptable!
		error e;
		m_server.send(hdl, "{\"alert\": \"Invalid JSON data\"}", opcode::text, e);
		m_server.close(hdl, 1003, "Invalid JSON data");
		return;
	}
	auto search = message.find("action");
	std::string action = "none";
	if (search != message.end()) {
		// FIX: crashes if not string
		action = message["action"];
	}
	std::cout << "action: " << action << std::endl;
	// Handle the message
	std::cout << "Active connections: " << m_nicknames.size() << std::endl;

}

std::string Server::getNickname(const connection_hdl con_hdl) const {
	auto search = m_nicknames.find(con_hdl);
	if (search != m_nicknames.end()) {
		return search->second;
	} else {
		return "";
	}
}

bool Server::setNickname(connection_hdl con_hdl, std::string nick) {
	if (getNickname(con_hdl) != "") {
		// Nickname already reserved
		return false;
	}
	std::lock_guard<std::mutex> lock(m_lock);
	m_nicknames[con_hdl] = nick;
	return true;
}

} // namespace
