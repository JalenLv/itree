#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <string.h>
#include "file_tree.h"

/**
 * Push a synthetic FileTreeNode onto a tree. Used by tests to build small,
 * deterministic trees without touching the filesystem.
 */
static inline void synth_push(FileTree *tree, NodeType type, const char *name,
                              int depth, int collapsed, const char *target) {
    FileTreeNode node = {0};
    node.type = type;
    node.depth = depth;
    node.collapsed = collapsed;
    snprintf(node.name, sizeof(node.name), "%s", name ? name : "");
    if (target) {
        snprintf(node.target, sizeof(node.target), "%s", target);
    }
    DA_PUSH(FileTreeNode, tree, node);
}

#endif // TEST_UTIL_H
