#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

// Forward declaration
struct sqlite3;

namespace chat {

class ChatroomData {
public:
	ChatroomData() : data(new std::vector<std::string>) {}
	~ChatroomData();

	std::vector<std::string>* data;
private:
	ChatroomData(const ChatroomData& ref);
};

struct Message {
	long timestamp;
	std::string user;
	std::string content;
};

class MessageData {
public:
	MessageData() : data(new std::vector<Message*>) {}
	~MessageData();

	std::vector<Message*>* data;
private:
	MessageData(const MessageData& ref);
};

namespace database {

void getChatrooms(sqlite3* db, ChatroomData& data);
bool insertChatroom(sqlite3* db, std::string name);
void getMessages(sqlite3* db, MessageData& data, unsigned int howMany);
bool insertMessage(sqlite3* db, long timestamp, std::string user, std::string content);

} // namespace database

} // namespace

#endif /* DATABASE_H */
