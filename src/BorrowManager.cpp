#include "../include/BorrowManager.h"
#include <stdexcept>
#include <ctime>

BorrowManager::BorrowManager(BookManager* bookManager, UserManager* userManager)
    : bookManager(bookManager), userManager(userManager) {}

bool BorrowManager::borrowBook(const std::string& isbn, const std::string& username) {
    const User* user = userManager->findUser(username);
    if (!user) return false;
    Book* book = bookManager->findBookByIsbn(isbn);
    if (!book) return false;
    for (size_t i = 0; i < records.getSize(); i++) {
        if (records[i].getBookIsbn() == isbn && 
            records[i].getUsername() == username && 
            !records[i].getIsReturned()) {
            return false;
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