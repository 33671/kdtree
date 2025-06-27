#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reject_sampling.h"

Image *create_image(const char *text, const char *font_path, int width,
                   int height) {
  // Create white background image
  Image image = GenImageColor(width, height, WHITE);
  Font font = GetFontDefault();
  bool font_loaded = false;
  int font_size = 120;

  // Load custom font if available, else use default
  if (FileExists(font_path)) {
    font = LoadFontEx(font_path, font_size, NULL, 0);
    font_loaded = true;
  }

  // Measure text dimensions
  Vector2 text_size = MeasureTextEx(font, text, font_size, 0);
  Vector2 position = {(width - text_size.x) / 2, (height - text_size.y) / 2};

  // Draw text in black
  ImageDrawTextEx(&image, font, text, position, font_size, 2, BLACK);

  // Convert to grayscale
  ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
  // ExportImage(image, path);

  // Cleanup resources
  if (font_loaded)
    UnloadFont(font);
  Image* image_p = (Image*)malloc(sizeof(Image));
  *image_p = image;
  // UnloadImage(image);

  // Return duplicated path string
  return image_p;
}

Point *distribute_points_on_image(Image *img_p, int num_points,
                                  float min_distance, int *out_num_points,
                                  int *out_width, int *out_height) {
  // Initialize output parameters
  *out_num_points = 0;
  *out_width = 0;
  *out_height = 0;

  // Load image
  // Image img = LoadImage(image_path);
  Image img = *img_p;
  free(img_p);
  if (!img.data) {
    fprintf(stderr, "Error: Image empty");
    return NULL;
  }

  // Convert to grayscale if needed
  if (img.format != PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) {
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
  }

  int width = img.width;
  int height = img.height;
  unsigned char *pixels = (unsigned char *)img.data;
  size_t num_pixels = (size_t)width * height;

  // Calculate weights based on pixel darkness
  double *weights = (double *)malloc(num_pixels * sizeof(double));
  double total_weight = 0.0;
  if (!weights) {
    UnloadImage(img);
    return NULL;
  }

  for (size_t i = 0; i < num_pixels; i++) {
    double w = 255.0 - pixels[i] + 1e-6;
    weights[i] = w;
    total_weight += w;
  }

  // Build cumulative distribution
  double *cumulative = (double *)malloc(num_pixels * sizeof(double));
  if (!cumulative) {
    free(weights);
    UnloadImage(img);
    return NULL;
  }

  if (num_pixels > 0) {
    cumulative[0] = weights[0];
    for (size_t i = 1; i < num_pixels; i++) {
      cumulative[i] = cumulative[i - 1] + weights[i];
    }
  }
  free(weights);

  // Prepare points array
  Point *points = (Point *)malloc(num_points * sizeof(Point));
  if (!points) {
    free(cumulative);
    UnloadImage(img);
    return NULL;
  }

  int accepted = 0;
  int attempts = 0;
  int max_attempts = num_points * 200;
  double min_dist_sq = min_distance * min_distance;

  // Generate points with rejection sampling
  while (accepted < num_points && attempts < max_attempts) {
    attempts++;

    // Random position weighted by darkness
    double r = ((double)rand() / RAND_MAX) * total_weight;
    size_t index = 0;
    while (index < num_pixels - 1 && cumulative[index] < r) {
      index++;
    }

    int x = index % width;
    int y = index / width;

    // Check minimum distance
    int valid = 1;
    for (int i = 0; i < accepted; i++) {
      int dx = points[i].x - x;
      int dy = points[i].y - y;
      float dist_sq = dx * dx + dy * dy;
      if (dist_sq < min_dist_sq) {
        valid = 0;
        break;
      }
    }

    // Add valid point
    if (valid) {
      points[accepted].x = x;
      points[accepted].y = y;
      accepted++;
    }
  }

  // Cleanup and return results
  free(cumulative);
  UnloadImage(img);

  *out_num_points = accepted;
  *out_width = width;
  *out_height = height;

  if (attempts >= max_attempts) {
    fprintf(stderr, "Warning: Reached max attempts. Generated %d/%d points\n",
            accepted, num_points);
  }

  return points;
}