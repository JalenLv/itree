#ifndef FILE_TREE_H
#define FILE_TREE_H

#include "helpers.h"

typedef struct FileTreeNode FileTreeNode;
typedef struct {
    DA_FIELDS(FileTreeNode);
} FileTree;

typedef enum {
    FILE_NODE,
    DIRECTORY_NODE,
    LINK_NODE
} NodeType;

typedef struct FileTreeNode {
    NodeType    type;           // type of the node: file, directory, or link
    char        name[256];      // name of the file or directory
    int         collapsed;      // 0 for expanded, 1 for collapsed; expanded by default
    int         depth;          // depth in the tree, root is 0
    char        target[256];    // target path if it's a link; NULL otherwise
} FileTreeNode;

/**
 * Creates a file tree from the given path.
 * All directories are expanded (collapsed = 0) initially.
 * The collapsed field is 0 for file nodes.
 * 
 * Returns 0 on success, non-zero on failure.
 * `file_tree` is empty if fails and is populated if succeeds.
 * The caller is responsible for freeing the returned tree.
 */
int create_file_tree_from_path(FileTree *file_tree, const char *path);

#endif // FILE_TREE_H