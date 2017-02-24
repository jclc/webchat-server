#include "chatroom.hpp"
#include "server.hpp"

namespace chat {

Chatroom::Chatroom(Server* server) {
	m_server = server;
}

Chatroom::~Chatroom() {

}

} // namespace
