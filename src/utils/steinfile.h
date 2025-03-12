//
// Created by Geisthardt on 26.02.2025.
//

#ifndef STEINFILE_H
#define STEINFILE_H
#include <stdint.h>

void make_filename_save(char *filename);
int create_directory_if_not_exists(const char *dir_path);
char *get_current_working_directory();
uint32_t *read_file_as_binary(const char *filepath, size_t *codeSize);

#endif //STEINFILE_H
