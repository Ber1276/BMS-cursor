#include "../include/BorrowRecord.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

// 静态成员初始化
int BorrowRecord::nextId = 1;

BorrowRecord::BorrowRecord(const std::string& bookIsbn, 
                         const std::string& username,
                         time_t borrowDate,
                         time_t dueDate)
    : id(nextId++)
    , bookIsbn(bookIsbn)
    , username(username)
    , borrowDate(borrowDate)
    , dueDate(dueDate)
    , returnDate(0)
    , isReturned(false) {
}

// getter 方法实现
int BorrowRecord::getId() const {
    return id;
}

std::string BorrowRecord::getRecordId() const {
    std::stringstream ss;
    ss << "REC" << std::setfill('0') << std::setw(6) << id;
    return ss.str();
}

std::string BorrowRecord::getIsbn() const {
    return bookIsbn;
}

std::string BorrowRecord::getBookIsbn() const {
    return bookIsbn;
}

std::string BorrowRecord::getUsername() const {
    return username;
}

time_t BorrowRecord::getBorrowDate() const {
    return borrowDate;
}

time_t BorrowRecord::getDueDate() const {
    return dueDate;
}

time_t BorrowRecord::getReturnDate() const {
    return returnDate;
}

bool BorrowRecord::getIsReturned() const {
    return isReturned;
}

// 格式化日期字符串
std::string BorrowRecord::getBorrowDateStr() const {
    if (borrowDate == 0) return "";
    std::tm* tm = std::localtime(&borrowDate);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << (tm->tm_year + 1900) << "-"
       << std::setfill('0') << std::setw(2) << (tm->tm_mon + 1) << "-"
       << std::setfill('0') << std::setw(2) << tm->tm_mday;
    return ss.str();
}

std::string BorrowRecord::getDueDateStr() const {
    if (dueDate == 0) return "";
    std::tm* tm = std::localtime(&dueDate);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << (tm->tm_year + 1900) << "-"
       << std::setfill('0') << std::setw(2) << (tm->tm_mon + 1) << "-"
       << std::setfill('0') << std::setw(2) << tm->tm_mday;
    return ss.str();
}

std::string BorrowRecord::getReturnDateStr() const {
    if (returnDate == 0) return "";
    std::tm* tm = std::localtime(&returnDate);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << (tm->tm_year + 1900) << "-"
       << std::setfill('0') << std::setw(2) << (tm->tm_mon + 1) << "-"
       << std::setfill('0') << std::setw(2) << tm->tm_mday;
    return ss.str();
}

std::string BorrowRecord::getStatus() const {
    if (isReturned) {
        return "已归还";
    }
    
    time_t now = std::time(nullptr);
    if (dueDate < now) {
        return "逾期";
    } else {
        return "借阅中";
    }
}

// setter 方法实现
void BorrowRecord::setDueDate(time_t date) {
    if (date < borrowDate) {
        throw std::invalid_argument("到期日期不能早于借阅日期");
    }
    dueDate = date;
}

void BorrowRecord::setReturnDate(time_t date) {
    if (date < borrowDate) {
        throw std::invalid_argument("归还日期不能早于借阅日期");
    }
    returnDate = date;
}

void BorrowRecord::setIsReturned(bool returned) {
    isReturned = returned;
}