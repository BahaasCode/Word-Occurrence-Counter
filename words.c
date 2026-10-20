#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>


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


    //everything below here is crazy

    size_t dir_path_len = strlen(dir_path);
    int need_slash = (dir_path_len > 0 && dir_path[dir_path_len - 1] != '/');
    struct dirent *entry;

    while((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        if(name[0] == '.') {
            continue; // skip hidden files and directories
        }
        size_t name_len = strlen(name);


        char *full_path = xmalloc(dir_path_len + name_len + 2); //might wanna double check if its malloc or xmalloc

        memcpy(full_path, dir_path, dir_path_len);
        if(need_slash) {
            full_path[dir_path_len] = '/';
        }
        memcpy(full_path + dir_path_len + need_slash, name, name_len);

        struct stat st;
        //check if the file is a directory or a regular file
        if(stat(full_path, &st) == 0) {
            //if the file is a directory, recursively traverse it
            if (stat(full_path, &st) == 0) {
                if(S_ISDIR(st.st_mode)) {
                    traverse_directory(full_path);
                } else if(S_ISREG(st.st_mode) && has_suffix(name)) {
                    // process through the file, retiteration of the process_file function
                    process_file(full_path);
                } else {
                    fprintf(stderr, "words: cannot stat %s: %s\n", full_path, strerror(errno));
                }
                free(full_path);
            }   
            closedir(dir);
        }
    }
} //i need to make sure the brackets are correct