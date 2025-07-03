#ifndef BORROW_RECORD_H
#define BORROW_RECORD_H

#include <string>
#include <ctime>
#include "Book.h"
#include "User.h"

class BorrowRecord {
private:
    std::string recordId;
    std::string bookIsbn;
    std::string username;
    time_t borrowDate;
    time_t dueDate;
    time_t returnDate;
    bool isReturned;
public:
    BorrowRecord(const std::string& bookIsbn, 
                const std::string& username,
                time_t borrowDate,
                time_t dueDate);
    BorrowRecord() : borrowDate(0), dueDate(0), returnDate(0), isReturned(false) {}
    std::string getRecordId() const;
    std::string getBookIsbn() const;
    std::string getUsername() const;
    time_t getBorrowDate() const;
    time_t getDueDate() const;
    time_t getReturnDate() const;
    bool getIsReturned() const;
    void setDueDate(time_t date);
    void setReturnDate(time_t date);
    void setIsReturned(bool returned);
};

#endif 