#include "../include/BookManager.h"
#include <iterator>
#include "../include/Mysort.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstring>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

void BookManager::addBook(const Book& book) {
    books.push_back(book);
}

bool BookManager::removeBook(const std::string& isbn) {
    int index = books.hashFindByIsbn(isbn);
    if (index >= 0) {
        books.removeAt(index);
        return true;
    }
    return false;
}

bool BookManager::updateBook(const std::string& isbn, const Book& updatedBook) {
    int index = books.hashFindByIsbn(isbn);
    if (index >= 0 && updatedBook.getIsbn() == isbn) {
        books[index] = updatedBook;
        return true;
    }
    return false;
}

bool BookManager::updateBookField(const std::string& isbn, const std::string& field, const std::string& newValue) {
    int index = books.hashFindByIsbn(isbn);
    if (index >= 0) {
        try {
            if (field == "title") {
                books[index].setTitle(newValue);
            } else if (field == "author") {
                books[index].setAuthor(newValue);
            } else if (field == "publisher") {
                books[index].setPublisher(newValue);
            } else if (field == "year") {
                int year = std::stoi(newValue);
                books[index].setPublishYear(year);
            } else {
                return false;
            }
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    return false;
}

Book* BookManager::findBookByIsbn(const std::string& isbn) {
    auto getIsbn = [](const Book& book) -> std::string { return book.getIsbn(); };
    auto comp = [](const std::string& a, const std::string& b) { return a < b; };
    int index = books.hashFindByIsbn(isbn);
    return index >= 0 ? &books[index] : nullptr;
}

MyVector<Book> BookManager::findBooksByPublisher(const std::string& publisher) {
    MyVector<Book> result;
    for (size_t i = 0; i < books.getSize(); ++i) {
        const Book& book = books[i];
        if (book.getPublisher().find(publisher) != std::string::npos) {
            result.add(book);
        }
    }
    return result;
}

MyVector<Book> BookManager::findBooksByYear(int year) {
    // 1. 拷贝一份 books
    MyVector<Book> sortedBooks = books;
    // 2. 排序
    auto comp = [](const Book& a, const Book& b) { return a.getPublishYear() < b.getPublishYear(); };
    if (sortedBooks.getSize() > 1) {
        MyAlgorithm::sort(&sortedBooks[0], static_cast<int>(sortedBooks.getSize()), comp);
    }
    // 3. 二分查找
    MyVector<Book> result;
    auto getYear = [](const Book& book) -> int { return book.getPublishYear(); };
    auto yearComp = [](const int a, const int b) { return a < b; };
    int index = sortedBooks.binarySearch(year, getYear, yearComp);
    if (index >= 0) {
        int left = index, right = index;
        while (left >= 0 && sortedBooks[left].getPublishYear() == year) {
            result.push_back(sortedBooks[left]);
            --left;
        }
        while (right < sortedBooks.getSize() && sortedBooks[right].getPublishYear() == year) {
            if (right > index) {
                result.push_back(sortedBooks[right]);
            }
            ++right;
        }
    }
    return result;
}

MyVector<Book> BookManager::findBooksByYearRange(int startYear, int endYear) {
    MyVector<Book> result;
    auto getYear = [](const Book& book) -> int { return book.getPublishYear(); };
    auto comp = [](const int a, const int b) { return a < b; };
    int startIndex = books.binarySearch(startYear, getYear, comp);
    if (startIndex < 0) {
        startIndex = 0;
    }
    for (int i = startIndex; i < books.getSize(); ++i) {
        int year = books[i].getPublishYear();
        if (year > endYear) break;
        if (year >= startYear) {
            result.push_back(books[i]);
        }
    }
    return result;
}

const MyVector<Book>& BookManager::getAllBooks() const {
    return books;
}

MyVector<Book> BookManager::findBooksByTitle(const std::string& title) {
    MyVector<Book> result;
    for (size_t i = 0; i < books.getSize(); ++i) {
        const Book& book = books[i];
        if (book.getTitle().find(title) != std::string::npos) {
            result.add(book);
        }
    }
    return result;

}

MyVector<Book> BookManager::findBooksByAuthor(const std::string& author) {
    MyVector<Book> result;
    for (size_t i = 0; i < books.getSize(); ++i) {
        const Book& book = books[i];
        if (book.getAuthor().find(author) != std::string::npos) {
            result.add(book);
        }
    }
    return result;
}

size_t BookManager::getBookCount() const {
    return books.getSize();
}

static bool compareBooks(const Book& a, const Book& b, SortBy sortBy, SortOrder order) {
    bool result;
    switch (sortBy) {
        case SortBy::TITLE:
            result = a.getTitle() < b.getTitle();
            break;
        case SortBy::AUTHOR:
            result = a.getAuthor() < b.getAuthor();
            break;
        case SortBy::PUBLISHER:
            result = a.getPublisher() < b.getPublisher();
            break;
        case SortBy::YEAR:
            result = a.getPublishYear() < b.getPublishYear();
            break;
        default:
            result = false;
    }
    return order == SortOrder::ASCENDING ? result : !result;
}

void BookManager::sortBooks(MyVector<Book>& bookList, SortBy sortBy, SortOrder order) const {
    size_t length = bookList.getSize();
    if (length <= 1) return;
    Book* arr = new Book[length];
    for (size_t i = 0; i < length; ++i) {
        arr[i] = bookList[i];
    }
    auto comp = [sortBy, order](const Book& a, const Book& b) -> bool {
        return compareBooks(a, b, sortBy, order);
    };
    MyAlgorithm::sort(arr, length, comp);
    for (size_t i = 0; i < length; ++i) {
        bookList[i] = arr[i];
    }
    delete[] arr;
}

MyVector<Book> BookManager::getSortedBooks(SortBy sortBy, SortOrder order) const {
    MyVector<Book> sortedBooks = books;
    sortBooks(sortedBooks, sortBy, order);
    return sortedBooks;
}

MyVector<Book> BookManager::sortSearchResults(const MyVector<Book>& searchResults, SortBy sortBy, SortOrder order) const {
    MyVector<Book> sortedResults = searchResults;
    sortBooks(sortedResults, sortBy, order);
    return sortedResults;
}

bool BookManager::importBooksFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return false;
    }
    std::string line;
    int successCount = 0;
    int totalCount = 0;
    std::getline(file, line);
    while (std::getline(file, line)) {
        totalCount++;
        Book book;
        if (parseBookLine(line, book)) {
            try {
                addBook(book);
                successCount++;
            } catch (const std::exception& e) {
                std::cerr << "添加图书失败: " << e.what() << std::endl;
                continue;
            }
        }
    }
    file.close();
    std::cout << "导入完成: 成功 " << successCount << "/" << totalCount << " 条记录" << std::endl;
    return successCount > 0;
}

bool BookManager::parseBookLine(const std::string& line, Book& book) {
    try {
        auto getValue = [](const std::string& src, const std::string& key) -> std::string {
            size_t keyPos = src.find(key);
            if (keyPos == std::string::npos) return "";
            size_t valStart = src.find("'", keyPos + key.length());
            if (valStart == std::string::npos) return "";
            valStart += 1;
            size_t valEnd = src.find("'", valStart);
            if (valEnd == std::string::npos) return "";
            return src.substr(valStart, valEnd - valStart);
        };
        book.setTitle(getValue(line, "'书名': "));
        book.setAuthor(getValue(line, "'作者': "));
        book.setPublisher(getValue(line, "'出版社': "));
        book.setIsbn(getValue(line, "'ISBN': "));
        // 出版年限为数字
        const std::string yearKeyStr = "'出版年限': ";
        size_t yearKey = line.find(yearKeyStr);
        if (yearKey == std::string::npos) return false;
        size_t yearStart = yearKey + yearKeyStr.length();
        size_t yearEnd = line.find("}", yearStart);
        std::string yearStr = line.substr(yearStart, yearEnd - yearStart);
        int year = std::stoi(yearStr);
        book.setPublishYear(year);
        return true;
    } catch (...) {
        return false;
    }
}

bool BookManager::exportBooksToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法创建文件: " << filename << std::endl;
        return false;
    }
    for (size_t i = 0; i < books.getSize(); i++) {
        const Book& book = books[i];
        file << "{'书名': '" << book.getTitle() << "', "
             << "'作者': '" << book.getAuthor() << "', "
             << "'出版社': '" << book.getPublisher() << "', "
             << "'ISBN': '" << book.getIsbn() << "', "
             << "'出版年限': " << book.getPublishYear() << "}\n";
    }
    file.close();
    return true;
}

void BookManager::addBooks(const MyVector<Book>& booksVec) {
    for (size_t i = 0; i < booksVec.getSize(); ++i) {
        try {
            addBook(booksVec[i]);
        } catch (...) {
            // 忽略重复或异常
        }
    }
}

// 数据持久化方法实现
bool BookManager::saveToFile(const QString& filename) const {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "无法打开文件进行写入:" << filename;
        return false;
    }
    
    QJsonArray booksArray;
    for (size_t i = 0; i < books.getSize(); ++i) {
        booksArray.append(books[i].toJson());
    }
    
    QJsonObject rootObject;
    rootObject["books"] = booksArray;
    rootObject["count"] = static_cast<int>(books.getSize());
    
    QJsonDocument doc(rootObject);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
    
    qint64 bytesWritten = file.write(jsonData);
    file.close();
    
    if (bytesWritten == -1) {
        qDebug() << "写入文件失败:" << filename;
        return false;
    }
    
    qDebug() << "成功保存" << books.getSize() << "本图书到文件:" << filename;
    return true;
}

bool BookManager::loadFromFile(const QString& filename) {
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
    if (!rootObject.contains("books")) {
        qDebug() << "文件格式错误: 缺少books字段";
        return false;
    }
    
    QJsonArray booksArray = rootObject["books"].toArray();
    
    // 清空现有数据
    books = MyVector<Book>();
    
    // 加载图书数据
    int successCount = 0;
    for (const QJsonValue& value : booksArray) {
        if (value.isObject()) {
            Book book;
            book.fromJson(value.toObject());
            try {
                addBook(book);
                successCount++;
            } catch (const std::exception& e) {
                qDebug() << "加载图书失败:" << e.what();
                continue;
            }
        }
    }
    
    qDebug() << "成功加载" << successCount << "本图书从文件:" << filename;
    return successCount > 0;
} 
