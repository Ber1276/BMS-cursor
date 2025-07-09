#pragma once
#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>
#include <stdexcept>

struct User;
struct Book;

/**
 * @brief The MyVector class 动态数组
 * @author 陈子涵
 */
template<typename T>
class MyVector {
private:
    T* data; //数据
    size_t capacity; //容量
    size_t size; //大小
    size_t* hashValues; //哈希值
    size_t* indices; //哈希值对应索引
    size_t hashTableSize; //哈希表大小

public:
    //构建函数和析构函数
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
    // 添加元素每添加一个元素就重建哈希表
    void add(const T& value) { push_back(value); }
    void push_back(const T& value) {
        // 如果容量已满，扩展容量
        if (size == capacity) {
            T* newData = new T[capacity * 2];
            for (size_t i = 0; i < size; ++i) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
            capacity *= 2;
            // 扩展哈希表
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
        // 对于 User 和 Book 类型，重建哈希表
        if constexpr (std::is_same<T, User>::value) {
            rebuildHashTable();
        }
        if constexpr (std::is_same<T, Book>::value) {
            rebuildBookHashTable();
        }
    }
    // 不重建哈希表的 push_back 方法
    void push_back_no_rebuild(const T& value) {
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
        // 不重建哈希表
    }
    // 删除元素
    void removeAt(size_t index) {
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        for (size_t i = index; i < size - 1; ++i) {
            data[i] = data[i + 1];
        }
        if (size > 0) {
            data[size - 1] = T(); // 置为默认值，防止悬挂
        }
        --size;
        // 对于 User 和 Book 类型，重建哈希表
        if constexpr (std::is_same<T, User>::value) {
            rebuildHashTable();
        }
        if constexpr (std::is_same<T, Book>::value) {
            rebuildBookHashTable();
        }
    }
    /**
     * @brief 二分查找
     * @author 陈子涵
     * @param target 要查找的目标值
     * @param getKey 获取元素键的函数
     * @param comp 比较函数
     * @return 返回目标值的索引，如果未找到则返回 -1
     */
    template<typename Key, typename GetKey,typename Comparator>
    int binarySearch(Key target, GetKey getKey, Comparator comp) const {
        if (size == 0) {
            return -1;
        }
        size_t left = 0;
        size_t right = size - 1;
        while (left <= right) {
            size_t mid = left + (right - left) / 2;
            Key current = getKey(data[mid]);
            if (!comp(current, target) && !comp(target, current)) { //相等
                return static_cast<int>(mid);
            } else if (comp(current, target)) { // current < target 
                left = mid + 1;
            } else { // current > target
                if (mid == 0) break;
                right = mid - 1;
            }
        }
        return -1;
    }
    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }
    size_t getSize() const { return size; }
    size_t customHash(const std::string& str) const {
        size_t hash = 5381;
        for (char c : str) {
            hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
        }
        return hash;
    }

    // 赋值运算符重载
    MyVector& operator=(const MyVector& other) {
        if (this == &other) return *this;
        delete[] data;
        delete[] hashValues;
        delete[] indices;
        capacity = other.capacity;
        size = other.size;
        hashTableSize = other.hashTableSize;
        data = new T[capacity];
        hashValues = new size_t[hashTableSize];
        indices = new size_t[hashTableSize];
        for (size_t i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
        for (size_t i = 0; i < hashTableSize; ++i) {
            hashValues[i] = other.hashValues[i];
            indices[i] = other.indices[i];
        }
        return *this;
    }

    // 拷贝构造函数
    MyVector(const MyVector& other)
        : capacity(other.capacity), size(other.size), hashTableSize(other.hashTableSize)
    {
        data = new T[capacity];
        hashValues = new size_t[hashTableSize];
        indices = new size_t[hashTableSize];
        for (size_t i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
        for (size_t i = 0; i < hashTableSize; ++i) {
            hashValues[i] = other.hashValues[i];
            indices[i] = other.indices[i];
        }
    }

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
    void clear() {
        size = 0;
        if constexpr (std::is_same<T, User>::value) {
            rebuildHashTable();
        }
        if constexpr (std::is_same<T, Book>::value) {
            rebuildBookHashTable();
        }
    }
}; 
