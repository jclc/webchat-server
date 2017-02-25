#ifndef SERVER_H
#define SERVER_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <sqlite3.h>
#include <map>
#include "chatroom.hpp"

namespace chat {

class Chatroom;

typedef websocketpp::connection_hdl connection_hdl;
typedef websocketpp::server<websocketpp::config::asio>::message_ptr message_ptr;

class Server {
public:
	Server();
	~Server();

	int run(const int port);

	// Connection handlers
	void onConnectionOpen(connection_hdl);
	void onConnectionClose(connection_hdl);
	void onMessage(connection_hdl, message_ptr);

	std::string getNickname(const connection_hdl con_hdl) const;
	bool setNickname(connection_hdl con_hdl, std::string nick);
	void addConnection(connection_hdl hdl);
	void removeConnection(connection_hdl hdl);

	websocketpp::server<websocketpp::config::asio> m_server;

private:
	bool initDatabases();

	std::mutex m_lock;
	int m_port;
	bool m_isListening;
	sqlite3* db_chatrooms;
	std::vector<chat::Chatroom*> m_chatrooms;
	std::map<connection_hdl, std::string, std::owner_less<connection_hdl> > m_nicknames;

};

} // namespace

#endif /* SERVER_H */
