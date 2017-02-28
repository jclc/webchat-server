#include "database.hpp"
#include <sqlite3.h>
#include <iostream>
#include <sstream>

namespace chat {

ChatroomData::~ChatroomData() {
	delete data;
}

MessageData::~MessageData() {
	for (Message* msg : *data) {
		delete msg;
	}
	delete data;
}

namespace database {

void getChatrooms(sqlite3 *db, ChatroomData& data) {
	char* err_msg = 0;

	std::string query = "SELECT Name FROM Chatrooms;";
	// Execute SQL query and define callback function as a lambda
	sqlite3_exec(db, query.c_str(),
	[](void* param, int argc, char** argv, char** azColName) -> int {
		ChatroomData* data = (ChatroomData*) param;
		data->data->push_back(std::string(argv[0]));
		return 0;
	}, &data, &err_msg);
	return;
}

bool insertChatroom(sqlite3 *db, std::string name) {
	sqlite3_stmt* res;

	std::string query = "INSERT INTO Chatrooms (Name) VALUES( ? )";
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, 0);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	sqlite3_bind_text(res, 1, name.c_str(), -1, nullptr);
	sqlite3_step(res);

	sqlite3_finalize(res);
	return true;
}

void getMessages(sqlite3 *db, MessageData &data, unsigned int howMany) {
	char* err_msg = 0;
	std::ostringstream os;
	os << "SELECT User, Timestamp, Content FROM Messages ORDER BY Id DESC LIMIT ";
	os << howMany;
	os << ";";
	std::string query = os.str();

	// Execute SQL query and define callback function as a lambda
	sqlite3_exec(db, query.c_str(),
	[](void* param, int argc, char** argv, char** azColName) -> int {
		MessageData* data = (MessageData*) param;
		Message* msg = new Message();
		msg->user = argv[0];
		msg->timestamp = std::atoi(argv[1]);
		msg->content = argv[2];
		data->data->insert(data->data->begin(), msg);
		return 0;
	}, &data, &err_msg);

	return;
}

bool insertMessage(sqlite3 *db, long timestamp, std::string user, std::string content) {
	sqlite3_stmt* res;

	std::string query = "INSERT INTO Messages (Timestamp, User, Content) VALUES(?, ?, ?);"
			"SELECT last_inserted_rowid()";
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, 0);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	sqlite3_bind_int64(res, 1, timestamp);
	sqlite3_bind_text(res, 2, user.c_str(), -1, nullptr);
	sqlite3_bind_text(res, 3, content.c_str(), -1, nullptr);
	sqlite3_step(res);

	sqlite3_finalize(res);
}

} // namespace database

} // namespace
