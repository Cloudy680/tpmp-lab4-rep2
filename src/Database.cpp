#include "Database.h"
#include <iostream>
#include <fstream>
#include <sstream>

Database& Database::getInstance() {
    static Database instance;
    return instance;
}

bool Database::open(const std::string& filename) {
    int rc = sqlite3_open(filename.c_str(), &db_);
    if (rc) {
        lastError_ = sqlite3_errmsg(db_);
        std::cerr << "Can't open database: " << lastError_ << std::endl;
        return false;
    }
    return true;
}

void Database::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::execute(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        lastError_ = errMsg;
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::query(const std::string& sql, std::function<int(int, char**, char**)> callback) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), [](void* data, int argc, char** argv, char** colName) -> int {
        auto cb = *reinterpret_cast<std::function<int(int, char**, char**)>*>(data);
        return cb(argc, argv, colName);
    }, &callback, &errMsg);
    if (rc != SQLITE_OK) {
        lastError_ = errMsg;
        std::cerr << "Query error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::initFromScript(const std::string& scriptPath) {
    // Скрипт должен быть идемпотентным (CREATE TABLE IF NOT EXISTS / INSERT OR IGNORE),
    // но для скорости запуска выполняем его только если отсутствует хотя бы одна ключевая таблица.
    auto hasTable = [this](const std::string& table) -> bool {
        bool exists = false;
        std::string checkSql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + table + "';";
        auto callback = [&exists](int argc, char** argv, char** colName) -> int {
            if (argc > 0) exists = true;
            return 0;
        };
        if (!query(checkSql, callback)) return false;
        return exists;
    };

    const bool hasFurfarms = hasTable("Furfarms");
    const bool hasUsers = hasTable("Users");
    const bool hasExhibited = hasTable("Exhibited_fur");
    const bool hasResults = hasTable("Auction_Results");
    if (hasFurfarms && hasUsers && hasExhibited && hasResults) return true;
    
    std::ifstream file(scriptPath);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть SQL-скрипт: " << scriptPath << std::endl;
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sqlScript = buffer.str();
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sqlScript.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка выполнения SQL-скрипта: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    std::cout << "База данных успешно инициализирована из скрипта." << std::endl;
    return true;
}

std::string Database::getLastError() const {
    return lastError_;
}