//
// Created by Geisthardt on 06.03.2025.
//

#ifndef SGE_RENDER_H
#define SGE_RENDER_H
#include <stdbool.h>

#include "../utils/sge_math.h"
#include "../core/os_specific/sge_window.h"
#include <vulkan/vulkan.h>
//#include "../core/logging.h"
//#include "../core/memory_control.h"


typedef enum sge_render_api {
        RENDER_API_VULKAN,
        RENDER_API_DIRECTX,
        RENDER_API_OPENGL
} render_api;

typedef enum SGE_RESULT {
        SGE_SUCCESS = 0,
        SGE_ERROR,
        SGE_RESIZE,
        SGE_ERROR_INVALID_API,
        SGE_ERROR_FAILED_ALLOCATION,
        SGE_UNSUPPORTED_SYSTEM
} SGE_RESULT;

typedef struct sge_vertex_attribute {
        uint32_t location;
        VkFormat format;
        uint32_t offset;
} sge_vertex_attribute;

typedef struct sge_vertex_format {
        uint32_t stride;
        sge_vertex_attribute *attributes;
        size_t attribute_count;
} sge_vertex_format;

typedef struct sge_shader { //SPIR-V vulkan, HLSL directx, GLSL opengl
        render_api api;
        void *api_shader;
} sge_shader;

typedef struct sge_pipeline_settings {
        sge_shader *vertex_shader;
        sge_shader *fragment_shader;
        sge_vertex_format *vertex_format;
        VkPrimitiveTopology topology;
        VkPolygonMode polygon_mode;
        VkCullModeFlags cull_mode;
        VkFrontFace front_face;
} sge_pipeline_settings;

typedef struct sge_mesh {
        VkBuffer vertex_buffer;
        VkDeviceMemory vertex_memory;
        VkBuffer index_buffer;
        VkDeviceMemory index_memory;
        uint32_t vertex_count;
        uint32_t index_count;
        sge_vertex_format *format;
} sge_mesh;

typedef struct sge_material {
        sge_shader      *vertex_shader;
        sge_shader      *fragment_shader;
        vec4            color;
} sge_material;

typedef struct sge_renderable {
        sge_mesh *mesh;
        sge_material *material;
} sge_renderable;

typedef struct sge_pipeline {
        render_api api;
        void *api_pipeline;
} sge_pipeline;

typedef struct sge_render_settings sge_render_settings;
typedef struct sge_render sge_render;

typedef struct sge_renderer_interface {
        SGE_RESULT (*initialize)(sge_render *render, sge_render_settings *settings);
        SGE_RESULT (*shutdown)(sge_render *render);
        SGE_RESULT (*begin_frame)(sge_render *render);
        SGE_RESULT (*draw)(sge_render *render);
        SGE_RESULT (*end_frame)(sge_render *render);
        SGE_RESULT (*set_pipeline)(sge_render *render, sge_pipeline *pipeline);
        SGE_RESULT (*update_uniform)(sge_render *render);
} sge_renderer_interface;

#include "sge_camera.h"
typedef struct sge_render {
        render_api              api;
        void                    *api_context;
        sge_window              *window;
        sge_renderer_interface  *sge_interface;
        sge_renderable          sge_renderables;
        int                     sge_renderables_count;
        sge_camera              camera;
} sge_render;



#include "vulkan_renderer/vulkan_renderer.h"

typedef struct sge_vulkan_render_settings {
        bool                    enable_validation_layers;
        bool                    use_dynamic_rendering;
        bool                    use_sge_allocator; //resource expensive, use at debug if u want to
        vulkan_app_info         app_info;
} sge_vulkan_render_settings;

typedef struct sge_opengl_render_settings {

} sge_opengl_render_settings;

typedef struct sge_directx_render_settings {

} sge_directx_render_settings;

typedef struct sge_render_settings {
        sge_vulkan_render_settings vulkan;
        sge_opengl_render_settings opengl;
        sge_directx_render_settings directx;
} sge_render_settings;




sge_render *sge_render_create(render_api api, sge_window *window);
SGE_RESULT sge_render_initialize(sge_render *render, sge_render_settings *settings);

SGE_RESULT sge_draw_frame(sge_render *render);
SGE_RESULT sge_begin_frame(sge_render *render);
SGE_RESULT sge_draw(sge_render *render, sge_renderable *renderable, sge_pipeline *pipeline);
SGE_RESULT sge_end_frame(sge_render *render);

sge_mesh *sge_mesh_create(sge_render *render, void *vertices, uint32_t vertex_size, uint32_t vertex_count, uint32_t *indices, uint32_t index_count, sge_vertex_format *format);
sge_shader *sge_shader_create();
sge_renderable *sge_renderable_create(sge_mesh *mesh, sge_material *material);
sge_pipeline *get_default_pipeline();
sge_pipeline *sge_pipeline_create();
SGE_RESULT sge_render_shutdown(sge_render *render);


SGE_RESULT sge_move_forward(sge_render *render);
SGE_RESULT sge_move_left(sge_render *render);
SGE_RESULT sge_move_backwards(sge_render *render);
SGE_RESULT sge_move_right(sge_render *render);

SGE_RESULT sge_enable_mouse_movement_tracking(sge_render *render);
SGE_RESULT sge_disable_mouse_movement_tracking(sge_render *render);



sge_mesh *create_logo_mesh(sge_render *render);
sge_renderable *create_logo_renderable(sge_render *render);

SGE_RESULT sge_add_renderable(sge_render *render, sge_renderable *renderable);

#endif //SGE_RENDER_H
