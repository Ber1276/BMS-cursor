#pragma once
#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>
#include <stdexcept>

struct User;
struct Book;

template<typename T>
class MyVector {
private:
    T* data;
    size_t capacity;
    size_t size;
    size_t* hashValues;
    size_t* indices;
    size_t hashTableSize;

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
    void add(const T& value) { push_back(value); }
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
    void removeAt(size_t index) {
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        data[index].~T();
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
            if (!comp(current, target) && !comp(target, current)) {
                return static_cast<int>(mid);
            } else if (comp(current, target)) {
                left = mid + 1;
            } else {
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