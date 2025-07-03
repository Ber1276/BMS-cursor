#include "../head/PermissionManager.h"

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

const User* PermissionManager::getCurrentUser() const {
    return currentUser; // 返回当前用户
}

void PermissionManager::logout() {
    currentUser = nullptr; // 清除当前用户
}

bool PermissionManager::registerUser(const std::string& username, const std::string& password, Role role) {
    if (!userManager) return false;
    if (userManager -> findUser(username)) {
        std::cout << "用户名已存在！" << std::endl;
        return false;
    }
    User newUser(username, password, role);
    userManager -> addUser(newUser);
    return true;
}

bool PermissionManager::deleteCurrentUser() {
    if (!userManager) return false;
    if (!currentUser) {
        std::cout << "没有当前用户登录！" << std::endl;
        return false;
    }
    bool result = userManager -> removeUser(currentUser->username);
    if (result) {
        currentUser = nullptr; // 注销后清除当前用户
    }
    return result;
}