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
#include "../include/MyQueue.h"
#include <map>

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
    // 检查用户是否已借阅此书
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getBookIsbn() == isbn && 
            records[i].getUsername() == username && 
            !records[i].getIsReturned()) {
            throw std::runtime_error("不可多次借阅同一本书");
        }
    }
    // 书已被借出，处理等待队列
    if (book->getStatus() == 1) {
        // 队列不存在则创建
        if (waitingQueues.find(isbn) == waitingQueues.end()) {
            waitingQueues[isbn] = MyQueue<std::string>();
        }
        MyQueue<std::string>& queue = waitingQueues[isbn];
        if (queue.contains(username)) {
            throw std::runtime_error("已在排队之中");
        } else {
            queue.enqueue(username);
            saveWaitingQueues("waiting_queues.json"); // 实时保存队列
            int pos = static_cast<int>(queue.size()) - 1;
            throw std::runtime_error(("已添加到等待队列，前方还有" + std::to_string(pos) + "人在排队").c_str());
        }
    }
    // 书可借，直接借阅
    time_t now = std::time(nullptr);
    time_t dueDate = now + (DEFAULT_BORROW_DAYS * 24 * 60 * 60);
    BorrowRecord record(isbn, username, now, dueDate);
    records.add(record);
    bookManager->updateBookStatus(isbn,1); //借出
    saveToFile("borrow_records.json"); // 实时保存借阅记录
    return true;
}

bool BorrowManager::returnBook(const std::string& isbn, const std::string& username) {
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getBookIsbn() == isbn && 
            records[i].getUsername() == username && 
            !records[i].getIsReturned()) {
            records[i].setReturnDate(std::time(nullptr));
            records[i].setIsReturned(true);
            bookManager->updateBookStatus(isbn,0); //归还
            // 检查等待队列
            auto it = waitingQueues.find(isbn);
            if (it != waitingQueues.end()) {
                MyQueue<std::string>& queue = it->second;
                if (!queue.isEmpty()) {
                    std::string nextUser = queue.front();
                    queue.dequeue();
                    saveWaitingQueues("waiting_queues.json"); // 实时保存队列
                    // 自动为队首用户借阅
                    try {
                        borrowBook(isbn, nextUser);
                        saveToFile("borrow_records.json"); // 自动借阅后保存记录
                    } catch (const std::exception& e) {
                        // 如果自动借阅失败（如用户已借阅等），忽略
                    }
                    // 如果队列空了，移除队列
                    if (queue.isEmpty()) {
                        waitingQueues.erase(it);
                        saveWaitingQueues("waiting_queues.json");
                    }
                }
            }
            saveToFile("borrow_records.json");
            saveWaitingQueues("waiting_queues.json");
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

bool BorrowManager::returnBookByRecordId(const std::string& recordId) {
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getRecordId() == recordId && !records[i].getIsReturned()) {
            records[i].setReturnDate(std::time(nullptr));
            records[i].setIsReturned(true);
            bookManager->updateBookStatus(records[i].getBookIsbn(), 0);
            // 自动处理等待队列
            std::string isbn = records[i].getBookIsbn();
            auto it = waitingQueues.find(isbn);
            if (it != waitingQueues.end()) {
                MyQueue<std::string>& queue = it->second;
                if (!queue.isEmpty()) {
                    std::string nextUser = queue.front();
                    queue.dequeue();
                    saveWaitingQueues("waiting_queues.json"); // 实时保存队列
                    try {
                        borrowBook(isbn, nextUser);
                        saveToFile("borrow_records.json"); // 自动借阅后保存记录
                    } catch (const std::exception& e) {
                        // 自动借阅失败忽略
                    }
                    if (queue.isEmpty()) {
                        waitingQueues.erase(it);
                        saveWaitingQueues("waiting_queues.json");
                    }
                }
            }
            saveToFile("borrow_records.json");
            saveWaitingQueues("waiting_queues.json");
            return true;
        }
    }
    return false;
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

//查找方法实现
BorrowRecord* BorrowManager::findByRecordId(MyVector<BorrowRecord> &record, const std::string& recordId){
    int index = record.hashFindByRecordId(recordId);
    return index >= 0 ? &record[index] : nullptr;
}

MyVector<BorrowRecord> BorrowManager::findByISBN(MyVector<BorrowRecord> &record, const std::string& ISBN){
    MyVector<BorrowRecord> result;
    for (size_t i = 0; i < record.getSize(); ++i) {
        const BorrowRecord& borrowRecord = record[i];
        if (borrowRecord.getIsbn() == ISBN) {
            result.add(borrowRecord);
        }
    }
    return result;
}

MyVector<BorrowRecord> BorrowManager::findByUsername(MyVector<BorrowRecord> &record, const std::string& username){
    MyVector<BorrowRecord> result;
    for (size_t i = 0; i < record.getSize(); ++i) {
        const BorrowRecord& borrowRecord = record[i];
        if (borrowRecord.getUsername().find(username) != std::string::npos) {
            result.add(borrowRecord);
        }
    }
    return result;
}

MyVector<BorrowRecord> BorrowManager::findByBorrowDate(MyVector<BorrowRecord> &record, const std::string& borrowDate){
    MyVector<BorrowRecord> sortedRecords = record;
    //  排序
    auto comp = [](const BorrowRecord& a, const BorrowRecord& b) { return a.getBorrowDateStr() < b.getBorrowDateStr(); };
    if (sortedRecords.getSize() > 1) {
        MyAlgorithm::sort(&sortedRecords[0], static_cast<int>(sortedRecords.getSize()), comp);
    }
    MyVector<BorrowRecord> result;
    auto getBorrowDate = [](const BorrowRecord& record) -> std::string {
        return record.getBorrowDateStr();
    };

    auto timeComparator = [](std::string a, std::string b) {
        return a < b;
    };
    int index = sortedRecords.binarySearch(borrowDate, getBorrowDate, timeComparator);
    if (index >= 0) {
        int left = index, right = index;
        while (left >= 0 && sortedRecords[left].getBorrowDateStr() == borrowDate) {
            result.push_back(sortedRecords[left]);
            --left;
        }
        while (right < sortedRecords.getSize() && sortedRecords[right].getBorrowDateStr() == borrowDate) {
            if (right > index) {
                result.push_back(sortedRecords[right]);
            }
            ++right;
        }
    }
    return result;
}

MyVector<BorrowRecord> BorrowManager::findByDueDate(MyVector<BorrowRecord> &record, const std::string& dueDate){
    MyVector<BorrowRecord> sortedRecords = record;
    //  排序
    auto comp = [](const BorrowRecord& a, const BorrowRecord& b) { return a.getDueDateStr() < b.getDueDateStr(); };
    if (sortedRecords.getSize() > 1) {
        MyAlgorithm::sort(&sortedRecords[0], static_cast<int>(sortedRecords.getSize()), comp);
    }
    MyVector<BorrowRecord> result;
    auto getDueDate = [](const BorrowRecord& record) -> std::string {
        return record.getDueDateStr();
    };

    auto timeComparator = [](std::string a, std::string b) {
        return a < b;
    };
    int index = sortedRecords.binarySearch(dueDate, getDueDate, timeComparator);
    if (index >= 0) {
        int left = index, right = index;
        while (left >= 0 && sortedRecords[left].getDueDateStr() == dueDate) {
            result.push_back(sortedRecords[left]);
            --left;
        }
        while (right < sortedRecords.getSize() && sortedRecords[right].getDueDateStr() == dueDate) {
            if (right > index) {
                result.push_back(sortedRecords[right]);
            }
            ++right;
        }
    }
    return result;
}

MyVector<BorrowRecord> BorrowManager::findByStatus(MyVector<BorrowRecord> &record, const std::string& status){
    MyVector<BorrowRecord> result;
    for (size_t i = 0; i < record.getSize(); ++i) {
        const BorrowRecord& borrowRecord = record[i];
        if (borrowRecord.getStatus() == status) {
            result.add(borrowRecord);
        }
    }
    return result;
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

int BorrowManager::getWaitingCount(const std::string& isbn) const {
    auto it = waitingQueues.find(isbn);
    if (it == waitingQueues.end()) return 0;
    return static_cast<int>(it->second.size());
}

bool BorrowManager::isUserInQueue(const std::string& isbn, const std::string& username) const {
    auto it = waitingQueues.find(isbn);
    if (it == waitingQueues.end()) return false;
    return it->second.contains(username);
}

void BorrowManager::saveWaitingQueues(const QString& filename) const {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "无法打开队列文件进行写入:" << filename;
        return;
    }
    QJsonObject rootObj;
    for (const auto& pair : waitingQueues) {
        rootObj[QString::fromStdString(pair.first)] = pair.second.toJsonArray();
    }
    QJsonDocument doc(rootObj);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

bool BorrowManager::loadWaitingQueues(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开队列文件进行读取:" << filename;
        return false;
    }
    QByteArray jsonData = file.readAll();
    file.close();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "队列JSON解析错误:" << parseError.errorString();
        return false;
    }
    QJsonObject rootObj = doc.object();
    waitingQueues.clear();
    for (auto it = rootObj.begin(); it != rootObj.end(); ++it) {
        std::string isbn = it.key().toStdString();
        MyQueue<std::string> queue;
        queue.fromJsonArray(it.value().toArray());
        waitingQueues[isbn] = queue;
    }
    return true;
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
