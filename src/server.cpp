#include <functional>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <sqlite3.h>
#include <string>
#include <mutex>
#include "server.hpp"
#include "chatroom.hpp"
#include "json.hpp"
#include <regex>
#include "database.hpp"
#include <fstream>

namespace chat {

#define CHATROOM_NAME_MIN_LENGTH 2
#define CHATROOM_NAME_MAX_LENGTH 64

using namespace nlohmann; // for json (jesus, who names namespaces after themselves?)
typedef websocketpp::server<websocketpp::config::asio> t_server;
typedef websocketpp::frame::opcode::value opcode;
typedef websocketpp::lib::error_code error;

Server::Server() {
	using std::placeholders::_1;
	using std::placeholders::_2;

	m_isListening = false;
	db_chatrooms = nullptr;
	m_server.init_asio();
	m_server.set_open_handler(std::bind(&Server::onConnectionOpen, this, _1));
	m_server.set_close_handler(std::bind(&Server::onConnectionClose, this, _1));
	m_server.set_message_handler(std::bind(&Server::onMessage, this, _1, _2));
	m_server.set_http_handler(std::bind(&Server::onHttp, this, _1));
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
		std::cerr << "Failed to open chatroom database: " << sqlite3_errmsg(db_chatrooms) << std::endl;
		sqlite3_close(db_chatrooms);
		db_chatrooms = nullptr;
		return false;
	}

	char* error = 0;
	std::string query =
			"CREATE TABLE IF NOT EXISTS Chatrooms(Id INT PRIMARY KEY, Name TEXT)";
	rc = sqlite3_exec(db_chatrooms, query.c_str(), 0, 0, &error);

	if (rc != SQLITE_OK) {
		std::cerr << "SQL error when creating database table: " << error << std::endl;
		sqlite3_free(error);
		sqlite3_close(db_chatrooms);
		throw std::exception();
	}

	ChatroomData chdata;
	database::getChatrooms(db_chatrooms, chdata);
	for (auto name : *chdata.data) {
		auto chatroom = new Chatroom(this, name);
		m_chatrooms.push_back(chatroom);
		std::cout << "Opened existing chatroom " << name << std::endl;
	}

	return true;
}

bool Server::createChatroom(std::string name) {
	for (Chatroom* chatroom : m_chatrooms) {
		if (chatroom->m_name == name) {
			// Chatroom already exists; return
			return false;
		}
	}
	if (name.length() < CHATROOM_NAME_MIN_LENGTH || name.length() > CHATROOM_NAME_MAX_LENGTH)
		return false;

	// Validate the name: can only have alphanumeric characters or _-().,!?';
	std::regex validate("^[-_().,!?'; [:alnum:]]+$");
	if (std::regex_match(name, validate)) {
		if (database::insertChatroom(db_chatrooms, name)) {
			Chatroom* chatroom = new Chatroom(this, name);
			m_chatrooms.push_back(chatroom);
			return true;
		}
	}
	return false;
}

void Server::sendMessage(connection_hdl hdl, std::string msg) {
	m_server.send(hdl, msg, websocketpp::frame::opcode::text);
}

void Server::addConnection(connection_hdl hdl) {
	std::cout << "New connection: " << hdl.lock().get() << std::endl;
	std::lock_guard<std::mutex> lock(m_lock);
	// Give an empty nickname at the start
	m_nicknames[hdl] = "";
}

void Server::removeConnection(connection_hdl hdl) {
	std::cout << "Connection closed: " << hdl.lock().get() << std::endl;
	std::lock_guard<std::mutex> lock(m_lock);
	m_nicknames.erase(hdl);
}

void Server::onHttp(connection_hdl hdl) {
	// Included with this server comes a basic single-page HTML client.
	// This is NOT a scalable solution, recommended to switch to a reverse-
	// proxy setup to a real HTTP server in the future. Alternatively
	// external clients can be used with the JSON API.
	t_server::connection_ptr connection = m_server.get_con_from_hdl(hdl);
	std::ifstream index;
	index.open("index.html");
	if (!index) {
		std::cout << "Error: No index.html found" << std::endl;
		connection->set_body("404 Not found");
		connection->set_status(websocketpp::http::status_code::not_found);
		return;
	}
	index.seekg(0, std::ios::end);
	std::string response;
	response.reserve(index.tellg());
	index.seekg(0, std::ios::beg);

	response.assign((std::istreambuf_iterator<char>(index)), std::istreambuf_iterator<char>());

	connection->set_body(response);
	connection->set_status(websocketpp::http::status_code::ok);
}

void Server::onConnectionOpen(connection_hdl hdl) {
	addConnection(hdl);
}

void Server::onConnectionClose(connection_hdl hdl) {
	removeConnection(hdl);
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
		m_server.close(hdl, 1003, "Invalid JSON data");
		return;
	}

	// Handle the message
	auto search = message.find("create_chatroom");
	if (search != message.end() && message["create_chatroom"].is_string()) {
		// Instructed to create a new chatroom
		std::string name = message["create_chatroom"];
		if (createChatroom(name)) {
			std::cout << "Created new chatroom " << name << std::endl;
		}
		sendChatrooms(hdl);
		return;
	}
	search = message.find("join_chatroom");
	if (search != message.end() && message["join_chatroom"].is_string()) {
		// Instructed to join a new chatroom
		std::string name = message["join_chatroom"];
		for (Chatroom* chatroom : m_chatrooms) {
			if (chatroom->m_name == name) {
				sendMessage(hdl, "{\"status\":\"Joined channel\"}");
				chatroom->connectUser(hdl);
				return;
			}
		}
		// Chatroom not found; return
		sendMessage(hdl, "{\"alert\":\"Chatroom doesn't exist\"}");
		return;
	}
	search = message.find("set_nickname");
	if (search != message.end() && message["set_nickname"].is_string()) {
		// User wants to reserve a nickname
		std::string nick = message["set_nickname"];
		if (setNickname(hdl, nick)) {
			sendMessage(hdl, "{\"status\":\"Nickname set\"}");
		} else {
			sendMessage(hdl, "{\"status\":\"Nickname already reserved\"}");
		}
		return;
	}
	search = message.find("get_chatrooms");
	if (search != message.end()) {
		// User wants to get a list of chatrooms
		sendChatrooms(hdl);
	}
}

void Server::sendChatrooms(connection_hdl hdl) {
	json response;
	response["chatrooms"] = json::array();
	for (Chatroom* chatroom : m_chatrooms) {
		response["chatrooms"].push_back(chatroom->m_name);
	}
	sendMessage(hdl, response.dump());
	return;
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
	// TODO: this is an inefficient method since the data in std::map is unordered
	for (auto it = m_nicknames.begin(); it != m_nicknames.end(); ++it) {
		if (it->second == nick) {
			// Nickname already reserved by someone else
			return false;
		}
	}
	std::lock_guard<std::mutex> lock(m_lock);
	m_nicknames[con_hdl] = nick;
	return true;
}

void Server::clearNickname(connection_hdl con_hdl) {
	std::lock_guard<std::mutex> lock(m_lock);
	m_nicknames[con_hdl] = "";
}

} // namespace
