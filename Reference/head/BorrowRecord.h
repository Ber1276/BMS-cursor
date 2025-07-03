#ifndef BORROW_RECORD_H
#define BORROW_RECORD_H

#include <string>
#include <ctime>
#include "Book.h"
#include "User.h"

class BorrowRecord {
private:
    std::string recordId;        // 借阅记录ID
    std::string bookIsbn;        // 图书ISBN
    std::string username;        // 借阅者用户名
    time_t borrowDate;          // 借阅日期
    time_t dueDate;             // 应还日期
    time_t returnDate;          // 实际归还日期
    bool isReturned;            // 是否已归还

public:
    BorrowRecord(const std::string& bookIsbn, 
                const std::string& username,
                time_t borrowDate,
                time_t dueDate);

    
    // getter 方法
    std::string getRecordId() const;
    std::string getBookIsbn() const;
    std::string getUsername() const;
    time_t getBorrowDate() const;
    time_t getDueDate() const;
    time_t getReturnDate() const;
    bool getIsReturned() const;

    // setter 方法
    void setDueDate(time_t date);
    void setReturnDate(time_t date);
    void setIsReturned(bool returned);
};

#endif