#pragma once

namespace MyAlgorithm {
    // 分区函数
    template<typename T>
    int partition(T arr[], int low, int high) {
        T pivot = arr[high];    // 选择最后一个元素作为基准
        int i = low - 1;        // 较小元素的位置

        for (int j = low; j < high; j++) {
            if (arr[j] <= pivot) {
                i++;
                T temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
        T temp = arr[i + 1];
        arr[i + 1] = arr[high];
        arr[high] = temp;
        return i + 1;
    }

    // 快速排序递归函数
    template<typename T>
    void quickSort(T arr[], int low, int high) {
        if (low < high) {
            int pi = partition(arr, low, high);
            quickSort(arr, low, pi - 1);
            quickSort(arr, pi + 1, high);
        }
    }

    // 基础版本：升序排序
    template<typename T>
    void sort(T arr[], int length) {
        quickSort(arr, 0, length - 1);
    }

    // 带比较器的分区函数
    template<typename T, typename Compare>
    int partition(T arr[], int low, int high, Compare compare) {
        T pivot = arr[high];
        int i = low - 1;

        for (int j = low; j < high; j++) {
            if (compare(arr[j], pivot)) {
                i++;
                T temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
        T temp = arr[i + 1];
        arr[i + 1] = arr[high];
        arr[high] = temp;
        return i + 1;
    }

    // 带比较器的快速排序
    template<typename T, typename Compare>
    void quickSort(T arr[], int low, int high, Compare compare) {
        if (low < high) {
            int pi = partition(arr, low, high, compare);
            quickSort(arr, low, pi - 1, compare);
            quickSort(arr, pi + 1, high, compare);
        }
    }

    // 高级版本：支持任意比较器类型
    template<typename T, typename Compare>
    void sort(T arr[], int length, Compare compare) {
        quickSort(arr, 0, length - 1, compare);
    }
}