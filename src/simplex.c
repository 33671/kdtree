#include "simplex.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// 排列表大小
#define PERM_SIZE 256
#define PERM_MASK 0xFF

// 梯度表（一维只有两个方向）
static const int8_t grad1[2] = {1, -1};

// 排列表（512元素，重复一次用于快速索引）
static uint8_t perm[PERM_SIZE * 2];

// 初始化排列表（使用当前时间作为随机种子随机生成perm表）
void simplex1d_init() {
  // 初始化perm数组的前256个位置为0~255
  for (int i = 0; i < PERM_SIZE; i++) {
    perm[i] = (uint8_t)i;
  }

  // 使用当前时间作为随机种子
  srand((unsigned int)time(NULL));

  // Fisher-Yates洗牌算法打乱perm数组
  for (int i = PERM_SIZE - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    uint8_t temp = perm[i];
    perm[i] = perm[j];
    perm[j] = temp;
  }

  // 复制一份到后面256个位置，形成512长度表
  for (int i = 0; i < PERM_SIZE; i++) {
    perm[PERM_SIZE + i] = perm[i];
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
  t0 *= t0;
  t0 *= t0; // (1-x^2)^4
  t1 *= t1;
  t1 *= t1;

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
  float max_value = 0.0f; // 用于归一化

  for (int i = 0; i < octaves; i++) {
    total += simplex1d(x * frequency) * amplitude;
    max_value += amplitude;
    amplitude *= persistence;
    frequency *= 2.0f;
  }

  return total / max_value;
}
// 2D梯度表 (12个方向，常用配置)
static const int8_t grad2[12][2] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1},
                                    {1, 0}, {-1, 0}, {1, 0},  {-1, 0},
                                    {0, 1}, {0, -1}, {0, 1},  {0, -1}};

// 斜率常量 (2D Simplex skewing/unskewing factors)
#define F2 0.366025403f // (sqrt(3)-1)/2
#define G2 0.211324865f // (3-sqrt(3))/6

// 2D Simplex噪声核心函数
float simplex2d(float x, float y) {
  // Skew the input space to determine which simplex cell we are in
  float s = (x + y) * F2;
  int i = (int)floorf(x + s);
  int j = (int)floorf(y + s);

  // Unskew the cell origin back to (x, y) space
  float t = (i + j) * G2;
  float X0 = i - t;
  float Y0 = j - t;
  float x0 = x - X0;
  float y0 = y - Y0;

  // Determine which simplex triangle we are in
  int i1, j1; // Offsets for second corner of simplex in (i,j)
  if (x0 > y0) {
    i1 = 1;
    j1 = 0;
  } else {
    i1 = 0;
    j1 = 1;
  }

  // Offsets for middle corner in (x,y) unskewed coords
  float x1 = x0 - i1 + G2;
  float y1 = y0 - j1 + G2;
  // Offsets for last corner in (x,y) unskewed coords
  float x2 = x0 - 1.0f + 2.0f * G2;
  float y2 = y0 - 1.0f + 2.0f * G2;

  // Hash coordinates of the three simplex corners
  uint8_t ii = i & PERM_MASK;
  uint8_t jj = j & PERM_MASK;
  uint8_t gi0 = perm[ii + perm[jj]] % 12;
  uint8_t gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
  uint8_t gi2 = perm[ii + 1 + perm[jj + 1]] % 12;

  // Calculate contribution from the three corners
  float n0, n1, n2;

  float t0 = 0.5f - x0 * x0 - y0 * y0;
  if (t0 < 0)
    n0 = 0.0f;
  else {
    t0 *= t0;
    n0 = t0 * t0 * (grad2[gi0][0] * x0 + grad2[gi0][1] * y0);
  }

  float t1 = 0.5f - x1 * x1 - y1 * y1;
  if (t1 < 0)
    n1 = 0.0f;
  else {
    t1 *= t1;
    n1 = t1 * t1 * (grad2[gi1][0] * x1 + grad2[gi1][1] * y1);
  }

  float t2 = 0.5f - x2 * x2 - y2 * y2;
  if (t2 < 0)
    n2 = 0.0f;
  else {
    t2 *= t2;
    n2 = t2 * t2 * (grad2[gi2][0] * x2 + grad2[gi2][1] * y2);
  }

  // The result is scaled to return values in the interval [-1,1]
  return 70.0f * (n0 + n1 + n2);
}

// 2D分形Simplex噪声（多层叠加）
float fractal_simplex2d(float x, float y, int octaves, float persistence) {
  float total = 0.0f;
  float frequency = 1.0f;
  float amplitude = 1.0f;
  float max_value = 0.0f;

  for (int i = 0; i < octaves; i++) {
    total += simplex2d(x * frequency, y * frequency) * amplitude;
    max_value += amplitude;
    amplitude *= persistence;
    frequency *= 2.0f;
  }

  return total / max_value;
}