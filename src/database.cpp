#include <iostream>
#include "database.h"
#include <sqlite3.h>

namespace chat {


Database::Database() :
	dbHandle(nullptr),
	name(""){
	std::cout << "Database created" << std::endl;

}

Database::~Database() {
	std::cout << "Database " << name << "closed" << std::endl;
	if (dbHandle)
		sqlite3_close(dbHandle);
}

bool Database::openDb(std::string name) {
	std::cout << "Opening database " << std::endl;
	std::string path(DB_DIR);
	path += "/";
	path += name;
	int result = sqlite3_open(path.c_str(), &(this->dbHandle));
	if (result != SQLITE_OK) {
		std::cerr << "Couldn't open database " << DB_DIR << "/" << name << ": "
		<< sqlite3_errmsg(this->dbHandle) << std::endl;
		sqlite3_close(this->dbHandle);
	}
	return true;
}

} // namespace
