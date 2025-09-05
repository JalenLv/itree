/**
 * The program directly manipulates cJSON objects representing the file tree.
 */
#ifndef FILE_TREE_H
#define FILE_TREE_H

#include "cJSON.h"

/**
 * Creates a file tree from the given input file.
 * The items in the returned cJSON object represent either files or directories.
 * And the items contain another field "collapsed" to represent whether the directory is expanded or not.
 * The initial state is that all directories are expanded (collapsed = 0).
 */
cJSON *create_file_tree(FILE *input);

/**
 * Parses JSON from the given input file and returns the corresponding cJSON object.
 * The returned cJSON object does not have the "collapsed" field initialized.
 * Returns NULL if parsing fails.
 */
cJSON *parse_json(FILE *input);

/**
 * Initializes the "collapsed" field to 0 (expanded) for all directories.
 * Returns the modified cJSON object.
 */
cJSON *init_collapsed_state(cJSON *file_tree);

#endif // FILE_TREE_H