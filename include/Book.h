#ifndef BOOK_H
#define BOOK_H

#include <string>
#include <stdexcept>

// 前向声明
class QJsonObject;

class Book {
private:
    std::string isbn;
    std::string title;
    std::string author;
    std::string publisher;
    int publishYear;
    int status = 0;//未借出

public:
    Book();
    Book(const std::string& isbn, const std::string& title, 
         const std::string& author, const std::string& publisher, 
         int publishYear);

    std::string getIsbn() const;
    std::string getTitle() const;
    std::string getAuthor() const;
    std::string getPublisher() const;
    int getPublishYear() const;
    int getStatus() const;

    void setIsbn(const std::string& isbn);
    void setTitle(const std::string& title);
    void setAuthor(const std::string& author);
    void setPublisher(const std::string& publisher);
    void setPublishYear(int year);
    void setStatus(int status);
    
    // 数据持久化方法
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
};

#endif // BOOK_H 
