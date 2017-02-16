add_library(database STATIC
	"database.cpp")
target_include_directories(database PUBLIC ${SQLITE3_INCLUDE_DIR})

link_libraries(database ${SQLITE3_LIBRARY})
