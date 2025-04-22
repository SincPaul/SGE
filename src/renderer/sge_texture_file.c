//
// Created by Geisthardt on 16.04.2025.
//

#include "renderer/sge_texture_file.h"

#include <sge_version.h>
#include <stdio.h>
#include <string.h>
#include <core/memory_control.h>
#include <core/sge_internal_logging.h>
#include <utils/sge_string.h>
#include <utils/sge_time.h>
#include <utils/sge_utils.h>
#include <utils/hash/sge_crc32.h>
#include <utils/image/sge_png.h>


sge_texture *sge_texture_create(char *texture_name, char *author_name, char *description) {
        char *function_name = "sge_texture_create";
        sge_texture *texture = allocate_memory(sizeof(sge_texture), MEMORY_TAG_VULKAN);
        if (!texture) {
                allocation_error();
                return NULL;
        }

        sge_texture_header *header = allocate_memory(sizeof(sge_texture_header), MEMORY_TAG_TEXTURE);
        if (!header) {
                allocation_error();
                return NULL;
        }

        if (texture_name) {
                header->texture_name_size = strlen(texture_name);
                header->texture_name = allocate_memory(header->texture_name_size, MEMORY_TAG_TEXTURE);
                if (!header->texture_name) {
                        allocation_error();
                        return NULL;
                }
                if (!copy_memory(header->texture_name, texture_name, header->texture_name_size, 0, 0)) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: failed to copy texture name", function_name);
                        return NULL;
                }
        }

        if (author_name) {
                header->author_name_size = strlen(author_name);
                header->author_name = allocate_memory(header->author_name_size, MEMORY_TAG_TEXTURE);
                if (!header->author_name) {
                        allocation_error();
                        return NULL;
                }
                if (!copy_memory(header->author_name, author_name, header->author_name_size, 0, 0)) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: failed to copy author name", function_name);
                        return NULL;
                }
        }


        if (description) {
                header->description_size = strlen(description);
                header->description = allocate_memory(header->description_size, MEMORY_TAG_TEXTURE);
                if (!header->description) {
                        allocation_error();
                        return NULL;
                }
                if (!copy_memory(header->description, description, header->description_size, 0, 0)) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: failed to copy description", function_name);
                        return NULL;
                }
        }


        texture->header = header;


        return texture;
}

sge_texture_section *sge_texture_section_create_color(SGE_TEXTURE_BODY_INCLUDE_TYPE include_type, char *filepath, char *section_name) {
        char *function_name = "sge_texture_section_create_color";
        if (!filepath) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: no filepath passed", function_name);
                return NULL;
        }
        sge_texture_section *section = allocate_memory(sizeof(sge_texture_section), MEMORY_TAG_TEXTURE);
        if (!section) {
                allocation_error();
                return NULL;
        }

        sge_texture_section_header *header = allocate_memory(sizeof(sge_texture_section_header), MEMORY_TAG_TEXTURE);
        if (!header) {
                allocation_error();
                return NULL;
        }

        sge_png_metadata image_data = {0};
        if (sge_png_parse_data(filepath, &image_data) != SGE_SUCCESS) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: failed to parse png", function_name);
                return NULL;
        }

        uint64_t current_timestamp = get_current_ms_time();
        *header = (sge_texture_section_header){
                .section_type = SGE_TEXTURE_SECTION_TYPE_COLOR,
                .creation_date_timestamp = current_timestamp,
                .last_modified_timestamp = current_timestamp,
                // todo extension
                .section_extension_count = 0,
                .extensions = NULL,
                .section_offset = 0,
                .data_size = 0,
                .section_checksum = 0,
        };

        if (section_name) {
                header->section_name_size = strlen(section_name);
                header->section_name = allocate_memory(header->section_name_size, MEMORY_TAG_TEXTURE);
                if (!header->section_name) {
                        allocation_error();
                        return NULL;
                }
                if (!copy_memory(header->section_name, section_name, header->section_name_size, 0, 0)) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: failed to copy section name", function_name);
                        return NULL;
                }
        }

        if (include_type == SGE_TEXTURE_BODY_INCLUDE_TYPE_EXTERNAL) {
                sge_texture_section_color_external *color_external = allocate_memory(sizeof(sge_texture_section_color_external), MEMORY_TAG_TEXTURE);
                if (!color_external) {
                        allocation_error();
                        return NULL;
                }

                color_external->include_type = SGE_TEXTURE_BODY_INCLUDE_TYPE_EXTERNAL;
                color_external->filepath_length = strlen(filepath);
                color_external->filepath = allocate_memory(color_external->filepath_length, MEMORY_TAG_TEXTURE);
                if (!color_external->filepath) {
                        allocation_error();
                        return NULL;
                }
                if (!copy_memory(color_external->filepath, filepath, color_external->filepath_length, 0, 0)) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: failed to copy filepath", function_name);
                        return NULL;
                }

                section->parsed_data.color_external = color_external;

                size_t data_size = SGE_TEXTURE_SECTION_HEADER_COLOR_EXTERNAL_FIXED_SIZE + color_external->filepath_length;
                section->raw_data = allocate_memory(data_size, MEMORY_TAG_TEXTURE);
                if (!section->raw_data) {
                        allocation_error();
                        return NULL;
                }

                size_t raw_data_offset = 0;
                if (!copy_memory(section->raw_data, &color_external->include_type, sizeof(uint8_t), raw_data_offset, 0)) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: failed to copy data", function_name);
                        return NULL;
                }
                raw_data_offset += sizeof(uint8_t);

                if (!copy_memory(section->raw_data, &color_external->filepath_length, sizeof(uint32_t), raw_data_offset, 0)) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: failed to copy data", function_name);
                        return NULL;
                }
                raw_data_offset += sizeof(uint32_t);

                if (!copy_memory(section->raw_data, color_external->filepath, color_external->filepath_length, raw_data_offset, 0)) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: failed to copy data", function_name);
                        return NULL;
                }
                raw_data_offset += color_external->filepath_length;

                header->data_size = data_size;
                printf("SEC DATA SIZE: %llu\n", data_size);

        } else if (include_type == SGE_TEXTURE_BODY_INCLUDE_TYPE_EMBEDDED) {
                sge_texture_section_color_embedded *color_embedded = allocate_memory(SGE_TEXTURE_SECTION_HEADER_COLOR_EMBEDDED_FIXED_SIZE, MEMORY_TAG_TEXTURE);
                if (!color_embedded) {
                        allocation_error();
                        return NULL;
                }

                color_embedded->include_type = include_type;
                color_embedded->color_type = image_data.color_type;
                color_embedded->height = image_data.height;
                color_embedded->width = image_data.width;
                color_embedded->bit_depth = image_data.bit_depth;

                color_embedded->data_size = image_data.idat_length;
                header->data_size = image_data.idat_length;
                color_embedded->pixel_data = allocate_memory(image_data.idat_length, MEMORY_TAG_TEXTURE);
                if (!color_embedded->pixel_data) {
                        allocation_error();
                        return NULL;
                }
                if (!copy_memory(color_embedded->pixel_data, image_data.idat_data, image_data.idat_length, 0, 0)) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: failed to copy image data", function_name);
                        return NULL;
                }


                section->raw_data = allocate_memory(sizeof(sge_texture_section_color_embedded) + color_embedded->data_size, MEMORY_TAG_TEXTURE);
                if (!section->raw_data) {
                        allocation_error();
                        return NULL;
                }
                size_t raw_data_offset = 0;
                if (!copy_memory(section->raw_data, &color_embedded->include_type, sizeof(uint8_t), raw_data_offset, 0)) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: failed to copy data", function_name);
                        return NULL;
                }
                raw_data_offset += sizeof(uint8_t);

                copy_memory(section->raw_data, &color_embedded->color_type, sizeof(uint8_t), raw_data_offset, 0);
                raw_data_offset += sizeof(uint8_t);

                copy_memory(section->raw_data, &color_embedded->bit_depth, sizeof(uint8_t), raw_data_offset, 0);
                raw_data_offset += sizeof(uint8_t);

                copy_memory(section->raw_data, &color_embedded->width, sizeof(uint32_t), raw_data_offset, 0);
                raw_data_offset += sizeof(uint32_t);

                copy_memory(section->raw_data, &color_embedded->bit_depth, sizeof(uint32_t), raw_data_offset, 0);
                raw_data_offset += sizeof(uint32_t);

                copy_memory(section->raw_data, &color_embedded->data_size, sizeof(uint64_t), raw_data_offset, 0);
                raw_data_offset += sizeof(uint64_t);

                copy_memory(section->raw_data, color_embedded->pixel_data, color_embedded->data_size, raw_data_offset, 0);
                raw_data_offset += color_embedded->data_size;

                section->parsed_data.color_embedded = color_embedded;
        } else {
                log_internal_event(LOG_LEVEL_ERROR, "%s: unkown include type: %s", function_name, include_type);
                return NULL;
        }

        section->header = *header;

        return section;
}


SGE_RESULT sge_texture_add_section(sge_texture *texture, sge_texture_section *section) {
        if (!texture || !section) {
                return SGE_INVALID_API_CALL;
        }

        texture->header->section_count++;
        texture->sections = reallocate_memory(texture->sections, sizeof(sge_texture_section) * texture->header->section_count, MEMORY_TAG_TEXTURE);
        if (!texture->sections) {
                allocation_error();
                return SGE_ERROR_FAILED_ALLOCATION;
        }

        texture->sections[texture->header->section_count - 1] = *section;

        return SGE_SUCCESS;
}

SGE_RESULT sge_texture_save(char *filename, sge_texture *texture, int flags) {
        char *function_name = "sge_texture_save";
        if (!filename || !texture) {
                return SGE_INVALID_API_CALL;
        }

        char *new_filename = NULL;
        char *file_ending = "sgetext";
        char *grab_file_ending = NULL;
        if (get_file_ending(filename, grab_file_ending) != SGE_SUCCESS) {
                new_filename = reallocate_memory(NULL, strlen(filename) + strlen(file_ending) + 2, MEMORY_TAG_TEXTURE); // +1 for .
                strcpy(new_filename, filename);
                strcat(new_filename, ".");
                strcat(new_filename, file_ending);
                printf("NEW FILENAME: %s\n", filename);
        } else {
                if (strcmp(grab_file_ending, file_ending) != 0) {
                        new_filename = reallocate_memory(NULL, strlen(filename) + strlen(file_ending) + 2, MEMORY_TAG_TEXTURE); // +1 for .
                        strcpy(new_filename, filename);
                        strcat(new_filename, ".");
                        strcat(new_filename, file_ending);
                        printf("NEW FILENAME: %s\n", filename);
                } else {
                        new_filename = filename;
                }
        }

        FILE *fd = fopen(new_filename, "r");
        if (fd) {
                fclose(fd);
                if (!(flags & SGE_TEXTURE_FLAG_FORCE_WRITE)) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: File already exists and flag `SGE_TEXTURE_FLAG_FORCE_WRITE` is not set", function_name);
                        return SGE_ERROR;
                }
        }

        fd = fopen(new_filename, "wb");
        if (!fd) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: Failed creating file", function_name);
                return SGE_ERROR;
        }


        SGE_VERSION current_version = get_sge_version();

        uint16_t major_version = current_version.major;
        uint16_t minor_version = current_version.minor;
        uint16_t patch_version = current_version.patch;

        void *global_header = allocate_memory(
                SGE_TEXTURE_HEADER_FIXED_SIZE + texture->header->header_extension_size + texture->header->
                texture_name_size + texture->header->author_name_size + texture->header->description_size,
                MEMORY_TAG_RENDERER);
        if (global_header == NULL) {
                allocation_error();
                return SGE_ERROR_FAILED_ALLOCATION;
        }

        size_t file_offset = 0;
        log_event(LOG_LEVEL_DEBUG, "starting texture global header");
        //GLOBAL HEADER                                                                                                                               //
        copy_memory(global_header, SGE_TEXTURE_MAGIC_NUMBER, sizeof(SGE_TEXTURE_MAGIC_NUMBER), file_offset, 0);                                             //
        file_offset += sizeof(SGE_TEXTURE_MAGIC_NUMBER);                                                                                                 //
        //VERSION                                                                                                                                     //
        copy_memory(global_header, &major_version, sizeof(uint16_t), file_offset, 0);                                                                 //
        file_offset += sizeof(major_version);                                                                                                         //
        copy_memory(global_header, &minor_version, sizeof(uint16_t), file_offset, 0);                                                                 //
        file_offset += sizeof(minor_version);                                                                                                         //
        copy_memory(global_header, &patch_version, sizeof(uint16_t), file_offset, 0);                                                                 //
        file_offset += sizeof(patch_version);                                                                                                         //
        //SECTION                                                                                                                                     //
        copy_memory(global_header, &texture->header->section_count, sizeof(uint32_t), file_offset, 0);                                                                 //
        file_offset += sizeof(texture->header->section_count);
        //TEXTURE NAME
        copy_memory(global_header, &texture->header->texture_name_size, sizeof(uint32_t), file_offset, 0);
        file_offset += sizeof(texture->header->texture_name_size);
        if (texture->header->texture_name_size > 0) {
                copy_memory(global_header, texture->header->texture_name, texture->header->texture_name_size, file_offset, 0);
                file_offset += texture->header->texture_name_size;
        }
        //AUTHOR NAME
        copy_memory(global_header, &texture->header->author_name_size, sizeof(uint32_t), file_offset, 0);
        file_offset += sizeof(texture->header->author_name_size);
        if (texture->header->author_name_size > 0) {
                copy_memory(global_header, texture->header->author_name, texture->header->author_name_size, file_offset, 0);
                file_offset += texture->header->author_name_size;
        }
        //Timestamps
        copy_memory(global_header, &texture->header->creation_date_timestamp, sizeof(uint64_t), file_offset, 0);
        file_offset += sizeof(uint64_t);
        copy_memory(global_header, &texture->header->last_modified_date_timestamp, sizeof(uint64_t), file_offset, 0);
        file_offset += sizeof(uint64_t);
        //DESCRIPTION
        copy_memory(global_header, &texture->header->description_size, sizeof(uint32_t), file_offset, 0);
        file_offset += sizeof(texture->header->description_size);
        if (texture->header->description_size > 0) {
                copy_memory(global_header, texture->header->description, texture->header->description_size, file_offset, 0);
                file_offset += texture->header->description_size;
        }
        //EXTENSIONS                                                                                                                                  //
        copy_memory(global_header, &texture->header->header_extension_count, sizeof(uint16_t), file_offset, 0);                                                               //
        file_offset += sizeof(texture->header->header_extension_count);
        if (texture->header->header_extension_size > 0) {
                copy_memory(global_header, &texture->header->header_extensions, texture->header->header_extension_size, file_offset, 0);                                                                //
                file_offset += texture->header->header_extension_size;
        }
        //CHECKSUM                                                                                                                                    //
        uint32_t global_header_checksum = calculate_crc32(global_header, file_offset);
        copy_memory(global_header, &global_header_checksum, sizeof(uint32_t), file_offset, 0);                                          //
        file_offset += sizeof(global_header_checksum);                                                                                                //
        //WRITE TO FILE
        log_event(LOG_LEVEL_DEBUG, "Finished global header");
        fwrite(global_header, file_offset, 1, fd);
        //FREE GLOBAL HEADER BUFFER
        free_memory(global_header, MEMORY_TAG_RENDERER);
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





        //SECTIONS
        for (int i = 0; i < texture->header->section_count; ++i) {
                log_internal_event(LOG_LEVEL_DEBUG, "SECTION START");
                sge_texture_section section = texture->sections[i];
                uint32_t section_buffer_offset = 0;
                size_t section_header_size = SGE_TEXTURE_SECTION_HEADER_FIXED_SIZE + section.header.section_name_size + section.header.section_extension_size;
                void *section_header_buffer = allocate_memory(section_header_size, MEMORY_TAG_RENDERER);
                //TYPE
                copy_memory(section_header_buffer, &section.header.section_type, sizeof(uint16_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.header.section_type);
                //printf("%i\n", section_buffer_offset);
                //SECTION OFFSET
                uint64_t section_offset = file_offset + section_header_size;
                copy_memory(section_header_buffer, &section_offset, sizeof(uint64_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section_offset);
                //printf("%i\n", section_buffer_offset);
                //DATA SIZE
                copy_memory(section_header_buffer, &section.header.data_size, sizeof(uint64_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.header.data_size);
                //NAME
                copy_memory(section_header_buffer, &section.header.section_name_size, sizeof(uint32_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.header.section_name_size);
                copy_memory(section_header_buffer, section.header.section_name, section.header.section_name_size, section_buffer_offset, 0);
                section_buffer_offset += section.header.section_name_size;
                //TIMESTAMPS
                //todo
                copy_memory(section_header_buffer, &section.header.creation_date_timestamp, sizeof(uint64_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.header.creation_date_timestamp);
                copy_memory(section_header_buffer, &section.header.last_modified_timestamp, sizeof(uint64_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.header.last_modified_timestamp);
                //EXTENSIONS
                copy_memory(section_header_buffer, &section.header.section_extension_count, sizeof(uint16_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.header.section_extension_count);
                //printf("%i\n", section_buffer_offset);
                //todo if exists need raw data rn pointer
                copy_memory(section_header_buffer, section.header.extensions, section.header.section_extension_size, section_buffer_offset, 0);
                section_buffer_offset += section.header.section_extension_size;
                // CHECKSUM
                uint32_t section_header_checksum = calculate_crc32(section_header_buffer, section_buffer_offset);
                section.header.section_checksum = section_header_checksum;
                copy_memory(section_header_buffer, &section_header_checksum, sizeof(uint32_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section_header_checksum);
                //printf("%i\n", section_buffer_offset);
                //SIZE CHECK
                if (section_buffer_offset != section_header_size) {
                        log_internal_event(LOG_LEVEL_FATAL, "wrong section header, wanted size: %d, got size: %d instead", section_header_size, section_buffer_offset);
                        return SGE_ERROR;
                }
                //WRITE FILE
                fwrite(section_header_buffer, section_buffer_offset, 1, fd);
                //FREE
                free_memory(section_header_buffer, MEMORY_TAG_RENDERER);

                //DATA
                fwrite(section.raw_data, section.header.data_size, 1, fd);
                printf("WROTE %llu to file\n", section.header.data_size);
                //SIZE OFFSET
                file_offset += section_buffer_offset + section.header.data_size;
        }
        fclose(fd);
        return SGE_SUCCESS;
}

sge_texture *sge_texture_load(char *filename) {
        char *function_name = "sge_scene_load";
        if (!filename) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", invalid api call", function_name);
                return NULL;
        }

        FILE *fd = fopen(filename, "rb");
        if (!fd) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to open file", function_name);
                return NULL;
        }

        sge_texture *texture= allocate_memory(sizeof(sge_texture), MEMORY_TAG_SCENE);
        if (!texture) {
                allocation_error();
                return NULL;
        }

        sge_texture_header *texture_header = allocate_memory(sizeof(sge_texture_header), MEMORY_TAG_TEXTURE);
        if (!texture_header) {
                allocation_error();
                return NULL;
        }

        char magic_number[8];
        if (fread(magic_number, 8, 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read magic number", function_name);
                return NULL;
        }

        if (strcmp(magic_number, SGE_TEXTURE_MAGIC_NUMBER) != 0) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", magic number doesnt match", function_name);
                return NULL;
        }

        if (fread(&texture_header->major_version, sizeof(uint16_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read version", function_name);
                return NULL;
        }

        if (fread(&texture_header->minor_version, sizeof(uint16_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read version", function_name);
                return NULL;
        }

        if (fread(&texture_header->patch_version, sizeof(uint16_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read version", function_name);
                return NULL;
        }

        if (fread(&texture_header->section_count, sizeof(uint32_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read section_count", function_name);
                return NULL;
        }

        if (fread(&texture_header->texture_name_size, sizeof(uint32_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read scene name size", function_name);
                return NULL;
        }

        if (texture_header->texture_name_size > 0) {
                texture_header->texture_name = allocate_memory(texture_header->texture_name_size+1, MEMORY_TAG_SCENE);
                if (!texture_header->texture_name) {
                        allocation_error();
                        return NULL;
                }

                if (fread(texture_header->texture_name, texture_header->texture_name_size, 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read texture name", function_name);
                        return NULL;
                }
                texture_header->texture_name[texture_header->texture_name_size] = '\0';
        }


        if (fread(&texture_header->author_name_size, sizeof(uint32_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read author name size", function_name);
                return NULL;
        }

        if (texture_header->author_name_size > 0) {
                texture_header->author_name = allocate_memory(texture_header->author_name_size + 1, MEMORY_TAG_SCENE);
                if (!texture_header->author_name) {
                        allocation_error();
                        return NULL;
                }

                if (fread(texture_header->author_name , texture_header->author_name_size , 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read author name", function_name);
                        return NULL;
                }

                texture_header->author_name[texture_header->author_name_size] = '\0';
        }


        if (fread(&texture_header->creation_date_timestamp, sizeof(uint64_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read timestamp", function_name);
                return NULL;
        }

        if (fread(&texture_header->last_modified_date_timestamp, sizeof(uint64_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read timestamp", function_name);
                return NULL;
        }


        if (fread(&texture_header->description_size, sizeof(uint32_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read description size", function_name);
                return NULL;
        }

        if (texture_header->description_size > 0) {
                texture_header->description = allocate_memory(texture_header->description_size + 1, MEMORY_TAG_SCENE);
                if (!texture_header->description) {
                        allocation_error();
                        return NULL;
                }

                if (fread(texture_header->description , texture_header->description_size , 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read description", function_name);
                        return NULL;
                }

                texture_header->description[texture_header->description_size] = '\0';
        }


        if (fread(&texture_header->header_extension_count , sizeof(uint16_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header extension count", function_name);
                return NULL;
        }

        if (texture_header->header_extension_count > 0) {
                log_event(LOG_LEVEL_WARNING, "extensions in use but no supported for now");
                texture_header->header_extensions = allocate_memory( sizeof(sge_texture_extension) * texture_header->header_extension_count, MEMORY_TAG_SCENE);
                if (!texture_header->header_extensions) {
                        allocation_error();
                        return NULL;
                }
        }

        for (int i = 0; i < texture_header->header_extension_count; ++i) {
               sge_texture_extension *extension = &texture_header->header_extensions[i];
                if (!extension) {
                        log_event(LOG_LEVEL_ERROR, "failed to grab extension pointer");
                        return NULL;
                }

                if (fread(&extension->type, sizeof(uint16_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header extension type", function_name);
                        return NULL;
                }

                if (fread(&extension->data_size, sizeof(uint32_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header extension size", function_name);
                        return NULL;
                }

                extension->data = allocate_memory(extension->data_size, MEMORY_TAG_SCENE);
                if (!extension->data) {
                        allocation_error();
                        return NULL;
                }

                if (fread(&extension->data, extension->data_size, 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header extension data", function_name);
                        return NULL;
                }
        }

        if (fread(&texture_header->crc32_checksum, sizeof(uint32_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header checksum", function_name);
                return NULL;
        }

        log_event(LOG_LEVEL_INFO, "Finished reading scene header, Found %d Sections and %d Extensions, Author is %s", texture_header->section_count, texture_header->header_extension_count, texture_header->author_name);


        if (texture_header->section_count > 0) {
                texture->sections = allocate_memory(sizeof(sge_texture_section) * texture_header->section_count, MEMORY_TAG_SCENE);
                if (!texture->sections) {
                        allocation_error();
                        return NULL;
                }
        }
        log_internal_event(LOG_LEVEL_DEBUG, "READ texture global header");
        for (int i = 0; i < texture_header->section_count; ++i) {
                sge_texture_section *section = &texture->sections[i];
                if (!section) {
                        log_event(LOG_LEVEL_ERROR, "Failed to grab section pointer");
                        return NULL;
                }

                sge_texture_section_header *section_header = allocate_memory(sizeof(sge_texture_section_header), MEMORY_TAG_SCENE);
                if (!section_header) {
                        allocation_error();
                        return NULL;
                }

                if (fread(&section_header->section_type, sizeof(uint8_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read section header type", function_name);
                        return NULL;
                }

                if (fread(&section_header->section_offset, sizeof(uint64_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read section offset", function_name);
                        return NULL;
                }

                if (fread(&section_header->data_size, sizeof(uint64_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read section data size", function_name);
                        return NULL;
                }

                if (fread(&section_header->section_name_size, sizeof(uint32_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read section name size", function_name);
                        return NULL;
                }

                printf("SECTION NAME SIZE: %d\n", section_header->section_name_size);
                if (section_header->section_name_size > 0) {
                        section_header->section_name = allocate_memory(section_header->section_name_size + 1, MEMORY_TAG_SCENE);
                        if (!section_header->section_name) {
                                allocation_error();
                                return NULL;
                        }

                        if (fread(section_header->section_name, section_header->section_name_size,1, fd) != 1) {
                                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read section name", function_name);
                                return NULL;
                        }

                        section_header->section_name[section_header->section_name_size] = '\0';
                }


                if (fread(&section_header->creation_date_timestamp, sizeof(uint64_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read timestamp", function_name);
                        return NULL;
                }

                if (fread(&section_header->last_modified_timestamp, sizeof(uint64_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read timestamp", function_name);
                        return NULL;
                }


                if (fread(&section_header->section_extension_count , sizeof(uint16_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header extension count", function_name);
                        return NULL;
                }

                if (section_header->section_extension_count > 0) {
                        log_event(LOG_LEVEL_WARNING, "extensions in use but no supported for now");
                        section_header->extensions = allocate_memory( sizeof(sge_texture_extension) * section_header->section_extension_count, MEMORY_TAG_SCENE);
                        if (!section_header->extensions) {
                                allocation_error();
                                return NULL;
                        }
                }

                for (int j = 0; j < section_header->section_extension_count; ++j) {
                        sge_texture_extension *extension = &section_header->extensions[j];
                        if (!extension) {
                                log_event(LOG_LEVEL_ERROR, "failed to grab extension pointer");
                                return NULL;
                        }

                        if (fread(&extension->type, sizeof(uint16_t), 1, fd) != 1) {
                                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header extension type", function_name);
                                return NULL;
                        }

                        if (fread(&extension->data_size, sizeof(uint32_t), 1, fd) != 1) {
                                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header extension size", function_name);
                                return NULL;
                        }

                        extension->data = allocate_memory(extension->data_size, MEMORY_TAG_SCENE);
                        if (!extension->data) {
                                allocation_error();
                                return NULL;
                        }

                        if (fread(&extension->data, extension->data_size, 1, fd) != 1) {
                                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header extension data", function_name);
                                return NULL;
                        }
                }

                if (fread(&section_header->section_checksum, sizeof(uint32_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header checksum", function_name);
                        return NULL;
                }

                log_event(LOG_LEVEL_INFO, "Read section header, Type: %d, Extension Count: %d, Name: %s", section_header->section_type, section_header->section_extension_count, section_header->section_name);

                //printf("DATA SIZE: %llu\n", section_header->data_size);
                section->raw_data = allocate_memory(section_header->data_size, MEMORY_TAG_SCENE);
                if (!section->raw_data) {
                        allocation_error();
                        return NULL;
                }

                if (fread(section->raw_data, section_header->data_size, 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read section data", function_name);
                        return NULL;
                }

                //PARSING DATA
                switch (section_header->section_type) {
                        case SGE_TEXTURE_SECTION_TYPE_COLOR: {
                                if (sge_texture_parse_color_section(&section->parsed_data, section->raw_data, section_header->data_size) != SGE_SUCCESS) {
                                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to parse section data", function_name);
                                        return NULL;
                                }
                                //if (section->parsed_data->sgerend->include_type == SGE_SCENE_SGEREND_INCLUDE_TYPE_EXTERNAL) {
                                //        sge_hexdump(section->parsed_data->sgerend->sgerend_source_data, section->parsed_data->sgerend->sgerend_source_size_non_embedded);
                                //}
                        } break;
                        default: {
                                log_event(LOG_LEVEL_WARNING, "\"%s\", unknown section type, cant parse data", function_name);
                        };
                }


                section->header = *section_header;
        }

        texture->header = texture_header;

        fclose(fd);
        return texture;
}

SGE_RESULT sge_texture_parse_color_section(sge_parsed_section_data *parsed_output, void *raw_data, size_t data_size) {
        if (!parsed_output || !raw_data || !data_size) {
                return SGE_INVALID_API_CALL;
        }

        uint8_t include_type = ((uint8_t *)raw_data)[0];
        if (include_type == SGE_TEXTURE_BODY_INCLUDE_TYPE_EXTERNAL) {
                sge_texture_section_color_external *color_external = allocate_memory(sizeof(sge_texture_section_color_external), MEMORY_TAG_TEXTURE);
                if (!color_external) {
                        allocation_error();
                        return SGE_ERROR_FAILED_ALLOCATION;
                }
                color_external->include_type = include_type;
                copy_memory(&color_external->filepath_length, raw_data, sizeof(uint32_t), 0, 1);
                color_external->filepath = allocate_memory(color_external->filepath_length + 1, MEMORY_TAG_TEXTURE);
                if (!color_external->filepath) {
                        allocation_error();
                        return SGE_ERROR_FAILED_ALLOCATION;
                }
                copy_memory(color_external->filepath, raw_data, color_external->filepath_length, 0, 5);
                color_external->filepath[color_external->filepath_length] = '\n';
                sge_hexdump(raw_data, data_size);
                sge_hexdump(color_external->filepath, color_external->filepath_length);

                parsed_output->color_external = color_external;
        } else if (include_type == SGE_TEXTURE_BODY_INCLUDE_TYPE_EMBEDDED) {
                sge_texture_section_color_embedded *color_embedded = allocate_memory(sizeof(sge_texture_section_color_embedded), MEMORY_TAG_TEXTURE);
                if (!color_embedded) {
                        allocation_error();
                        return SGE_ERROR_FAILED_ALLOCATION;
                }
                color_embedded->include_type;

                size_t raw_data_offset = 1;
                copy_memory(&color_embedded->color_type, raw_data, sizeof(uint8_t), 0, raw_data_offset);
                raw_data_offset += sizeof(uint8_t);

                copy_memory(&color_embedded->bit_depth, raw_data, sizeof(uint8_t), 0, raw_data_offset);
                raw_data_offset += sizeof(uint8_t);

                copy_memory(&color_embedded->width, raw_data, sizeof(uint32_t), 0, raw_data_offset);
                raw_data_offset += sizeof(uint32_t);

                copy_memory(&color_embedded->height, raw_data, sizeof(uint32_t), 0, raw_data_offset);
                raw_data_offset += sizeof(uint32_t);

                copy_memory(&color_embedded->data_size, raw_data, sizeof(uint64_t), 0, raw_data_offset);
                raw_data_offset += sizeof(uint64_t);

                color_embedded->pixel_data = allocate_memory(color_embedded->data_size, MEMORY_TAG_TEXTURE);
                if (!color_embedded->pixel_data) {
                        allocation_error();
                        return SGE_ERROR_FAILED_ALLOCATION;
                }

                copy_memory(color_embedded->pixel_data, raw_data, color_embedded->data_size, 0, raw_data_offset);
                parsed_output->color_embedded = color_embedded;
        } else {
                log_internal_event(LOG_LEVEL_ERROR, "Tried parsing texture file but got unknown include type");
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}