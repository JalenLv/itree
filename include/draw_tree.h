#ifndef DRAW_TREE_H
#define DRAW_TREE_H

#include "file_tree.h"
#include <stdio.h>

/**
 * Draws the file tree to the given output file.
 */
int draw_tree(FileTreeNode *file_tree, FILE *output);

#endif // DRAW_TREE_H