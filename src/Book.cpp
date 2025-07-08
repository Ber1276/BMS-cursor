#include "../include/Book.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>

Book::Book() : isbn(""), title(""), author(""), publisher(""), publishYear(0) {}

Book::Book(const std::string& isbn, const std::string& title, 
           const std::string& author, const std::string& publisher, 
           int publishYear) : isbn(isbn), title(title), 
                            author(author), publisher(publisher),
                            publishYear(publishYear) {}

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

// 数据持久化方法实现
QJsonObject Book::toJson() const {
    QJsonObject json;
    json["isbn"] = QString::fromStdString(isbn);
    json["title"] = QString::fromStdString(title);
    json["author"] = QString::fromStdString(author);
    json["publisher"] = QString::fromStdString(publisher);
    json["publishYear"] = publishYear;
    return json;
}

void Book::fromJson(const QJsonObject& json) {
    if (json.contains("isbn")) {
        isbn = json["isbn"].toString().toStdString();
    }
    if (json.contains("title")) {
        title = json["title"].toString().toStdString();
    }
    if (json.contains("author")) {
        author = json["author"].toString().toStdString();
    }
    if (json.contains("publisher")) {
        publisher = json["publisher"].toString().toStdString();
    }
    if (json.contains("publishYear")) {
        publishYear = json["publishYear"].toInt();
    }
} 