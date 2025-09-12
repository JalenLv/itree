/**
 * The program directly manipulates cJSON objects representing the file tree.
 */
#ifndef FILE_TREE_H
#define FILE_TREE_H

#include "cJSON.h"
#include <stdio.h>

typedef enum NodeType {
    FILE_NODE,
    DIRECTORY_NODE
} NodeType;

typedef struct FileTreeNode FileTreeNode;

typedef struct Children {
    int count;
    int capacity;
    FileTreeNode **items;
} Children;

typedef struct FileTreeNode {
    NodeType type;
    char *name;
    Children children;
    FileTreeNode *parent;
    int collapsed; // 0 for expanded, 1 for collapsed; expanded by default
} FileTreeNode;

/**
 * Creates a file tree from the given input file.
 * All directories are expanded (collapsed = 0) initially.
 * The collapsed field is NULL for file nodes.
 * Returns NULL if fails.
 * The caller is responsible for freeing the returned tree.
 */
FileTreeNode *create_file_tree(FILE *input);

/**
 * Parses JSON from the given input file and returns the corresponding cJSON object.
 * Returns NULL if parsing fails.
 */
cJSON *parse_json(FILE *input);

/**
 * Creates a FileTreeNode structure from the given cJSON object.
 * Returns NULL if fails.
 * The caller is responsible for freeing the returned tree.
 */
FileTreeNode *create_tree_from_cjson(cJSON *json);

#endif // FILE_TREE_H