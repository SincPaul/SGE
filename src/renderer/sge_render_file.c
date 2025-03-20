//
// Created by Geisthardt on 17.03.2025.
//

#include "sge_render_file.h"

#include <stdio.h>

#include "../SGE.h"
#include "../core/logging.h"
#include "../core/memory_control.h"


uint32_t calculate_crc32(void *data, size_t length) {
        uint32_t crc = 0xFFFFFFFF;
        uint8_t *bytes = (uint8_t *)data;

        for (size_t i = 0; i < length; i++) {
                uint8_t byte = bytes[i];
                crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ byte];
        }

        return crc ^ 0xFFFFFFFF;
}

SGE_RESULT sge_rend_save(char *filename, SGE_REND_SECTION *sections, uint16_t section_count) {
        char save_filename[256];
        zero_memory(save_filename, sizeof(save_filename), 0);
        strcat(save_filename, filename);
        strcat(save_filename, ".sgerend");

        //todo make sure file not exist, make savepath configurable
        FILE *fd = fopen(save_filename, "wb");
        if (!fd) {
                log_event(LOG_LEVEL_FATAL, "failed to open file");
                return SGE_ERROR;
        }

        SGE_VERSION current_version = get_sge_version();

        uint16_t major_version = current_version.major;
        uint16_t minor_version = current_version.minor;
        uint16_t patch_version = current_version.patch;


        //todo dynamic thorugh function call
        uint16_t extension_count = 0;
        uint16_t extension_size = 0;
        char renderable_name[64] = "testerino objekto mi amigo";

        void *global_header = allocate_memory(SGE_REND_HEADER_FIXED_SIZE + extension_size, MEMORY_TAG_RENDERER);
        if (global_header == NULL) {
                log_event(LOG_LEVEL_FATAL, "FAiled to allocate for file save header");
                return SGE_ERROR_FAILED_ALLOCATION;
        }

        char *extension_data = "LALALALAL";

        size_t file_offset = 0;

        //GLOBAL HEADER                                                                                                                               //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        copy_memory(global_header, SGE_REND_MAGIC_NUMBER, sizeof(SGE_REND_MAGIC_NUMBER), file_offset, 0);                                             //
        file_offset += sizeof(SGE_REND_MAGIC_NUMBER);                                                                                                 //
        //VERSION                                                                                                                                     //
        copy_memory(global_header, &major_version, sizeof(uint16_t), file_offset, 0);                                                                 //
        file_offset += sizeof(major_version);                                                                                                         //
        copy_memory(global_header, &minor_version, sizeof(uint16_t), file_offset, 0);                                                                 //
        file_offset += sizeof(minor_version);                                                                                                         //
        copy_memory(global_header, &patch_version, sizeof(uint16_t), file_offset, 0);                                                                 //
        file_offset += sizeof(patch_version);                                                                                                         //
        //SECTION                                                                                                                                     //
        copy_memory(global_header, &section_count, sizeof(uint16_t), file_offset, 0);                                                                 //
        file_offset += sizeof(section_count);                                                                                                         //
        //EXTENSIONS                                                                                                                                  //
        copy_memory(global_header, &extension_count, sizeof(uint16_t), file_offset, 0);                                                               //
        file_offset += sizeof(extension_count);                                                                                                       //
        copy_memory(global_header, &extension_data, extension_size, file_offset, 0);                                                                //
        file_offset += extension_size;
        //NAME
        copy_memory(global_header, &renderable_name, 64, file_offset, 0);
        file_offset += 64;
        //CHECKSUM                                                                                                                                    //
        uint32_t global_header_checksum = calculate_crc32(global_header, file_offset);
        copy_memory(global_header, &global_header_checksum, sizeof(uint32_t), file_offset, 0);                                          //
        file_offset += sizeof(global_header_checksum);                                                                                                //
        //WRITE TO FILE                                                                                                                               //
        fwrite(global_header, file_offset, 1, fd);
        //FREE GLOBAL HEADER BUFFER
        free_memory(global_header, MEMORY_TAG_RENDERER);
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





        //////////////////////////////////////////////////////////////////////////////////////////
        ////GLOBAL HEADER                                                                       //
        //fwrite(SGE_REND_MAGIC_NUMBER, 1, 8, fd);                                              //
        ////VERSION                                                                             //
        //fwrite(&major_version, sizeof(uint16_t), 1, fd);                                      //
        //fwrite(&minor_version, sizeof(uint16_t), 1, fd);                                      //
        //fwrite(&patch_version, sizeof(uint16_t), 1, fd);                                      //
        ////SECTION                                                                             //
        //fwrite(&section_count, sizeof(uint16_t), 1, fd);                                      //
        //fwrite(&extension_count, sizeof(uint16_t), 1, fd);                                    //
        //fwrite(&extension_data, 1 * extension_size, 1, fd);                                   //
        //fwrite(&global_header_checksum, sizeof(uint32_t), 1, fd);                             //
        //////////////////////////////////////////////////////////////////////////////////////////
        //file_offset = file_offset + SGE_REND_HEADER_FIXED_SIZE + extension_size;
        //////////////////////////////////////////////////////////////////////////////////
        //SECTIONS
        for (int i = 0; i < section_count; ++i) {
                SGE_REND_SECTION section = sections[i];
                int section_buffer_offset = 0;
                int section_header_size = SGE_REND_SECTION_HEADER_FIXED_SIZE + section.section_header.extension_size;
                void *section_header_buffer = allocate_memory(section_header_size, MEMORY_TAG_RENDERER);
                //TYPE
                copy_memory(section_header_buffer, &section.section_header.type, sizeof(uint16_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.section_header.type);
                printf("%i\n", section_buffer_offset);
                //SECTION OFFSET
                uint64_t section_offset = file_offset + section_header_size;
                copy_memory(section_header_buffer, &section_offset, sizeof(uint64_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section_offset);
                printf("%i\n", section_buffer_offset);
                //DATA SIZE
                copy_memory(section_header_buffer, &section.section_header.data_size, sizeof(uint64_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.section_header.data_size);
                printf("%i\n", section_buffer_offset);
                //EXTENSIONS
                copy_memory(section_header_buffer, &section.section_header.extension_count, sizeof(uint16_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.section_header.extension_count);
                printf("%i\n", section_buffer_offset);
                copy_memory(section_header_buffer, &section.section_header.extensions, section.section_header.extension_size, section_buffer_offset, 0);
                section_buffer_offset += section.section_header.extension_size;
                printf("%i\n", section_buffer_offset);
                //NAME
                copy_memory(section_header_buffer, &section.section_header.name, sizeof(section.section_header.name), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section.section_header.name);
                // CHECKSUM
                uint32_t section_header_checksum = calculate_crc32(section_header_buffer, section_buffer_offset);
                section.section_header.checksum = section_header_checksum;
                copy_memory(section_header_buffer, &section_header_checksum, sizeof(uint32_t), section_buffer_offset, 0);
                section_buffer_offset += sizeof(section_header_checksum);
                printf("%i\n", section_buffer_offset);
                //SIZE CHECK
                if (section_buffer_offset != SGE_REND_SECTION_HEADER_FIXED_SIZE + section.section_header.extension_size) {
                        log_event(LOG_LEVEL_FATAL, "wrong section header - sgerend");
                        return SGE_ERROR;
                }
                //WRITE FILE
                fwrite(section_header_buffer, section_buffer_offset, 1, fd);
                //FREE
                free_memory(section_header_buffer, MEMORY_TAG_RENDERER);

                //DATA
                fwrite(section.data, section.section_header.data_size, 1, fd);

                //SIZE OFFSET
                file_offset += section_buffer_offset + section.section_header.data_size;

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


SGE_RESULT sge_rend_load(char *filename, SGE_REND_FILE **outFile) {
        FILE *fd = fopen(filename, "rb");
        if (!fd) {
            log_event(LOG_LEVEL_ERROR, "failed to open file");
            return SGE_ERROR;
        }

        *outFile = (SGE_REND_FILE*)allocate_memory(sizeof(SGE_REND_FILE), MEMORY_TAG_RENDERER);
        if (*outFile == NULL) {
            log_event(LOG_LEVEL_ERROR, "Memory allocation failed for outFile");
            return SGE_ERROR;
        }

        char magic[8];
        uint16_t major, minor, patch, section_count, extension_count;
        if (fread(magic, 8, 1, fd) != 1 ||
            fread(&major, sizeof(uint16_t), 1, fd) != 1 ||
            fread(&minor, sizeof(uint16_t), 1, fd) != 1 ||
            fread(&patch, sizeof(uint16_t), 1, fd) != 1 ||
            fread(&section_count, sizeof(uint16_t), 1, fd) != 1 ||
            fread(&extension_count, sizeof(uint16_t), 1, fd) != 1) {
            log_event(LOG_LEVEL_FATAL, "Failed to read global header");
            return SGE_ERROR;
        }

        if (memcmp(magic, SGE_REND_MAGIC_NUMBER, 8) != 0) {
            log_event(LOG_LEVEL_FATAL, "Invalid Magic");
            return SGE_ERROR;
        }

        (*outFile)->header.section_count = section_count;

        (*outFile)->sections = (SGE_REND_SECTION*)allocate_memory(sizeof(SGE_REND_SECTION) * section_count, MEMORY_TAG_RENDERER);
        if ((*outFile)->sections == NULL) {
            log_event(LOG_LEVEL_ERROR, "Memory allocation failed for sections");
            return SGE_ERROR;
        }

        //todo store extension data
        if (extension_count > 0) {
                uint16_t extension_type;
                uint32_t extension_data_size;
                void *extension_data;
                for (int i = 0; i < extension_count; ++i) {
                        if (fread(&extension_type, sizeof(uint16_t), 1, fd) != 1) {
                            log_event(LOG_LEVEL_FATAL, "Invalid Extension Type");
                            return SGE_ERROR;
                        }
                        if (fread(&extension_data_size, sizeof(uint32_t), 1, fd) != 1) {
                            log_event(LOG_LEVEL_FATAL, "Invalid Extension Size");
                            return SGE_ERROR;
                        }
                        extension_data = allocate_memory(extension_data_size, MEMORY_TAG_RENDERER);
                        if (extension_data == NULL) {
                            log_event(LOG_LEVEL_FATAL, "Failed to allocate extension data");
                            return SGE_ERROR;
                        }
                        if (fread(extension_data, extension_data_size, 1, fd) != 1) {
                            log_event(LOG_LEVEL_FATAL, "Invalid Extension Data");
                            free_memory(extension_data, MEMORY_TAG_RENDERER);
                            return SGE_ERROR;
                        }
                        free_memory(extension_data, MEMORY_TAG_RENDERER);
                }
        }

        if (fread(&(*outFile)->header.name, sizeof((*outFile)->header.name), 1, fd) != 1) {
                log_event(LOG_LEVEL_FATAL, "Failed to read name");
                return SGE_ERROR;
        }

        uint32_t global_header_checksum;
        if (fread(&global_header_checksum, sizeof(uint32_t), 1, fd) != 1) {
                log_event(LOG_LEVEL_FATAL, "Failed to read global header checksum");
                return SGE_ERROR;
        }

        //todo verify checksum

        for (uint16_t i = 0; i < section_count; ++i) {
                SGE_REND_SECTION *section = &(*outFile)->sections[i];

                if (fread(&section->section_header.type, sizeof(uint16_t), 1, fd) != 1) {
                    log_event(LOG_LEVEL_FATAL, "Failed to read section type");
                    return SGE_ERROR;
                }
                if (fread(&section->section_header.offset, sizeof(uint64_t), 1, fd) != 1) {
                    log_event(LOG_LEVEL_FATAL, "Failed to read section offset");
                    return SGE_ERROR;
                }
                if (fread(&section->section_header.data_size, sizeof(uint64_t), 1, fd) != 1) {
                    log_event(LOG_LEVEL_FATAL, "Failed to read data size");
                    return SGE_ERROR;
                }
                if (fread(&section->section_header.extension_count, sizeof(uint16_t), 1, fd) != 1) {
                    log_event(LOG_LEVEL_FATAL, "Failed to read extension count");
                    return SGE_ERROR;
                }

                //todo store extension data
                if (section->section_header.extension_count > 0) {
                    section->section_header.extensions = allocate_memory(section->section_header.extension_count, MEMORY_TAG_RENDERER);
                    if(section->section_header.extensions == NULL){
                        log_event(LOG_LEVEL_FATAL, "Failed to allocate section extensions");
                        return SGE_ERROR;
                    }
                    if (fread(section->section_header.extensions, section->section_header.extension_size, 1, fd) != 1) {
                        log_event(LOG_LEVEL_FATAL, "Failed to read section extensions");
                        return SGE_ERROR;
                    }
                }

                if (fread(&section->section_header.name, sizeof(section->section_header.name), 1, fd) != 1) {
                    log_event(LOG_LEVEL_FATAL, "Failed to read name");
                    return SGE_ERROR;
                }

                if (fread(&section->section_header.checksum, sizeof(uint32_t), 1, fd) != 1) {
                    log_event(LOG_LEVEL_FATAL, "Failed to read checksum");
                    return SGE_ERROR;
                }

                //todo validate checksum

                section->data = allocate_memory(section->section_header.data_size, MEMORY_TAG_RENDERER);
                if (section->data == NULL) {
                    log_event(LOG_LEVEL_FATAL, "Failed to allocate section data");
                    return SGE_ERROR;
                }

                if (fread(section->data, section->section_header.data_size, 1, fd) != 1) {
                    log_event(LOG_LEVEL_FATAL, "Failed to read data");
                    return SGE_ERROR;
                }
        }

    return SGE_SUCCESS;
}


SGE_MESH_DATA *sge_parse_mesh_data(void *raw_data, size_t data_size) {
        SGE_MESH_DATA *mesh_data = allocate_memory(sizeof(SGE_MESH_DATA), MEMORY_TAG_RENDERER);
        if (mesh_data == NULL) {
                log_event(LOG_LEVEL_FATAL, "failed allocate for mesh data");
                return NULL;
        }

        uint8_t *data = (uint8_t*)raw_data;
        uint32_t offset = 0;

        copy_memory(&mesh_data->vertex_count, data, sizeof(uint32_t), 0, offset);
        offset += sizeof(uint32_t);

        copy_memory(&mesh_data->vertex_size, data, sizeof(uint32_t), 0, offset);
        offset += sizeof(uint32_t);

        copy_memory(&mesh_data->attribute_count, data, sizeof(uint32_t), 0, offset);
        offset += sizeof(uint32_t);

        mesh_data->attributes = allocate_memory(sizeof(SGE_MESH_ATTRIBUTE) * mesh_data->attribute_count, MEMORY_TAG_RENDERER);
        if (mesh_data->attributes == NULL) {
                log_event(LOG_LEVEL_FATAL, "failed to allocate attributes");
                return NULL;
        }

        uint32_t attribute_offset = 0;
        for (int i = 0; i < mesh_data->attribute_count; ++i) {

                copy_memory(&mesh_data->attributes->type, data, sizeof(uint16_t), attribute_offset, offset);
                attribute_offset += sizeof(uint16_t);
                offset += sizeof(uint16_t);

                copy_memory(&mesh_data->attributes->format, data, sizeof(uint16_t), attribute_offset, offset);
                attribute_offset += sizeof(uint16_t);
                offset += sizeof(uint16_t);

                copy_memory(&mesh_data->attributes->components, data, sizeof(uint16_t), attribute_offset, offset);
                attribute_offset += sizeof(uint16_t);
                offset += sizeof(uint16_t);

                copy_memory(&mesh_data->attributes->offset, data, sizeof(uint16_t), attribute_offset, offset);
                attribute_offset += sizeof(uint16_t);
                offset += sizeof(uint16_t);
        }

        size_t vertex_data_size = mesh_data->vertex_count * mesh_data->vertex_size;
        mesh_data->vertex_data = allocate_memory(vertex_data_size, MEMORY_TAG_RENDERER);
        if (mesh_data->vertex_data == NULL) {
                log_event(LOG_LEVEL_FATAL, "failed to allocate vertex data");
                return NULL;
        }

        copy_memory(mesh_data->vertex_data, data, vertex_data_size, 0, offset);

        return mesh_data;
}



SGE_REND_SECTION *sge_create_mesh_section(
        char *name,
        void *vertex_data,
        uint32_t vertex_count,
        uint32_t vertex_size,
        SGE_MESH_ATTRIBUTE *attributes,
        uint32_t attribute_count,
        SGE_REND_EXTENSION *extensions,
        uint32_t extension_count

) {
        SGE_REND_SECTION *mesh_section = allocate_memory(sizeof(SGE_REND_SECTION), MEMORY_TAG_RENDERER);

        mesh_section->section_header.type = SGE_SECTION_MESH;
        copy_memory(mesh_section->section_header.name, name, sizeof(name), 0, 0);

        mesh_section->section_header.extension_count = extension_count;

        uint32_t total_extension_size = 0;
        for (uint32_t i = 0; i < extension_count; ++i) {
                total_extension_size += extensions[i].data_size + SGE_REND_EXTENSION_FIXED_SIZE;
        }

        mesh_section->section_header.extension_size = total_extension_size;
        mesh_section->section_header.extensions = allocate_memory(total_extension_size, MEMORY_TAG_RENDERER);

        if (mesh_section->section_header.extensions) {
                uint32_t offset = 0;
                for (uint32_t i = 0; i < extension_count; ++i) {
                        uint32_t extension_offset = 0;
                        //Type
                        copy_memory(mesh_section->section_header.extensions, &extensions[i].type, sizeof(uint16_t), offset, extension_offset);
                        offset += sizeof(uint16_t);
                        extension_offset += sizeof(uint16_t);

                        //Size
                        copy_memory(mesh_section->section_header.extensions, &extensions[i].data_size, sizeof(uint32_t), offset, extension_offset);
                        offset += sizeof(uint32_t);
                        extension_offset += sizeof(uint32_t);

                        //DATA
                        copy_memory(mesh_section->section_header.extensions, &extensions[i].data, extensions[i].data_size, offset, extension_offset);
                        offset += sizeof(uint32_t);
                        extension_offset += sizeof(uint32_t);
                }

        } else {
                mesh_section->section_header.extensions = NULL;
                mesh_section->section_header.extension_size = 0;
                mesh_section->section_header.extension_count = 0;
        }

        size_t data_header_size =
                sizeof(uint32_t) +      //vertex_count
                sizeof(uint32_t) +      //vertex_size
                sizeof(uint32_t);       //attribute_count


        size_t attribute_data_size = attribute_count * sizeof(SGE_MESH_ATTRIBUTE);
        size_t vertex_data_size = vertex_count * vertex_size;
        size_t total_data_size = data_header_size + attribute_data_size + vertex_data_size;

        uint8_t *data = allocate_memory(total_data_size, MEMORY_TAG_RENDERER);
        if (data == NULL) {
                log_event(LOG_LEVEL_FATAL, "failed to alloc for data");
                return NULL;
        }

        uint32_t data_offset = 0;

        copy_memory(data, &vertex_count, sizeof(uint32_t), data_offset, 0);
        data_offset += sizeof(uint32_t);

        copy_memory(data, &vertex_size, sizeof(uint32_t), data_offset, 0);
        data_offset += sizeof(uint32_t);

        copy_memory(data, &attribute_count, sizeof(uint32_t), data_offset, 0);
        data_offset += sizeof(uint32_t);

        for (uint32_t i = 0; i < attribute_count; ++i) {
                copy_memory(data, &attributes[i], sizeof(SGE_MESH_ATTRIBUTE), data_offset, 0);
                data_offset += sizeof(SGE_MESH_ATTRIBUTE);
        }

        copy_memory(data, vertex_data, vertex_data_size, data_offset, 0);

        mesh_section->data = data;
        mesh_section->section_header.data_size = total_data_size;

        return mesh_section;
}


SGE_REND_SECTION sge_create_index_section();
SGE_REND_SECTION sge_create_material_section();
SGE_REND_SECTION sge_create_shader_binding_section();
