//
// Created by Geisthardt on 16.04.2025.
//

#ifndef SGE_TEXTURE_FILE_H
#define SGE_TEXTURE_FILE_H

#define SGE_TEXTURE_MAGIC_NUMBER "SGETEXT" //has 0 terminator automatically so 8 bytes

#define SGE_TEXTURE_HEADER_FIXED_SIZE 52
#define SGE_TEXTURE_SECTION_HEADER_FIXED_SIZE 43
#define SGE_TEXTURE_SECTION_HEADER_COLOR_EXTERNAL_FIXED_SIZE 5
#define SGE_TEXTURE_SECTION_HEADER_COLOR_EMBEDDED_FIXED_SIZE 19

#define SGE_TEXTURE_FLAG_FORCE_WRITE 0x01

#include "sge_types.h"

typedef enum SGE_TEXTURE_SECTION_TYPE {
        SGE_TEXTURE_SECTION_TYPE_INVALID = 0,
        SGE_TEXTURE_SECTION_TYPE_COLOR = 1
} SGE_TEXTURE_TYPE;

typedef enum SGE_TEXTURE_BODY_INCLUDE_TYPE {
        SGE_TEXTURE_BODY_INCLUDE_TYPE_EXTERNAL = 0,
        SGE_TEXTURE_BODY_INCLUDE_TYPE_EMBEDDED = 1
} SGE_TEXTURE_BODY_INCLUDE_TYPE;

typedef struct sge_texture_extension {
        uint16_t                type;
        uint32_t                data_size;
        uint8_t*                data;
} sge_texture_extension;

typedef struct sge_texture_section_header {
        uint8_t                 section_type;
        uint64_t                section_offset;
        uint64_t                data_size;
        uint32_t                section_name_size;
        char                    *section_name;
        uint64_t                creation_date_timestamp;
        uint64_t                last_modified_timestamp;
        size_t                  section_extension_size;
        uint16_t                section_extension_count;
        sge_texture_extension   *extensions;
        uint32_t                section_checksum;
} sge_texture_section_header;

typedef struct sge_texture_section_color_external {
        uint8_t         include_type;
        uint32_t        filepath_length;
        char            *filepath;
} sge_texture_section_color_external;

typedef struct sge_texture_section_color_embedded {
        uint8_t         include_type;
        uint8_t         color_type;
        uint8_t         bit_depth;
        uint32_t        width;
        uint32_t        height;
        uint64_t        data_size;
        void            *pixel_data;
} sge_texture_section_color_embedded;

typedef union sge_parsed_section_data {
        union {
                sge_texture_section_color_external       *color_external;
                sge_texture_section_color_embedded       *color_embedded;
        };
} sge_parsed_section_data;

typedef struct sge_texture_section {
        sge_texture_section_header      header;
        void                            *raw_data;
        sge_parsed_section_data         parsed_data;
} sge_texture_section;



typedef struct sge_texture_header {
        uint16_t                major_version;
        uint16_t                minor_version;
        uint16_t                patch_version;
        uint32_t                section_count;
        uint32_t                texture_name_size;
        char                    *texture_name;
        uint32_t                author_name_size;
        char                    *author_name;
        uint64_t                creation_date_timestamp;
        uint64_t                last_modified_date_timestamp;
        uint32_t                description_size;
        char                    *description;
        uint16_t                header_extension_count;
        sge_texture_extension   *header_extensions;
        size_t                  header_extension_size;
        uint32_t                crc32_checksum;
} sge_texture_header;


typedef struct sge_texture {
        sge_texture_header      *header;
        sge_texture_section     *sections;
} sge_texture;


sge_texture *sge_texture_create(char *texture_name, char *author_name, char *description);


sge_texture_section *sge_texture_section_create_color(SGE_TEXTURE_BODY_INCLUDE_TYPE include_type, char *filepath, char *section_name);

SGE_RESULT sge_texture_add_section(sge_texture *texture, sge_texture_section *section);


SGE_RESULT sge_texture_save(char *filename, sge_texture *texture, int flags);
sge_texture *sge_texture_load(char *filepath);


SGE_RESULT sge_texture_parse_color_section(sge_parsed_section_data *parsed_output, void *raw_data, size_t data_size);

#endif //SGE_TEXTURE_FILE_H
