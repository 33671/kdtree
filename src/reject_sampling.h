#ifndef _SAMPLING
#define _SAMPLING
#include "raylib.h"
Image *create_image(const char *text, const char *font_path, int width,
                   int height);
// Create image with a pre-loaded font (thread-safe, no texture loading)
Image *create_image_with_font(const char *text, Font font, int font_size,
                              int width, int height);
typedef struct {
  int x;
  int y;
} Point;

Point *distribute_points_on_image(Image *img_p, int num_points,
                                  float min_distance, int *out_num_points,
                                  int *out_width, int *out_height);

#endif