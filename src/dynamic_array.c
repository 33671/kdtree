#include "dynamic_array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 初始化动态数组
DynamicArray* da_init(size_t initial_capacity, size_t element_size) {
    DynamicArray *da = malloc(sizeof(DynamicArray));
    if (!da) return NULL;
    
    da->array = malloc(initial_capacity * sizeof(void*));
    if (!da->array) {
        free(da);
        return NULL;
    }
    
    da->size = 0;
    da->capacity = initial_capacity;
    da->element_size = element_size;
    return da;
}
// 清空动态数组中的所有元素
void da_clear(DynamicArray *da) {
    if (!da) return;
    
    // 释放所有元素内存
    for (size_t i = 0; i < da->size; i++) {
        free(da->array[i]);
    }
    
    da->size = 0;
    
    // 可选：缩小容量到初始大小以节省内存
    // 这里可以根据需求决定是否保留当前容量
    // size_t initial_capacity = 4; // 或其他初始值
    // if (da->capacity > initial_capacity) {
    //     da_resize(da, initial_capacity);
    // }
}
// 释放动态数组
void da_free(DynamicArray *da) {
    if (da) {
        // 先释放所有元素内存
        for (size_t i = 0; i < da->size; i++) {
            free(da->array[i]);
        }
        // 再释放数组和结构体
        free(da->array);
        free(da);
    }
}

// 扩容动态数组
static int da_resize(DynamicArray *da, size_t new_capacity) {
    void **new_array = realloc(da->array, new_capacity * sizeof(void*));
    if (!new_array) return 0; // 扩容失败
    
    da->array = new_array;
    da->capacity = new_capacity;
    return 1;
}

// 向动态数组尾部添加元素
int da_push(DynamicArray *da, const void *element) {
    if (da->size == da->capacity) {
        // 容量不足时扩容为原来的2倍
        size_t new_capacity = da->capacity * 2;
        if (!da_resize(da, new_capacity)) {
            return 0; // 扩容失败
        }
    }
    
    // 分配新元素内存并复制数据
    void *new_element = malloc(da->element_size);
    if (!new_element) return 0;
    
    memcpy(new_element, element, da->element_size);
    da->array[da->size++] = new_element;
    
    return 1;
}

// 从动态数组尾部移除元素
int da_pop(DynamicArray *da, void *element) {
    if (da->size == 0) return 0; // 数组为空
    
    // 复制要弹出的元素数据
    if (element) {
        memcpy(element, da->array[da->size - 1], da->element_size);
    }
    
    // 释放元素内存
    free(da->array[--da->size]);
    
    // 如果元素数量减少到容量的1/4，缩小容量为原来的1/2
    if (da->size > 0 && da->size == da->capacity / 4) {
        size_t new_capacity = da->capacity / 2;
        da_resize(da, new_capacity);
    }
    
    return 1;
}

// 获取动态数组当前大小
size_t da_size(const DynamicArray *da) {
    return da ? da->size : 0;
}

// 获取动态数组当前容量
size_t da_capacity(const DynamicArray *da) {
    return da ? da->capacity : 0;
}

// 获取指定位置的元素
void* da_get(const DynamicArray *da, size_t index) {
    if (!da || index >= da->size) return NULL;
    return da->array[index];
}