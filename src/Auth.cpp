#include "Auth.h"
#include "Database.h"
#include <sqlite3.h>
#include <iostream>

User Auth::login(const std::string& username, const std::string& password) {
    Database& db = Database::getInstance();
    sqlite3* h = db.getHandle();
    if (!h) return User{-1, "", "", 0};

    const char* sql = "SELECT id, username, role, COALESCE(farm_id, 0) "
                      "FROM Users WHERE username = ?1 AND password = ?2 LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(h, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Auth error: " << sqlite3_errmsg(h) << std::endl;
        return User{-1, "", "", 0};
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

    User u{-1, "", "", 0};
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        u.id = sqlite3_column_int(stmt, 0);
        const unsigned char* un = sqlite3_column_text(stmt, 1);
        const unsigned char* rl = sqlite3_column_text(stmt, 2);
        u.username = un ? reinterpret_cast<const char*>(un) : "";
        u.role = rl ? reinterpret_cast<const char*>(rl) : "";
        u.farm_id = sqlite3_column_int(stmt, 3);
    }
    sqlite3_finalize(stmt);
    return u;
}