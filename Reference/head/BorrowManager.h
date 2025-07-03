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
    
    // 默认借阅期限（天）
    static const int DEFAULT_BORROW_DAYS = 30;

public:
    BorrowManager(BookManager* bookManager, UserManager* userManager);

    // 借书
    bool borrowBook(const std::string& isbn, const std::string& username);
    
    // 还书
    bool returnBook(const std::string& isbn, const std::string& username);
    
    // 续借
    bool renewBook(const std::string& isbn, const std::string& username);
    
    // 查询功能
    MyVector<BorrowRecord> getUserBorrowRecords(const std::string& username);
    MyVector<BorrowRecord> getBookBorrowRecords(const std::string& isbn);
    MyVector<BorrowRecord> getOverdueRecords();
    
    // 统计功能
    size_t getBorrowCount(const std::string& username) const;
    size_t getOverdueCount(const std::string& username) const;
};

#endif