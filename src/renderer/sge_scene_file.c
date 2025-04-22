//
// Created by Geisthardt on 18.03.2025.
//

#include "renderer/sge_scene_file.h"

#include <stdio.h>
#include <string.h>

#include "sge_version.h"
#include "core/memory_control.h"
#include "core/sge_internal_logging.h"
#include "utils/sge_file.h"
#include "utils/sge_time.h"
#include "utils/sge_utils.h"
#include "utils/hash/sge_crc32.h"


sge_scene *sge_scene_create(char *scene_name, char *author_name, char *description) {
        sge_scene *scene = allocate_memory(sizeof(sge_scene), MEMORY_TAG_SCENE);
        if (!scene) {
                allocation_error();
                return NULL;
        }

        scene->header.author_name = allocate_memory(strlen(author_name), MEMORY_TAG_SCENE);
        if (!scene->header.author_name) {
                allocation_error();
                return NULL;
        }
        copy_memory(scene->header.author_name, author_name, strlen(author_name), 0, 0);
        scene->header.author_name_size = strlen(author_name);
        scene->header.creation_date_timestamp = get_current_ms_time();
        scene->header.last_modified_date_timestamp = get_current_ms_time();
        scene->header.description = NULL;
        scene->header.description_size = 0;
        scene->header.header_extension_count = 0;
        scene->header.header_extensions = NULL;
        scene->header.header_extension_size = 0;
        scene->header.scene_name = allocate_memory(strlen(scene_name), MEMORY_TAG_SCENE);
        if (!scene->header.scene_name) {
                allocation_error();
                return NULL;
        }
        copy_memory(scene->header.scene_name, scene_name, strlen(scene_name), 0, 0);
        scene->header.scene_name_size = strlen(scene_name);
        scene->header.section_count = 0;

        scene->sections = NULL;

        return scene;
}


SGE_RESULT sge_scene_save(char *filename, sge_scene *scene) {
        if (!filename || !scene) {
                return SGE_INVALID_API_CALL;
        }
        char save_filename[512];
        zero_memory(save_filename, sizeof(save_filename), 0);
        strcat(save_filename, filename);
        strcat(save_filename, ".sgescne");

        //todo make sure file not exist, make savepath configurable
        FILE *fd = fopen(save_filename, "wb");
        if (!fd) {
                log_internal_event(LOG_LEVEL_FATAL, "failed to open file");
                return SGE_ERROR;
        }

        SGE_VERSION current_version = get_sge_version();

        uint16_t major_version = current_version.major;
        uint16_t minor_version = current_version.minor;
        uint16_t patch_version = current_version.patch;

        void *global_header = allocate_memory(SGE_SCENE_HEADER_FIXED_SIZE + scene->header.header_extension_size + scene->header.scene_name_size + scene->header.author_name_size + scene->header.description_size, MEMORY_TAG_RENDERER);
        if (global_header == NULL) {
                log_internal_event(LOG_LEVEL_FATAL, "FAiled to allocate for file save header");
                return SGE_ERROR_FAILED_ALLOCATION;
        }

        size_t file_offset = 0;
        log_event(LOG_LEVEL_DEBUG, "starting scene global header");
        //GLOBAL HEADER                                                                                                                               //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        copy_memory(global_header, SGE_SCENE_MAGIC_NUMBER, sizeof(SGE_SCENE_MAGIC_NUMBER), file_offset, 0);                                             //
        file_offset += sizeof(SGE_SCENE_MAGIC_NUMBER);                                                                                                 //
        //VERSION                                                                                                                                     //
        copy_memory(global_header, &major_version, sizeof(uint16_t), file_offset, 0);                                                                 //
        file_offset += sizeof(major_version);                                                                                                         //
        copy_memory(global_header, &minor_version, sizeof(uint16_t), file_offset, 0);                                                                 //
        file_offset += sizeof(minor_version);                                                                                                         //
        copy_memory(global_header, &patch_version, sizeof(uint16_t), file_offset, 0);                                                                 //
        file_offset += sizeof(patch_version);                                                                                                         //
        //SECTION                                                                                                                                     //
        copy_memory(global_header, &scene->header.section_count, sizeof(uint32_t), file_offset, 0);                                                                 //
        file_offset += sizeof(scene->header.section_count);
        //SCENE NAME
        copy_memory(global_header, &scene->header.scene_name_size, sizeof(uint32_t), file_offset, 0);
        file_offset += sizeof(scene->header.scene_name_size);
        copy_memory(global_header, scene->header.scene_name, scene->header.scene_name_size, file_offset, 0);
        file_offset += scene->header.scene_name_size;
        //AUTHOR NAME
        copy_memory(global_header, &scene->header.author_name_size, sizeof(uint32_t), file_offset, 0);
        file_offset += sizeof(scene->header.author_name_size);
        copy_memory(global_header, scene->header.author_name, scene->header.author_name_size, file_offset, 0);
        file_offset += scene->header.author_name_size;
        //Timestamps
        copy_memory(global_header, &scene->header.creation_date_timestamp, sizeof(uint64_t), file_offset, 0);
        file_offset += sizeof(scene->header.creation_date_timestamp);
        copy_memory(global_header, &scene->header.last_modified_date_timestamp, sizeof(uint64_t), file_offset, 0);
        file_offset += sizeof(scene->header.last_modified_date_timestamp);
        //DESCRIPTION
        copy_memory(global_header, &scene->header.description_size, sizeof(uint32_t), file_offset, 0);
        file_offset += sizeof(scene->header.description_size);
        copy_memory(global_header, scene->header.description, scene->header.description_size, file_offset, 0);
        file_offset += scene->header.description_size;
        //EXTENSIONS                                                                                                                                  //
        copy_memory(global_header, &scene->header.header_extension_count, sizeof(uint16_t), file_offset, 0);                                                               //
        file_offset += sizeof(scene->header.header_extension_count);                                                                                                       //
        copy_memory(global_header, &scene->header.header_extensions, scene->header.header_extension_size, file_offset, 0);                                                                //
        file_offset += scene->header.header_extension_size;
        //CHECKSUM                                                                                                                                    //
        uint32_t global_header_checksum = calculate_crc32(global_header, file_offset);
        copy_memory(global_header, &global_header_checksum, sizeof(uint32_t), file_offset, 0);                                          //
        file_offset += sizeof(global_header_checksum);                                                                                                //
        //WRITE TO FILE
        //
        log_event(LOG_LEVEL_DEBUG, "Finished global header");
        fwrite(global_header, file_offset, 1, fd);
        //FREE GLOBAL HEADER BUFFER
        free_memory(global_header, MEMORY_TAG_RENDERER);
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





        //SECTIONS
        for (int i = 0; i < scene->header.section_count; ++i) {
                sge_scene_section section = scene->sections[i];
                uint32_t section_buffer_offset = 0;
                size_t section_header_size = section.section_header->header_size;
                void *section_header_buffer = allocate_memory(section_header_size, MEMORY_TAG_RENDERER);
                //TYPE
                copy_memory(section_header_buffer, &section.section_header->sge_scene_section_type, sizeof(uint16_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.section_header->sge_scene_section_type);
                //printf("%i\n", section_buffer_offset);
                //SECTION OFFSET
                uint64_t section_offset = file_offset + section_header_size;
                copy_memory(section_header_buffer, &section_offset, sizeof(uint64_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section_offset);
                //printf("%i\n", section_buffer_offset);
                //DATA SIZE
                copy_memory(section_header_buffer, &section.section_header->data_size, sizeof(uint64_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.section_header->data_size);
                //NAME
                copy_memory(section_header_buffer, &section.section_header->section_name_size, sizeof(uint32_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.section_header->section_name_size);
                copy_memory(section_header_buffer, section.section_header->section_name, section.section_header->section_name_size, section_buffer_offset, 0);
                section_buffer_offset += section.section_header->section_name_size;
                //TIMESTAMPS
                //todo
                copy_memory(section_header_buffer, &section.section_header->creation_date_timestamp, sizeof(uint64_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.section_header->creation_date_timestamp);
                copy_memory(section_header_buffer, &section.section_header->last_modified_date_timestamp, sizeof(uint64_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.section_header->last_modified_date_timestamp);
                //EXTENSIONS
                copy_memory(section_header_buffer, &section.section_header->section_extension_count, sizeof(uint16_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.section_header->section_extension_count);
                //printf("%i\n", section_buffer_offset);
                copy_memory(section_header_buffer, &section.section_header->extensions, section.section_header->section_extension_size, section_buffer_offset, 0);
                section_buffer_offset += section.section_header->section_extension_size;
                // CHECKSUM
                uint32_t section_header_checksum = calculate_crc32(section_header_buffer, section_buffer_offset);
                section.section_header->crc32_checksum = section_header_checksum;
                copy_memory(section_header_buffer, &section_header_checksum, sizeof(uint32_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section_header_checksum);
                //printf("%i\n", section_buffer_offset);
                //SIZE CHECK
                if (section_buffer_offset != section.section_header->header_size) {
                        log_internal_event(LOG_LEVEL_FATAL, "wrong section header - sgerend");
                        return SGE_ERROR;
                }
                //WRITE FILE
                fwrite(section_header_buffer, section_buffer_offset, 1, fd);
                //FREE
                free_memory(section_header_buffer, MEMORY_TAG_RENDERER);

                //DATA
                sge_hexdump(section.data, section.section_header->data_size);
                fwrite(section.data, section.section_header->data_size, 1, fd);
                //SIZE OFFSET
                file_offset += section_buffer_offset + section.section_header->data_size;

                //fwrite(&section.section_header.type, sizeof(uint16_t), 1, fd);
                //file_offset += SGE_REND_SECTION_HEADER_FIXED_SIZE + section->extension_size;
                //fwrite(&file_offset, sizeof(uint64_t), 1, fd);
                //size_t section_size
                //fwrite(&section_size, sizeof(uint64_t), 1, fd);
                //fwrite(&section->extension_count, sizeof(uint32_t), 1, fd);
                //fwrite(&section->extension_data, section->extension_size, 1, fd);
                //fwrite(&section->section_checksum, sizeof(uint32_t), 1, fd);
                //fwrite(&section->section_data, 1*section->section_data_size, 1, fd);
        }
        fclose(fd);

        return SGE_SUCCESS;
}


sge_scene *sge_scene_load(char *filename) {
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

        sge_scene *scene = allocate_memory(sizeof(sge_scene), MEMORY_TAG_SCENE);
        if (!scene) {
                allocation_error();
                return NULL;
        }

        sge_scene_header *scene_header = &scene->header;
        if (!scene_header) {
                allocation_error();
                return NULL;
        }

        char magic_number[8];
        if (fread(magic_number, 8, 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read magic number", function_name);
                return NULL;
        }

        if (strcmp(magic_number, SGE_SCENE_MAGIC_NUMBER) != 0) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", magic number doesnt match", function_name);
                return NULL;
        }

        if (fread(&scene_header->major_version, sizeof(uint16_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read version", function_name);
                return NULL;
        }

        if (fread(&scene_header->minor_version, sizeof(uint16_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read version", function_name);
                return NULL;
        }

        if (fread(&scene_header->patch_version, sizeof(uint16_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read version", function_name);
                return NULL;
        }

        if (fread(&scene_header->section_count, sizeof(uint32_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read section_count", function_name);
                return NULL;
        }

        if (fread(&scene_header->scene_name_size, sizeof(uint32_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read scene name size", function_name);
                return NULL;
        }

        if (scene_header->scene_name_size > 0) {
                scene_header->scene_name = allocate_memory(scene_header->scene_name_size+1, MEMORY_TAG_SCENE);
                if (!scene_header->scene_name) {
                        allocation_error();
                        return NULL;
                }

                if (fread(scene_header->scene_name, scene_header->scene_name_size, 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read scene name", function_name);
                        return NULL;
                }
                scene_header->scene_name[scene_header->scene_name_size] = '\0';
        }


        if (fread(&scene_header->author_name_size, sizeof(uint32_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read author name size", function_name);
                return NULL;
        }

        if (scene_header->author_name_size > 0) {
                scene_header->author_name = allocate_memory(scene_header->author_name_size + 1, MEMORY_TAG_SCENE);
                if (!scene_header->author_name) {
                        allocation_error();
                        return NULL;
                }

                if (fread(scene_header->author_name , scene_header->author_name_size , 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read author name", function_name);
                        return NULL;
                }

                scene_header->author_name[scene_header->author_name_size] = '\0';
        }


        if (fread(&scene_header->creation_date_timestamp, sizeof(uint64_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read timestamp", function_name);
                return NULL;
        }

        if (fread(&scene_header->last_modified_date_timestamp, sizeof(uint64_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read timestamp", function_name);
                return NULL;
        }


        if (fread(&scene_header->description_size, sizeof(uint32_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read description size", function_name);
                return NULL;
        }

        if (scene_header->description_size > 0) {
                scene_header->description = allocate_memory(scene_header->description_size + 1, MEMORY_TAG_SCENE);
                if (!scene_header->description) {
                        allocation_error();
                        return NULL;
                }

                if (fread(scene_header->description , scene_header->description_size , 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read description", function_name);
                        return NULL;
                }

                scene_header->description[scene_header->description_size] = '\0';
        }


        if (fread(&scene_header->header_extension_count , sizeof(uint16_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header extension count", function_name);
                return NULL;
        }

        if (scene_header->header_extension_count > 0) {
                log_event(LOG_LEVEL_WARNING, "extensions in use but no supported for now");
                scene_header->header_extensions = allocate_memory( sizeof(sge_scene_extension) * scene_header->header_extension_count, MEMORY_TAG_SCENE);
                if (!scene_header->header_extensions) {
                        allocation_error();
                        return NULL;
                }
        }

        for (int i = 0; i < scene_header->header_extension_count; ++i) {
               sge_scene_extension *extension = &scene_header->header_extensions[i];
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

        if (fread(&scene_header->crc32_checksum, sizeof(uint32_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header checksum", function_name);
                return NULL;
        }

        log_event(LOG_LEVEL_INFO, "Finished reading scene header, Found %d Sections and %d Extensions, Author is %s", scene_header->section_count, scene_header->header_extension_count, scene_header->author_name);


        if (scene_header->section_count > 0) {
                scene->sections = allocate_memory(sizeof(sge_scene_section) * scene_header->section_count, MEMORY_TAG_SCENE);
                if (!scene->sections) {
                        allocation_error();
                        return NULL;
                }
        }
        for (int i = 0; i < scene_header->section_count; ++i) {
                sge_scene_section *section = &scene->sections[i];
                if (!section) {
                        log_event(LOG_LEVEL_ERROR, "Failed to grab section pointer");
                        return NULL;
                }

                sge_scene_section_header *section_header = allocate_memory(sizeof(sge_scene_section_header), MEMORY_TAG_SCENE);
                if (!section_header) {
                        allocation_error();
                        return NULL;
                }

                if (fread(&section_header->sge_scene_section_type, sizeof(uint16_t), 1, fd) != 1) {
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

                if (fread(&section_header->last_modified_date_timestamp, sizeof(uint64_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read timestamp", function_name);
                        return NULL;
                }


                if (fread(&section_header->section_extension_count , sizeof(uint16_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header extension count", function_name);
                        return NULL;
                }

                if (section_header->section_extension_count > 0) {
                        log_event(LOG_LEVEL_WARNING, "extensions in use but no supported for now");
                        section_header->extensions = allocate_memory( sizeof(sge_scene_extension) * section_header->section_extension_count, MEMORY_TAG_SCENE);
                        if (!section_header->extensions) {
                                allocation_error();
                                return NULL;
                        }
                }

                for (int j = 0; j < section_header->section_extension_count; ++j) {
                        sge_scene_extension *extension = &section_header->extensions[j];
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

                if (fread(&section_header->crc32_checksum, sizeof(uint32_t), 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read header checksum", function_name);
                        return NULL;
                }

                log_event(LOG_LEVEL_INFO, "Read section header, Type: %d, Extension Count: %d, Name: %s", section_header->sge_scene_section_type, section_header->section_extension_count, section_header->section_name);

                //printf("DATA SIZE: %llu\n", section_header->data_size);
                section->data = allocate_memory(section_header->data_size, MEMORY_TAG_SCENE);
                if (!section->data) {
                        allocation_error();
                        return NULL;
                }

                if (fread(section->data, section_header->data_size, 1, fd) != 1) {
                        log_event(LOG_LEVEL_ERROR, "\"%s\", failed to read section data", function_name);
                        return NULL;
                }

                //PARSING DATA
                switch (section_header->sge_scene_section_type) {
                        case SGE_SCENE_SECTION_TYPE_SGEREND: {
                                if (sge_scene_parse_sgerend_section(&section->parsed_data, section->data, section_header->data_size) != SGE_SUCCESS) {
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


                section->section_header = section_header;
        }
        fclose(fd);
        return scene;
}

SGE_RESULT sge_scene_parse_sgerend_section(sge_scene_section_data **parsed_output, void *data, size_t data_size) {
        *parsed_output = NULL;
        sge_scene_sgerend_section *sgerend_data = allocate_memory(sizeof(sge_scene_sgerend_section), MEMORY_TAG_SCENE);
        if (!sgerend_data) {
                allocation_error();
                return SGE_ERROR_FAILED_ALLOCATION;
        }

        size_t data_offset = 0;
        copy_memory(&sgerend_data->include_type, data, sizeof(uint8_t), 0, data_offset);
        data_offset += sizeof(uint8_t);

        copy_memory(&sgerend_data->additional_section_count, data, sizeof(uint16_t), 0, data_offset);
        data_offset += sizeof(uint16_t);

        size_t source_data_size = 0;
        SGE_BOOL is_string = SGE_FALSE;
        if (sgerend_data->include_type == SGE_SCENE_SGEREND_INCLUDE_TYPE_EXTERNAL) {
                copy_memory(&sgerend_data->sgerend_source_size_non_embedded, data, sizeof(uint16_t), 0, data_offset);
                data_offset += sizeof(uint16_t);
                source_data_size = sgerend_data->sgerend_source_size_non_embedded;
                is_string = SGE_TRUE;
        } else {
                copy_memory(&sgerend_data->sgerend_source_size_embedded, data, sizeof(uint32_t), 0, data_offset);
                data_offset += sizeof(uint32_t);
                source_data_size = sgerend_data->sgerend_source_size_embedded;
        }

        //printf("SOURCE DATA SIZE: %llu, IS_STRING %d\n", data_size, is_string);
        sgerend_data->sgerend_source_data = allocate_memory(source_data_size + is_string, MEMORY_TAG_SCENE);
        if (!sgerend_data->sgerend_source_data) {
                allocation_error();
                return SGE_ERROR_FAILED_ALLOCATION;
        }

        copy_memory(sgerend_data->sgerend_source_data, data, source_data_size, 0, data_offset);
        if (is_string) {
                ((char*)sgerend_data->sgerend_source_data)[source_data_size] = '\0';
        }

        data_offset += source_data_size;

        if (sgerend_data->additional_section_count > 0) {
                copy_memory(&sgerend_data->section_data_size, data, sizeof(uint32_t), 0, data_offset);
                data_offset += sizeof(uint32_t);

                copy_memory(sgerend_data->additional_sge_rend_sections, data, sgerend_data->section_data_size, 0, data_offset);
                data_offset += sgerend_data->section_data_size;
        }

        copy_memory(&sgerend_data->transformation_flags, data, sizeof(uint16_t), 0, data_offset);
        data_offset += sizeof(uint16_t);

        uint16_t transformation_flag_size = 0;
        if (sgerend_data->transformation_flags & SGE_SCENE_TRANSFORMATION_FLAG_POSITION) transformation_flag_size += SGE_TRANSFORMATION_POSITION_SIZE;
        if (sgerend_data->transformation_flags & SGE_SCENE_TRANSFORMATION_FLAG_ROTATION) transformation_flag_size += SGE_TRANSFORMATION_ROTATION_SIZE;
        if (sgerend_data->transformation_flags & SGE_SCENE_TRANSFORMATION_FLAG_SCALE) transformation_flag_size += SGE_TRANSFORMATION_SCALE_SIZE;

        if (transformation_flag_size > 0) {
                //printf("TRANSFO DATA SIZE: %d\n", transformation_flag_size);
                sgerend_data->transformation_data = allocate_memory(transformation_flag_size, MEMORY_TAG_SCENE);
                if (!sgerend_data->transformation_data) {
                        allocation_error();
                        return SGE_ERROR_FAILED_ALLOCATION;
                }
                copy_memory(sgerend_data->transformation_data, data, transformation_flag_size, 0, data_offset);
                //sge_hexdump(sgerend_data->transformation_data, transformation_flag_size);
                data_offset += transformation_flag_size;
        }

        if (data_offset != data_size) {
                log_event(LOG_LEVEL_ERROR, "Failed to parse, different offset than given");
                return SGE_ERROR;
        }

        *parsed_output = allocate_memory(sizeof(sge_scene_section_data), MEMORY_TAG_SCENE);
        if (!*parsed_output) {
                allocation_error();
                return SGE_ERROR_FAILED_ALLOCATION;
        }
        (*parsed_output)->sgerend = sgerend_data;

        return SGE_SUCCESS;
}

sge_scene_section *sge_scene_create_sgerend_section(
        SGE_SCENE_SGEREND_INCLUDE_TYPE include_type,
        char            *section_name,
        void            *source_data,
        size_t          source_data_size,
        uint16_t        transformation_flags,
        void            *transformation_data
        ) {
        sge_scene_section *scene_section = allocate_memory(sizeof(sge_scene_section), MEMORY_TAG_SCENE);
        if (!scene_section) {
                allocation_error();
                return NULL;
        }

        sge_scene_section_header *header = allocate_memory(sizeof(sge_scene_section_header), MEMORY_TAG_SCENE);
        if (!header) {
                allocation_error();
                return NULL;
        }

        header->sge_scene_section_type = SGE_SCENE_SECTION_TYPE_SGEREND;

        header->section_name_size = strlen(section_name);
        header->section_name = allocate_memory(header->section_name_size, MEMORY_TAG_SCENE);
        if (!header->section_name) {
                allocation_error();
                return NULL;
        }
        copy_memory(header->section_name, section_name, header->section_name_size, 0, 0);

        header->creation_date_timestamp = get_current_ms_time();
        header->last_modified_date_timestamp = get_current_ms_time();

        //todo configurable
        header->section_extension_count = 0;
        header->section_extension_size = 0;

        header->section_offset = 0;
        header->data_size = 0;

        if (header->section_extension_count > 0) {
                header->extensions = allocate_memory(header->section_extension_size, MEMORY_TAG_SCENE);
                copy_memory(header->extensions, NULL, header->section_extension_size, 0, 0);
        } else {
                header->extensions = NULL;
        }

        header->crc32_checksum = calculate_crc32(source_data, source_data_size - sizeof(uint32_t));

        //DATA

        sge_scene_sgerend_section *sgerend_data = allocate_memory(sizeof(sge_scene_sgerend_section), MEMORY_TAG_SCENE);
        if (!sgerend_data) {
                allocation_error();
                return NULL;
        }

        sgerend_data->include_type = include_type;

        //todo make configurable
        sgerend_data->additional_section_count = 0;
        sgerend_data->additional_sge_rend_sections = NULL;
        sgerend_data->section_data_size = 0;

        if (sgerend_data->section_data_size > 0) {
                //todo
        } else {
                sgerend_data->additional_sge_rend_sections = NULL;
        }



        if (include_type == SGE_SCENE_SGEREND_INCLUDE_TYPE_EXTERNAL) { //external file
                sgerend_data->sgerend_source_size_non_embedded = source_data_size;
        } else { //embedded
                sgerend_data->sgerend_source_size_embedded = source_data_size;
        }
        sgerend_data->sgerend_source_data = allocate_memory(source_data_size, MEMORY_TAG_SCENE);
        if (!sgerend_data->sgerend_source_data) {
                allocation_error();
                return NULL;
        }
        copy_memory(sgerend_data->sgerend_source_data, source_data, source_data_size, 0, 0);


        sgerend_data->transformation_flags = transformation_flags;

        size_t transformation_data_size = 0;
        if (transformation_flags & SGE_SCENE_TRANSFORMATION_FLAG_POSITION)      transformation_data_size += sizeof(vec3);
        if (transformation_flags & SGE_SCENE_TRANSFORMATION_FLAG_ROTATION)      transformation_data_size += sizeof(vec3);
        if (transformation_flags & SGE_SCENE_TRANSFORMATION_FLAG_SCALE)         transformation_data_size += sizeof(vec3);

        if (transformation_data_size > 0) {
                sgerend_data->transformation_data = allocate_memory(transformation_data_size, MEMORY_TAG_RENDERER);
                copy_memory(sgerend_data->transformation_data, transformation_data, transformation_data_size, 0, 0);
        } else {
                sgerend_data->transformation_data = NULL;
        }

        size_t header_size = SGE_SCENE_SECTION_HEADER_FIXED_SIZE + header->section_name_size + header->section_extension_size;

        size_t data_size = 0;
        if (include_type == SGE_SCENE_SGEREND_INCLUDE_TYPE_EXTERNAL) {
                data_size = sizeof(uint8_t) + // include
                        sizeof(uint16_t) + //additional section count
                        sizeof(uint16_t) + //source size
                        source_data_size + //
                        sizeof(uint32_t) +// additional section count
                        sizeof(uint16_t) +//transformation flags
                        transformation_data_size;
        } else {
                data_size =sizeof(uint8_t) + // include
                        sizeof(uint16_t) + //additional section count
                        sizeof(uint32_t) + //source size
                        source_data_size + //
                        sizeof(uint32_t) +// additional section count
                        sizeof(uint16_t) +//transformation flags
                        transformation_data_size;
        }
        if (sgerend_data->additional_section_count <= 0) {
                data_size -= sizeof(uint32_t);
        }
        header->data_size = data_size;

        uint8_t *raw_data = allocate_memory(data_size, MEMORY_TAG_SCENE);
        if (!raw_data) {
                allocation_error();
                return NULL;
        }

        size_t raw_data_offset = 0;
        copy_memory(raw_data, &sgerend_data->include_type, sizeof(uint8_t), raw_data_offset, 0);
        raw_data_offset += sizeof(uint8_t);

        copy_memory(raw_data, &sgerend_data->additional_section_count, sizeof(uint16_t), raw_data_offset, 0);
        raw_data_offset += sizeof(uint16_t);

        size_t source_data_size_raw = 0;
        if (include_type == SGE_SCENE_SGEREND_INCLUDE_TYPE_EXTERNAL) {
                copy_memory(raw_data, &sgerend_data->sgerend_source_size_non_embedded, sizeof(uint16_t), raw_data_offset, 0);
                raw_data_offset += sizeof(uint16_t);
                source_data_size_raw = sgerend_data->sgerend_source_size_non_embedded;
        } else {
                copy_memory(raw_data, &sgerend_data->sgerend_source_size_embedded, sizeof(uint32_t), raw_data_offset, 0);
                raw_data_offset += sizeof(uint32_t);
                source_data_size_raw = sgerend_data->sgerend_source_size_embedded;
        }

        copy_memory(raw_data, sgerend_data->sgerend_source_data, source_data_size_raw, raw_data_offset, 0);
        raw_data_offset += source_data_size_raw;

        if (sgerend_data->additional_section_count > 0) {
                copy_memory(raw_data, &sgerend_data->section_data_size, sizeof(uint32_t), raw_data_offset, 0);
                raw_data_offset += sizeof(uint32_t);

                copy_memory(raw_data, sgerend_data->additional_sge_rend_sections, sgerend_data->section_data_size, raw_data_offset, 0);
                raw_data_offset += sgerend_data->section_data_size;
        }


        copy_memory(raw_data, &sgerend_data->transformation_flags, sizeof(uint16_t), raw_data_offset, 0);
        raw_data_offset += sizeof(uint16_t);

        copy_memory(raw_data, sgerend_data->transformation_data, transformation_data_size,raw_data_offset, 0);
        raw_data_offset += transformation_data_size;

        if (raw_data_offset != data_size) {
                log_event(LOG_LEVEL_FATAL, "RAW DATA OFFSET \"SGE SCENE CREATE SGEREND SECTION\"");
                return NULL;
        }

        scene_section->section_header   = header;
        scene_section->data             = raw_data;
        scene_section->section_header->header_size = header_size;

        return scene_section;
}


SGE_RESULT sge_scene_add_section(sge_scene *scene, sge_scene_section *section) {
        if (!scene || !section) {
                return SGE_INVALID_API_CALL;
        }

        scene->header.section_count++;
        scene->sections = reallocate_memory(scene->sections, sizeof(sge_scene_section) * scene->header.section_count, MEMORY_TAG_SCENE);
        if (!scene->sections) {
                allocation_error();
                return SGE_ERROR_FAILED_ALLOCATION;
        }

        scene->sections[scene->header.section_count -1] = *section;
        return SGE_SUCCESS;
}
