#ifndef BORROW_MANAGER_H
#define BORROW_MANAGER_H

#include "MyVector.h"
#include "BorrowRecord.h"
#include "BookManager.h"
#include "User.h"

// 前向声明
class QString;

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
    
    // 通过ID操作的方法
    void returnBook(int recordId);
    void renewBook(int recordId);
    
    MyVector<BorrowRecord> getUserBorrowRecords(const std::string& username);
    MyVector<BorrowRecord> getBookBorrowRecords(const std::string& isbn);
    MyVector<BorrowRecord> getOverdueRecords();
    const MyVector<BorrowRecord>& getAllBorrowRecords() const { return records; }
    size_t getBorrowCount(const std::string& username) const;
    size_t getOverdueCount(const std::string& username) const;
    
    // 数据持久化方法
    bool saveToFile(const QString& filename) const;
    bool loadFromFile(const QString& filename);
};

#endif 