#pragma once
#include "User.h"
/**
 * @brief The PermissionManager class 权限控制模块
 * 负责用户登录、登出、注册、权限验证等功能
 * @author 陈子涵
 */
class PermissionManager {
private:
    // 当前登录用户
    const User* currentUser = nullptr;
    UserManager* userManager = nullptr;
public:
    PermissionManager(UserManager* userManager) : userManager(userManager) {};
    //登录
    bool login(const std::string& username, const std::string& password);
    //获取当前用户
    const User* getCurrentUser() const;
    //需要权限的操作
    template<typename Func>
    void requirePermission(Role role, Func&& func) {
        if (!currentUser) {
            throw std::runtime_error("No user is logged in.");
        };
        if (static_cast<int>(currentUser->role) >= static_cast<int>(role)) {
            std::forward<Func>(func)();
        } else {
            throw std::runtime_error("Access denied: insufficient permissions.");
        }
    };
    //登出
    void logout();
    //注册用户
    bool registerUser(const std::string& username, const std::string& password, Role role = USER);
    bool deleteCurrentUser();
}; 
