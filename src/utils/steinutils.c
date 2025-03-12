//
// Created by Geisthardt on 27.02.2025.
//

#include "steinutils.h"

#include <stdbool.h>

#include "../core/logging.h"
#include <stdlib.h>

#include "../core/memory_control.h"
#include "steintime.h"

int compare_values(const void *a, const void *b, const size_t offset_sort_attribute, size_t sort_attribute_size, const bool reversed);
void *set_next_pointer(void *node, void *target_ptr, const size_t next_offset, size_t next_size);

void terminate_program() {
        stop_logger();
        exit(1);
}

void shutdown_program() {
        stop_logger();
        exit(0);
}

void *sort_linked_list(void *head_ptr, const size_t offset_sort_attribute, const size_t offset_next, const size_t sort_attribute_size,const size_t next_size, const bool reversed, void *(*get_next)(const void *)) {
        if (head_ptr == NULL || get_next== NULL) {
                log_event(LOG_LEVEL_ERROR, "Tried to sort linked list but didnt specify pointer or function pointer");
                return NULL;
        }
        bool not_done = true;
        do {
                bool has_swapped = false;
                void **current = &head_ptr; //first node

                void *a = *current;
                void *b = get_next(a);
                void *prev = NULL;
                while (a && b) {
                        if (compare_values(a, b, offset_sort_attribute, sort_attribute_size, reversed)) {
                                if (a == head_ptr) {
                                        head_ptr = b;
                                }

                                set_next_pointer(a, get_next(b), offset_next, next_size);
                                set_next_pointer(b, a, offset_next, next_size);

                                if (prev) {
                                        set_next_pointer(prev, b, offset_next, next_size);
                                }

                                prev = b;
                                b = get_next(a);
                                has_swapped = true;
                        } else {
                                prev = a;
                                a = b;
                                b = get_next(b);
                                //*(void **)current = b;
//
                                //b = get_next(prev); // b wird zu a, b braucht pointer zu a
                                //a = (void **)get_next(b); // a wird zu b, a braucht pointer zu c
                                //void *c;
                                //if (b != NULL) {
                                //        c = get_next(b);
                                //}
//
                                //// prev braucht pointer zu b
                                //void *ptr_to_a = &a;
                                //void *ptr_to_b = &b;
//
//
                                //if (c) {
                                //        void *ptr_to_c = &c;
                                //        set_next_pointer(a, c, offset_next, next_size);
                                //}
//
                                //set_next_pointer(b, a, offset_next, next_size);
                                //set_next_pointer(prev, b, offset_next, next_size);
                                //prev = b;
                                //has_swapped = true;
                        }

                }

                if (!has_swapped) {
                        not_done = false;
                }
                //current = (void**)get_next(*current);
        } while (not_done);

        return head_ptr;
}

int compare_values(const void *a, const void *b, const size_t offset_sort_attribute, size_t sort_attribute_size, const bool reversed) {
        const void *value_a = (const char *)a + offset_sort_attribute;
        const void *value_b = (const char *)b + offset_sort_attribute;

        int result;
        switch (sort_attribute_size) {
                case sizeof(int): {
                        result = *(int*)value_a - *(int *)value_b;
                        break;
                }
                case sizeof(size_t): {
                        result = *(size_t*)value_a - *(size_t*)value_b;
                        break;
                }default: {
                        log_event(LOG_LEVEL_WARNING, "Comparing Values of unsupported type");
                        return 0;
                }
        }

        if (reversed && result < 0) {
                return 1;
        }
        if (reversed) {
                return 0;
        }

        if (result < 0) {
                return 0;
        }

        return 1;

        log_event(LOG_LEVEL_FATAL, "sth went wrong comparing values");
        return 1;

}

void *set_next_pointer(void *node, void *target_ptr, const size_t next_offset, size_t next_size) {
        void **next_ptr = (void **)((char *)node + next_offset);
        *next_ptr = target_ptr;

        return node;
}