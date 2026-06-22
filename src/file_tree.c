#include "file_tree.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

static int entry_cmp(const void *a, const void *b) {
    const FileTreeNode *node_a = (const FileTreeNode *)a;
    const FileTreeNode *node_b = (const FileTreeNode *)b;

    return strcasecmp(
        node_a->name + (node_a->name[0] == '.'),
        node_b->name + (node_b->name[0] == '.')
    );
}

static int walk(FileTree *file_tree, const char *path, int depth, size_t offset, int show_hidden) {
    DIR *d = opendir(path);
    if (d == NULL) {
        perror("opendir");
        return 1;
    }

    int ret = 0;
    FileTree tmp_tree = {0};
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
            continue;

        char fullpath[4096];
        if (strlen(path) + strlen(ent->d_name) + 1 >= sizeof(fullpath)) {
            fprintf(stderr, "Error: Path too long: %s/%s\n", path, ent->d_name);
            ret = 1; goto CLEANUP;
        } else snprintf(fullpath, sizeof(fullpath), "%s/%s", path, ent->d_name);

        int is_hidden = (ent->d_name[0] == '.');
        if (!show_hidden && is_hidden) continue;
        int is_reg    = 0;
        int is_dir    = 0;
        int is_syml   = 0;
        switch (ent->d_type) {
            case DT_REG: { is_reg  = 1; break; }
            case DT_DIR: { is_dir  = 1; break; }
            case DT_LNK: { is_syml = 1; break; }
            case DT_UNKNOWN: {
                // must stat to know
                struct stat st;
                if (lstat(fullpath, &st) == 0) {
                    if (S_ISREG(st.st_mode)) is_reg  = 1;
                    if (S_ISDIR(st.st_mode)) is_dir  = 1;
                    if (S_ISLNK(st.st_mode)) is_syml = 1;
                } else {
                    perror("lstat");
                    ret = 1; goto CLEANUP;
                }
                break;
            }
            default: {
                // DT_CHR, DT_BLK, DT_FIFO, DT_SOCK, etc.
                fprintf(stderr, "Warning: Skipping unsupported file type: %s\n", fullpath);
                continue;
            }
        }

        FileTreeNode node = {0};
        if (is_reg)       node.type = FILE_NODE;
        else if (is_dir)  node.type = DIRECTORY_NODE;
        else if (is_syml) node.type = LINK_NODE;

        if (strlen(ent->d_name) >= sizeof(node.name)) {
            fprintf(stderr, "Error: Filename too long: %s\n", ent->d_name);
            ret = 1; goto CLEANUP;
        } else snprintf(node.name, sizeof(node.name), "%s", ent->d_name);

        node.collapsed = is_hidden ? 1 : 0;
        node.depth     = depth;

        if (is_syml) {
            ssize_t len = readlink(fullpath, node.target, sizeof(node.target) - 1);
            if (len == -1) {
                perror("readlink");
                ret = 1; goto CLEANUP;
            } else {
                node.target[len] = '\0';
            }
        }

        FileTree_push(&tmp_tree, &node);
    }
    if (FileTree_empty(&tmp_tree)) goto CLEANUP;
    FileTree_sort(&tmp_tree, entry_cmp);
    FileTree_insert(file_tree, &tmp_tree, offset);

    size_t i = tmp_tree.count - 1;
    for (;;) {
        FileTreeNode *node;
        if (FileTree_get_ptr(&tmp_tree, i, &node) != 0) {
            ret = 1; goto CLEANUP;
        }
        if (node->type == DIRECTORY_NODE) {
            char child_path[4096];
            if (strlen(path) + strlen(node->name) + 1 >= sizeof(child_path)) {
                fprintf(stderr, "Error: Path too long: %s/%s\n", path, node->name);
                ret = 1; goto CLEANUP;
            } else snprintf(child_path, sizeof(child_path), "%s/%s", path, node->name);

            if (walk(file_tree, child_path, depth + 1, offset + i + 1, show_hidden) != 0) {
                ret = 1; goto CLEANUP;
            }
        }

        if (i == 0) break;
        i--;
    }

CLEANUP:
    closedir(d);
    FileTree_free(&tmp_tree);
    return ret;
}

int create_file_tree_from_path(FileTree *file_tree, const Args *args) {
    char *path = args->path;
    if (path == NULL) {
        fprintf(stderr, "Error: Path is required.\n");
        return 1;
    }

    // Add root node
    FileTreeNode root;

    root.type = DIRECTORY_NODE;

    // basename may modify the input path, so we copy it first
    char path_copy[4096];
    if (strlen(path) >= sizeof(path_copy)) {
        fprintf(stderr, "Error: Path too long: %s\n", path);
        return 1;
    } else snprintf(path_copy, sizeof(path_copy), "%s", path);
    char *path_basename = basename(path_copy);
    if (strlen(path_basename) >= sizeof(root.name)) {
        fprintf(stderr, "Error: Path basename too long: %s\n", path_basename);
        return 1;
    } else snprintf(root.name, sizeof(root.name), "%s", path_basename);

    root.collapsed = 0;
    root.depth = 0;
    root.target[0] = '\0';
    FileTree_push(file_tree, &root);

    // Iterate over directory entries
    if (walk(file_tree, path, 1, 1, args->show_hidden) != 0) {
        FileTree_free(file_tree);
        return 1;
    }

    return 0;
}

size_t next(FileTree *file_tree, size_t idx) {
    if (idx == FILE_TREE_SENTINEL_HEAD) return 0;
    if (idx == FILE_TREE_SENTINEL_TAIL) return FILE_TREE_SENTINEL_TAIL;

    FileTreeNode *node;
    if (FileTree_get_ptr(file_tree, idx, &node) != 0) {
        // TODO: We barely do error checking on next()
        // Better design a new way of handling next() and its potential error
        exit(1);
    }

    // If the node is a file or a link, return the next index
    if (node->type == FILE_NODE || node->type == LINK_NODE) {
        return (idx + 1 < file_tree->count) ? idx + 1 : FILE_TREE_SENTINEL_TAIL;
    }

    // If the node is a directory and expanded, return the next index
    if (node->type == DIRECTORY_NODE && node->collapsed == 0) {
        return (idx + 1 < file_tree->count) ? idx + 1 : FILE_TREE_SENTINEL_TAIL;
    }

    // If the node is a directory and collapsed, skip its children
    int depth = node->depth;
    for (size_t i = idx + 1; i < file_tree->count; i++) {
        FileTreeNode *next_node;
        if (FileTree_get_ptr(file_tree, i, &next_node) != 0) {
            // TODO: better error handling
            exit(1);
        }
        if (next_node->depth <= depth) {
            return i;
        }
    }

    // No more visible nodes
    return FILE_TREE_SENTINEL_TAIL;
}

size_t prev(FileTree *file_tree, size_t idx) {
    if (idx == 0) return FILE_TREE_SENTINEL_HEAD;
    if (idx == FILE_TREE_SENTINEL_HEAD) return FILE_TREE_SENTINEL_HEAD;
    if (idx == FILE_TREE_SENTINEL_TAIL) {
        size_t i = 0, next_i;
        while ((next_i = next(file_tree, i)) != FILE_TREE_SENTINEL_TAIL) {
            i = next_i;
        }
        return i;
    }

    FileTreeNode *node;
    if (FileTree_get_ptr(file_tree, idx, &node) != 0) {
        // TODO: better error handling
        exit(1);
    }

    int depth = node->depth;
    size_t candidate = idx - 1;
    FileTreeNode *candidate_node;
    if (FileTree_get_ptr(file_tree, candidate, &candidate_node) != 0) {
        // TODO: better error handling
        exit(1);
    }
    int candidate_depth = candidate_node->depth;
    if (candidate_depth <= depth) {
        // If the previous node is at the same or shallower depth, return it
        return candidate;
    } else {
        // Otherwise, find its ancestors and infer from their state
        int cur_depth = candidate_depth;
        size_t i = candidate;
        for (;;) {
            FileTreeNode *node_i;
            if (FileTree_get_ptr(file_tree, i, &node_i) != 0) {
                // TODO: better error handling
                exit(1);
            }
            if (node_i->depth == cur_depth - 1) {
                if (node_i->collapsed) {
                    candidate = i;
                }
                if (--cur_depth == depth) {
                    return candidate;
                }
            }

            if (i == 0) break;
            i--;
        }
    }

    // TODO: better error handling
    exit(1);
}
