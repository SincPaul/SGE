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
#include <renderer/vulkan_renderer/vulkan_renderer.h>
#include <vulkan/vulkan_core.h>
#include "renderer/sge_render.h"

#include "utils/steinutils.h"

void seg_fault_handler(int sig) {
       log_event(LOG_LEVEL_INFO, "SEG FAULT OCCURED");
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

        sge_renderable *logo_renderable = create_logo_renderable(render);
        sge_add_renderable(render, logo_renderable);

        //m4 test_matrix =
        m4 transformation_matrix;
        vec3 translation_vector = {1.5, 1.0, 1.5};
        m4 translation_matrix;
        sge_m4_set_identity(translation_matrix);
        m4 x_rotation_matrix;
        sge_m4_set_identity(x_rotation_matrix);
        m4 y_rotation_matrix;
        sge_m4_set_identity(y_rotation_matrix);
        sge_m4_set_translate(translation_matrix, translation_vector);
        sge_m4_set_rotate_x(x_rotation_matrix, 180);
        sge_m4_set_rotate_y(y_rotation_matrix, 90);
        m4 temp;
        sge_m4_multiply(temp, translation_matrix, x_rotation_matrix);
        sge_m4_multiply(transformation_matrix, temp, y_rotation_matrix);

        vec4 init_pos = {0, 1, 0, 1};
        vec4 end_pos = sge_m4_transform_vec4(transformation_matrix, init_pos);
        printf("end pos: %f", end_pos.x);

        while (!window_should_close()) {
                //do stuff
                timeBeginPeriod(1);
                const DWORD start_time = get_current_ms_time();

                //do everything in here
                if (is_key_down(KEY_ESCAPE)) {
                        printf("PRESSED ESCAPE\n");
                }

                if (is_key_down(KEY_A)) {
                        printf("HOLDING A\n");
                        //sge_camera_move_left(render);
                }

                if (is_key_pressed(KEY_A)) {
                        printf("PRESSED A ONCE\n");
                        sge_camera_move_left(render);
                }

                if (is_key_pressed(KEY_W)) {
                        sge_camera_move_forward(render);
                }

                if (is_key_pressed(KEY_S)) {
                        sge_camera_move_backwards(render);
                }

                if (is_key_pressed(KEY_D)) {
                        sge_camera_move_right(render);
                }

                if (is_key_pressed(KEY_SPACE)) {
                        sge_camera_move_up(render);
                }

                if (is_key_pressed(KEY_SHIFT)) {
                        sge_camera_move_down(render);
                }

                if (is_key_pressed(KEY_ARROW_LEFT)) {
                        sge_camera_rotate_y(render, -15);
                }

                if (is_key_pressed(KEY_ARROW_RIGHT)) {
                        sge_camera_rotate_y(render, 15);
                }

                if (is_key_pressed(KEY_ARROW_UP)) {
                        sge_camera_rotate_x(render, 15);
                }

                if (is_key_pressed(KEY_ARROW_DOWN)) {
                        sge_camera_rotate_x(render, -15);
                }

                const struct mouse_pos current_mouse_pos = get_mouse_position();

                //printf("Current Mouse Pos: x: %d, y: %d\n", current_mouse_pos.x, current_mouse_pos.y);
                //draw_frame(vk_context);
                //printf("DREW 1 FRAME\n");
                sge_update_uniform_buffer(render);
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