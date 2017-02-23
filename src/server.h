#ifndef SERVER_H
#define SERVER_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <sqlite3.h>

namespace chat {

class Server {
public:
	Server();
	~Server();

	int run(int port);

	// Connection handlers
	typedef websocketpp::connection_hdl connection_hdl;
	typedef websocketpp::server<websocketpp::config::asio>::message_ptr message_ptr;
	void onConnectionOpen(connection_hdl);
	void onConnectionClose(connection_hdl);
	void onMessage(connection_hdl, message_ptr);

private:
	bool initDatabases();

	int m_port;
	websocketpp::server<websocketpp::config::asio> m_server;
	bool m_isListening;
	sqlite3* db_users;
	sqlite3* db_chatrooms;
	std::mutex m_lock;

};

} // namespace

#endif /* SERVER_H */
