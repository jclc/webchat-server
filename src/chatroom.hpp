#ifndef CHATROOM_H
#define CHATROOM_H

#include <sqlite3.h>
#include <mutex>

namespace chat {

class Server;

class Chatroom {
public:
	Chatroom(Server* server);
	~Chatroom();

private:
	sqlite3* db_chatroom;
	std::mutex m_lock;
	Server* m_server;

};

} // namespace

#endif /* CHATROOM_H */
