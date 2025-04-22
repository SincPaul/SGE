//
// Created by Geisthardt on 27.02.2025.
//

#ifndef SGE_UTILS_H
#define SGE_UTILS_H

#include <stdbool.h>
#include <stddef.h>

void terminate_program();

void shutdown_program();

void *sort_linked_list(void *head_ptr, size_t offset_sort_attribute, size_t offset_next, size_t sort_attribute_size, size_t next_size, bool reversed, void *(*get_next)(const void *));

void sge_hexdump(const void *data, size_t size);
#endif //SGE_UTILS_H
