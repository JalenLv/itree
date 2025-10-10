#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "file_tree.h"
#include "helpers.h"

FileTreeNode *create_file_tree(FILE *input) {
    // Read and parse JSON
    cJSON *file_tree_json = parse_json(input);
    if (file_tree_json == NULL) {
        fprintf(stderr, "Error: Failed to parse JSON input.\n");
        return NULL;
    }

    // Create file tree structure from cJSON object
    FileTreeNode *file_tree = create_tree_from_cjson(file_tree_json);
    if (file_tree == NULL) {
        fprintf(stderr, "Error: Failed to create file tree from JSON.\n");
        cJSON_Delete(file_tree_json);
        return NULL;
    }

    // Clean up
    cJSON_Delete(file_tree_json);
    return file_tree;
}

cJSON *parse_json(FILE *input) {
    char *json_string = strdup("");

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), input) != NULL) {
        char *old_json_string = json_string;
        json_string = concat(old_json_string, buffer);
        free(old_json_string);
    }

    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        free(json_string);
        return NULL;
    }

    free(json_string);
    return json;
}

FileTreeNode *create_tree_from_cjson_recursive(cJSON *file_tree_json, int depth) {
    FileTreeNode *node = malloc(sizeof(FileTreeNode));
    if (node == NULL) {
        return NULL;
    }

    cJSON *type = cJSON_GetObjectItemCaseSensitive(file_tree_json, "type");
    char *type_str = cJSON_GetStringValue(type);
    if (type_str == NULL) {
        free(node);
        return NULL;
    } else if (strcmp(type_str, "file") == 0) {
        node->type = FILE_NODE;
    } else if (strcmp(type_str, "directory") == 0) {
        node->type = DIRECTORY_NODE;
    } else {
        free(node);
        return NULL;
    }

    cJSON *name = cJSON_GetObjectItemCaseSensitive(file_tree_json, "name");
    char *name_str = cJSON_GetStringValue(name);
    if (name_str == NULL) {
        free(node);
        return NULL;
    }
    node->name = strdup(name_str);
    node->children = (Children){0};
    node->collapsed = (node->type == DIRECTORY_NODE) ? 0 : NULL;
    node->depth = depth;
    cJSON *child = NULL;
    cJSON *children = cJSON_GetObjectItemCaseSensitive(file_tree_json, "contents");
    cJSON_ArrayForEach(child, children) {
        FileTreeNode *child_node = create_tree_from_cjson_recursive(child, depth + 1);
        if (child_node != NULL) {
            DA_PUSH(FileTreeNode *, &(node->children), child_node);
        } else {
            // Clean up should be done from top to bottom,
            // i.e., the `free` function should recursively
            // do the clean up for the root.
            return NULL;
        }
    }

    return node;
}

FileTreeNode *create_tree_from_cjson(cJSON *file_tree_json) {
    // Discard report from the tree program
    cJSON *real_file_tree_json = cJSON_GetArrayItem(file_tree_json, 0);
    if (real_file_tree_json == NULL) {
        return NULL;
    }

    FileTreeNode *root = malloc(sizeof(FileTreeNode));
    if (root == NULL) {
        return NULL;
    }
    root->type = DIRECTORY_NODE;
    root->name = strdup(".");
    root->children = (Children){0};
    root->collapsed = 0;
    root->depth = 0;
    // Recursively build the tree
    cJSON *child = NULL;
    cJSON *children = cJSON_GetObjectItemCaseSensitive(real_file_tree_json, "contents");
    cJSON_ArrayForEach(child, children) {
        FileTreeNode *child_node = create_tree_from_cjson_recursive(child, 1);
        if (child_node != NULL) {
            // Add child_node to root's children
            DA_PUSH(FileTreeNode *, &(root->children), child_node);
        } else {
            // TODO: implement a proper free function for the tree
            free(root);
            return NULL;
        }
    }

    return root;
}
