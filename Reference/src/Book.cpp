#include "../head/Book.h"

// 默认构造函数
Book::Book() : isbn(""), title(""), author(""), publisher(""), publishYear(0) {}

// 带参数构造函数
Book::Book(const std::string& isbn, const std::string& title, 
           const std::string& author, const std::string& publisher, 
           int publishYear) : isbn(isbn), title(title), 
                            author(author), publisher(publisher),
                            publishYear(publishYear) {}

// Getter方法实现
std::string Book::getIsbn() const { 
    return isbn; 
}

std::string Book::getTitle() const { 
    return title; 
}

std::string Book::getAuthor() const { 
    return author; 
}

std::string Book::getPublisher() const { 
    return publisher; 
}

int Book::getPublishYear() const { 
    return publishYear; 
}

// Setter方法实现
void Book::setIsbn(const std::string& isbn) {
    if (isbn.empty()) {
        throw std::invalid_argument("ISBN不能为空");
    }
    this->isbn = isbn;
}

void Book::setTitle(const std::string& title) {
    if (title.empty()) {
        throw std::invalid_argument("书名不能为空");
    }
    this->title = title;
}

void Book::setAuthor(const std::string& author) {
    if (author.empty()) {
        throw std::invalid_argument("作者不能为空");
    }
    this->author = author;
}

void Book::setPublisher(const std::string& publisher) {
    if (publisher.empty()) {
        throw std::invalid_argument("出版社不能为空");
    }
    this->publisher = publisher;
}

void Book::setPublishYear(int year) {
    if (year < 1000 || year > 2024) {
        throw std::invalid_argument("出版年份无效");
    }
    this->publishYear = year;
}