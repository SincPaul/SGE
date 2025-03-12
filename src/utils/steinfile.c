//
// Created by Geisthardt on 26.02.2025.
//

#include "steinfile.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "../core/logging.h"
#include "../core/memory_control.h"

#ifdef WIN32
#include <windows.h>
#define create_dir(path) CreateDirectory(path, NULL)
#define get_cwd() _getcwd(NULL, 0)
#else
#ifdef Unix
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#define create_dir(path) mkdir(path, 0777)
#define get_cwd() _getcwd(NULL, 0)
#endif
#endif



void make_filename_save(char *filename) {
        for(int i = 0; i < strlen(filename); i++) {
                if (filename[i] == ':' || filename[i] == '.') {
                        filename[i] = '-';
                }
                else if (filename[i] == ' ') {
                        filename[i] = '_';
                }
        }
}

int create_directory_if_not_exists(const char *dir_path) {
        struct stat st = {0};
        if (stat(dir_path, &st) == -1) {
                #ifdef WIN32
                if(create_dir(dir_path) == 0) {
                        perror("Error creating directory");
                        return 1;
                }
                #else
                #ifdef Unix
                if (create_dir(dir_path) != 0) {
                        perror("Error creating directory");
                        return 1;
                }
                #endif
                #endif
        }

        return 0;
}

char *get_current_working_directory() {
        char *current_working_directory = getcwd(NULL, 0);
        printf("%s\n", current_working_directory);

        return current_working_directory;
}

uint32_t *read_file_as_binary(const char *filepath, size_t *code_size) {
        FILE *file = fopen(filepath, "rb");
        if (file == NULL) {
                log_event(LOG_LEVEL_FATAL, "Failed to open file with path: %s", filepath);
        }
        fseek(file, 0, SEEK_END);
        *code_size = ftell(file);
        rewind(file);

        uint32_t *buffer = allocate_memory(*code_size, MEMORY_TAG_INPUT);
        if (buffer == NULL) {
                log_event(LOG_LEVEL_FATAL, "Failed to allocate Memory");
        }

        size_t bytes_read = fread(buffer, 1, *code_size, file);
        fclose(file);

        if (bytes_read != *code_size) {
                log_event(LOG_LEVEL_FATAL, "File size does not match read bytes");
        }

        return buffer;
}