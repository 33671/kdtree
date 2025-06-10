#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stddef.h> // for size_t

// 动态数组结构体
typedef struct {
    void **array;      // 指向指针数组的指针
    size_t size;       // 当前元素数量
    size_t capacity;   // 数组容量
    size_t element_size; // 每个元素的大小(字节)
} DynamicArray;

// 初始化动态数组
DynamicArray* da_init(size_t initial_capacity, size_t element_size);
// 清空动态数组中的所有元素
void da_clear(DynamicArray *da);
// 释放动态数组
void da_free(DynamicArray *da);

// 向动态数组尾部添加元素
int da_push(DynamicArray *da, const void *element);

// 从动态数组尾部移除元素
int da_pop(DynamicArray *da, void *element);

// 获取动态数组当前大小
size_t da_size(const DynamicArray *da);

// 获取动态数组当前容量
size_t da_capacity(const DynamicArray *da);

// 获取指定位置的元素
void* da_get(const DynamicArray *da, size_t index);

#endif // DYNAMIC_ARRAY_H