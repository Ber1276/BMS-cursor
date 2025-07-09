#ifndef USER_H
#define USER_H

#include "./MyVector.h"
#include <string>
#include <iostream>

enum Role { USER, ADMIN };

/**
 * @brief The User struct 用户信息
 * 包含用户名、密码和角色
 * @author 陈子涵
 */
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

/**
 * @brief The UserManager class 用户管理模块
 * 负责用户的添加、删除、更新、查找等功能
 * @author 陈子涵
 */
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
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;
    const MyVector<User>& getAllUsers() const { return users; }
};

#endif // USER_H
