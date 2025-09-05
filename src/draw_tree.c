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
int draw_node_recursive(cJSON *sub_file_tree, FILE *output, char *prefix, int is_last, int is_root) {
    // Print the current node
    cJSON *type = cJSON_GetObjectItemCaseSensitive(sub_file_tree, "type");
    cJSON *name = cJSON_GetObjectItemCaseSensitive(sub_file_tree, "name");
    char *type_str = cJSON_GetStringValue(type);
    char *name_str = cJSON_GetStringValue(name);

    char *suffix = NULL;
    if (strcmp(type_str, "file") == 0) {
        suffix = "";
    } else if (strcmp(type_str, "directory") == 0) {
        suffix = "/";
    } else {
        fprintf(stderr, "Unknown type: %s\n", type_str);
        return 1;
    }

    fprintf(output, "%s%s%s%s\n",
            prefix, (is_root ? "" : (is_last ? TREE_LAST : TREE_BRANCH)), name_str, suffix);

    // If it's a directory and not collapsed, print its children
    cJSON *collapsed = cJSON_GetObjectItemCaseSensitive(sub_file_tree, "collapsed");
    if (
        (!cJSON_IsTrue(collapsed))
        && cJSON_HasObjectItem(sub_file_tree, "contents")
    ) {
        cJSON *contents = cJSON_GetObjectItemCaseSensitive(sub_file_tree, "contents");
        cJSON *child = NULL;
        size_t n_children = cJSON_GetArraySize(contents);
        size_t i = 1;
        cJSON_ArrayForEach(child, contents) {
            char *new_prefix = is_root ? prefix : concat(prefix, is_last ? strdup(TREE_SPACE) : strdup(TREE_VERT));
            int new_is_last = (i == n_children);
            draw_node_recursive(child, output, new_prefix, new_is_last, 0);
            i++;
        }
    }

    return 0;
}

int draw_tree(cJSON *file_tree, FILE *output) {
    cJSON *real_file_tree = cJSON_GetArrayItem(file_tree, 0);

    char *prefix = strdup("");
    int is_last = 1; // Root is always the last node at its level
    draw_node_recursive(real_file_tree, output, prefix, is_last, 1);

    return 0;
}
