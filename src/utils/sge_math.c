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

vec3 sge_vec3_scale(vec3 vector1, float scale_factor) {
        vec3 scaled = {
                vector1.x * scale_factor,
                vector1.y * scale_factor,
                vector1.z * scale_factor,
        };


        return scaled;
}

vec3 sge_vec3_rotate_yaw_pitch(vec3 vector, vec3 rotation) {
        vec3 rotated = sge_vec3_rotate_yaw(vector, rotation);
        rotated = sge_vec3_rotate_pitch(rotated, rotation);
        return rotated;
}

vec3 sge_vec3_rotate_pitch(vec3 vector, vec3 rotation) {
        float radiant = -rotation.x * M_PI / 180.0f;
        float c = cos(radiant);
        float s = sin(radiant);

        vec3 rotated = {
                vector.x,
                vector.y * c - vector.z * s,
                vector.y * s + vector.z * c
        };

        return rotated;
}

vec3 sge_vec3_rotate_yaw(vec3 vector, vec3 rotation) {
        float radiant = -rotation.y * M_PI / 180.0f;
        float c = cos(radiant);
        float s = sin(radiant);

        vec3 rotated = {
                vector.x * c + vector.z * s,
                vector.y,
                -vector.x * s + vector.z * c
        };

        return rotated;
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


//taken from https://github.com/gcc-mirror/gcc/blob/master/libiberty/crc32.c
//todo calc self maybe if not to lazy
unsigned int crc32_table[] = {
        0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
        0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
        0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
        0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
        0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
        0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
        0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
        0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
        0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
        0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
        0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
        0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
        0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
        0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
        0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
        0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
        0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
        0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
        0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
        0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
        0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
        0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
        0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
        0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
        0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
        0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
        0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
        0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
        0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
        0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
        0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
        0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
        0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
        0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
        0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
        0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
        0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
        0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
        0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
        0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
        0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
        0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
        0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
        0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
        0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
        0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
        0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
        0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
        0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
        0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
        0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
        0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
        0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
        0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
        0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
        0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
        0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
        0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
        0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
        0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
        0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
        0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
        0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
        0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};
