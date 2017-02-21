#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

class Database {
public:
	Database();
	~Database();

private:
	sqlite3* dbHandle;
};

#endif /* DATABASE_H */
