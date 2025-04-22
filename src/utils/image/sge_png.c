//
// Created by Geisthardt on 15.04.2025.
//

#include "utils/image/sge_png.h"

#include <SGE.h>
#include <stdio.h>
#include <string.h>
#include <tgmath.h>

#include <zlib.h>


#include "core/memory_control.h"
#include "core/sge_internal_logging.h"
#include "utils/cpu/sge_endianess.h"
#include "utils/hash/sge_crc32.h"


typedef struct png_size {
        size_t bytes_per_pixel;
        size_t scanline_width;
        size_t decompressed_size;
        size_t unfiltered_size;
} png_size;

SGE_RESULT sge_parse_ihdr_chunk(FILE *fd, sge_png_metadata *metadata);
SGE_RESULT sge_collect_idat_chunk(void *chunk_data, uint32_t chunk_length, sge_png_metadata *metadata);
SGE_RESULT sge_parse_plte_chunk(void *chunk_data, uint32_t chunk_length, sge_png_metadata *metadata);
SGE_RESULT sge_parse_idat_data(sge_png_metadata *metadata);
png_size *get_decompressed_size(sge_png_metadata *metadata);


SGE_BOOL was_last_idat = SGE_FALSE;

SGE_RESULT sge_png_parse_data(char *filename, sge_png_metadata *metadata) {
        char *function_name = "sge_png_parse_data";
        if (!filename || !metadata) {
                log_internal_event(LOG_LEVEL_ERROR,"%s: Invalid arguments (filename or metadata is NULL)", function_name);
                return SGE_INVALID_API_CALL;
        }
        FILE *fd = fopen(filename, "rb");
        if (!fd) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but no valid file passed, `%s`", function_name, filename);
                return SGE_INVALID_API_CALL;
        }

        metadata->idat_length = 0;
        metadata->idat_data = NULL;


        char png_file_signature[PNG_FILE_SIGNATURE_SIZE];
        if (fread(png_file_signature, PNG_FILE_SIGNATURE_SIZE, 1, fd) != 1) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but no valid file passed, cant read header", function_name);
                return SGE_ERROR;
        }

        if (memcmp(png_file_signature, PNG_FILE_SIGNATURE, PNG_FILE_SIGNATURE_SIZE) != 0) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but no valid file passed, invalid signature", function_name);
                return SGE_ERROR;
        }


        //IHDR chunk

        if (sge_parse_ihdr_chunk(fd, metadata) != SGE_SUCCESS) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png IHDR Header but failed", function_name);
        }

        SGE_BOOL finished_parsing = SGE_FALSE;

        while (!finished_parsing) {
                uint32_t chunk_length;
                uint32_t chunk_type_be;
                uint32_t chunk_type_ne;
                if (fread_be(&chunk_length, sizeof(uint32_t), 1, fd) != 1) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading chunk length", function_name);
                        return SGE_ERROR;
                }

                if (chunk_length > PNG_CHUNK_MAX_SAFE_SIZE) {
                        log_internal_event(LOG_LEVEL_ERROR, "chunk length is very high, aborting (%d Bytes)", chunk_length);
                        return SGE_ERROR;
                }
                void *chunk_data = alloca(sizeof(uint32_t) + chunk_length);
                if (!chunk_data) {
                        allocation_error();
                        continue;
                }
                uint32_t chunk_data_offset = 0;

                if (fread(&chunk_type_be, sizeof(uint32_t), 1, fd) != 1) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading chunk type", function_name);
                        return SGE_ERROR;
                }
                copy_memory(&chunk_type_ne, &chunk_type_be, sizeof(uint32_t), 0, 0);
                be_to_ne(&chunk_type_ne, sizeof(uint32_t), 1);
                copy_memory(chunk_data, &chunk_type_be, sizeof(chunk_type_be), chunk_data_offset, 0);
                chunk_data_offset += sizeof(chunk_type_be);

                if (chunk_length > 0) {
                        if (fread(chunk_data + chunk_data_offset, chunk_length, 1, fd) != 1) {
                                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading chunk data", function_name);
                                return SGE_ERROR;
                        }
                        chunk_data_offset += chunk_length;
                }

                uint32_t crc;
                if (fread_be(&crc, sizeof(uint32_t), 1, fd) != 1) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading IHDR chunk: `crc`", function_name);
                }


                uint32_t validation_crc = calculate_crc32(chunk_data, chunk_data_offset);

                if (memcmp(&validation_crc, &crc, sizeof(uint32_t)) != 0) {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but got wrong crc", function_name);
                }

                switch (chunk_type_ne) {
                        case PNG_FILE_CHUNK_TYPE_IDAT: {
                                if (sge_collect_idat_chunk(chunk_data + 4, chunk_length, metadata) != SGE_SUCCESS) {
                                        log_internal_event(LOG_LEVEL_ERROR, "Failed to collect idat chunk");
                                        return SGE_ERROR;
                                }
                                was_last_idat = SGE_TRUE;
                        } break;
                        case PNG_FILE_CHUNK_TYPE_PLTE: {
                                if (sge_parse_plte_chunk(chunk_data + 4, chunk_length, metadata) != SGE_SUCCESS) {
                                        log_internal_event(LOG_LEVEL_ERROR, "Failed to parse plte chunk");
                                        return SGE_ERROR;
                                }
                                was_last_idat = SGE_TRUE;
                        } break;
                        case PNG_FILE_CHUNK_TYPE_IEND: {
                                //printf("READ END CHUNK\n");
                                finished_parsing = SGE_TRUE;
                                was_last_idat = SGE_TRUE;
                                break;
                        } break;
                        default: {
                                log_internal_event(LOG_LEVEL_WARNING, "Currently no support for chunk naming rules like private or ancillary");
                                was_last_idat = SGE_TRUE;
                                printf("%d\n", chunk_type_ne);
                        }
                }
        }

        if (sge_parse_idat_data(metadata) != SGE_SUCCESS) {
                log_internal_event(LOG_LEVEL_ERROR, "Failed to parse idat data");
                return SGE_ERROR;
        }

        fclose(fd);
        return SGE_SUCCESS;
}


SGE_RESULT sge_parse_ihdr_chunk(FILE *fd, sge_png_metadata *metadata) {
        char *function_name = "sge_parse_ihdr_chunk";
        uint32_t chunk_length;
        uint32_t chunk_type_be;
        uint32_t chunk_type_ne;
        if (fread_be(&chunk_length, sizeof(uint32_t), 1, fd) != 1) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading chunk length", function_name);
                return SGE_ERROR;
        }

        void *chunk_data = alloca(sizeof(uint32_t) + chunk_length);
        uint32_t chunk_data_offset = 0;

        if (fread(&chunk_type_be, sizeof(uint32_t), 1, fd) != 1) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading chunk type", function_name);
                return SGE_ERROR;
        }
        copy_memory(&chunk_type_ne, &chunk_type_be, sizeof(uint32_t), 0, 0);
        be_to_ne(&chunk_type_ne, sizeof(uint32_t), 1);
        copy_memory(chunk_data, &chunk_type_be, sizeof(chunk_type_be), chunk_data_offset, 0);
        chunk_data_offset += sizeof(chunk_type_be);

        if (chunk_type_ne != PNG_FILE_CHUNK_TYPE_IHDR) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but chunk type is invalid should be IHDR", function_name);
                return SGE_ERROR;
        }
        //printf("CHUNK LENGTH %d\nCHUNK TYPE %d\n", chunk_length, chunk_type_ne);


        if (fread(&metadata->width, sizeof(uint32_t), 1, fd) != 1) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading IHDR chunk: `width`", function_name);
                return SGE_ERROR;
        }
        copy_memory(chunk_data, &metadata->width, sizeof(metadata->width), chunk_data_offset, 0);
        chunk_data_offset += sizeof(metadata->width);
        be_to_ne(&metadata->width, sizeof(uint32_t), 1);

        if (metadata->width == 0) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but width is invalid, cant be 0", function_name);
                return SGE_ERROR;
        }

        if (fread(&metadata->height, sizeof(uint32_t), 1, fd) != 1) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading IHDR chunk: `height`", function_name);
                return SGE_ERROR;
        }
        copy_memory(chunk_data, &metadata->height, sizeof(metadata->height), chunk_data_offset, 0);
        chunk_data_offset += sizeof(metadata->height);
        be_to_ne(&metadata->height, sizeof(uint32_t), 1);

        if (metadata->height == 0) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but height is invalid, cant be 0", function_name);
                return SGE_ERROR;
        }

        if (fread(&metadata->bit_depth, sizeof(uint8_t), 1, fd) != 1) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading IHDR chunk: `bit_depth`", function_name);
                return SGE_ERROR;
        }
        copy_memory(chunk_data, &metadata->bit_depth, sizeof(metadata->bit_depth), chunk_data_offset, 0);
        chunk_data_offset += sizeof(metadata->bit_depth);

        if (fread(&metadata->color_type, sizeof(uint8_t), 1, fd) != 1) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading IHDR chunk: `color_type`", function_name);
                return SGE_ERROR;
        }
        copy_memory(chunk_data, &metadata->color_type, sizeof(metadata->color_type), chunk_data_offset, 0);
        chunk_data_offset += sizeof(metadata->color_type);

        switch (metadata->color_type) {
                case 0: { //GRAYSCALE
                        if (metadata->bit_depth != 1 &&
                                metadata->bit_depth != 2 &&
                                metadata->bit_depth != 4 &&
                                metadata->bit_depth != 8 &&
                                metadata->bit_depth != 16) {
                                log_internal_event(LOG_LEVEL_ERROR,
                                                   "%s: Invalid combination of color type and bit depth, BIT DEPTH: %d, Color Type: %d",
                                                   function_name, metadata->bit_depth, metadata->color_type);
                                return SGE_ERROR;
                        }
                } break;
                case 2: { //RGB TRIPPLE
                        if (metadata->bit_depth != 8 &&
                                metadata->bit_depth != 16) {
                                log_internal_event(LOG_LEVEL_ERROR, "%s: Invalid combination of color type and bit depth, BIT DEPTH: %d, "
                                                                    "Color Type: %d", function_name, metadata->bit_depth, metadata->color_type);
                                return SGE_ERROR;
                        }

                } break;
                case 3: { //PALETTE INDEX, NEEDS PLTE chunk
                        if (metadata->bit_depth != 1 &&
                                metadata->bit_depth != 2 &&
                                metadata->bit_depth != 4 &&
                                metadata->bit_depth != 8) {
                                log_internal_event(LOG_LEVEL_ERROR,
                                                   "%s: Invalid combination of color type and bit depth, BIT DEPTH: %d, Color Type: %d",
                                                   function_name, metadata->bit_depth, metadata->color_type);
                                return SGE_ERROR;
                        }
                } break;
                case 4: { //GRAYSCALE WITH ALPHA
                        if (metadata->bit_depth != 8 &&
                                metadata->bit_depth != 16) {
                                log_internal_event(LOG_LEVEL_ERROR,
                                                   "%s: Invalid combination of color type and bit depth, BIT DEPTH: %d, Color Type: %d",
                                                   function_name, metadata->bit_depth, metadata->color_type);
                                return SGE_ERROR;
                        }
                } break;
                case 6: { //RGB TRIPPE WITH ALPHA
                        if (metadata->bit_depth != 8 &&
                                metadata->bit_depth != 16) {
                                log_internal_event(LOG_LEVEL_ERROR,
                                                   "%s: Invalid combination of color type and bit depth, BIT DEPTH: %d, Color Type: %d",
                                                   function_name, metadata->bit_depth, metadata->color_type);
                                return SGE_ERROR;
                        }
                } break;
                default: {
                        log_internal_event(LOG_LEVEL_ERROR, "%s: Invalid Color Type");
                        return SGE_ERROR;
                }
        }

        if (fread(&metadata->compression_method, sizeof(uint8_t), 1, fd) != 1) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading IHDR chunk: `compression_method`", function_name);
                return SGE_ERROR;
        }
        copy_memory(chunk_data, &metadata->compression_method, sizeof(metadata->compression_method), chunk_data_offset, 0);
        chunk_data_offset += sizeof(metadata->compression_method);

        if (metadata->compression_method != 0) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but compression_method is invalid, %d",
                                   function_name, metadata->compression_method);
                return SGE_ERROR;
        }

        if (fread(&metadata->filter_method, sizeof(uint8_t), 1, fd) != 1) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading IHDR chunk: `filter_method`", function_name);
                return SGE_ERROR;
        }
        copy_memory(chunk_data, &metadata->filter_method, sizeof(metadata->filter_method), chunk_data_offset, 0);
        chunk_data_offset += sizeof(metadata->filter_method);

        if (metadata->filter_method != 0) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but filter method is invalid, %d",
                                   function_name, metadata->filter_method);
                return SGE_ERROR;
        }

        if (fread(&metadata->interlace_method, sizeof(uint8_t), 1, fd) != 1) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading IHDR chunk: `interlace_method`", function_name);
                return SGE_ERROR;
        }
        copy_memory(chunk_data, &metadata->interlace_method, sizeof(metadata->interlace_method), chunk_data_offset, 0);
        chunk_data_offset += sizeof(metadata->interlace_method);

        if (metadata->interlace_method != 0 && metadata->interlace_method != 1) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but interlace method is invalid, %d",
                                   function_name, metadata->interlace_method);
                return SGE_ERROR;
        }

        uint32_t crc;
        if (fread_be(&crc, sizeof(uint32_t), 1, fd) != 1) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but failed reading IHDR chunk: `crc`", function_name);
        }


        uint32_t validation_crc = calculate_crc32(chunk_data, chunk_data_offset);

        if (memcmp(&validation_crc, &crc, sizeof(uint32_t)) != 0) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: tried parsing png but got wrong crc    [FILE CRC]: `%04x`, [CALCULATED CRC]: `%04x`", function_name, crc, validation_crc);
        }


        //printf("PNG HEADER\n"
        //       "Width: %d\n"
        //       "Height: %d\n"
        //       "Bit Depth: %d\n"
        //       "Color Type %d\n"
        //       "Compression Method %d\n"
        //       "Filter Method: %d\n"
        //       "Interlace Method: %d\n",
        //       metadata->width,metadata->height, metadata->bit_depth,
        //       metadata->color_type, metadata->compression_method,
        //       metadata->filter_method, metadata->interlace_method);

        return SGE_SUCCESS;
}

SGE_RESULT sge_collect_idat_chunk(void *chunk_data, uint32_t chunk_length, sge_png_metadata *metadata) {
        //printf("GOT IDAT CHUNK\n");

        if (metadata->idat_length > 0 && !was_last_idat) {
                log_internal_event(LOG_LEVEL_ERROR, "Error collecting idat data, already has data but last was not idat");
                return SGE_ERROR;
        }

        metadata->idat_length += chunk_length;
        metadata->idat_data = reallocate_memory(metadata->idat_data, metadata->idat_length, MEMORY_TAG_IMAGE);
        if (!metadata->idat_data) {
                allocation_error();
                return SGE_ERROR;
        }

        copy_memory(metadata->idat_data, chunk_data, chunk_length, metadata->idat_length - chunk_length, 0);
        return SGE_SUCCESS;
}

SGE_RESULT sge_parse_plte_chunk(void *chunk_data, uint32_t chunk_length, sge_png_metadata *metadata) {
        //printf("GOT PLTE CHUNK\n");
        log_internal_event(LOG_LEVEL_ERROR, "currently not supporting indexed pngs");
        return SGE_ERROR;
}

SGE_RESULT unfilter_none(uint8_t *unfiltered_dest, uint8_t *current_line_src, size_t scanline_width);
SGE_RESULT unfilter_sub(uint8_t *unfiltered_dest, uint8_t *current_line_src, size_t scanline_width, size_t bytes_per_pixel);
SGE_RESULT unfilter_up(uint8_t *unfiltered_dest, uint8_t *current_line_src, size_t scanline_width, uint8_t *prev_line_src);
SGE_RESULT unfilter_average(uint8_t *unfiltered_dest, uint8_t *current_line_src, size_t scanline_width, uint8_t *prev_line_src, size_t bytes_per_pixel);
SGE_RESULT unfilter_paeth(uint8_t *unfiltered_dest, uint8_t *current_line_src, size_t scanline_width, uint8_t *prev_line_src, size_t bytes_per_pixel);

SGE_RESULT sge_parse_idat_data(sge_png_metadata *metadata) {
        char *function_name = "sge_parse_idat_data";

        if (!metadata || !metadata->idat_data || metadata->idat_length == 0) {
                log_internal_event(LOG_LEVEL_ERROR, "%s: Invalid metadata or IDAT data", function_name);
                return SGE_ERROR;
        }

        png_size *metadata_sizes = get_decompressed_size(metadata);
        //printf("DECOMPRESSED SIZE: %llu\n",metadata_sizes->decompressed_size);

        void *decompressed_data = allocate_memory(metadata_sizes->decompressed_size, MEMORY_TAG_IMAGE);
        if (!decompressed_data) {
                allocation_error();
                return SGE_ERROR;
        }

        uLongf dest_len = (uLongf)metadata_sizes->decompressed_size;
        uLong source_len = (uLong)metadata->idat_length;

        if (uncompress2(decompressed_data, &dest_len, metadata->idat_data, &source_len) != Z_OK) {
                log_internal_event(LOG_LEVEL_ERROR, "Failed to uncompress idata data");
                return SGE_ERROR;
        }

        uint8_t *decomp_data = (uint8_t *)decompressed_data;

        if (dest_len != metadata_sizes->decompressed_size) {
                log_internal_event(LOG_LEVEL_ERROR, "Missmatch in decompressed size, Dest Len: %d, Expected Len: %d", dest_len, metadata_sizes->decompressed_size);
                return SGE_ERROR;
        }


        size_t unfiltered_size = metadata_sizes->unfiltered_size;
        //printf("UNFILTERTED SIZE: %llu\n", metadata_sizes->unfiltered_size);
        uint8_t *unfiltered_data = allocate_memory(unfiltered_size, MEMORY_TAG_IMAGE);
        if (!unfiltered_data) {
                allocation_error();
                return SGE_ERROR;
        }

        uint32_t filter_type_0 = 0;
        uint32_t filter_type_1 = 0;
        uint32_t filter_type_2 = 0;
        uint32_t filter_type_3 = 0;
        uint32_t filter_type_4 = 0;

        for (uint32_t scanlines = 0; scanlines < metadata->height; ++scanlines) {
                //printf("FILERT TYPE: %d, on Scan Line: %d\n", filter_type, scanlines);
                uint8_t *unfiltered_data_dest = &unfiltered_data[scanlines * metadata_sizes->scanline_width];
                uint8_t *prev_line_src = scanlines > 0 ? &unfiltered_data[metadata_sizes->scanline_width * (scanlines - 1)] : NULL;
                uint8_t *current_line_src = &decomp_data[(metadata_sizes->scanline_width + 1)* scanlines];
                uint8_t filter_type = current_line_src[0];
                switch (filter_type) {
                        case 0: {
                                filter_type_0++;
                                if (unfilter_none(unfiltered_data_dest, current_line_src, metadata_sizes->scanline_width) != SGE_SUCCESS) {
                                        log_internal_event(LOG_LEVEL_ERROR, "Failed to unfilter scanline");
                                        return SGE_ERROR;
                                }
                        } break;
                        case 1: {
                                filter_type_1++;
                                if (unfilter_sub(unfiltered_data_dest, current_line_src, metadata_sizes->scanline_width,
                                                 metadata_sizes->bytes_per_pixel) != SGE_SUCCESS) {
                                        log_internal_event(LOG_LEVEL_ERROR, "Failed to unfilter scanline");
                                        return SGE_ERROR;
                                }
                        } break;
                        case 2: {
                                filter_type_2++;
                                if (unfilter_up(unfiltered_data_dest, current_line_src, metadata_sizes->scanline_width,
                                                prev_line_src) != SGE_SUCCESS) {
                                        log_internal_event(LOG_LEVEL_ERROR, "Failed to unfilter scanline");
                                        return SGE_ERROR;
                                }
                        } break;
                        case 3: {
                                filter_type_3++;
                                if (unfilter_average(unfiltered_data_dest, current_line_src,
                                                     metadata_sizes->scanline_width, prev_line_src,
                                                     metadata_sizes->bytes_per_pixel) != SGE_SUCCESS) {
                                        log_internal_event(LOG_LEVEL_ERROR, "Failed to unfilter scanline");
                                        return SGE_ERROR;
                                }
                        } break;
                        case 4: {
                                filter_type_4++;
                                if (unfilter_paeth(unfiltered_data_dest, current_line_src,
                                                     metadata_sizes->scanline_width, prev_line_src,
                                                     metadata_sizes->bytes_per_pixel) != SGE_SUCCESS) {
                                        log_internal_event(LOG_LEVEL_ERROR, "Failed to unfilter scanline");
                                        return SGE_ERROR;
                                                     }
                        } break;
                        default: {
                                log_internal_event(LOG_LEVEL_ERROR, "Wrong filter type: %d, at line: %d", filter_type, scanlines);
                                return SGE_ERROR;
                        }
                }
        }

        //printf("F0: %d\nF1: %d\nF2: %d\nF3: %d\nF4: %d\n"
        //       , filter_type_0, filter_type_1, filter_type_2, filter_type_3, filter_type_4);
        free_memory(metadata_sizes, MEMORY_TAG_IMAGE);


        // DAS ANPASSEN WEO WEO WEO WEO WEO WEO WEO WEO
        // DAS ANPASSEN WEO WEO WEO WEO WEO WEO WEO WEO
        // DAS ANPASSEN WEO WEO WEO WEO WEO WEO WEO WEO
        // DAS ANPASSEN WEO WEO WEO WEO WEO WEO WEO WEO
        //FILE *f = fopen("output.ppm", "wb");
        //fprintf(f, "P6\n%d %d\n255\n", metadata->width, metadata->height);
        //for (int i = 0; i < metadata->width * metadata->height * 3; i += 1) {
        //        fwrite(&unfiltered_data[i], 1, 1, f); // skip alpha
        //}
        //fclose(f);

        metadata->idat_data = reallocate_memory(metadata->idat_data, unfiltered_size, MEMORY_TAG_IMAGE);
        copy_memory(metadata->idat_data, unfiltered_data, unfiltered_size, 0, 0);

        return SGE_SUCCESS;
}

png_size *get_decompressed_size(sge_png_metadata *metadata) {
        size_t bits_per_pixel;
        switch (metadata->color_type) {
                case 0: { // Grayscale
                        bits_per_pixel = metadata->bit_depth;
                } break;
                case 2: { // RGB
                        bits_per_pixel = 3 * metadata->bit_depth;
                } break;
                case 3: { // Palette
                        bits_per_pixel = metadata->bit_depth;
                } break;
                case 4: { // Grayscale + Alpha
                        bits_per_pixel = 2 * metadata->bit_depth;
                } break;
                case 6: { // RGB + Alpa
                        bits_per_pixel = 4 * metadata->bit_depth;
                } break;
                default: {
                        return NULL;
                }
        }

        size_t bytes_per_pixel = (bits_per_pixel + 7 ) / 8;
        size_t bytes_per_scanline = bytes_per_pixel * metadata->width;
        size_t unfiltered_size = bytes_per_scanline * metadata->height;
        size_t decompressed_size = (bytes_per_scanline + 1) * metadata->height;
        //printf("BYTES PER SCANLINE: %llu\n", bytes_per_scanline);
        //printf("BYTES PER PIXEL: %llu\n", bytes_per_pixel);


        png_size *byte_sizes = allocate_memory(sizeof(png_size), MEMORY_TAG_IMAGE);
        if (!byte_sizes) {
                allocation_error();
                return NULL;
        }

        byte_sizes->bytes_per_pixel = bytes_per_pixel;
        byte_sizes->scanline_width = bytes_per_scanline;
        byte_sizes->decompressed_size = decompressed_size;
        byte_sizes->unfiltered_size = unfiltered_size;

        return byte_sizes;
}



SGE_RESULT unfilter_none(uint8_t *unfiltered_dest, uint8_t *current_line_src, size_t scanline_width) {
        if (!unfiltered_dest ||!current_line_src || !scanline_width) {
                return SGE_INVALID_API_CALL;
        }
        if (!copy_memory(unfiltered_dest, current_line_src, scanline_width, 0, 1)) {
                return SGE_ERROR;
        }
        return SGE_SUCCESS;
}

SGE_RESULT unfilter_sub(uint8_t *unfiltered_dest, uint8_t *current_line_src, size_t scanline_width, size_t bytes_per_pixel) {
        if (!unfiltered_dest || !current_line_src || !scanline_width || !bytes_per_pixel) {
                return SGE_INVALID_API_CALL;
        }

        unfiltered_dest[0] = current_line_src[1];
        for (size_t i = 1; i < scanline_width; ++i) {
                uint8_t left = i >= bytes_per_pixel ? unfiltered_dest[i - bytes_per_pixel] : 0;
                unfiltered_dest[i] = current_line_src[i + 1] + left;

        }

        return SGE_SUCCESS;
}

SGE_RESULT unfilter_up(uint8_t *unfiltered_dest, uint8_t *current_line_src, size_t scanline_width, uint8_t *prev_line_src) {
        if (!unfiltered_dest || !current_line_src || !scanline_width) {
                return SGE_INVALID_API_CALL;
        }

        for (uint32_t i = 0; i < scanline_width; ++i) {
                uint8_t prev_value = prev_line_src ? prev_line_src[i] : 0;
                unfiltered_dest[i] = prev_value + current_line_src[i + 1];
                //if (i < 16 && prev_line_src && prev_line_src[i] > 0) {
                //        printf("prev_line_src[%u]: %u\n", i, prev_line_src[i]);
                //        printf("current line[%u]: %u\n", i, current_line_src[i]);
                //        printf("unfiltered_dest[%u]: %u\n", i, unfiltered_dest[i]);
                //}
        }

        return SGE_SUCCESS;
}

SGE_RESULT unfilter_average(uint8_t *unfiltered_dest, uint8_t *current_line_src, size_t scanline_width, uint8_t *prev_line_src, size_t bytes_per_pixel) {
        if (!unfiltered_dest || !current_line_src || !scanline_width || !bytes_per_pixel) {
                return SGE_INVALID_API_CALL;
        }


        for (uint32_t i = 0; i < scanline_width; ++i) {
                uint8_t prev_value = prev_line_src ? prev_line_src[i] : 0;
                uint8_t left = i >= bytes_per_pixel ? unfiltered_dest[i - bytes_per_pixel] : 0;
                unfiltered_dest[i] = current_line_src[i + 1] + ((left + prev_value) / 2);
                //if (i < 16) {
                //        printf("prev_line_src[%zu]: %u\n", i, prev_line_src ? prev_line_src[i] : 0);
                //        printf("current_line_src[%zu]: %u\n", i + 1, current_line_src[i + 1]);
                //        printf("left[%zu]: %u\n", i, left);
                //        printf("unfiltered_dest[%zu]: %u\n", i, unfiltered_dest[i]);
                //}
        }

        return SGE_SUCCESS;
}

uint8_t paeth_predictor(uint8_t left, uint8_t prior, uint8_t prior_left);

SGE_RESULT unfilter_paeth(uint8_t *unfiltered_dest, uint8_t *current_line_src, size_t scanline_width, uint8_t *prev_line_src, size_t bytes_per_pixel) {
        if (!unfiltered_dest || !current_line_src || !scanline_width || !bytes_per_pixel) {
                return SGE_INVALID_API_CALL;
        }

        uint8_t *paeth = current_line_src + 1;

        for (uint32_t i = 0; i < scanline_width; ++i) {
                uint8_t left = i >= bytes_per_pixel ? unfiltered_dest[i - bytes_per_pixel] : 0;
                uint8_t prior = prev_line_src ? prev_line_src[i] : 3;
                uint8_t prior_left = i >= bytes_per_pixel && prev_line_src ? prev_line_src[i - bytes_per_pixel] : 0;

                unfiltered_dest[i] = paeth[i] + paeth_predictor(left, prior, prior_left);
                //if (i < 16 || i == 22 || i == 23) {
                //        printf("prev_line_src[%zu]: %u\n", i, prev_line_src ? prev_line_src[i] : 0);
                //        printf("current_line_src[%zu]: %u\n", i + 1, current_line_src[i + 1]);
                //        printf("left[%zu]: %u\n", i, left);
                //        printf("prior_left[%zu]: %u\n", i, prior_left);
                //        printf("                                        unfiltered_dest[%zu]: %u\n", i, unfiltered_dest[i]);
                //}
        }
        return SGE_SUCCESS;
}

uint8_t paeth_predictor(uint8_t left, uint8_t prior, uint8_t prior_left) {
        int path_estimate = left + prior - prior_left;
        int path_a = abs(path_estimate - left);
        int path_b = abs(path_estimate - prior);
        int path_c = abs(path_estimate - prior_left);

        if (path_a <= path_b && path_a <= path_c) {
                return left;
        } else if (path_b <= path_c) {
                return prior;
        } else {
                return prior_left;
        }
}
