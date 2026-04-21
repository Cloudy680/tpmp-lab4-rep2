#ifndef AUTH_H
#define AUTH_H

#include <string>

struct User {
    int id;
    std::string username;
    std::string role;
    int farm_id;
};

class Auth {
public:
    static User login(const std::string& username, const std::string& password);
};

#endif