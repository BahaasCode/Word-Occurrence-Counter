#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>


typedef struct entry {
    char *word;
    size_t count;
    struct entry *next;
} entry;


static entry **table;
static size_t table_size;
static size_t tables_used;








//------------------------------- directory traversal ---------------------------------

static const char *suffix = ".txt";

static int has_suffix(const char *filename) {
    size_t len = strlen(filename), suffix_len = strlen(suffix);
    return len >= suffix_len && strcmp(filename + len - suffix_len, suffix) == 0;
}

static void traverse_directory(const char *dir_path) {
    // Implementation for directory traversal to find .txt files
    // This function should fill the hash table with words from the text files found
    
    DIR *dir = opendir(dir_path);
    if(!dir) {
        fprintf(stderr, "words: cannot open directory %s: %s\n", dir_path, strerror(errno));
        return;
    }

}