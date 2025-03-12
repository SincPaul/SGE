//
// Created by Geisthardt on 07.03.2025.
//

#ifndef SGE_MATH_H
#define SGE_MATH_H

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

void sge_m4_set_identity(m4 matrix);
void sge_m4_set_value(m4 matrix, int row, int col, float value);
float sge_m4_get_value(m4 matrix, int row, int col);
void sge_m4_set_translate(m4 matrix, vec3 translate);
void sge_m4_set_scale(m4 matrix, vec3 scale);
void sge_m4_set_rotate_x(m4 matrix, float angle);
void sge_m4_set_rotate_y(m4 matrix, float angle);
void sge_m4_set_rotate_z(m4 matrix, float angle);
void sge_m4_multiply(m4 m_result, m4 m_a, m4 m_b);
vec4 sge_m4_transform_vec4(m4 matrix, vec4 vec);


#endif //SGE_MATH_H
