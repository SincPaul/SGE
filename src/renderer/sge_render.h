//
// Created by Geisthardt on 06.03.2025.
//

#ifndef SGE_RENDER_H
#define SGE_RENDER_H


#include <stdbool.h>

#include "../utils/sge_math.h"
#include "../core/os_specific/sge_window.h"
#include "../SGE.h"
#include "sge_render_file.h"

#include <stdio.h>
#include <vulkan/vulkan.h>

//#include "../core/logging.h"
//#include "../core/memory_control.h"

typedef enum sge_render_api {
        RENDER_API_VULKAN,
        RENDER_API_DIRECTX,
        RENDER_API_OPENGL
} render_api;

typedef enum SGE_GEOMETRY_TYPE {
        SGE_GEOMETRY_2D,
        SGE_GEOMETRY_3D
} SGE_GEOMETRY_TYPE;

typedef struct sge_buffer {
        void            *api_handle;
        size_t          size;
        void            *data;
} sge_buffer;

typedef struct sge_vertex_attribute {
        uint32_t        location;
        uint32_t        format;
        uint32_t        components;
        uint32_t        offset;
} sge_vertex_attribute;

typedef struct sge_vertex_format {
        uint32_t                stride;
        sge_vertex_attribute    *attributes;
        size_t                  attribute_count;
} sge_vertex_format;

typedef struct sge_shader { //SPIR-V vulkan, HLSL directx, GLSL opengl
        render_api api;
        void *api_shader;
} sge_shader;

//todo use
typedef struct sge_pipeline_settings {
        sge_vertex_format       *vertex_format;

        SGE_PRIMITIVE_TYPE      topology;
        uint32_t                patch_control_points;

        SGE_POLYGON_MODE        polygon_mode;
        float                   line_width;

        SGE_CULL_MODE           cull_mode;
        SGE_FRONT_FACE          front_face;
        SGE_BOOL                tesselation_enabled;
        SGE_BOOL                geometry_enabled;

        SGE_MSAA_COUNT          msaa_count;
        SGE_BOOL                per_sample_shading_enabled;
        float                   min_sample_shading;

        SGE_BOOL                is_3d;
} sge_pipeline_settings;

typedef struct sge_mesh {
        sge_buffer              vertex_buffer;
        sge_buffer              index_buffer;
        uint32_t                vertex_size;
        uint32_t                attribute_count;
        uint32_t                vertex_count;
        uint32_t                index_count;
        SGE_MESH_ATTRIBUTE      *attributes;
        sge_vertex_format       *format;
        char                    name[64];
} sge_mesh;

typedef struct sge_material {
        sge_shader      *vertex_shader;
        sge_shader      *fragment_shader;
        vec4            color;
        char name[64];
} sge_material;

typedef struct sge_renderable {
        sge_mesh        *mesh;
        sge_material    *material;
        char            name[64];
        void            *pipeline;
        void            *pipeline_layout;
} sge_renderable;

typedef struct sge_pipeline {
        render_api api;
        void *api_pipeline;
} sge_pipeline;

typedef struct sge_viewport {
        float x;                //vulkan types maybe add more when opengl and so added
        float y;                //vulkan types maybe add more when opengl and so added
        float width;            //vulkan types maybe add more when opengl and so added
        float height;           //vulkan types maybe add more when opengl and so added
        float min_depth;        //vulkan types maybe add more when opengl and so added
        float max_depth;        //vulkan types maybe add more when opengl and so added
} sge_viewport;

typedef struct sge_scissor {
        float offset_x; //vulkan types maybe add more when opengl and so on added
        float offset_y; //vulkan types maybe add more when opengl and so on added
        float extent_width; //vulkan types maybe add more when opengl and so on added
        float extent_height; //vulkan types maybe add more when opengl and so on added
} sge_scissor;


typedef struct sge_render_settings sge_render_settings;
typedef struct sge_render sge_render;
typedef struct sge_uniform_buffer_type sge_uniform_buffer_type;
typedef struct sge_window sge_window;
#include "sge_region.h"
typedef struct sge_renderer_interface {
        SGE_RESULT (*initialize)(sge_render *render, sge_render_settings *settings);
        SGE_RESULT (*shutdown)(sge_render *render);
        SGE_RESULT (*begin_frame)(sge_render *render);
        SGE_RESULT (*draw)(sge_render *render);
        SGE_RESULT (*end_frame)(sge_render *render);
        SGE_RESULT (*set_pipeline)(sge_render *render, sge_pipeline *pipeline);
        SGE_RESULT (*update_uniform)(sge_render *render, sge_region *region);
        SGE_RESULT (*create_buffer)(sge_render *render, void **buffer_ptr);                             // !! uniform buffer !! todo to make setting
        SGE_RESULT (*allocate_buffer_memory)(sge_render *render, void **memory_ptr, void *buffer);      // !! uniform buffer !! todo to make setting
        SGE_RESULT (*create_descriptor_pool)(sge_render *render, void *pool_ptr);
        SGE_RESULT (*allocate_descriptor_set)(sge_render *render, void *descriptor_ptr, void *layout_ptr, void *descriptor_pool);
        SGE_RESULT (*update_descriptor_set)(sge_render *render, sge_uniform_buffer_type *buffer);
        SGE_RESULT (*create_renderable_resources)(sge_render *render, sge_renderable *renderable);
} sge_renderer_interface;

#include "sge_camera.h"
typedef struct sge_view {
        sge_viewport                    viewport;
        sge_scissor                     scissor;
        sge_camera                      *camera;
        sge_uniform_buffer_object       ubo_transform;
        sge_renderable                  **renderables;
        size_t                          renderable_count;
} sg_view;

typedef struct sge_render {
        render_api              api;
        void                    *api_context;
        sge_window              *window;
        sge_renderer_interface  *sge_interface;
        sge_region              **regions;
        size_t                  region_count;
} sge_render;



#include "vulkan_renderer/vulkan_renderer.h"

typedef struct sge_vulkan_render_settings {
        bool                    enable_validation_layers;
        bool                    use_dynamic_rendering;
        bool                    use_sge_allocator; //resource expensive/slow, use at debug if u want to - todo improve is needed if more than 1024 allocation done
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

//sge_mesh *sge_mesh_create(sge_render *render, void *vertices, uint32_t vertex_size, uint32_t vertex_count, uint32_t *indices, uint32_t index_count, sge_vertex_format *format);
//sge_shader *sge_shader_create();
sge_renderable *sge_renderable_create(sge_mesh *mesh, sge_material *material);
//sge_pipeline *get_default_pipeline();
//sge_pipeline *sge_pipeline_create();
//SGE_RESULT sge_render_shutdown(sge_render *render);


SGE_RESULT sge_move_forward(sge_render *render);
SGE_RESULT sge_move_left(sge_render *render);
SGE_RESULT sge_move_backwards(sge_render *render);
SGE_RESULT sge_move_right(sge_render *render);

SGE_RESULT sge_enable_mouse_movement_tracking(sge_render *render);
SGE_RESULT sge_disable_mouse_movement_tracking(sge_render *render);



sge_mesh *create_logo_mesh(sge_render *render);
sge_renderable *create_logo_renderable(sge_render *render);


#include "sge_render_file.h"
sge_renderable *create_renderable_from_rend_file(sge_render *render, SGE_REND_FILE *file);


SGE_RESULT sge_create_buffer(sge_render *render, void **buffer_ptr);
SGE_RESULT sge_allocate_buffer(sge_render *render, void **memory_ptr, void *buffer);
SGE_RESULT sge_create_descriptor_pool(sge_render *render, void *pool_ptr);
SGE_RESULT sge_allocate_descriptor_set(sge_render *render, void *descriptor_ptr, void *layout_ptr, void *descriptor_pool);
SGE_RESULT sge_update_descriptor_set(sge_render *render, sge_uniform_buffer_type *buffer);

SGE_RESULT sge_renderable_create_api_resources(sge_render *render, sge_renderable *renderable);
#endif //SGE_RENDER_H

