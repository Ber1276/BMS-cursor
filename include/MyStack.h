#ifndef MYSTACK_H
#define MYSTACK_H
#include "MyVector.h"

template <typename T>
class MyStack {
private:
    MyVector<T> data;
    const int maxSize;

public:
    explicit MyStack(int maxCapacity = 10) : maxSize(maxCapacity) {}

    // 压入元素（若超出容量则删除栈底元素）
    void push(const T& value) {

        data.push_back(value);
        while (data.getSize() > maxSize) {
            data.removeAt(0); // 移除栈底元素
        }
    }

    // 弹出栈顶元素
    void pop() {
        if (empty()) {
            throw std::out_of_range("Stack is empty");
        }
        data.removeAt(data.getSize() - 1);
    }

    // 获取栈顶元素
    T top() const {
        if (empty()) {
            throw std::out_of_range("Stack is empty");
        }
        return data[data.getSize() - 1];
    }

    MyStack& operator=(const MyStack& other) {
        if (this != &other) {
            // 1. 清空当前栈数据
            data.clear();

            // 2. 拷贝数据（依赖 MyVector 的赋值运算符）
            data = other.data;

            // 3. 注意：maxSize 无法修改，因此只在构造函数中初始化
        }
        return *this;
    }

    // 判断栈是否为空
    bool empty() const {
        return data.getSize() == 0;
    }

    // 获取栈的大小
    int size() const {
        return data.getSize();
    }

};

#endif // MYSTACK_H
