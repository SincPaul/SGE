//
// Created by Geisthardt on 03.03.2025.
//

#include "input.h"

#include <stdbool.h>
#include <stdio.h>

#include "logging.h"
#include "memory_control.h"

int *key_states = NULL;
int *last_key_states = NULL;
bool is_tracking_enabled = false;
struct mouse_pos last_mouse_pos = {0, 0};

void enable_input_tracking() {
        is_tracking_enabled = true;
        key_states = allocate_memory(256 * sizeof(int), MEMORY_TAG_INPUT);
        last_key_states = allocate_memory(256 * sizeof(int), MEMORY_TAG_INPUT);
        if (key_states == NULL || last_key_states == NULL) {
                log_event(LOG_LEVEL_FATAL, "Failed to init Key states");
        }
}


struct mouse_pos get_mouse_position() {
        return last_mouse_pos;
}

int is_key_down(const keys key) {
        if (!is_tracking_enabled) {
                return 0;
        }
        //printf("CHECKING KEY: %d", key);
        return key_states[key];
}

int is_key_pressed(const keys key) {
        if (!is_tracking_enabled) {
                return 0;
        }
        if (last_key_states[key]) {
                //printf("LAST KEY STATE: %d", last_key_states[key]);
                //printf("ALREADY PRESSED\n");
                return 0;
        }
        if (key_states[key]) {
                return 1;
        }
        return 0;
}

int is_shift_active() {
        if (is_key_down(KEY_LSHIFT) || is_key_down(KEY_RSHIFT) && !is_key_down(KEY_CAPSLOCK)) {
                return 1;
        }

        if (is_key_down(KEY_CAPSLOCK) && !is_key_down(KEY_LSHIFT) && !is_key_down(KEY_RSHIFT)) {
                return 1;
        }
        return 0;
}

void update_key_states() {
        //printf("Updating last key states\n");
        copy_memory(last_key_states, key_states, 256*sizeof(int), 0, 0);
}