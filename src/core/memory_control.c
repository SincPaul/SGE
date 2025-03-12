//
// Created by Geisthardt on 28.02.2025.
//

#include "memory_control.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "logging.h"
#include <string.h>

#include "../utils/steinutils.h"

extern bool started_logging;

typedef struct allocation_info {
        void *ptr;
        size_t size;
        memory_tag tag;
        struct allocation_info *next;
} allocation_info;

typedef struct memory_tag_usage {
        memory_tag tag;
        size_t usage;
        struct memory_tag_usage *next;
} memory_tag_usage;


static allocation_info *allocation_list = NULL;
static memory_tag_usage *memory_tag_usage_tracker = NULL;
static memory_tag_usage *copy_memory_tag_usage_tracker;
static int longest_tag_name = 0;

static size_t tracker_memory_usage = 0;
static size_t total_memory_usage = 0;

const char *memory_tag_to_string(memory_tag tag);
int amount_chars_in_int(int number);
void *get_next_allocation_memory_tag_usage(const void *node);

void *allocate_memory(const size_t size, const memory_tag tag) {
        void *ptr = malloc(size);
        zero_memory(ptr, size, 0);
        if (!ptr) {
                log_event(LOG_LEVEL_ERROR, "Failed allocating Memory");
                return NULL;
        }

        allocation_info *info = malloc(sizeof(allocation_info));
        if (!info) {
                free(ptr);
                log_event(LOG_LEVEL_ERROR, "Failed to track Memory Allocation");
                return NULL;
        }

        info->ptr = ptr;
        info->size = size;
        info->tag = tag;
        info->next = allocation_list;
        allocation_list = info;

        char memory_message[256];
        snprintf(memory_message, sizeof(memory_message), "Allocated %d Bytes of Memory for Tag %s", size, memory_tag_to_string(tag));
        log_event(LOG_LEVEL_INFO, memory_message);


        memory_tag_usage *tag_usage = memory_tag_usage_tracker;
        bool tag_found = false;

        if (!tag_usage) {
                log_event(LOG_LEVEL_ERROR, "Failed to track Tag Usage");
                free(tag_usage);
        } else {
                while (tag_usage != NULL) {
                        if (tag_usage->tag == tag) {
                                tag_usage->usage += size;
                                tag_found = true;
                                break;
                        }
                        tag_usage = tag_usage->next;
                }
        }

        if (!tag_found) {
                memory_tag_usage *new_memory_tag_usage = malloc(sizeof(memory_tag_usage));
                if (new_memory_tag_usage) {
                        new_memory_tag_usage->tag = tag;
                        new_memory_tag_usage->usage = size;
                        new_memory_tag_usage->next = memory_tag_usage_tracker;
                        memory_tag_usage_tracker = new_memory_tag_usage;
                        const int tag_length = strlen(memory_tag_to_string(tag));
                        //printf("TAG LENGTH: %d\n", tag_length);
                        printf("MEMORY TAG STRING: %s\n", memory_tag_to_string(tag));
                        if (tag_length > longest_tag_name) {
                                longest_tag_name = tag_length;
                        }
                }
        }


        tracker_memory_usage += sizeof(info);
        tracker_memory_usage += sizeof(memory_tag_usage);
        total_memory_usage += sizeof(info);
        total_memory_usage += sizeof(memory_tag_usage);
        total_memory_usage += size;




        char tracker_memory_message[256];
        snprintf(tracker_memory_message, sizeof(tracker_memory_message), "Memory Tracker now uses %d Bytes of Memory", tracker_memory_usage);
        log_event(LOG_LEVEL_INFO, tracker_memory_message);

        char total_memory_usage_message[256];
        snprintf(total_memory_usage_message, sizeof(total_memory_usage_message), "Total Memory Used: %d", total_memory_usage);
        log_event(LOG_LEVEL_INFO, total_memory_usage_message);

        return ptr;
}

const char *memory_tag_to_string(const memory_tag tag) {
        switch (tag) {
                case MEMORY_TAG_UNKNOWN: return "UNKNOWN";
                case MEMORY_TAG_LOGGER: return "LOGGER";
                case MEMORY_TAG_INPUT: return "INPUT";
                case MEMORY_TAG_VULKAN: return "VULKAN";
                case MEMORY_TAG_TEST_1: return "Test 1";
                case MEMORY_TAG_TEST_2: return "TEst 2";
                case MEMORY_TAG_TEST_3: return "TITATEST 3";
                default: return "INVALID_TAG";
        }
}

void free_memory(void *ptr,const memory_tag tag) {
        if (!ptr) {
                log_event(LOG_LEVEL_ERROR, "Tried to free memory without passing pointer");
                return;
        }

        allocation_info **current = &allocation_list;
        allocation_info *prev = NULL;

        while (*current) {
                if ((*current)->ptr == ptr) {

                        allocation_info *to_free = *current;
                        *current = to_free->next;

                        if (prev) {
                                prev->next = to_free->next;
                        }


                        memory_tag_usage *current_tag = memory_tag_usage_tracker;
                        memory_tag_usage *prev_tag = NULL;
                        while (current_tag) {
                                if (current_tag->tag == to_free->tag) {
                                        if (current_tag->usage < to_free->size) {
                                                char memory_underflow_msg[256];
                                                snprintf(memory_underflow_msg, sizeof(memory_underflow_msg), "Memory usage underflow detected for tag %s", memory_tag_to_string(current_tag->tag));
                                                log_event(LOG_LEVEL_ERROR, memory_underflow_msg);
                                                current_tag->usage = 0;
                                        } else {
                                                current_tag->usage -= to_free->size;
                                        }

                                        if (current_tag->usage == 0) {
                                                if (prev == NULL) {
                                                       memory_tag_usage_tracker = current_tag->next;
                                                } else {
                                                        prev_tag->next = current_tag->next;
                                                }

                                                free(current_tag);
                                                break;
                                        }

                                        break;
                                }
                                prev_tag = current_tag;
                                current_tag = current_tag->next;
                        }

                        tracker_memory_usage -= sizeof(current);
                        tracker_memory_usage -= sizeof(memory_tag_usage);
                        total_memory_usage -= to_free->size;
                        total_memory_usage -= sizeof(current);
                        total_memory_usage -= sizeof(memory_tag_usage);

                        char freed_memory_msg[256];
                        snprintf(freed_memory_msg, sizeof(freed_memory_msg), "Freed %d Bytes of Memory for Tag %s",
                                to_free->size, memory_tag_to_string(tag));
                        log_event(LOG_LEVEL_INFO, freed_memory_msg);


                        char tracker_memory_message[256];
                        snprintf(tracker_memory_message, sizeof(tracker_memory_message), "Memory Tracker now uses %d Bytes of Memory",
                                tracker_memory_usage);
                        log_event(LOG_LEVEL_INFO, tracker_memory_message);

                        char total_memory_usage_message[256];
                        snprintf(total_memory_usage_message, sizeof(total_memory_usage_message), "Total Memory Used: %d",
                                total_memory_usage);
                        log_event(LOG_LEVEL_INFO, total_memory_usage_message);

                        free(to_free);
                        free(ptr);
                        return;
                }
                prev = *current;
                current = &((*current)->next);
        }

        log_event(LOG_LEVEL_ERROR, "Tried to free untracked Memory");
 }

void *reallocate_memory(void *old_ptr, size_t new_size, memory_tag new_tag) {
        if (!old_ptr) {
                return allocate_memory(new_size, new_tag);
        }

        allocation_info *info = allocation_list;
        while (info != NULL) {
                if (info->ptr == old_ptr) {
                        break;
                }
                info = info->next;
        }

        if (!info) {
                log_event(LOG_LEVEL_ERROR, "Tried to reallocate untracked Memory");
                return NULL;
        }

        if (new_size == 0) {
                free_memory(old_ptr, info->tag);
                return NULL;
        }

        size_t old_size = info->size;

        void *new_ptr = allocate_memory(new_size, new_tag);
        if (!new_ptr) {
                log_event(LOG_LEVEL_ERROR, "Failed to allocate new memory block during reallocation");
                return NULL;
        }

        size_t copy_size = (old_size < new_size) ? old_size : new_size;
        memcpy(new_ptr, old_ptr, copy_size);

        free_memory(old_ptr, info->tag);

        return new_ptr;
}


void *zero_memory(void *ptr, const size_t size, const int offset) {
        if (ptr == NULL) {
                log_event(LOG_LEVEL_ERROR, "Tried to zero memory but didnt pass pointer");
                return NULL;
        }
        // todo check size of memory so that offset doesnt break stuff
        memset((char*)ptr + offset, 0, size);
        return ptr;
}

void *copy_memory(void *dest_ptr, void *src_ptr, const size_t size, const size_t offset_dest, const size_t offset_src) {
        if (dest_ptr == NULL || src_ptr == NULL) {
                log_event(LOG_LEVEL_ERROR, "Tried to copy memory but at least 1 pointer is not set");
                return NULL;
        }

        // todo check size of memory so that offset doesnt break stuff
        memcpy((char*)dest_ptr + offset_dest, (char*)src_ptr + offset_src, size);

        return dest_ptr;
}

void *set_memory(void *ptr, int value, size_t size, int offset) {
        if (ptr == NULL) {
                log_event(LOG_LEVEL_ERROR, "Tried to set Memory but didnt pass pointer");
                return NULL;
        }

        // todo check size of memory so that offset doesnt break stuff
        memset((char*)ptr + offset, value, size);
        return ptr;
}

void print_memory_usage_str() {
        char memory_usage_str[4096];

        snprintf(memory_usage_str, sizeof(memory_usage_str),

        "--------------------------------------------------\n"
             "|              Current Memory Usage              |\n"
             "--------------------------------------------------\n"
             "| Total Usage | %d %-*s |\n"
             "--------------------------------------------------\n", total_memory_usage, 48-13-5-amount_chars_in_int(total_memory_usage), "Bytes");

        copy_memory_tag_usage_tracker = memory_tag_usage_tracker; // todo actually copy/duplicate linked list
        copy_memory_tag_usage_tracker =
                sort_linked_list(copy_memory_tag_usage_tracker,
                        offsetof(memory_tag_usage, usage),
                        offsetof(memory_tag_usage, next),
                        sizeof(size_t),
                        sizeof(memory_tag_usage),
                        true,
                get_next_allocation_memory_tag_usage);
        while (copy_memory_tag_usage_tracker != NULL) {
                //printf("NEW COPY %s", memory_tag_to_string(copy_memory_tag_usage_tracker->tag));
                        snprintf(memory_usage_str + strlen(memory_usage_str), sizeof(memory_usage_str),
        "| %-*s | %d %-*s |\n%s\n",
                longest_tag_name, memory_tag_to_string(copy_memory_tag_usage_tracker->tag),
                copy_memory_tag_usage_tracker->usage, 48-7-longest_tag_name-amount_chars_in_int(copy_memory_tag_usage_tracker->usage), "Bytes",
                "--------------------------------------------------");
                copy_memory_tag_usage_tracker= copy_memory_tag_usage_tracker->next;
        }
        printf("%s\n", memory_usage_str);
}

int amount_chars_in_int(const int number) {
        const int char_amount = log10(number);
        return char_amount;
 }

void *get_next_allocation_memory_tag_usage(const void *node) {
        return ((memory_tag_usage *)node)->next;
}

uint32_t find_memory_type(sge_render *render, uint32_t type_filter, VkMemoryPropertyFlags properties) {
        sge_vulkan_context *vk_context = render->api_context;
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(vk_context->physical_device, &memory_properties);

        for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
                if ((type_filter & (1 << i)) &&
                    (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                        return i;
                    }
        }

        log_event(LOG_LEVEL_ERROR, "Failed to find suitable memory type");
        return UINT32_MAX;
}