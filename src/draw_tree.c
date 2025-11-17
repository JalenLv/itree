#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "draw_tree.h"
#include "helpers.h"

// Box-drawing characters
#define TREE_BRANCH "├── "
#define TREE_LAST   "└── "
#define TREE_VERT   "│   "
#define TREE_SPACE  "    "

int draw_tree(FileTree *file_tree, FILE *output) {
	int max_depth = 0;
	for (int i = 0; i < file_tree->count; ++i) {
		if (file_tree->items[i].depth > max_depth) {
			max_depth = file_tree->items[i].depth;
		}
	}

	// Track whether an ancestor at each depth still has siblings to print pipes accordingly
	int *ancestor_last = calloc((size_t)max_depth + 1, sizeof(int));
	if (ancestor_last == NULL) {
		perror("calloc");
		return 1;
	}

	int collapsed_depth = -1;

	for (int i = 0; i < file_tree->count; ++i) {
		FileTreeNode *node = &file_tree->items[i];
		int depth = node->depth;

		if (collapsed_depth != -1) {
			if (depth <= collapsed_depth) {
				collapsed_depth = -1;
			} else {
				continue;
			}
		}

		int is_last = 1;
		for (int j = i + 1; j < file_tree->count; ++j) {
			int next_depth = file_tree->items[j].depth;
			if (next_depth < depth) {
				break;
			}
			if (next_depth == depth) {
				is_last = 0;
				break;
			}
		}

		for (int level = 1; level < depth; ++level) {
			fputs(ancestor_last[level] ? TREE_SPACE : TREE_VERT, output);
		}
		if (depth > 0) {
			fputs(is_last ? TREE_LAST : TREE_BRANCH, output);
		}

		switch (node->type) {
			case FILE_NODE:
				fprintf(output, "%s\n", node->name);
				break;
			case DIRECTORY_NODE:
				fprintf(output, "%s/\n", node->name);
				break;
			case LINK_NODE:
				fprintf(output, "%s -> %s\n", node->name, node->target);
				break;
			default:
				free(ancestor_last);
				fprintf(stderr, "Error: Unknown node type encountered.\n");
				return 1;
		}

		ancestor_last[depth] = is_last;
		if (node->type == DIRECTORY_NODE && node->collapsed) {
			collapsed_depth = depth;
		}
	}

	free(ancestor_last);
	return 0;
}
