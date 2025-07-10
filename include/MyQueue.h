#ifndef MYQUEUE_H
#define MYQUEUE_H

#include "MyVector.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <stdexcept>

// 模板队列，底层用MyVector实现
// 要求T支持QJsonValue的序列化/反序列化

template<typename T>
class MyQueue {
private:
    MyVector<T> data;
    size_t head = 0; // 队首索引
public:
    MyQueue() = default;
    void enqueue(const T& value) {
        data.push_back(value);
    }
    void dequeue() {
        if (isEmpty()) throw std::out_of_range("队列为空");
        head++;
    }
    T& front() {
        if (isEmpty()) throw std::out_of_range("队列为空");
        return data[head];
    }
    const T& front() const {
        if (isEmpty()) throw std::out_of_range("队列为空");
        return data[head];
    }
    bool isEmpty() const {
        return head >= data.getSize();
    }
    size_t size() const {
        return isEmpty() ? 0 : data.getSize() - head;
    }
    // 判断队列中是否包含某元素
    bool contains(const T& value) const {
        for (size_t i = head; i < data.getSize(); ++i) {
            if (data[i] == value) return true;
        }
        return false;
    }
    // 清空队列
    void clear() {
        data.clear();
        head = 0;
    }
    // QJsonArray序列化
    QJsonArray toJsonArray() const {
        QJsonArray arr;
        for (size_t i = head; i < data.getSize(); ++i) {
            if constexpr (std::is_same<T, std::string>::value) {
                arr.append(QString::fromStdString(data[i]));
            } else {
                arr.append(data[i].toJson());
            }
        }
        return arr;
    }
    // QJsonArray反序列化
    void fromJsonArray(const QJsonArray& arr) {
        clear();
        for (const auto& v : arr) {
            if constexpr (std::is_same<T, std::string>::value) {
                data.push_back(v.toString().toStdString());
            } else {
                T obj;
                obj.fromJson(v.toObject());
                data.push_back(obj);
            }
        }
    }
};

#endif // MYQUEUE_H 