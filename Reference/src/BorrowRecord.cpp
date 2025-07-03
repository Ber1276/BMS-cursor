#include "../head/BorrowRecord.h"
#include <sstream>
#include <iomanip>
#include <chrono>

BorrowRecord::BorrowRecord(const std::string& bookIsbn, 
                         const std::string& username,
                         time_t borrowDate,
                         time_t dueDate)
    : bookIsbn(bookIsbn)
    , username(username)
    , borrowDate(borrowDate)
    , dueDate(dueDate)
    , returnDate(0)
    , isReturned(false) {
    // 生成唯一的记录ID：ISBN + 用户名 + 时间戳
    std::stringstream ss;
    ss << bookIsbn << "_" << username << "_" << borrowDate;
    recordId = ss.str();
}

// getter 方法实现
std::string BorrowRecord::getRecordId() const {
    return recordId;
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

// setter 方法实现
void BorrowRecord::setReturnDate(time_t date) {
    if (date < borrowDate) {
        throw std::invalid_argument("归还日期不能早于借阅日期");
    }
    returnDate = date;
}

void BorrowRecord::setIsReturned(bool returned) {
    isReturned = returned;
}
void BorrowRecord::setDueDate(time_t date) {
    if (date < borrowDate) {
        throw std::invalid_argument("到期日期不能早于借阅日期");
    }
    dueDate = date;
}