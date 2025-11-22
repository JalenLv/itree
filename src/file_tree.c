#include "file_tree.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

int walk(FileTree *file_tree, const char *path, int depth) {
    DIR *d = opendir(path);
    if (d == NULL) {
        perror("opendir");
        return 1;
    }

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
            continue;

        char fullpath[4096];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, ent->d_name);

        int is_hidden = (ent->d_name[0] == '.');
        int is_reg = 0;
        int is_dir = 0;
        int is_syml = 0;
        switch (ent->d_type) {
            case DT_REG:
                is_reg = 1;
                break;
            case DT_DIR:
                is_dir = 1;
                break;
            case DT_LNK:
                is_syml = 1;
                break;
            case DT_UNKNOWN:
                // must stat to know
                struct stat st;
                if (lstat(fullpath, &st) == 0) {
                    if (S_ISREG(st.st_mode))
                        is_reg = 1;
                    if (S_ISDIR(st.st_mode))
                        is_dir = 1;
                    if (S_ISLNK(st.st_mode))
                        is_syml = 1;
                }
                break;
            default:
                // DT_CHR, DT_BLK, DT_FIFO, DT_SOCK, etc.
                fprintf(stderr, "Warning: Skipping unsupported file type: %s\n", fullpath);
                return 1;
        }


        FileTreeNode node;
        if (is_reg) {
            node.type = FILE_NODE;
        } else if (is_dir) {
            node.type = DIRECTORY_NODE;
        } else if (is_syml) {
            node.type = LINK_NODE;
        }
        if (strlen(ent->d_name) >= sizeof(node.name)) {
            fprintf(stderr, "Error: Filename too long: %s\n", ent->d_name);
            return 1;
        } else {
            snprintf(node.name, sizeof(node.name), "%s", ent->d_name);
        }
        node.collapsed = is_hidden ? 1 : 0;
        node.depth = depth;
        if (is_syml) {
            ssize_t len = readlink(fullpath, node.target, sizeof(node.target) - 1);
            if (len == -1) {
                perror("readlink");
                return 1;
            } else {
                node.target[len] = '\0';
            }
        }
        DA_PUSH(FileTreeNode, file_tree, node);
        
        if (is_dir && walk(file_tree, fullpath, depth + 1) != 0) {
            return 1;
        }
    }

    closedir(d);
    return 0;
}

int create_file_tree_from_path(FileTree *file_tree, const char *path) {
    // Add root node
    FileTreeNode root;
    root.type = DIRECTORY_NODE;
    char path_copy[4096];
    snprintf(path_copy, sizeof(path_copy), "%s", path);
    snprintf(root.name, sizeof(root.name), "%s", basename(path_copy));
    root.collapsed = 0;
    root.depth = 0;
    root.target[0] = '\0';
    DA_PUSH(FileTreeNode, file_tree, root);

    // Iterate over directory entries
    if (walk(file_tree, path, 1) != 0) {
        DA_FREE(FileTreeNode, file_tree);
        return 1;
    }

    return 0;
}

int next(FileTree *file_tree, int idx) {
    FileTreeNode *node = DA_GET_PTR(FileTreeNode *, file_tree, idx);
    if (node == NULL) {
        return -1;
    }

    // If the node is a file or a link, return the next index
    if (node->type == FILE_NODE || node->type == LINK_NODE) {
        return (idx + 1 < file_tree->count) ? idx + 1 : 0;
    }

    // If the node is a directory and expanded, return the next index
    if (node->type == DIRECTORY_NODE && node->collapsed == 0) {
        return (idx + 1 < file_tree->count) ? idx + 1 : 0;
    }

    // If the node is a directory and collapsed, skip its children
    int depth = node->depth;
    for (int i = idx + 1; i < file_tree->count; i++) {
        FileTreeNode *next_node = DA_GET_PTR(FileTreeNode *, file_tree, i);
        if (next_node->depth <= depth) {
            return i;
        }
    }

    return 0;
}

int prev(FileTree *file_tree, int idx) {
    FileTreeNode *node = DA_GET_PTR(FileTreeNode *, file_tree, idx);
    if (node == NULL) {
        return -1;
    }

    if (idx == 0) {
        while (next(file_tree, idx) != 0) {
            idx = next(file_tree, idx);
        }
        return idx;
    }

    int depth = node->depth;
    if (DA_GET(FileTreeNode, file_tree, idx - 1).depth <= depth) {
        // If the previous node is at the same or shallower depth, return it
        return idx - 1;
    } else {
        // Otherwise, find its ancestors and infer from their state
        int candidate = idx - 1;
        int cur_depth = DA_GET(FileTreeNode, file_tree, candidate).depth;
        for (int i = candidate; i >= 0; i--) {
            if (DA_GET(FileTreeNode, file_tree, i).depth == cur_depth - 1) {
                if (DA_GET(FileTreeNode, file_tree, i).collapsed) {
                    candidate = i;
                }
                if (--cur_depth == depth) return candidate;
            }
        }
    }

    return -1; // should not reach here
}
