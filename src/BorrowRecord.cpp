#include "../include/BorrowRecord.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QVariant>

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

// 数据持久化方法实现
QJsonObject BorrowRecord::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["bookIsbn"] = QString::fromStdString(bookIsbn);
    json["username"] = QString::fromStdString(username);
    json["borrowDate"] = static_cast<qint64>(borrowDate);
    json["dueDate"] = static_cast<qint64>(dueDate);
    json["returnDate"] = static_cast<qint64>(returnDate);
    json["isReturned"] = isReturned;
    return json;
}

void BorrowRecord::fromJson(const QJsonObject& json) {
    if (json.contains("id")) {
        id = json["id"].toInt();
        // 更新nextId以确保ID唯一性
        if (id >= nextId) {
            nextId = id + 1;
        }
    }
    if (json.contains("bookIsbn")) {
        bookIsbn = json["bookIsbn"].toString().toStdString();
    }
    if (json.contains("username")) {
        username = json["username"].toString().toStdString();
    }
    if (json.contains("borrowDate")) {
        borrowDate = static_cast<time_t>(json["borrowDate"].toVariant().toLongLong());
    }
    if (json.contains("dueDate")) {
        dueDate = static_cast<time_t>(json["dueDate"].toVariant().toLongLong());
    }
    if (json.contains("returnDate")) {
        returnDate = static_cast<time_t>(json["returnDate"].toVariant().toLongLong());
    }
    if (json.contains("isReturned")) {
        isReturned = json["isReturned"].toBool();
    }
}