//
// Created by Geisthardt on 27.02.2025.
//

#include "steinstring.h"
#include <string.h>

int get_longest_element_in_array(char *array[]) {
        int longest_element_length = 0;
        int i = 0;
        while (array[i] != NULL) {
                const char *element = array[i];
                const int length = strlen(element);
                if (length > longest_element_length) {
                        longest_element_length = length;
                }
                i++;
        }
        return longest_element_length;
}