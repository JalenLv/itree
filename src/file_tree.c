#include "file_tree.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

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
        if (sizeof(ent->d_name) > sizeof(node.name)) {
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
    root.name[0] = '.'; root.name[1] = '\0';    // represent the root with "."
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
