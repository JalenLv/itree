#ifndef FILE_TREE_H
#define FILE_TREE_H

#include "helpers.h"
#include <limits.h>

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
 * Creates a sorted file tree from the given path.
 * All directories are expanded (collapsed = 0) initially.
 * The collapsed field is 0 for file nodes.
 * 
 * Returns 0 on success, non-zero on failure.
 * `file_tree` is empty if fails and is populated if succeeds.
 * The caller is responsible for freeing the returned tree.
 */
int create_file_tree_from_path(FileTree *file_tree, const Args *args);

/**
 * next(last_visible) is FILE_TREE_SENTINEL_TAIL
 * next(FILE_TREE_SENTINEL_HEAD) is the first visible index
 * next(FILE_TREE_SENTINEL_TAIL) is FILE_TREE_SENTINEL_TAIL
 * prev(first_visible) is FILE_TREE_SENTINEL_HEAD
 * prev(FILE_TREE_SENTINEL_TAIL) is the last visible index
 * prev(FILE_TREE_SENTINEL_HEAD) is FILE_TREE_SENTINEL_HEAD
 */
#define FILE_TREE_SENTINEL_HEAD (INT_MIN)
#define FILE_TREE_SENTINEL_TAIL (INT_MAX)

/**
 * Given a file tree and a visible node index, returns the next visible index.
 * Skips over collapsed directories.
 * 
 * Returns -1 on failure (e.g. invalid index)
 * 
 * Behavior is undefined if the given index is not visible.
 * Assumes file_tree is non-empty.
 */
int next(FileTree *file_tree, int idx);

/**
 * Given a file tree and a visible node index, returns the previous visible index.
 * Skips over collapsed directories.
 * 
 * Returns -1 on failure (e.g. invalid index)
 * 
 * Behavior is undefined if the given index is not visible.
 * Assumes file_tree is non-empty.
 */
int prev(FileTree *file_tree, int idx);

#endif // FILE_TREE_H
