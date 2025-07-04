#ifndef BORROW_RECORD_H
#define BORROW_RECORD_H

#include <string>
#include <ctime>
#include "Book.h"
#include "User.h"

class BorrowRecord {
private:
    int id;
    std::string bookIsbn;
    std::string username;
    time_t borrowDate;
    time_t dueDate;
    time_t returnDate;
    bool isReturned;
    static int nextId;
public:
    BorrowRecord(const std::string& bookIsbn, 
                const std::string& username,
                time_t borrowDate,
                time_t dueDate);
    BorrowRecord() : id(0), borrowDate(0), dueDate(0), returnDate(0), isReturned(false) {}
    
    // 获取方法
    int getId() const;
    std::string getRecordId() const;
    std::string getIsbn() const;
    std::string getBookIsbn() const;
    std::string getUsername() const;
    time_t getBorrowDate() const;
    time_t getDueDate() const;
    time_t getReturnDate() const;
    bool getIsReturned() const;
    
    // 获取格式化的日期字符串
    std::string getBorrowDateStr() const;
    std::string getDueDateStr() const;
    std::string getReturnDateStr() const;
    std::string getStatus() const;
    
    // 设置方法
    void setDueDate(time_t date);
    void setReturnDate(time_t date);
    void setIsReturned(bool returned);
};

#endif 