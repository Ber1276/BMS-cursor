#ifndef USER_H
#define USER_H

#include "./MyVector.h"
#include <string>
#include <iostream>

enum Role { USER, ADMIN };

struct User {
    std::string username;
    std::string password;
    Role role;

    User() : username(""), password(""), role(USER) {}

    User(const std::string &username, const std::string &password, Role role)
        : username(username), password(password), role(role) {}

    void print() const {
        std::cout << "Username: " << username
                  << ", Role: " << (role == ADMIN ? "ADMIN" : "USER") << std::endl;
    }
};

class UserManager
{
private:
    MyVector<User> users;
public:
    void addUser(const User &user);
    bool removeUser(const std::string &username);
    bool updateUser(const std::string &oldUsername, const User &newUser);
    void printAllUsers() const;
    const User* findUser(const std::string &username) const;
    MyVector<User> fuzzyFindUsers(const std::string& keyword) const;
    MyVector<User> findAndSortUsers(const std::string& keyword, bool ascending = true) const;
    bool adminRemoveUsers(const MyVector<std::string>& usernames);
};

#endif // USER_H