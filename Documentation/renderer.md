start by creating render struct, this will be the main way you interact with render api

input your window struct and the wanted graphics api 
it returns a render struct
```c++
sge_render *render = sge_render_create(RENDER_API_VULKAN, &window);
typedef enum render_api {
        RENDER_API_VULKAN,
        RENDER_API_DIRECTX,
        RENDER_API_OPENGL
} render_api

typedef struct sge_render {
        render_api              api;
        void                    *api_context;
        sge_window              *window;
        sge_renderer_interface  *sge_interface;
        sge_region              **regions;
        size_t                  region_count;
} sge_render;
typedef struct sge_window {
        sge_platform_handle     handle;
        int                     width;
        int                     height;
} sge_window;

#ifdef WIN32
        typedef struct sge_platform_handle {
                HWND hwnd;
                HINSTANCE hinstance;
        } sge_platform_handle;
#elif UNIX
        typedef struct sge_platform_handle {
                //whatever unix needs
        } sge_platform_handle
#endif
```

after crating a renderer instance you can start by initializing the renderer, this will do the initialization of the graphics api
returns SGE_TRUE on success else SGE_ERROR
```c++
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

SGE_RESULT sge_render_initialize(sge_render *render, sge_render_settings *render_settings);
```

after that you can create different regions in which you can render whatever you want
```c++
typedef enum sge_region_type {
        SGE_REGION_3D,
        SGE_REGION_2D
} sge_region_type;

typedef struct sge_region_settings {
        sge_region_type                 type;
        bool                            auto_update;
        bool                            auto_scale_on_resize;
        bool                            auto_reposition_on_resize;
        int                             width;
        int                             height;
        int                             offset_x;
        int                             offset_y;
        float                           min_depth;
        float                           max_depth;
        int                             z_index;
} sge_region_settings;

typedef struct sge_uniform_buffer_type {
        void                            *memory;
        void                            *buffer;
        void                            *descriptor;
        void                            *descriptor_layout;
} sge_uniform_buffer_type;

typedef struct sge_region {
        sge_region_type                 *type;
        sge_camera                      *camera;
        sge_viewport                    *viewport;
        sge_scissor                     *scissor;
        sge_renderable                  **renderables;
        size_t                          renderable_count;
        sge_uniform_buffer_object       transform_ubo;
        uint32_t                        region_index;
        bool                            auto_scale_on_resize;
        bool                            auto_update_frames;
        bool                            reposition_on_resize;
        sge_uniform_buffer_type         uniform_buffers[3];
        void                            *descriptor_pool;
        int                             z_index;
} sge_region;

sge_region *main_region = sge_region_create(render, &region_setting_3d);
```

this will return a region handle


to add renderable objects to the region call 

```c++
SGE_RESULT sge_region_add_renderable(sge_region *region, sge_renderable *renderable);
```

this will return SGE_SUCCESS on success 
else SGE_ERROR
#