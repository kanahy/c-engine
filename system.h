#ifndef system_h
#define system_h


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>


#define array_size(array) \
    ((unsigned int)(sizeof(array) / sizeof(array[0])))


typedef enum directory_content_type_e {
    DIRECTORY_CONTENT_TYPE_UNKNOWN = 0,
    DIRECTORY_CONTENT_TYPE_FIFO = 1,
    DIRECTORY_CONTENT_TYPE_DEVICE = 2,
    DIRECTORY_CONTENT_TYPE_DIRECTORY = 4,
    DIRECTORY_CONTENT_TYPE_BLOCK_DEVICE = 6,
    DIRECTORY_CONTENT_TYPE_REGULAR_FILE = 8,
    DIRECTORY_CONTENT_TYPE_SYMBOLIC_LINK = 10,
    DIRECTORY_CONTENT_TYPE_LOCAL_DOMAIN_SOCKET = 12,
    DIRECTORY_CONTENT_TYPE_WHITEOUT = 14
} directory_content_type_e;


typedef struct directory_content_t {
    char* name;
    directory_content_type_e type;
} directory_content_t;

typedef struct directory_t {
    char* path;
    directory_content_t* content;
    size_t content_count;
} directory_t;


void* file_load(const char* file_name, bool is_text, size_t* size) {
    void* result = NULL;
    FILE* stream = NULL;
    long stream_size = 0;

    *size = 0;
    stream = fopen(file_name, is_text ? "rt" : "rb");

    if (stream) {
        if (!fseek(stream, 0, SEEK_END)) {
            stream_size = ftell(stream);

            if (stream_size) {
                *size = (size_t)stream_size;

                if (!fseek(stream, 0, SEEK_SET)) {
                    result = calloc(*size, sizeof(char));

                    if (result) {
                        if (!fread(result, *size, sizeof(char), stream)) {
                            free(result);

                            result = NULL;
                            *size = 0;
                        }
                    }
                }
            }
        }

        if (fclose(stream)) {
            printf("%s is not closed\n", file_name);
        }
    }

    return result;
}

void file_free(void** file) {
    free(file[0]);
    file[0] = NULL;
}

const char* file_get_extension(const char* file_name) {
    const char* dot = strrchr(file_name, '.');

    if (!dot || strcmp(file_name, dot) <= 0) {
        return NULL;
    }

    return dot + 1;
}

bool file_check_extension(const char* file_name, const char* extension) {
    return strcmp(file_get_extension(file_name), extension) == 0;
}


directory_content_t directory_content_new(struct dirent* dirent) {
    directory_content_t result = {
        .name = NULL,
        .type = DIRECTORY_CONTENT_TYPE_UNKNOWN
    };

    size_t path_size = 0;

    path_size = strlen(dirent->d_name);

    if (path_size) {
        result.name = (char*)calloc(path_size + 1, sizeof(char));

        if (result.name) {
            result.name = strncpy(result.name, dirent->d_name, path_size);
            result.name[path_size] = '\0';

            switch (dirent->d_type) {
                case 1: result.type = DIRECTORY_CONTENT_TYPE_FIFO; break;
                case 2: result.type = DIRECTORY_CONTENT_TYPE_DEVICE; break;
                case 4: result.type = DIRECTORY_CONTENT_TYPE_DIRECTORY; break;
                case 6: result.type = DIRECTORY_CONTENT_TYPE_BLOCK_DEVICE; break;
                case 8: result.type = DIRECTORY_CONTENT_TYPE_REGULAR_FILE; break;
                case 10: result.type = DIRECTORY_CONTENT_TYPE_SYMBOLIC_LINK; break;
                case 12: result.type = DIRECTORY_CONTENT_TYPE_LOCAL_DOMAIN_SOCKET; break;
                case 14: result.type = DIRECTORY_CONTENT_TYPE_WHITEOUT; break;
                default: result.type = DIRECTORY_CONTENT_TYPE_UNKNOWN; break;
            }
        }
    }

    return result;
}

void directory_content_delete(directory_content_t* directory_content) {
    if (directory_content->name) {
        free(directory_content->name);
        directory_content->name = NULL;
    }

    directory_content->type = DIRECTORY_CONTENT_TYPE_UNKNOWN;
}


directory_t directory_load(const char* path) {
    directory_t result = {
        .path = NULL,
        .content = NULL,
        .content_count = 0
    };

    DIR* dir = NULL;
    struct dirent* dirent = NULL;
    size_t count = 0;
    size_t current = 0;
    size_t path_size = 0;

    dir = opendir(path);

    if (dir) {
        path_size = strlen(path);
        result.path = (char*)calloc(path_size + 1, sizeof(char));
        result.path = strncpy(result.path, path, path_size);
        result.path[path_size] = '\0';
        path_size = 0;

        while ((dirent = readdir(dir))) {
            ++count;
        }

        rewinddir(dir);

        result.content = (directory_content_t*)calloc(count, sizeof(directory_content_t));

        if (result.content) {
            while ((dirent = readdir(dir))) {
                result.content[current] = directory_content_new(dirent);
                ++current;
            }

            result.content_count = count;
        }

        closedir(dir);
    }

    return result;
}

void directory_free(directory_t* directory) {
    if (directory->path) {
        free(directory->path);
        directory->path = NULL;
    }

    if (directory->content) {
        for (size_t i = 0; i < directory->content_count; ++i) {
            directory_content_delete(&directory->content[i]);
        }

        free(directory->content);
        directory->content = NULL;
    }

    directory->content_count = 0;
}


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus


#endif // system_h
