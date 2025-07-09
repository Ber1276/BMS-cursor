#ifndef BOOK_MANAGER_H
#define BOOK_MANAGER_H

#include "MyVector.h"
#include <memory>
#include <algorithm>
#include "Book.h"

// 前向声明
class QString;

enum class SortBy {
    ISBN,
    TITLE,
    AUTHOR,
    PUBLISHER,
    YEAR
};

enum class SortOrder {
    ASCENDING,
    DESCENDING
};

class BookManager {
private:
    MyVector<Book> books;
    void sortBooks(MyVector<Book> &bookList, SortBy sortBy, SortOrder order) const;
    //bool parseBookLine(const std::string& line, Book& book);
public:
    void addBook(const Book &book);
    void addBookNoRebuild(const Book &book);
    void rebuildBookHashTable();
    bool updateBook(const std::string& isbn, const Book& updatedBook);
    bool updateBookField(const std::string& isbn, const std::string& field, const std::string& newValue);
    bool removeBook(const std::string &isbn);
    Book *findBookByIsbn(const std::string &isbn);
    const MyVector<Book> &getAllBooks() const;
    MyVector<Book> findBooksByTitle(const std::string &title);
    MyVector<Book> findBooksByAuthor(const std::string &author);
    MyVector<Book> findBooksByPublisher(const std::string &publisher);
    MyVector<Book> findBooksByYear(int year);
    MyVector<Book> findBooksByYearRange(int startYear, int endYear);
    MyVector<Book> getSortedBooks(SortBy sortBy, SortOrder order = SortOrder::ASCENDING) const;
    MyVector<Book> sortSearchResults(const MyVector<Book> &searchResults, SortBy sortBy, SortOrder order = SortOrder::ASCENDING) const;
    size_t getBookCount() const;
    bool importBooksFromFile(const std::string& filename);
    bool exportBooksToFile(const std::string& filename) const;
    static bool parseBookLine(const std::string& line, Book& book);
    void addBooks(const MyVector<Book>& books);
    
    // 数据持久化方法
    bool saveToFile(const QString& filename) const;
    bool loadFromFile(const QString& filename);
};

#endif // BOOK_MANAGER_H 
