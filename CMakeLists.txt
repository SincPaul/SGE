cmake_minimum_required(VERSION 3.30)
project(SGE C)

set(CMAKE_C_STANDARD 11)

find_package(Vulkan REQUIRED)


add_library(SGE
        src/core/logging.c
        src/utils/steintime.c
        src/utils/steinfile.c
        src/utils/steinstring.c
        src/utils/steinutils.c
        src/core/os_specific/sge_window_win32.c
        src/core/memory_control.c
        src/core/input.c
        src/core/os_specific/os_utils.c
        src/utils/sge_math.c
        src/renderer/sge_render.c
        src/renderer/vulkan_renderer/vulkan_renderer.c
        src/renderer/vulkan_renderer/sge_vulkan_instance.c
        src/renderer/vulkan_renderer/sge_vulkan_surface.c
        src/renderer/vulkan_renderer/sge_vulkan_device.c
        src/renderer/vulkan_renderer/sge_vulkan_swapchain.c
        src/renderer/vulkan_renderer/sge_vulkan_command.c
        src/renderer/vulkan_renderer/sge_vulkan_allocator.c
        src/renderer/vulkan_renderer/sge_vulkan_sync.c
        src/renderer/vulkan_renderer/sge_vulkan_pipeline.c
        src/renderer/vulkan_renderer/sge_vulkan_shader.c
        src/renderer/vulkan_renderer/sge_vulkan_draw.c
        src/renderer/vulkan_renderer/sge_vulkan_resize.c
        src/renderer/sge_camera.c
        src/renderer/vulkan_renderer/sge_vulkan_buffer.c
        src/renderer/vulkan_renderer/sge_vulkan_uniform.c
        src/renderer/vulkan_renderer/sge_vulkan_descriptor.c
        src/renderer/sge_region.c
)

add_executable(TEST test.c
)
target_link_libraries(SGE PRIVATE winmm Vulkan::Vulkan)
target_include_directories(TEST PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(TEST PRIVATE SGE winmm Vulkan::Vulkan DbgHelp)
