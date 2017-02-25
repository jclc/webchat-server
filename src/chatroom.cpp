#include "chatroom.hpp"
#include "server.hpp"
#include <sqlite3.h>
#include <exception>
#include <functional>
#include "database.hpp"
#include <sstream>

namespace chat {

using namespace nlohmann; // json
typedef websocketpp::server<websocketpp::config::asio> t_server;
typedef websocketpp::connection_hdl connection_hdl;
typedef websocketpp::server<websocketpp::config::asio>::message_ptr message_ptr;
typedef websocketpp::frame::opcode::value opcode;
typedef websocketpp::lib::error_code error;

using std::placeholders::_1;
using std::placeholders::_2;
using std::bind;

Chatroom::Chatroom(Server* server, std::string name) {
	m_parentServer = server;
	std::string filename = "db/chatrooms/" + name + ".db";
	int rc = sqlite3_open(filename.c_str(), &db_chatroom);
	if (rc != SQLITE_OK) {
		std::cerr << "Error opening chatroom database: " << sqlite3_errmsg(db_chatroom) << std::endl;
		throw std::exception();
	}

	char* error = 0;
	std::string query =
			"CREATE TABLE IF NOT EXISTS Messages(Id INT PRIMARY KEY, User TEXT, Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, Content TEXT)";
	rc = sqlite3_exec(db_chatroom, query.c_str(), 0, 0, &error);

	if (rc != SQLITE_OK) {
		std::cerr << "SQL error when creating database table: " << error << std::endl;
		sqlite3_free(error);
		sqlite3_close(db_chatroom);
		throw std::exception();
	}
	m_name = name;
}

Chatroom::~Chatroom() {
//	std::lock_guard<std::mutex> lock(m_lock);
	sqlite3_close(db_chatroom);
	std::cout << "Chatroom " << m_name << " closed" << std::endl;
}

void Chatroom::connectUser(connection_hdl hdl) {
	// Change the handlers for this connection
	// First, we upgrade the connection pointer from weak to shared:
	t_server::connection_ptr connection = m_parentServer->m_server.get_con_from_hdl(hdl);
	connection->set_message_handler(std::bind(&Chatroom::onMessage,this,_1,_2));
	connection->set_close_handler(std::bind(&Chatroom::onConnectionClose,this,_1));

	m_lock.lock();
	connections.insert(hdl);
	m_lock.unlock();

	json messages = json::array();
	MessageData msgdata;
	database::getMessages(db_chatroom, msgdata, 100);
	for (Message* message : *msgdata.data) {
		json msg;
		msg["timestamp"] = message->timestamp;
		msg["user"] = message->user;
		msg["content"] = message->content;
		messages.push_back(msg);
	}
	m_parentServer->sendMessage(hdl, messages.dump());
}

void Chatroom::onConnectionClose(connection_hdl hdl) {
	// We need to remove the connection pointer from both this chatroom
	// as well as the parent server. This will free up the nickname.
	m_parentServer->removeConnection(hdl);
	std::lock_guard<std::mutex> lock(m_lock);
	connections.erase(hdl);
}

void Chatroom::onMessage(connection_hdl hdl, message_ptr msg) {
	json message;
	try {
		message = json::parse(msg->get_payload());
	} catch (...) {
		// Client sent us invalid JSON
		error e;
		m_parentServer->m_server.send(hdl, "{\"alert\": \"Invalid JSON data\"}", opcode::text, e);
		m_parentServer->m_server.close(hdl, 1003, "Invalid JSON data");
		connections.erase(hdl);
		return;
	}
	auto search = message.find("message");
	if (search != message.end() && message["message"].is_string()) {
		std::cout << "got message" << std::endl;
	}
}

} // namespace
