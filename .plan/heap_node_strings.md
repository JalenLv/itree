Strings inside a file tree node are currently stored as string literals inside the struct. Refactor the strings out onto the heap and store pointers to them.
