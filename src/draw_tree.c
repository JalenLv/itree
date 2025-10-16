#include <string.h>

#include "draw_tree.h"
#include "helpers.h"

// Box-drawing characters
#define TREE_BRANCH "├── "
#define TREE_LAST   "└── "
#define TREE_VERT   "│   "
#define TREE_SPACE  "    "

/**
 * Recursively draws the sub file tree.
 */
int draw_node_recursive(FileTreeNode *sub_file_tree, FILE *output, char *prefix, int is_last, int is_root) {
    // Print the current node
    NodeType type = sub_file_tree->type;
    char *name = sub_file_tree->name;

    char *suffix = NULL;
    if (type == FILE_NODE) {
        suffix = "";
    } else if (type == LINK_NODE) {
        suffix = " -> ";
    } else if (type == DIRECTORY_NODE) {
        suffix = "/";
    } else {
        fprintf(stderr, "Unknown type\n");
        return 1;
    }

    fprintf(output, "%s%s%s%s%s\n",
            prefix, (is_root ? "" : (is_last ? TREE_LAST : TREE_BRANCH)),
            name, suffix, (type == LINK_NODE ? sub_file_tree->target : ""));

    // If it's a directory and not collapsed, print its children
    if ((type == DIRECTORY_NODE) && (!sub_file_tree->collapsed)) {
        int n_children = sub_file_tree->children.count;
        for (int i = 0; i < n_children; i++) {
            // FileTreeNode *child = sub_file_tree->children.items[i];
            FileTreeNode *child = DA_GET(FileTreeNode *, &(sub_file_tree->children), i);
            char *new_prefix = is_root ? prefix : concat(prefix, is_last ? strdup(TREE_SPACE) : strdup(TREE_VERT));
            int new_is_last = (i == n_children - 1);
            draw_node_recursive(child, output, new_prefix, new_is_last, 0);
        }
    }

    return 0;
}

int draw_tree(FileTreeNode *file_tree, FILE *output) {
    char *prefix = strdup("");
    int is_last = 1; // Root is always the last node at its level
    if (draw_node_recursive(file_tree, output, prefix, is_last, 1) != 0) {
        fprintf(stderr, "Error: Failed to draw some node.\n");
        return 1;
    }

    return 0;
}
