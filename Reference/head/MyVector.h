#pragma once
#include <cstddef>   // 提供 size_t 定义
#include <iostream>
#include <string>
#include <type_traits>    // for std::enable_if_t, std::is_same_v
#include <stdexcept>      // for std::out_of_range

// 前置声明 User
struct User;
// 前置声明 Book
struct Book;


template<typename T>
class MyVector {
private:
    T* data;
    size_t capacity;
    size_t size;

    // 哈希表相关
    size_t* hashValues;  // 存储每个对象的哈希值
    size_t* indices;     // 存储对应对象在 data 中的索引
    size_t hashTableSize; // 哈希表容量

public:
    explicit MyVector(size_t initialCapacity = 4)
        : capacity(initialCapacity), size(0), hashTableSize(initialCapacity * 2) {
        data = new T[capacity];
        hashValues = new size_t[hashTableSize];
        indices = new size_t[hashTableSize];

        for (size_t i = 0; i < hashTableSize; ++i) {
            hashValues[i] = 0;
            indices[i] = static_cast<size_t>(-1);
        }
    }

    ~MyVector() {
        delete[] data;
        delete[] hashValues;
        delete[] indices;
    }

    // 添加元素
    void add(const T& value) {
        push_back(value); 
    }

    void push_back(const T& value) {
        if (size == capacity) {
            T* newData = new T[capacity * 2];
            for (size_t i = 0; i < size; ++i) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
            capacity *= 2;

            size_t newHashTableSize = capacity * 2;
            size_t* newHashValues = new size_t[newHashTableSize];
            size_t* newIndices = new size_t[newHashTableSize];
            for (size_t i = 0; i < newHashTableSize; ++i) {
                newHashValues[i] = 0;
                newIndices[i] = static_cast<size_t>(-1);
            }
            delete[] hashValues;
            delete[] indices;
            hashValues = newHashValues;
            indices = newIndices;
            hashTableSize = newHashTableSize;
        }

        data[size++] = value;

        if constexpr (std::is_same<T, User>::value) {
            rebuildHashTable();
        }

        if constexpr (std::is_same<T, Book>::value) {
            rebuildBookHashTable();
        }
    }

    //删除元素
    void removeAt(size_t index) {
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }

        // 调用析构函数（如果是复杂对象）
        data[index].~T();

        // 将后面的元素前移
        for (size_t i = index; i < size - 1; ++i) {
            data[i] = data[i + 1];
        }

        --size;

        if constexpr (std::is_same<T, User>::value) {
            rebuildHashTable();
        }

        if constexpr (std::is_same<T, Book>::value) {
            rebuildBookHashTable();
        }
    }

    /**
     * 二分查找
     * @param target 要查找的目标值
     * @param getKey 获取元素键值的函数
     * @param comp 比较函数，返回 true 表示第一个参数小于第二个参数
     * @return 返回目标值的索引，如果未找到则返回 -1
     */
    template<typename Key, typename GetKey,typename Comparator>
    int binarySearch(Key target, GetKey getKey, Comparator comp) const {
        if (size == 0) {
            return -1; // 空向量直接返回未找到
        }
        size_t left = 0;
        size_t right = size - 1;

        while (left <= right) {
            size_t mid = left + (right - left) / 2;
            Key current = getKey(data[mid]);

            if (!comp(current, target) && !comp(target, current)) {
                return static_cast<int>(mid); // 相等
            } else if (comp(current, target)) {
                left = mid + 1;
            } else {
                if (mid == 0) break; // 防止 size_t underflow
                right = mid - 1;
            }
        }

        return -1; // 未找到
    }

    // 下标访问
    T& operator[](size_t index) {
        return data[index];
    }

    const T& operator[](size_t index) const {
        return data[index];
    }

    // 获取大小
    size_t getSize() const {
        return size;
    }

    // 自定义哈希函数
    size_t customHash(const std::string& str) const {
        size_t hash = 5381;
        for (char c : str) {
            hash = ((hash << 5) + hash) + static_cast<unsigned char>(c); // hash * 33 + c
        }
        return hash;
    }

    // 只有当 T == User 时才启用 rebuildHashTable
    template<typename U = T>
    std::enable_if_t<std::is_same<U, User>::value, void>
    rebuildHashTable() {
        for (size_t i = 0; i < hashTableSize; ++i) {
            hashValues[i] = 0;
            indices[i] = static_cast<size_t>(-1);
        }

        for (size_t i = 0; i < size; ++i) {
            size_t hashValue = customHash(data[i].username);
            size_t pos = hashValue % hashTableSize;

            while (hashValues[pos] != 0 && hashValues[pos] != hashValue) {
                pos = (pos + 1) % hashTableSize;
            }

            hashValues[pos] = hashValue;
            indices[pos] = i;
        }
    }

    template<typename U = T>
    std::enable_if_t<std::is_same<U, User>::value, int>
    hashFindByUsername(const std::string& username) const {
        size_t targetHash = customHash(username);
        size_t pos = targetHash % hashTableSize;

        for (size_t i = 0; i < hashTableSize; ++i) {
            if (hashValues[pos] == 0) break;
            if (hashValues[pos] == targetHash) {
                if (data[indices[pos]].username == username) {
                    return static_cast<int>(indices[pos]);
                }
            }
            pos = (pos + 1) % hashTableSize;
        }

        return -1;
    }

    template<typename U = T>
    std::enable_if_t<std::is_same<U, Book>::value, void>
    rebuildBookHashTable() {
        for (size_t i = 0; i < hashTableSize; ++i) {
            hashValues[i] = 0;
            indices[i] = static_cast<size_t>(-1);
        }
        for (size_t i = 0; i < size; ++i) {
            size_t hashValue = customHash(data[i].getIsbn());
            size_t pos = hashValue % hashTableSize;
            while (hashValues[pos] != 0 && hashValues[pos] != hashValue) {
                pos = (pos + 1) % hashTableSize;
            }
            hashValues[pos] = hashValue;
            indices[pos] = i;
        }
    }

    template<typename U = T>
    std::enable_if_t<std::is_same<U, Book>::value, int>
    hashFindByIsbn(const std::string& isbn) const {
        size_t targetHash = customHash(isbn);
        size_t pos = targetHash % hashTableSize;
        for (size_t i = 0; i < hashTableSize; ++i) {
            if (hashValues[pos] == 0) break;
            if (hashValues[pos] == targetHash) {
                if (data[indices[pos]].getIsbn() == isbn) {
                    return static_cast<int>(indices[pos]);
                }
            }
            pos = (pos + 1) % hashTableSize;
        }
        return -1;
    }
};