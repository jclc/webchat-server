#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>

// Subdirectory for database files
#define DB_DIR "db"

namespace chat {

class Database {
public:
	Database();
	~Database();

	bool openDb(std::string name);

private:
	std::string name;
	sqlite3* dbHandle;
};

} // namespace
#endif /* DATABASE_H */
