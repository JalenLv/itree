#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "file_tree.h"
#include "helpers.h"

cJSON *create_file_tree(FILE *input) {
    // Read and parse JSON
    cJSON *file_tree = parse_json(input);

    // Init collapsed state
    file_tree = init_collapsed_state(file_tree);

    return file_tree;
}

cJSON *parse_json(FILE *input) {
    char *json_string = strdup("");

    size_t len;
    char *line;
    while ((line = fgetln(input, &len)) != NULL) {
        char *line_null = malloc(len + 1);
        memcpy(line_null, line, len);
        line_null[len] = '\0';

        char *old_json_string = json_string;
        json_string = concat(old_json_string, line_null);
        free(old_json_string);
        free(line_null);
    }

    cJSON *json = cJSON_Parse(json_string);
    free(json_string);
    return json;
}

/**
 * Recursively initializes the "collapsed" field to 0 (expanded) for all directories in the sub file tree.
 * Returns 0 on success, 1 on failure.
 */
int init_collapsed_recursive(cJSON *sub_file_tree) {
    cJSON *type = cJSON_GetObjectItemCaseSensitive(sub_file_tree, "type");
    char *type_str = cJSON_GetStringValue(type);

    // If the type is directory, add the "collapsed" item
    // and recursively init every children inside "contents".
    // If the directory has no children, just add the "collapsed" item
    // and stop the recursion.
    if (strcmp(type_str, "directory") == 0) {
        // Add "collapsed" item
        cJSON_AddBoolToObject(sub_file_tree, "collapsed", 0);

        // Recurse into every children inside "contents"
        if (cJSON_HasObjectItem(sub_file_tree, "contents")) {
            cJSON *contents = cJSON_GetObjectItemCaseSensitive(sub_file_tree, "contents");
            cJSON *child = NULL;
            cJSON_ArrayForEach(child, contents) {
                if (init_collapsed_recursive(child) != 0) {
                    fprintf(stderr, "Error: Failed to initialize collapsed state for child node %s.\n", cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(child, "name")));
                    return 1;
                }
            }
        }
        return 0;
    }

    // If the type is file, do nothing and stop the recursion.
    if (strcmp(type_str, "file") == 0) {
        return 0;
    }

    // Unknown type
    return 1;
}

cJSON *init_collapsed_state(cJSON *file_tree) {
    cJSON *real_file_tree = cJSON_GetArrayItem(file_tree, 0);

    init_collapsed_recursive(real_file_tree);

    return file_tree;
}
