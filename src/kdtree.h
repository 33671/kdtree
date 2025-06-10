#ifndef _KDTREE
#define _KDTREE
#include "raylib.h"
typedef struct TreeNode {
  Vector2 point;
  int dimension;
  struct TreeNode *left;
  struct TreeNode *right;
  struct TreeNode *parent;
} TreeNode;

TreeNode* buildKDTree(Vector2* points, int count, int depth, TreeNode* parent, double interpolation);
void freeTree(TreeNode *node);
void DrawKDTree(TreeNode *node, int xMin, int yMin, int xMax, int yMax);
// void RebuildTree(TreeNode *tree, Vector2 *points, int pointCount,
//                  double interpolation);
int CompareX(const void *a, const void *b);
int CompareY(const void *a, const void *b);
#endif