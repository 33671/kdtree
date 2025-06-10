#ifndef _SAMPLING
#define _SAMPLING
char *create_image(const char *text, const char *font_path, int width,
                   int height, const char *path);
typedef struct {
  int x;
  int y;
} Point;

Point *distribute_points_on_image(const char *image_path, int num_points,
                                  float min_distance, int *out_num_points,
                                  int *out_width, int *out_height);

#endif