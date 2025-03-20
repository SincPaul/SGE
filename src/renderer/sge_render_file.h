//
// Created by Geisthardt on 17.03.2025.
//

#ifndef SGE_RENDER_FILE_H
#define SGE_RENDER_FILE_H


#define SGE_REND_MAGIC_NUMBER "SGEREND" //has 0 terminator automatically so 8 bytes
#define SGE_REND_HEADER_FIXED_SIZE 86
#define SGE_REND_SECTION_HEADER_FIXED_SIZE 88
#define SGE_REND_EXTENSION_FIXED_SIZE 6
#include <stdint.h>

typedef enum {
        //Version 0.1
        SGE_SECTION_MESH = 1,
        SGE_SECTION_MATERIAL = 2,
        SGE_SECTION_TEXTURE = 3,
        SGE_SECTION_SHADER_BINDING = 4,
        SGE_SECTION_INDEX_BUFFER = 5,
        SGE_SECTION_METADATA = 6,
} SGE_SELECTION_TYPE;

typedef enum {
        SGE_FORMAT_FLOAT32 = 1,
        SGE_FORMAT_FLOAT16 = 2,
        SGE_FORMAT_INT8 = 3,
        SGE_FORMAT_UINT8 = 4,
        SGE_FORMAT_INT16 = 5,
        SGE_FORMAT_UINT16 = 6,
        SGE_FORMAT_INT32 = 7,
        SGE_FORMAT_UINT32 = 8,
} SGE_FORMAT_TYPE;

typedef enum {
        SGE_ATTRIBUTE_POSITION = 1,
        SGE_ATTRIBUTE_NORMAL = 2,
        SGE_ATTRIBUTE_COLOR = 3,
        SGE_ATTRIBUTE_TEXCOORDS = 4,
        SGE_ATTRIBUTE_TANGENT = 5,
        SGE_ATTRIBUTE_BITANGENT = 6,
        SGE_ATTRIBUTE_WEIGHTS = 7,
        SGE_ATTRIBUTE_JOINTIDS = 8,
} SGE_ATTRIBUTE_TYPE;

typedef enum {
        SGE_PRIMITIVE_POINTS = 1,
        SGE_PRIMITIVE_LINES = 2,
        SGE_PRIMITIVE_TRIANGLE = 3,
        SGE_PRIMITIVE_TRIANGLE_STRIP = 4,
} SGE_PRIMITIVE_TYPE;

typedef enum {
        SGE_MATERIAL_PARAM_FLOAT = 1,
        SGE_MATERIAL_PARAM_VEC2 = 2,
        SGE_MATERIAL_PARAM_VEC3 = 3,
        SGE_MATERIAL_PARAM_VEC4 = 4,
        SGE_MATERIAL_PARAM_TEXTURE = 5,
} SGE_MATERIAL_PARAM_TYPE;

typedef struct SGE_REND_EXTENSION {
        uint16_t                type;
        uint32_t                data_size;
        uint8_t*                data;
} SGE_REND_EXTENSION;

typedef struct SGE_REND_HEADER {
        uint16_t                major_version;
        uint16_t                minor_version;
        uint16_t                patch_version;
        uint16_t                section_count;
        uint16_t                extension_count;
        SGE_REND_EXTENSION      *extensions;
        char                    name[64];
        uint32_t                checksum;
} SGE_REND_HEADER;

typedef struct SGE_REND_SECTION_HEADER {
        uint16_t                type;
        uint64_t                offset;
        uint64_t                data_size;
        uint16_t                extension_count;
        size_t                  extension_size;
        SGE_REND_EXTENSION      *extensions;
        char                    name[64];
        uint32_t                checksum;
} SGE_REND_SECTION_HEADER;

typedef struct SGE_MESH_ATTRIBUTE {
        uint16_t                type;
        uint16_t                format;
        uint16_t                components;
        uint16_t                offset;
} SGE_MESH_ATTRIBUTE;

typedef struct SGE_MESH_DATA {
        uint32_t                vertex_count;
        uint32_t                vertex_size;
        uint32_t                attribute_count;
        SGE_MESH_ATTRIBUTE      *attributes;
        uint8_t                 *vertex_data;
} SGE_MESH_DATA;

typedef struct SGE_INDEX_BUFFER_DATA {
        uint32_t                index_count;
        uint32_t                primitive_type;
        uint16_t                index_size;
        uint8_t                 *index_data;
} SGE_INDEX_BUFFER_DATA;

typedef struct SGE_MATERIAL_PARAMETER {
        uint16_t                type;
        char                    name[32];
        union {
                float           float_value;
                float           vec2_value[2];
                float           vec3_value[3];
                float           vec4_value[4];
                uint32_t        texture_index;
        } value;
} SGE_MATERIAL_PARAMETER;

typedef struct SGE_MATERIAL_DATA {
        uint32_t                parameter_count;
        SGE_MATERIAL_PARAMETER  *parameters;
        uint32_t                shader_binding_index;
} SGE_MATERIAL_DATA;

typedef struct SGE_SHADER_PARAMETER_BINDING {
        char                    name[32];
        uint16_t                type;
} SGE_SHADER_PARAMETER_BINDING;

typedef struct SGE_SHADER_BINDING_DATA {
        char                            shader_identifier[64];
        uint32_t                        parameter_count;
        SGE_SHADER_PARAMETER_BINDING    *parameter_bindings;
} SGE_SHADER_BINDING_DATA;

typedef struct SGE_REND_SECTION {
        SGE_REND_SECTION_HEADER section_header;
        void                    *data;
} SGE_REND_SECTION;

typedef struct SGE_REND_FILE {
        SGE_REND_HEADER         header;
        SGE_REND_SECTION        *sections;
} SGE_REND_FILE;

#include "sge_render.h"

SGE_RESULT sge_rend_save(char *filename, SGE_REND_SECTION *sections, uint16_t section_count);
SGE_RESULT sge_rend_load(char *filename, SGE_REND_FILE **outFile);

SGE_MESH_DATA *sge_create_mesh_data();
SGE_INDEX_BUFFER_DATA *sge_create_index_buffer_data();
SGE_MATERIAL_DATA *sge_create_material_data();
SGE_SHADER_BINDING_DATA *sge_create_shader_binding();

SGE_MESH_DATA *sge_parse_mesh_data(void *raw_data, size_t data_size);
SGE_INDEX_BUFFER_DATA *sge_parse_index_buffer_data();
SGE_MATERIAL_DATA *sge_parse_material_data();
SGE_SHADER_BINDING_DATA *sge_parse_shader_binding();


SGE_REND_SECTION *sge_create_mesh_section(
                char *name,
                void *vertex_data,
                uint32_t vertex_count,
                uint32_t vertex_size,
                SGE_MESH_ATTRIBUTE *attributes,
                uint32_t attribute_count,
                SGE_REND_EXTENSION *extensions,
                uint32_t extension_count
        );
SGE_REND_SECTION sge_create_index_section();
SGE_REND_SECTION sge_create_material_section();
SGE_REND_SECTION sge_create_shader_binding_section();

#endif //SGE_RENDER_FILE_H
