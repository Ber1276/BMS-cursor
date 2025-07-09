#include "../include/PermissionManager.h"
#include <iostream>

//登录
bool PermissionManager::login(const std::string& username, const std::string& password) {
    if (!userManager){
        return false;
    }
    const User* user = userManager->findUser(username);
    if (user && user->password.compare(password) == 0) {
        currentUser = user;
        return true;
    }
    return false;
}

//获取当前用户
const User* PermissionManager::getCurrentUser() const {
    return currentUser;
}

//登出
void PermissionManager::logout() {
    currentUser = nullptr;
}

//注册
bool PermissionManager::registerUser(const std::string& username, const std::string& password, Role role) {
    if (!userManager) return false;
    if (userManager->findUser(username)) {
        std::cout << "用户名已存在！" << std::endl;
        return false;
    }
    User newUser(username, password, role);
    userManager->addUser(newUser);
    return true;
}

//删除当前用户
bool PermissionManager::deleteCurrentUser() {
    if (!userManager) return false;
    if (!currentUser) {
        std::cout << "没有当前用户登录！" << std::endl;
        return false;
    }
    bool result = userManager->removeUser(currentUser->username);
    if (result) {
        currentUser = nullptr;
    }
    return result;
} 