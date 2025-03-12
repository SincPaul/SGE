//
// Created by Geisthardt on 07.03.2025.
//

#include "sge_math.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../core/logging.h"
#include "../core/memory_control.h"

int amount_chars_in_float(float num);

vec3 sge_vec3_add(const vec3 a, const vec3 b) {
        const vec3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
        return result;
}

vec3 sge_vec3_sub(const vec3 a, const vec3 b) {
        const vec3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
        return result;
}

vec3 sge_vec3_normalize(vec3 vector) {
        float magnitude = sqrt(squared(vector.x) + squared(vector.y) + squared(vector.z));

        if (magnitude == 0) {
                return (vec3){0.0, 0.0, 0.0};
        }

        return (vec3){
                vector.x / magnitude,
                vector.y / magnitude,
                vector.z / magnitude
        };
}

vec3 sge_vec3_cross(vec3 vector1, vec3 vector2) {
        return (vec3){
                vector1.y * vector2.z - vector1.z * vector2.y,
                vector1.z * vector2.x - vector1.x * vector2.z,
                vector1.x * vector2.y - vector1.y * vector2.x
            };
}













void sge_m4_get_view_matrix(m4 matrix, vec3 left_vec3, vec3, vec3 up_vec3, vec3 vec3_forward_vec3, vec3 translation) {

}

void sge_m4_set_identity(m4 matrix) {
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        if (j == i) {
                                matrix[i][j] = 1;
                        } else {
                                matrix[i][j] = 0;
                        }
                }
        }
}

void sge_m4_set_value(m4 matrix, int row, int col, float value) {
        if (row > 3 || col > 3 || row < 0 || col < 0) {
                log_event(LOG_LEVEL_ERROR, "invalid dimensions in m4");
                return;
        }
        matrix[row][col] = value;
}

float sge_m4_get_value(m4 matrix, int row, int col) {
        if (row > 3 || col > 3 || row < 0 || col < 0) {
                log_event(LOG_LEVEL_ERROR, "invalid dimensions in m4");
                return 0;
        }
        float value = matrix[row][col];
        return value;
}

void sge_m4_set_translate(m4 matrix, vec3 translate) {
        matrix[0][3] = translate.x;
        matrix[1][3] = translate.y;
        matrix[2][3] = translate.z;
}

void sge_m4_set_scale(m4 matrix, vec3 scale) {
        matrix[0][0] = scale.x;
        matrix[1][1] = scale.y;
        matrix[2][2] = scale.z;
}

void sge_m4_set_rotate_x(m4 matrix, float angle) {
        float c = cos(angle * M_PI / 180.f);
        float s = sin(angle * M_PI / 180.0f);
        matrix[1][1] = c;
        matrix[1][2] = -s;
        matrix[2][1] = s;
        matrix[2][2] = c;
}
void sge_m4_set_rotate_y(m4 matrix, float angle) {
        float c = cos(angle * M_PI / 180.f);
        float s = sin(angle * M_PI / 180.0f);
        matrix[0][0] = c;
        matrix[0][2] = s;
        matrix[2][0] = -s;
        matrix[2][2] = c;
}

void sge_m4_set_rotate_z(m4 matrix, float angle) {
        float c = cos(angle * M_PI / 180.f);
        float s = sin(angle * M_PI / 180.0f);
        matrix[0][0] = c;
        matrix[1][0] = s;
        matrix[0][1] = -s;
        matrix[1][1] = c;
}
void sge_m4_multiply(m4 m_result, m4 m_a, m4 m_b) {
        m4 temp;
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        temp[i][j] = 0;
                        for (int l = 0; l < 4; l++) {
                                temp[i][j] += m_a[i][l] * m_b[l][j];
                        }
                }
        }
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        m_result[i][j] = temp[i][j];
                }
        }
}

void sge_m4_transpose(m4 matrix) {
        for (int i = 0; i < 4; i++) {
                for (int j = i + 1; j < 4; j++) {
                        const float temp = matrix[i][j];
                        matrix[i][j] = matrix[j][i];
                        matrix[j][i] = temp;
                }
        }
}

void sge_m4_set_rotate(m4 matrix, vec3 rotation) {
        m4 rx, ry, rz, temp;

        sge_m4_set_identity(rx);
        sge_m4_set_identity(ry);
        sge_m4_set_identity(rz);
        sge_m4_set_identity(temp);

        sge_m4_set_rotate_x(rx, rotation.x);
        sge_m4_set_rotate_y(ry, rotation.y);
        sge_m4_set_rotate_z(rz, rotation.z);

        sge_m4_multiply(temp, rx, ry);
        sge_m4_multiply(matrix, temp, rz);
}

void sge_m4_print(m4 matrix) {
        int longest_num = 0;
        char matrix_string[1024];
        zero_memory(matrix_string, sizeof(matrix_string), 0);
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        int number_length = amount_chars_in_float(matrix[i][j]);
                        if (number_length > longest_num) {
                                longest_num = number_length;
                        }
                }
        }

        int box_width = (longest_num * 4) + (4 * 4);

        for (int i = 0; i < box_width; ++i) {
                snprintf(matrix_string + strlen(matrix_string), sizeof(matrix_string), "-");
        }
        snprintf(matrix_string + strlen(matrix_string), sizeof(matrix_string), "\n");
        for (int i = 0; i < 4; ++i) {
                snprintf(matrix_string + strlen(matrix_string), sizeof(matrix_string), "| %*.4f | %*.4f | %*.4f | %*.4f |\n",
                        longest_num, matrix[i][0],
                        longest_num, matrix[i][1],
                        longest_num, matrix[i][2],
                        longest_num, matrix[i][3]);
                for (int j = 0; j < box_width; ++j) {
                        snprintf(matrix_string + strlen(matrix_string), sizeof(matrix_string), "-");
                }
                snprintf(matrix_string + strlen(matrix_string), sizeof(matrix_string), "\n");
        }
        snprintf(matrix_string + strlen(matrix_string), sizeof(matrix_string), "\n");
                        //printf("%f ", matrix[i][j]);

        printf(matrix_string);
}

vec4 sge_m4_transform_vec4(m4 matrix, vec4 vec) {
        vec4 result;
        result.x = matrix[0][0] * vec.x + matrix[0][1] * vec.y + matrix[0][2] * vec.z + matrix[0][3] * vec.a;
        result.y = matrix[1][0] * vec.x + matrix[1][1] * vec.y + matrix[1][2] * vec.z + matrix[1][3] * vec.a;
        result.z = matrix[2][0] * vec.x + matrix[2][1] * vec.y + matrix[2][2] * vec.z + matrix[2][3] * vec.a;
        result.a = matrix[3][0] * vec.x + matrix[3][1] * vec.y + matrix[3][2] * vec.z + matrix[3][3] * vec.a;
        return result;
}

int amount_chars_in_float(float num) {
       //todo
        //printf("%f\n", num);
        if (num < 0 || num == -0) {
                return 6;
        }
        return 6;
}


size_t squared(size_t num) {
        return num * num;
}