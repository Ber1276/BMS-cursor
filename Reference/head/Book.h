#ifndef BOOK_H
#define BOOK_H

#include <string>
#include <stdexcept>

class Book {
private:
    std::string isbn;        // ISBN码
    std::string title;       // 书名
    std::string author;      // 作者
    std::string publisher;   // 出版社
    int publishYear;         // 出版年份

public:
    // 构造函数
    Book();
    Book(const std::string& isbn, const std::string& title, 
         const std::string& author, const std::string& publisher, 
         int publishYear);

    // Getter方法
    std::string getIsbn() const;
    std::string getTitle() const;
    std::string getAuthor() const;
    std::string getPublisher() const;
    int getPublishYear() const;

    // Setter方法
    void setIsbn(const std::string& isbn);
    void setTitle(const std::string& title);
    void setAuthor(const std::string& author);
    void setPublisher(const std::string& publisher);
    void setPublishYear(int year);
};

#endif // BOOK_H