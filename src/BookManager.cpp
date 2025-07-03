#include "../include/BookManager.h"
#include <iterator>
#include "../include/Mysort.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstring>

void BookManager::addBook(const Book& book) {
    books.push_back(book);
}

bool BookManager::removeBook(const std::string& isbn) {
    auto getIsbn = [](const Book& book) -> std::string { return book.getIsbn(); };
    auto comp = [](const std::string& a, const std::string& b) { return a < b; };
    int index = books.binarySearch(isbn, getIsbn, comp);
    if (index >= 0) {
        books.removeAt(index);
        return true;
    }
    return false;
}

bool BookManager::updateBook(const std::string& isbn, const Book& updatedBook) {
    auto getIsbn = [](const Book& book) -> std::string { return book.getIsbn(); };
    auto comp = [](const std::string& a, const std::string& b) { return a < b; };
    int index = books.binarySearch(isbn, getIsbn, comp);
    if (index >= 0 && updatedBook.getIsbn() == isbn) {
        books[index] = updatedBook;
        return true;
    }
    return false;
}

bool BookManager::updateBookField(const std::string& isbn, const std::string& field, const std::string& newValue) {
    auto getIsbn = [](const Book& book) -> std::string { return book.getIsbn(); };
    auto comp = [](const std::string& a, const std::string& b) { return a < b; };
    int index = books.binarySearch(isbn, getIsbn, comp);
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
    auto getPublisher = [](const Book& book) -> std::string { return book.getPublisher(); };
    auto comp = [](const std::string& a, const std::string& b) { return a < b; };
    int index = books.binarySearch(publisher, getPublisher, comp);
    if (index >= 0) {
        int left = index, right = index;
        while (left >= 0 && books[left].getPublisher() == publisher) {
            result.push_back(books[left]);
            --left;
        }
        while (right < books.getSize() && books[right].getPublisher() == publisher) {
            if (right > index) {
                result.push_back(books[right]);
            }
            ++right;
        }
    }
    return result;
}

MyVector<Book> BookManager::findBooksByYear(int year) {
    MyVector<Book> result;
    auto getYear = [](const Book& book) -> int { return book.getPublishYear(); };
    auto comp = [](const int a, const int b) { return a < b; };
    int index = books.binarySearch(year, getYear, comp);
    if (index >= 0) {
        int left = index, right = index;
        while (left >= 0 && books[left].getPublishYear() == year) {
            result.push_back(books[left]);
            --left;
        }
        while (right < books.getSize() && books[right].getPublishYear() == year) {
            if (right > index) {
                result.push_back(books[right]);
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
    auto getTitle = [](const Book& book) -> std::string { return book.getTitle(); };
    auto comp = [](const std::string& a, const std::string& b) { return a < b; };
    int index = books.binarySearch(title, getTitle, comp);
    if (index >= 0) {
        int left = index, right = index;
        while (left >= 0 && books[left].getTitle() == title) {
            result.push_back(books[left]);
            --left;
        }
        while (right < books.getSize() && books[right].getTitle() == title) {
            if (right > index) {
                result.push_back(books[right]);
            }
            ++right;
        }
    }
    return result;
}

MyVector<Book> BookManager::findBooksByAuthor(const std::string& author) {
    MyVector<Book> result;
    auto getAuthor = [](const Book& book) -> std::string { return book.getAuthor(); };
    auto comp = [](const std::string& a, const std::string& b) { return a < b; };
    int index = books.binarySearch(author, getAuthor, comp);
    if (index >= 0) {
        int left = index, right = index;
        while (left >= 0 && books[left].getAuthor() == author) {
            result.push_back(books[left]);
            --left;
        }
        while (right < books.getSize() && books[right].getAuthor() == author) {
            if (right > index) {
                result.push_back(books[right]);
            }
            ++right;
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