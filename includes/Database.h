#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <functional>

class Database {
public:
    static Database& getInstance();
    bool open(const std::string& filename);
    void close();
    bool execute(const std::string& sql);
    bool query(const std::string& sql, std::function<int(int, char**, char**)> callback);
    std::string getLastError() const;
    sqlite3* getHandle() const { return db_; }
    
    bool initFromScript(const std::string& scriptPath);
    
private:
    Database() : db_(nullptr) {}
    ~Database() { close(); }
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    sqlite3* db_;
    std::string lastError_;
};

#endif