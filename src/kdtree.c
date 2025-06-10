#include "kdtree.h"
#include "raylib.h"
#include "raymath.h"
#include "stddef.h"
#include <stdlib.h>
// void RebuildTree(TreeNode *tree, Vector2 *points, int pointCount,
//                  double interpolation) {
//   freeTree(tree);
//   tree = buildKDTree(points, pointCount, 0, NULL, interpolation);
// }
TreeNode* buildKDTree(Vector2* points, int count, int depth, TreeNode* parent, double interpolation) {
    if (count == 0) return NULL;

    int dimension = depth % 2;

    // Sort points
    qsort(points, count, sizeof(Vector2), dimension == 0 ? CompareX : CompareY);

    // Calculate split index
    int index = (int)((count) * interpolation);
    index = Clamp(index, 0, count - 1);

    // Create node
    TreeNode* node = malloc(sizeof(TreeNode));
    node->point = points[index];
    node->dimension = dimension;
    node->parent = parent;
    node->left = node->right = NULL;

    // Recursively build subtrees
    node->left = buildKDTree(points, index, depth + 1, node, interpolation);
    node->right = buildKDTree(points + index + 1, count - index - 1, depth + 1, node, interpolation);

    return node;
}

void DrawKDTree(TreeNode *node, int xMin, int yMin, int xMax, int yMax) {
  if (node == NULL)
    return;

  Vector2 point = node->point;
  Color lineColor = node->dimension == 0 ? RED : RED;

  // Draw partition line
  if (node->dimension == 0) {
    DrawLine(point.x, yMin, point.x, yMax, lineColor);
  } else {
    DrawLine(xMin, point.y, xMax, point.y, lineColor);
  }

  // Draw subtrees
  if (node->left) {
    if (node->dimension == 0) {
      DrawKDTree(node->left, xMin, yMin, point.x, yMax);
    } else {
      DrawKDTree(node->left, xMin, yMin, xMax, point.y);
    }
  }

  if (node->right) {
    if (node->dimension == 0) {
      DrawKDTree(node->right, point.x, yMin, xMax, yMax);
    } else {
      DrawKDTree(node->right, xMin, point.y, xMax, yMax);
    }
  }
}

void freeTree(TreeNode *node) {
  if (node == NULL)
    return;
  freeTree(node->left);
  freeTree(node->right);
  free(node);
}

int CompareX(const void *a, const void *b) {
  const Vector2 *v1 = (const Vector2 *)a;
  const Vector2 *v2 = (const Vector2 *)b;
  if (v1->x < v2->x) return -1;
  if (v1->x > v2->x) return 1;
  if (v1->y < v2->y) return -1;
  if (v1->y > v2->y) return 1;
  return 0;
}

int CompareY(const void *a, const void *b) {
  const Vector2 *v1 = (const Vector2 *)a;
  const Vector2 *v2 = (const Vector2 *)b;
  if (v1->y < v2->y) return -1;
  if (v1->y > v2->y) return 1;
  if (v1->x < v2->x) return -1;
  if (v1->x > v2->x) return 1;
  return 0;
}
