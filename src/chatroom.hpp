#ifndef CHATROOM_H
#define CHATROOM_H

#include <sqlite3.h>
#include <mutex>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <json.hpp>
#include <set>

namespace chat {

class Server;

using namespace nlohmann; // json
typedef websocketpp::connection_hdl connection_hdl;
typedef websocketpp::server<websocketpp::config::asio>::message_ptr message_ptr;

class Chatroom {
public:
	Chatroom(Server* server, std::string m_name);
	~Chatroom();

	// Connection handlers
	void onConnectionClose(connection_hdl hdl);
	void onMessage(connection_hdl hdl, message_ptr msg);

	/// Connect user from Server to Chatroom
	void connectUser(connection_hdl hdl);

	std::string m_name;

private:
	sqlite3* db_chatroom;
	std::mutex m_lock;
	Server* m_parentServer;
	/// connection_hdl are weak pointers, so we need to store them like this
	std::set<connection_hdl, std::owner_less<connection_hdl>> connections;

	/// Get recent chat history as a json string
	std::string getChatHistory(int howMany);
	void receiveMessage(json message);
	void storeMessage(json message);
	void broadcastMessage(json message);

};

} // namespace

#endif /* CHATROOM_H */
