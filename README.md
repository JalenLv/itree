# itree
Interactive Tree - Interactively View the File Tree and Selectively Collapse/Expand Folders to Build a Desired Text-Based File Tree View just like Tree Does.

## Usage
```bash
# Generate JSON and pipe to itree
tree -J | itree 

# Or read from a file
tree -J > tree.json
itree -i tree.json -o output.txt
```
