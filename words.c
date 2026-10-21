#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


typedef struct entry {
    char *word;
    size_t count;
    struct entry *next;
} entry;


static entry **table;
static size_t table_size;
static size_t tables_used;

static void *xmalloc(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    return ptr;
}

static unsigned long hash_word(const char *word)
{
    unsigned long hash = 5381;

    while (*word) {
        hash = hash * 33 + (unsigned char)*word;
        word++;
    }

    return hash;
}

static void add_word(const char *word)
{
    if (table == NULL) {
        table_size = 1024;
        table = calloc(table_size, sizeof(entry *));

        if (table == NULL) {
            perror("calloc");
            exit(EXIT_FAILURE);
        }
    }

    size_t index = hash_word(word) % table_size;
    entry *current = table[index];

    while (current != NULL) {
        if (strcmp(current->word, word) == 0) {
            current->count++;
            return;
        }

        current = current->next;
    }

    entry *new_entry = xmalloc(sizeof(entry));
    new_entry->word = xmalloc(strlen(word) + 1);

    strcpy(new_entry->word, word);
    new_entry->count = 1;
    new_entry->next = table[index];

    table[index] = new_entry;
    tables_used++;
}

static int is_letter(unsigned char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z');
}

static void append_character(char **word, size_t *length,
                             size_t *capacity, unsigned char c)
{
    if (*length + 2 > *capacity) {
        *capacity *= 2;

        char *temp = realloc(*word, *capacity);
        if (temp == NULL) {
            perror("realloc");
            free(*word);
            exit(EXIT_FAILURE);
        }

        *word = temp;
    }

    (*word)[(*length)++] = c;
}

static void process_file(const char *path)
{
    int fd = open(path, O_RDONLY);

    if (fd == -1) {
        fprintf(stderr, "words: cannot open %s: %s\n",
                path, strerror(errno));
        return;
    }

    size_t capacity = 64;
    size_t length = 0;
    char *word = xmalloc(capacity);

    unsigned char buffer[4096];
    ssize_t bytes_read;

    int pending_hyphen = 0;
    int utf8_state = 0;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            unsigned char c = buffer[i];

            // Detect curly apostrophe (UTF-8: E2 80 99)
            if (utf8_state == 1) {
                utf8_state = 0;

                if (c == 0x80) {
                    utf8_state = 2;
                    continue;
                }
            }
            else if (utf8_state == 2) {
                utf8_state = 0;

                if (c == 0x99) {
                    if (pending_hyphen) {
                        word[length] = '\0';
                        add_word(word);
                        length = 0;
                        pending_hyphen = 0;
                    }

                    append_character(&word, &length,
                                     &capacity, 0xE2);
                    append_character(&word, &length,
                                     &capacity, 0x80);
                    append_character(&word, &length,
                                     &capacity, 0x99);
                    continue;
                }
            }
            else if (c == 0xE2) {
                utf8_state = 1;
                continue;
            }

            if (pending_hyphen && !is_letter(c)) {
                word[length] = '\0';
                add_word(word);
                length = 0;
                pending_hyphen = 0;
            }

            if (is_letter(c) || c == '\'') {
                if (pending_hyphen) {
                    append_character(&word, &length,
                                     &capacity, '-');
                    pending_hyphen = 0;
                }

                append_character(&word, &length,
                                 &capacity, c);
            }
            else if (c == '-' && length > 0 &&
                     is_letter((unsigned char)word[length - 1])) {
                pending_hyphen = 1;
            }
            else {
                if (length > 0) {
                    word[length] = '\0';
                    add_word(word);
                    length = 0;
                }

                pending_hyphen = 0;
            }
        }
    }

    if (bytes_read < 0) {
        fprintf(stderr, "words: cannot read %s: %s\n",
                path, strerror(errno));
    }

    if (length > 0) {
        word[length] = '\0';
        add_word(word);
    }

    free(word);
    close(fd);
}


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
        full_path[dir_path_len + need_slash + name_len] = '\0';

        struct stat st;
            //if the file is a directory, recursively traverse it
           if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                traverse_directory(full_path);
            } else if (S_ISREG(st.st_mode) && has_suffix(name)) {
                process_file(full_path);
            }
        }
        else {
            fprintf(stderr, "words: cannot stat %s: %s\n", full_path, strerror(errno));
        }
            free(full_path);
    }
        closedir(dir);           
}