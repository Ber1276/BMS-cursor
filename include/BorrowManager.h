#ifndef BORROW_MANAGER_H
#define BORROW_MANAGER_H

#include "MyVector.h"
#include "BorrowRecord.h"
#include "BookManager.h"
#include "User.h"

class BorrowManager {
private:
    MyVector<BorrowRecord> records;
    BookManager* bookManager;
    UserManager* userManager;
    static const int DEFAULT_BORROW_DAYS = 30;
public:
    BorrowManager(BookManager* bookManager, UserManager* userManager);
    bool borrowBook(const std::string& isbn, const std::string& username);
    bool returnBook(const std::string& isbn, const std::string& username);
    bool renewBook(const std::string& isbn, const std::string& username);
    MyVector<BorrowRecord> getUserBorrowRecords(const std::string& username);
    MyVector<BorrowRecord> getBookBorrowRecords(const std::string& isbn);
    MyVector<BorrowRecord> getOverdueRecords();
    size_t getBorrowCount(const std::string& username) const;
    size_t getOverdueCount(const std::string& username) const;
};

#endif 