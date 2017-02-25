#include "chatroom.hpp"
#include "server.hpp"
#include <sqlite3.h>
#include <exception>
#include <functional>

namespace chat {

using namespace nlohmann; // json
typedef websocketpp::server<websocketpp::config::asio> t_server;
typedef websocketpp::connection_hdl connection_hdl;
typedef websocketpp::server<websocketpp::config::asio>::message_ptr message_ptr;

using std::placeholders::_1;
using std::placeholders::_2;
using std::bind;

Chatroom::Chatroom(Server* server, std::string name) {
	m_parentServer = server;
	int rc = sqlite3_open(name.c_str(), &db_chatroom);
	if (rc != SQLITE_OK) {
		std::cerr << "Error opening chatroom database: " << sqlite3_errmsg(db_chatroom) << std::endl;
		throw std::exception();
	}

	char* error = 0;
	std::string query =
			"CREATE TABLE IF NOT EXISTS Messages(Id INT, Timestamp TEXT, Content TEXT)";
	rc = sqlite3_exec(db_chatroom, query.c_str(), 0, 0, &error);

	if (rc != SQLITE_OK) {
		std::cerr << "SQL error when creating database table: " << error << std::endl;
		sqlite3_free(error);
		sqlite3_close(db_chatroom);
		throw std::exception();
	}
}

Chatroom::~Chatroom() {
//	std::lock_guard<std::mutex> lock(m_lock);
	sqlite3_close(db_chatroom);
}

void Chatroom::connectUser(connection_hdl hdl) {
	// Change the handlers for this connection
	// First, we upgrade the connection pointer from weak to shared:
	t_server::connection_ptr connection = m_parentServer->m_server.get_con_from_hdl(hdl);
	connection->set_message_handler(std::bind(&Chatroom::onMessage,this,_1,_2));
	connection->set_close_handler(std::bind(&Chatroom::onConnectionClose,this,_1));

	std::lock_guard<std::mutex> lock(m_lock);
	connections.insert(hdl);
}

void Chatroom::onConnectionClose(connection_hdl hdl) {
	// We need to remove the connection pointer from both this chatroom
	// as well as the parent server. This will free up the nickname.
	m_parentServer->removeConnection(hdl);
	std::lock_guard<std::mutex> lock(m_lock);
	connections.erase(hdl);
}

void Chatroom::onMessage(connection_hdl hdl, message_ptr message) {

}

} // namespace
