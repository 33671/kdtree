#include "simplex.h"
#include <math.h>
#include <stdint.h>

// 排列表大小
#define PERM_SIZE 256
#define PERM_MASK 0xFF

// 梯度表（一维只有两个方向）
static const int8_t grad1[2] = {1, -1};

// 排列表（512元素，重复一次用于快速索引）
static uint8_t perm[PERM_SIZE * 2];

// 初始化排列表（使用固定种子）
void simplex1d_init() {
    // 固定排列表数据（随机生成的256个唯一字节）
    static const uint8_t base_perm[PERM_SIZE] = {
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
        8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
        35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
        134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
        55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,
        169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,
        124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,
        28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,
        129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,
        34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,
        214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,
        93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };
    
    // 复制两次形成512元素的表
    for (int i = 0; i < PERM_SIZE * 2; i++) {
        perm[i] = base_perm[i & PERM_MASK];
    }
}

// 光滑曲线函数（5次多项式）
static inline float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10); // 6t^5 - 15t^4 + 10t^3
}

// 一维Simplex噪声核心函数
float simplex1d(float x) {
    // 网格单元坐标
    int i0 = (int)floorf(x);
    int i1 = i0 + 1;

    // 网格内相对位置
    float x0 = x - i0;
    float x1 = x0 - 1.0f;

    // 梯度索引（使用排列表）
    uint8_t h0 = perm[i0 & PERM_MASK];
    uint8_t h1 = perm[i1 & PERM_MASK];
    
    // 计算梯度贡献
    float g0 = grad1[h0 & 0x01] * x0;
    float g1 = grad1[h1 & 0x01] * x1;

    // 计算衰减系数
    float t0 = 1.0f - x0 * x0;
    float t1 = 1.0f - x1 * x1;
    
    // 应用四次衰减函数
    t0 *= t0; t0 *= t0; // (1-x^2)^4
    t1 *= t1; t1 *= t1;
    
    // 贡献值计算
    float n0 = g0 * t0;
    float n1 = g1 * t1;
    
    // 缩放因子（将输出范围调整到[-1,1]）
    return 3.5f * (n0 + n1);
}

// 分形噪声（可选）
float fractal_simplex1d(float x, int octaves, float persistence) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float max_value = 0.0f;  // 用于归一化
    
    for(int i=0; i<octaves; i++) {
        total += simplex1d(x * frequency) * amplitude;
        max_value += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    
    return total / max_value;
}