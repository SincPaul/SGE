//
// Created by Geisthardt on 07.03.2025.
//

#ifndef SGE_MATH_H
#define SGE_MATH_H
#include <stddef.h>

typedef struct vec3 {
        float x;
        float y;
        float z;
} vec3;

typedef struct vec4 {
        float x;
        float y;
        float z;
        float a;
} vec4;

typedef float m4[4][4];

vec3 sge_vec3_add(vec3 a, vec3 b);
vec3 sge_vec3_sub(vec3 a, vec3 b);
vec3 sge_vec3_normalize(vec3 vector);
vec3 sge_vec3_cross(vec3 vector1, vec3 vector2);
vec3 sge_vec3_scale(vec3 vector1, float scale_factor);
vec3 sge_vec3_rotate_yaw_pitch(vec3 vector1, vec3 rotation);
vec3 sge_vec3_rotate_pitch(vec3 vector, vec3 rotation);
vec3 sge_vec3_rotate_yaw(vec3 vector1, vec3 rotation);


void sge_m4_get_view_matrix(m4 matrix, vec3 left_vec3, vec3, vec3 up_vec3, vec3 vec3_forward_vec3, vec3 translation);
void sge_m4_set_identity(m4 matrix);
void sge_m4_set_value(m4 matrix, int row, int col, float value);
float sge_m4_get_value(m4 matrix, int row, int col);
void sge_m4_set_translate(m4 matrix, vec3 translate);
void sge_m4_set_scale(m4 matrix, vec3 scale);
void sge_m4_set_rotate_x(m4 matrix, float angle);
void sge_m4_set_rotate_y(m4 matrix, float angle);
void sge_m4_set_rotate_z(m4 matrix, float angle);
void sge_m4_multiply(m4 m_result, m4 m_a, m4 m_b);
void sge_m4_transpose(m4 matrix);
void sge_m4_set_rotate(m4 matrix, vec3 rotation);
void sge_m4_print(m4 matrix);
vec4 sge_m4_transform_vec4(m4 matrix, vec4 vec);


size_t squared(size_t num);

#endif //SGE_MATH_H
