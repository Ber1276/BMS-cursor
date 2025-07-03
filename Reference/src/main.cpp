#include "../head/User.h"
#include "../head/PermissionManager.h"
#include <windows.h>
#include <iostream>
#include "../head/BookManager.h"

int main(){
    SetConsoleOutputCP(CP_UTF8);

    UserManager userManager = UserManager();
    PermissionManager permissionManager = PermissionManager(&userManager);
    BookManager bookManager = BookManager();

    bookManager.addBook({
        "978-7-121-30000-0",
        "C++ Primer",
        "Stanley B. Lippman",
        "Addison-Wesley",
        2012
    });

    bookManager.addBook({
        "978-7-121-30001-7",
        "Effective C++",
        "Scott Meyers",
        "Addison-Wesley",
        2014
    });

    Book* book = bookManager.findBookByIsbn("978-7-121-30001-7");
    if (book) {
        std::cout << "Found book: " << book->getTitle() << " by " << book->getAuthor() << std::endl;
    } else {
        std::cout << "Book not found." << std::endl;
    }

    userManager.addUser({
        "leisure",
        "123456",
        USER
    });
    userManager.printAllUsers();

    userManager.addUser({
        "leisure123",
        "123456",
        ADMIN
    });

    userManager.printAllUsers();

    if (permissionManager.login("leisure123","123456"))
    {
        std::cout << "login successful" << std::endl;

        try{
            permissionManager.requirePermission(ADMIN, [&]() {
                MyVector<std::string> usersToRemove;
                usersToRemove.push_back("leisure123");
                userManager.adminRemoveUsers(usersToRemove);
            });
        }catch(const std::exception& e){
            std::cerr << e.what() << '\n';
        } 
    }else {
        std::cerr << "login failed" << std::endl;
    }
    
    userManager.printAllUsers();

    return 0;

}