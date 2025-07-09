#include "../include/BorrowManager.h"
#include <stdexcept>
#include <ctime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include "../include/Mysort.h"

BorrowManager::BorrowManager(BookManager* bookManager, UserManager* userManager)
    : bookManager(bookManager), userManager(userManager) {}

bool BorrowManager::borrowBook(const std::string& isbn, const std::string& username) {
    const User* user = userManager->findUser(username);
    if (!user) {
        throw std::runtime_error("用户不存在");
    }
    Book* book = bookManager->findBookByIsbn(isbn);
    if (!book) {
        throw std::runtime_error("图书不存在");
    }
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getBookIsbn() == isbn && 
            records[i].getUsername() == username && 
            !records[i].getIsReturned()) {
            throw std::runtime_error("该用户已借阅此书且未归还");
        }
    }
    time_t now = std::time(nullptr);
    time_t dueDate = now + (DEFAULT_BORROW_DAYS * 24 * 60 * 60);
    BorrowRecord record(isbn, username, now, dueDate);
    records.add(record);
    return true;
}

bool BorrowManager::returnBook(const std::string& isbn, const std::string& username) {
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getBookIsbn() == isbn && 
            records[i].getUsername() == username && 
            !records[i].getIsReturned()) {
            records[i].setReturnDate(std::time(nullptr));
            records[i].setIsReturned(true);
            return true;
        }
    }
    return false;
}

bool BorrowManager::renewBook(const std::string& isbn, const std::string& username) {
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getBookIsbn() == isbn && 
            records[i].getUsername() == username && 
            !records[i].getIsReturned()) {
            time_t newDueDate = std::time(nullptr) + (DEFAULT_BORROW_DAYS * 24 * 60 * 60);
            records[i].setDueDate(newDueDate);
            return true;
        }
    }
    return false;
}

// 通过ID操作的方法
void BorrowManager::returnBook(int recordId) {
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getId() == recordId) {
            if (records[i].getIsReturned()) {
                throw std::runtime_error("该记录已归还");
            }
            records[i].setReturnDate(std::time(nullptr));
            records[i].setIsReturned(true);
            return;
        }
    }
    throw std::runtime_error("未找到指定的借阅记录");
}

void BorrowManager::renewBook(int recordId) {
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getId() == recordId) {
            if (records[i].getIsReturned()) {
                throw std::runtime_error("已归还的图书无法续借");
            }
            time_t newDueDate = std::time(nullptr) + (DEFAULT_BORROW_DAYS * 24 * 60 * 60);
            records[i].setDueDate(newDueDate);
            return;
        }
    }
    throw std::runtime_error("未找到指定的借阅记录");
}

MyVector<BorrowRecord> BorrowManager::getUserBorrowRecords(const std::string& username) {
    MyVector<BorrowRecord> userRecords;
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getUsername() == username) {
            userRecords.add(records[i]);
        }
    }
    return userRecords;
}

MyVector<BorrowRecord> BorrowManager::getBookBorrowRecords(const std::string& isbn) {
    MyVector<BorrowRecord> bookRecords;
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getBookIsbn() == isbn) {
            bookRecords.add(records[i]);
        }
    }
    return bookRecords;
}

MyVector<BorrowRecord> BorrowManager::getOverdueRecords() {
    MyVector<BorrowRecord> overdueRecords;
    time_t now = std::time(nullptr);
    for (size_t i = 0; i < records.getSize(); i++) {
        if (!records[i].getIsReturned() && records[i].getDueDate() < now) {
            overdueRecords.add(records[i]);
        }
    }
    return overdueRecords;
}

size_t BorrowManager::getBorrowCount(const std::string& username) const {
    size_t count = 0;
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getUsername() == username && !records[i].getIsReturned()) {
            count++;
        }
    }
    return count;
}

size_t BorrowManager::getOverdueCount(const std::string& username) const {
    size_t count = 0;
    time_t now = std::time(nullptr);
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getUsername() == username && 
            !records[i].getIsReturned() && 
            records[i].getDueDate() < now) {
            count++;
        }
    }
    return count;
}

// 数据持久化方法实现
bool BorrowManager::saveToFile(const QString& filename) const {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "无法打开文件进行写入:" << filename;
        return false;
    }
    
    QJsonArray recordsArray;
    for (size_t i = 0; i < records.getSize(); ++i) {
        recordsArray.append(records[i].toJson());
    }
    
    QJsonObject rootObject;
    rootObject["records"] = recordsArray;
    rootObject["count"] = static_cast<int>(records.getSize());
    
    QJsonDocument doc(rootObject);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
    
    qint64 bytesWritten = file.write(jsonData);
    file.close();
    
    if (bytesWritten == -1) {
        qDebug() << "写入文件失败:" << filename;
        return false;
    }
    
    qDebug() << "成功保存" << records.getSize() << "条借阅记录到文件:" << filename;
    return true;
}

bool BorrowManager::loadFromFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开文件进行读取:" << filename;
        return false;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误:" << parseError.errorString();
        return false;
    }
    
    QJsonObject rootObject = doc.object();
    if (!rootObject.contains("records")) {
        qDebug() << "文件格式错误: 缺少records字段";
        return false;
    }
    
    QJsonArray recordsArray = rootObject["records"].toArray();
    
    // 清空现有数据
    records = MyVector<BorrowRecord>();
    
    // 加载借阅记录数据
    int successCount = 0;
    for (const QJsonValue& value : recordsArray) {
        if (value.isObject()) {
            BorrowRecord record;
            record.fromJson(value.toObject());
            try {
                records.add(record);
                successCount++;
            } catch (const std::exception& e) {
                qDebug() << "加载借阅记录失败:" << e.what();
                continue;
            }
        }
    }
    
    qDebug() << "成功加载" << successCount << "条借阅记录从文件:" << filename;
    return successCount > 0;
}

// 排序功能实现
static bool compareBorrowRecords(const BorrowRecord& a, const BorrowRecord& b, BorrowSortBy sortBy, BorrowSortOrder order) {
    bool result = false;
    
    switch (sortBy) {
    case BorrowSortBy::RECORD_ID:
        result = a.getId() < b.getId();
        break;
    case BorrowSortBy::ISBN:
        result = a.getIsbn() < b.getIsbn();
        break;
    case BorrowSortBy::USERNAME:
        result = a.getUsername() < b.getUsername();
        break;
    case BorrowSortBy::BORROW_DATE:
        result = a.getBorrowDate() < b.getBorrowDate();
        break;
    case BorrowSortBy::DUE_DATE:
        result = a.getDueDate() < b.getDueDate();
        break;
    case BorrowSortBy::RETURN_DATE:
        result = a.getReturnDate() < b.getReturnDate();
        break;
    case BorrowSortBy::STATUS:
        result = a.getStatus() < b.getStatus();
        break;
    }
    
    return order == BorrowSortOrder::ASCENDING ? result : !result;
}

void BorrowManager::sortBorrowRecords(MyVector<BorrowRecord> &recordList, BorrowSortBy sortBy, BorrowSortOrder order) const {
    if (recordList.getSize() <= 1) {
        return;
    }
    
    auto comp = [sortBy, order](const BorrowRecord& a, const BorrowRecord& b) -> bool {
        return compareBorrowRecords(a, b, sortBy, order);
    };
    
    BorrowRecord* arr = &recordList[0];
    int length = static_cast<int>(recordList.getSize());
    MyAlgorithm::sort(arr, length, comp);
}

MyVector<BorrowRecord> BorrowManager::getSortedBorrowRecords(BorrowSortBy sortBy, BorrowSortOrder order) const {
    MyVector<BorrowRecord> sortedRecords = records;
    sortBorrowRecords(sortedRecords, sortBy, order);
    return sortedRecords;
}

MyVector<BorrowRecord> BorrowManager::sortSearchResults(const MyVector<BorrowRecord> &searchResults, BorrowSortBy sortBy, BorrowSortOrder order) const {
    MyVector<BorrowRecord> sortedResults = searchResults;
    sortBorrowRecords(sortedResults, sortBy, order);
    return sortedResults;
} 