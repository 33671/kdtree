#ifndef SIMPLEX_NOISE_H
#define SIMPLEX_NOISE_H

#ifdef __cplusplus
extern "C" {
#endif

// 初始化噪声系统
void simplex1d_init();

// 生成一维Simplex噪声
float simplex1d(float x);

// 分形噪声生成（可选）
float fractal_simplex1d(float x, int octaves, float persistence);

float simplex2d(float x, float y);
float fractal_simplex2d(float x, float y, int octaves, float persistence);
#ifdef __cplusplus
}
#endif

#endif // SIMPLEX_NOISE_H