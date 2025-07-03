#ifndef BOOK_MANAGER_H
#define BOOK_MANAGER_H

#include "MyVector.h"
#include <memory>
#include <algorithm>
#include "Book.h"

// 添加排序规则枚举
enum class SortBy
{
    TITLE,
    AUTHOR,
    PUBLISHER,
    YEAR
};

enum class SortOrder
{
    ASCENDING,
    DESCENDING
};

class BookManager
{
private:
    MyVector<Book> books; // 存储所有图书
    // 添加私有排序辅助方法
    void sortBooks(MyVector<Book> &bookList, SortBy sortBy, SortOrder order) const;

    // 添加私有辅助方法
    bool parseBookLine(const std::string& line, Book& book);

public:
    // 添加图书
    void addBook(const Book &book);

    // 修改图书信息
    bool updateBook(const std::string& isbn, const Book& updatedBook);
    // 根据ISBN修改图书的特定信息
    bool updateBookField(const std::string& isbn, const std::string& field, const std::string& newValue);

    //根据ISBN删除图书
    bool removeBook(const std::string &isbn);

    // 根据ISBN查找图书
    Book *findBookByIsbn(const std::string &isbn);

    // 获取所有图书
    const MyVector<Book> &getAllBooks() const;

    // 根据不同条件查找图书
    MyVector<Book> findBooksByTitle(const std::string &title);
    MyVector<Book> findBooksByAuthor(const std::string &author);
    MyVector<Book> findBooksByPublisher(const std::string &publisher);
    MyVector<Book> findBooksByYear(int year);
    MyVector<Book> findBooksByYearRange(int startYear, int endYear);

    // 添加新的排序方法
    MyVector<Book> getSortedBooks(SortBy sortBy, SortOrder order = SortOrder::ASCENDING) const;

    // 对搜索结果进行排序的方法
    MyVector<Book> sortSearchResults(const MyVector<Book> &searchResults,
                                        SortBy sortBy,
                                        SortOrder order = SortOrder::ASCENDING) const;

    // 获取图书总数
    size_t getBookCount() const;

    // 添加新方法
    bool importBooksFromFile(const std::string& filename);
    bool exportBooksToFile(const std::string& filename) const;
};

#endif // BOOK_MANAGER_H