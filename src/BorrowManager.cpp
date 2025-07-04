#include "../include/BorrowManager.h"
#include <stdexcept>
#include <ctime>

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