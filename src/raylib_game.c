#include "dynamic_array.h"
#include "kdtree.h"
#include "msg_queue.h"
#include "raylib.h"
#include "reject_sampling.h"
#include "simplex.h"
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
char *get_current_second() {
  static char second_str[3]; // Not thread-safe!
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  snprintf(second_str, sizeof(second_str), "%02d", timeinfo->tm_sec);

  return second_str;
}
float triangle_wave(float time) {
  float phase = fmodf(time, 4.0f) / 4.0f;
  if (phase < 0.25f) {
    return -1.0f + 4.0f * phase;
  } else if (phase < 0.75f) {
    return 1.0f - 4.0f * (phase - 0.25f);
  } else {
    return -1.0f + 4.0f * (phase - 0.75f);
  }
}
void gen_uniform(DynamicArray *arr, int width, int height, int layers) {
  if (arr->element_size != sizeof(Vector2)) {
    fprintf(stderr, "Error: DynamicArray element size mismatch.\n");
    return;
  }
  for (int current_layer = 0; current_layer < layers; current_layer++) {
    int box_x_width = width / pow(2, current_layer + 1);
    printf("box_x_width: %d\n", box_x_width);
    int box_y_height = height / pow(2, current_layer + 1);
    int xy_segment_count = pow(2, current_layer);
    for (int n = 0; n < pow(xy_segment_count, 2); n++) {
      int x = box_x_width + (n % xy_segment_count) * box_x_width * 2;
      int y_bias =
          (box_y_height / 2) + (n / xy_segment_count) * box_y_height * 2;
      int y1 = y_bias;
      int y2 = y_bias + (box_y_height / 2);
      int y3 = y_bias + box_y_height;
      Vector2 temp1 = (Vector2){.x = (float)x, .y = (float)y1};
      da_push(arr, &temp1);
      da_push(arr, &(Vector2){.x = x, .y = y2});
      da_push(arr, &(Vector2){.x = x, .y = y3});
    }
  }
  assert(arr->size == (pow(4, layers) - 1));
}
typedef struct thread_arg {
  int num_points_grid;
  MessageQueue *queue;
} thread_arg;
void *thread_func(void *arg) {
  thread_arg *arg1 = (thread_arg *)arg;
  MessageQueue *queue = arg1->queue;
  while (true) {
    sleep(1);
    Image *img = create_image(get_current_second(), "font.ttf", 256, 256);

    // Distribute points
    int num_points, width, height;
    Point *points = distribute_points_on_image(img, 1023,
                                               1.0f, // min distance
                                               &num_points, &width, &height);
    assert(num_points == arg1->num_points_grid);
    Vector2 *points_vector2 =
        (Vector2 *)malloc(arg1->num_points_grid * sizeof(Vector2));
    printf("Generated %d points on %dx%d image\n", num_points, width, height);
    for (int i = 0; i < num_points; i++) {
      points_vector2[i].x = points[i].x * 3;
      points_vector2[i].y = points[i].y * 3;
    }
    msg_queue_send_blocking(queue, points_vector2);
  }
  return NULL;
}

int main() {

  // pthread_join(thread, NULL);
  int num_points_grid = 0;
  int layers = 5; // Number of layers in the KD tree
  MessageQueue queue = {0};
  msg_queue_init(&queue, 2);
  DynamicArray *arr = da_init(pow(4, layers), sizeof(Vector2));
  gen_uniform(arr, 800, 800, layers);
  Vector2 **generated_vec = (Vector2 **)arr->array;
  num_points_grid = arr->size;
  printf("%d points generated", num_points_grid);
  InitWindow(800, 800, "kd tree");
  SetTargetFPS(30);

  Image *img = create_image(get_current_second(), "font.ttf", 256, 256);

  // Distribute points
  int num_points, width, height;
  Point *points = distribute_points_on_image(img, 1023,
                                             1.0f, // min distance
                                             &num_points, &width, &height);
  // Use points...
  printf("Generated %d points on %dx%d image\n", num_points, width, height);
  assert(num_points == num_points_grid);
  Vector2 *points_vector2 =
      (Vector2 *)malloc(num_points_grid * sizeof(Vector2));
  for (int i = 0; i < num_points; i++) {
    points_vector2[i].x = points[i].x * 3;
    points_vector2[i].y = points[i].y * 3;
  }
  pthread_t thread;
  thread_arg arg = {.num_points_grid = num_points_grid, .queue = &queue};
  pthread_create(&thread, NULL, thread_func, &arg);
  Vector2 *origin_points_vector2_ =
      (Vector2 *)malloc(num_points_grid * sizeof(Vector2));
  for (int i = 0; i < num_points; i++) {
    origin_points_vector2_[i].x = generated_vec[i]->x;
    origin_points_vector2_[i].y = generated_vec[i]->y;
  }
  float interpolation = 0;
  int noise_index = 0;
  simplex1d_init();
  float last_draw_secs = GetTime();
  while (!WindowShouldClose()) {

    BeginDrawing();
    ClearBackground(WHITE);
    double time_secs = GetTime();
    double interpo = sin(GetTime() * 2.0) / 2.0 + .5;
    if (time_secs - last_draw_secs > 1.0 && interpo <= 0.01) {
      Vector2 *temp = msg_queue_recv_nonblocking(&queue);
      if (temp) {
        free(points_vector2);
        points_vector2 = temp;
        last_draw_secs = time_secs;
      }
    }
    int fps = GetFPS();
    const char *fps_s = TextFormat("FPS:%d", fps);
    DrawText(fps_s, 0, 0, 10, RED);
    TreeNode *tree = buildKDTree(origin_points_vector2_, points_vector2,
                                 num_points_grid, 1, NULL, interpo);
    DrawKDTree(tree, 0, 0, 800, 800);
    EndDrawing();
    freeTree(tree);
  }
}