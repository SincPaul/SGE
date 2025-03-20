//
// Created by Geisthardt on 26.02.2025.
//
#include <signal.h>
#include <core/memory_control.h>
#include <stdio.h>
#include <utils/steintime.h>


#include "core/logging.h"
#include "core/os_specific/sge_window.h"
#include <unistd.h>
#include <windows.h>
#include <core/input.h>
#include <renderer/sge_render_file.h>
#include <renderer/vulkan_renderer/vulkan_renderer.h>
#include <vulkan/vulkan_core.h>
#include "renderer/sge_render.h"

#include "utils/steinutils.h"

void seg_fault_handler(int sig) {
       log_event(LOG_LEVEL_FATAL, "SEG FAULT OCCURED");
}

int main(void) {
        atexit(terminate_program);
        signal(SIGSEGV, seg_fault_handler);
        if (start_logger() != 0) {
                return 1;
        }
        log_event(LOG_LEVEL_INFO, "TEST EVENT");

        sge_window *window= sge_window_create(700, 500, "Test Window");
        sge_render *render = sge_render_create(RENDER_API_VULKAN, window);

        enable_input_tracking();
        //vulkan_context *vk_context = allocate_memory(sizeof(vulkan_context), MEMORY_TAG_VULKAN);
        //if (initialize_vk(vk_context ,wcw, "SGE TEST", VK_MAKE_API_VERSION(0,0,1,0)) != 0) {
        //        log_event(LOG_LEVEL_FATAL, "Failed to initialize Vk");
        //}

        sge_render_settings *settings = allocate_memory(sizeof(sge_render_settings), MEMORY_TAG_RENDERER);
        settings->vulkan.enable_validation_layers = true;
        settings->vulkan.use_dynamic_rendering = true;
        settings->vulkan.use_sge_allocator = false;
        sge_render_initialize(render, settings);

        const int target_fps = 0;

        //char *test_allocation = allocate_memory(256, MEMORY_TAG_UNKNOWN);
        //char *test_allocation_2 = allocate_memory(256, MEMORY_TAG_TEST_1);
        //char *test_allocation_3 = allocate_memory(256, MEMORY_TAG_TEST_2);
        //char *test_allocation_4 = allocate_memory(23256, MEMORY_TAG_TEST_3);
        //set_memory(test_allocation, 'A', 240, 16);
        //copy_memory(test_allocation_2, test_allocation, 240, 0, 16);
        //zero_memory(test_allocation_3, 256, 0);
        //free_memory(test_allocation_2, MEMORY_TAG_TEST_1);
        //free_memory(test_allocation_2, MEMORY_TAG_TEST_1);
        //free_memory(test_allocation_3, MEMORY_TAG_TEST_2);
        //free_memory(test_allocation, MEMORY_TAG_UNKNOWN);

        print_memory_usage_str();

        sge_region_settings region_setting_3d = {
                .auto_update = true,
                .auto_scale_on_resize = true,
                .auto_reposition_on_resize = false,
                .height = SGE_REGION_FULL_DIMENSION,
                .width = SGE_REGION_FULL_DIMENSION,
                .offset_x = 0,
                .offset_y = 0,
                .min_depth = 0.0f,
                .max_depth = 0.0f,
                .z_index = 0,
        };

        sge_region *main_region = sge_region_create(render, &region_setting_3d);

        sge_renderable *logo_renderable = create_logo_renderable(render);
        //sge_add_renderable(render, logo_renderable);

        //sge_region_add_renderable(main_region, logo_renderable);

        sge_region_settings second_setting_3d = {
                .auto_update = true,
                .auto_scale_on_resize = true,
                .auto_reposition_on_resize = true,
                .height = 200,
                .width = 200,
                .offset_x = 0,
                .offset_y = 0,
                .min_depth = 0.0f,
                .max_depth = 1.0f,
                .z_index = 1,
        };

        sge_region *secondary_region = sge_region_create(render, &second_setting_3d);

        //sge_region_add_renderable(secondary_region, logo_renderable);

        //m4 test_matrix =
        //m4 transformation_matrix;
        //vec3 translation_vector = {1.5, 1.0, 1.5};
        //m4 translation_matrix;
        //sge_m4_set_identity(translation_matrix);
        //m4 x_rotation_matrix;
        //sge_m4_set_identity(x_rotation_matrix);
        //m4 y_rotation_matrix;
        //sge_m4_set_identity(y_rotation_matrix);
        //sge_m4_set_translate(translation_matrix, translation_vector);
        //sge_m4_set_rotate_x(x_rotation_matrix, 180);
        //sge_m4_set_rotate_y(y_rotation_matrix, 90);
        //m4 temp;
        //sge_m4_multiply(temp, translation_matrix, x_rotation_matrix);
        //sge_m4_multiply(transformation_matrix, temp, y_rotation_matrix);
        //
        //vec4 init_pos = {0, 1, 0, 1};
        //vec4 end_pos = sge_m4_transform_vec4(transformation_matrix, init_pos);
        //printf("end pos: %f", end_pos.x);

        sge_movement_settings movement_settings_horizontal = {
                .mode = ROTATE_PITCH_AND_YAW,
                .delta_speed = 1,
        };
        sge_movement_settings left_right = {
                .mode = ROTATE_YAW_ONLY,
                .delta_speed = 1,
        };
        sge_movement_settings sprint_settings_horizontal = {
                .mode = ROTATE_PITCH_AND_YAW,
                .delta_speed = 3,
        };
        sge_movement_settings movement_settings_vertical = {
                .mode = ROTATE_NONE,
                .delta_speed = 0.5
        };





        //SGE_REND_SECTION sections[2];
//
        //strncpy(sections[0].section_header.name, "MESHI MESH", sizeof(sections[1].section_header.name) - 1);
        //sections[0].section_header.name[sizeof(sections[1].section_header.name) - 1] = '\0';
        //sections[0].section_header.type = 1;  // 1 = Mesh (for example)
        //sections[0].section_header.data_size = 9 * sizeof(float);
        //sections[0].section_header.extension_count = 0;
        //sections[0].section_header.extension_size = 0;
        //sections[0].section_header.extensions = NULL;
        //sections[0].data = allocate_memory(sections[0].section_header.data_size, MEMORY_TAG_RENDERER);
        //if (sections[0].data == NULL) {
        //    fprintf(stderr, "Failed to allocate mesh data\n");
        //    return -1;
        //}
        //float triangleVertices[9] = {
        //     0.0f, 0.0f, 0.0f,  // Vertex 1  0.0 0.0 0.0
        //     1.0f, 0.0f, 0.0f,  // Vertex 2
        //     0.0f, 1.0f, 0.0f   // Vertex 3
        //};
        //memcpy(sections[0].data, triangleVertices, sections[0].section_header.data_size);
//
        //strncpy(sections[1].section_header.name, "Materialo", sizeof(sections[1].section_header.name) - 1);
        //sections[1].section_header.name[sizeof(sections[1].section_header.name) - 1] = '\0';
        //sections[1].section_header.type = 2;
        //sections[1].section_header.data_size = 4 * sizeof(float);
        //sections[1].section_header.extension_count = 0;
        //sections[1].section_header.extension_size = 0;
        //sections[1].section_header.extensions = NULL;
        //sections[1].data = allocate_memory(sections[1].section_header.data_size, MEMORY_TAG_RENDERER);
        //if (sections[1].data == NULL) {
        //    fprintf(stderr, "Failed to allocate material data\n");
        //    free(sections[0].data);
        //    return -1;
        //}
        //float blueColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
        //memcpy(sections[1].data, blueColor, sections[1].section_header.data_size);


        SGE_MESH_ATTRIBUTE attributes[2] = {
                {
                        .type = SGE_ATTRIBUTE_POSITION,
                        .format = SGE_FORMAT_FLOAT32,
                        .components = 3,
                        .offset = 0,
                },
                {
                        .type = SGE_ATTRIBUTE_COLOR,
                        .format = SGE_FORMAT_UINT8,
                        .components = 4,
                        .offset = 12
                }
        };

        typedef struct {
                float position[3];
                uint8_t color[4];
        } ColoredVertex;

        ColoredVertex vertices[] = {
                {
                        .position = {-0.5f, 0.0f, 0.0f},
                        .color = {255, 0, 0, 255}
                },
                {
                        .position = {0.5f, 0.0f, 0.0f},
                        .color = {0, 255, 0, 255}
                },
                {
                        .position = {0.0f, -0.5f, 0.0f},
                        .color = {0, 0, 255, 255}
                }
        };

        SGE_REND_SECTION *triangle_test_mesh_section = sge_create_mesh_section(
                "test triangle",
                vertices,
                3,
                16,
                attributes,
                2,
                NULL,
                0
        );

        SGE_REND_SECTION sections[] = { *triangle_test_mesh_section };

        SGE_RESULT result = sge_rend_save("test", sections, 1);
        if (result != SGE_SUCCESS) {
            fprintf(stderr, "Failed to save render file\n");
        } else {
            printf("Render file saved successfully.\n");
        }



        SGE_REND_FILE *file = NULL;
        sge_rend_load("test.sgerend", &file);

        for (int i = 0; i < file->header.section_count; ++i) {
                SGE_REND_SECTION *section = &file->sections[i];
                printf("Section %d (type: %d, size: %zu bytes):\n",
                       i, section->section_header.type, section->section_header.data_size);

                unsigned char *bytes = (unsigned char*)section->data;
                for (size_t j = 0; j < section->section_header.data_size; j++) {
                        printf("%02X ", bytes[j]);
                        if ((j + 1) % 16 == 0) printf("\n");
                }
                printf("\n\n");
        }

        log_event(LOG_LEVEL_INFO, "creating test renderable");
        sge_renderable *test = create_renderable_from_rend_file(render, file);
        log_event(LOG_LEVEL_INFO, "finished creating test renderable");

        sge_region_add_renderable(main_region, test);

        //printf("%p\n", file->sections[1].data);

        while (!window_should_close()) {
                //do stuff
                timeBeginPeriod(1);
                const DWORD start_time = get_current_ms_time();

                sge_region *active_region = sge_region_get_active(render);
                int regions_count;
                sge_region **active_regions = sge_region_get_active_list(render, &regions_count);
                //for (int i = 0; i < regions_count; ++i) {
                //        printf("ACTIVE REGIONS: %i\n", i);
                //}
                //if (regions_count > 0) {
                //        printf("\n");
                //}

                //do everything in here
                if (is_key_down(KEY_ESCAPE)) {
                        //printf("PRESSED ESCAPE\n");
                }

                //if (is_key_down(KEY_A)) {
                //        //printf("HOLDING A\n");
                //        //sge_camera_move_left(render);
                //}

                if (is_key_down(KEY_A)) {
                        //printf("PRESSED A ONCE\n");
                        sge_camera_move_left(render, active_region, left_right);
                }

                if (is_key_down(KEY_W)) {
                        if (is_key_down(KEY_CTRL)) {
                                sge_camera_move_forward(render, active_region, sprint_settings_horizontal);
                        } else {
                                sge_camera_move_forward(render, active_region, movement_settings_horizontal);
                        }
                }

                if (is_key_down(KEY_S)) {
                        sge_camera_move_backwards(render, active_region, movement_settings_horizontal);
                }

                if (is_key_down(KEY_D)) {
                        sge_camera_move_right(render, active_region, left_right);
                }

                if (is_key_down(KEY_SPACE)) {
                        sge_camera_move_up(render, active_region, movement_settings_vertical);
                }

                if (is_key_down(KEY_SHIFT)) {
                        sge_camera_move_down(render, active_region, movement_settings_vertical);
                }

                if (is_key_pressed(KEY_ARROW_LEFT)) {
                        sge_camera_rotate_y(render,active_region, -15);
                }

                if (is_key_pressed(KEY_ARROW_RIGHT)) {
                        sge_camera_rotate_y(render, active_region, 15);
                }

                if (is_key_pressed(KEY_ARROW_UP)) {
                        sge_camera_rotate_x(render, active_region, 15);
                }

                if (is_key_pressed(KEY_ARROW_DOWN)) {
                        sge_camera_rotate_x(render, active_region, -15);
                }



                mouse_pos delta_mouse_pos = get_delta_mouse_position();

                sge_mouse_movement_settings mouse_movement_settings = {
                        .mouse_delta_x = delta_mouse_pos.x,
                        .mouse_delta_y = delta_mouse_pos.y,
                        .sensitivity = 1,
                        .flags = 0,
                };

                if (is_mouse_down(MBUTTON_RIGHT)) {
                        hide_mouse();
                        sge_camera_rotate(render, active_region, mouse_movement_settings);
                }

                if (was_mouse_down(MBUTTON_RIGHT)) {
                        printf("MOUSE WAS DUWN\n\n\n\n\n");
                        show_mouse();
                }

                //printf("Current Mouse Pos: x: %d, y: %d\n", current_mouse_pos.x, current_mouse_pos.y);
                //draw_frame(vk_context);
                //printf("DREW 1 FRAME\n");
                //sge_update_uniform_buffer(render, main_region);
                //sge_update_uniform_buffer(render, secondary_region);
                sge_update_uniform_buffer(render, NULL);
                sge_draw_frame(render);

                update_frame(target_fps, start_time, window);
                timeEndPeriod(1);
        }

        sge_window_destroy(window);

        stop_logger();

        log_event(LOG_LEVEL_WARNING, "NO LOGGER ACTIVE");

        sleep(3);

        log_event(LOG_LEVEL_FATAL, "ERROR HIHA");

        return 0;
}