#ifndef DRAW_TREE_H
#define DRAW_TREE_H

#include "cJSON.h"
#include <stdio.h>

/**
 * Draws the file tree to the given output file.
 */
int draw_tree(cJSON *file_tree, FILE *output);

#endif // DRAW_TREE_H