#pragma once
#include "User.h"
class PermissionManager {
private:
    const User* currentUser = nullptr;
    UserManager* userManager = nullptr;
public:
    PermissionManager(UserManager* userManager) : userManager(userManager) {};
    bool login(const std::string& username, const std::string& password);
    const User* getCurrentUser() const;
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
    void logout();
    bool registerUser(const std::string& username, const std::string& password, Role role = USER);
    bool deleteCurrentUser();
}; 