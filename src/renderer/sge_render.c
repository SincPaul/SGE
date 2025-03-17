//
// Created by Geisthardt on 06.03.2025.
//

#include "sge_render.h"

#include <stdio.h>

#include "../core/logging.h"
#include "../core/memory_control.h"
#include "vulkan_renderer/sge_vulkan_shader.h"

sge_render *sge_render_create(render_api api, sge_window *window) {
        sge_render *renderer = allocate_memory(sizeof(sge_render), MEMORY_TAG_RENDERER);
        if (renderer == NULL) {
                log_event(LOG_LEVEL_FATAL, "Failed to allocate memory for renderer");
                return NULL;
        }

        renderer->api = api;
        renderer->api_context = NULL;
        renderer->window = window;
        renderer->sge_interface = allocate_memory(sizeof(sge_renderer_interface), MEMORY_TAG_RENDERER);
        if (renderer->sge_interface == NULL) {
                log_event(LOG_LEVEL_FATAL, "failed allocate memory for sge interface");
                return NULL;
        }

        switch (api) {
                case RENDER_API_VULKAN: {
                        renderer->sge_interface = &sge_vulkan_interface;
                        renderer->api_context = allocate_memory(sizeof(sge_vulkan_context), MEMORY_TAG_VULKAN);
                        if (renderer->api_context == NULL) {
                                log_event(LOG_LEVEL_FATAL, "Failed allocate vulkan api context");
                                return NULL;
                        }
                } break;
                case RENDER_API_OPENGL: {
                        log_event(LOG_LEVEL_FATAL, "OpenGL not yet implemented");
                        return NULL;
                } break;
                case RENDER_API_DIRECTX: {
                        log_event(LOG_LEVEL_FATAL, "DirectX not yet implemented");
                        return NULL;
                } break;
                default: {
                        log_event(LOG_LEVEL_FATAL, "unsupported api");
                        return NULL;
                }
        }
        return renderer;
}

SGE_RESULT sge_render_initialize(sge_render *render, sge_render_settings *render_settings) {
        if (!render->sge_interface->initialize(render, render_settings)) {
             return SGE_ERROR;
        }

        log_event(LOG_LEVEL_INFO, "FINISHED RENDER INITIALIZATION");

        return SGE_SUCCESS;
}

SGE_RESULT sge_draw_frame(sge_render *render) {
        if (!render->sge_interface->draw(render)) {
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}

SGE_RESULT sge_begin_frame(sge_render *render) {
        if (!render->sge_interface->begin_frame(render)) {
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}

SGE_RESULT sge_end_frame(sge_render *render) {
        if (!render->sge_interface->end_frame(render)) {
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}


SGE_RESULT sge_create_buffer(sge_render *render, void **buffer_ptr) {
        if (render->sge_interface->create_buffer(render, buffer_ptr) != SGE_SUCCESS) {
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}

SGE_RESULT sge_allocate_buffer(sge_render *render, void **memory_ptr, void *buffer) {
        if (render->sge_interface->allocate_buffer_memory(render, memory_ptr, buffer) != SGE_SUCCESS) {
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}

SGE_RESULT sge_create_descriptor_pool(sge_render *render, void *pool_ptr) {
        if (render->sge_interface->create_descriptor_pool(render, pool_ptr) != SGE_SUCCESS) {
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}

SGE_RESULT sge_allocate_descriptor_set(sge_render *render, void *descriptor_ptr, void *layout_ptr, void *descriptor_pool) {
        if (render->sge_interface->allocate_descriptor_set(render, descriptor_ptr, layout_ptr, descriptor_pool) != SGE_SUCCESS) {
                return SGE_ERROR;
        }

        return SGE_SUCCESS;
}

SGE_RESULT sge_update_descriptor_set(sge_render *render, sge_uniform_buffer_type *buffer) {
        if (render->sge_interface->update_descriptor_set(render, buffer) != SGE_SUCCESS) {
                return SGE_ERROR;
        }
        return SGE_SUCCESS;
}
//todo dynamic array
//SGE_RESULT sge_add_renderable(sge_render *render, sge_renderable *renderable) {
//        render->sge_renderables = *renderable;
//        render->sge_renderables_count = 1;


//        return SGE_SUCCESS;
//}

sge_mesh *create_logo_mesh(sge_render *render) {

        sge_vulkan_context *vk_context = (sge_vulkan_context*)render->api_context;

        const float vertices[] = {
                //S
                -0.4, -0.5, 0.0,
                -0.4, -0.6, 0.0, // Oben balken
                -0.8, -0.6, 0.0,

                -0.8, -0.6, 0.0,
                -0.8, -0.5, 0.0,
                -0.4, -0.5, 0.0,


                -0.8, -0.5, 0.0,
                -0.8, -0.1, 0.0, //links blaken
                -0.7, -0.1, 0.0,

                -0.7, -0.1, 0.0,
                -0.7, -0.5, 0.0,
                -0.8, -0.5, 0.0,


                -0.8, -0.1, 0.0,
                -0.8, 0.1, 0.0, //mittel ding
                -0.4, 0.1, 0.0,

                -0.8, -0.1, 0.0,
                -0.4, -0.1, 0.0,
                -0.4, 0.1, 0.0,


                -0.5, 0.5, 0.0,
                -0.4, 0.1, 0.0, //rechts unten balkne
                -0.4, 0.5, 0.0,

                -0.5, 0.5, 0.0,
                -0.4, 0.1, 0.0,
                -0.5, 0.1, 0.0,


                -0.4, 0.5, 0.0,
                -0.4, 0.6, 0.0, //unten balken
                -0.8, 0.6, 0.0,

                -0.8, 0.6, 0.0,
                -0.8, 0.5, 0.0,
                -0.4, 0.5, 0.0,


                //G
                0.2, -0.5, 0.0,
                0.2, -0.6, 0.0, //oben balken
                -0.2, -0.6, 0.0,

                -0.2, -0.6, 0.0,
                -0.2, -0.5, 0.0,
                0.2, -0.5, 0.0,


                -0.2, -0.5, 0.0,
                -0.2, 0.5, 0.0, //links großer balken
                -0.1, 0.5, 0.0,

                -0.1, -0.5, 0.0,
                -0.1, 0.5, 0.0,
                -0.2, -0.5, 0.0,


                -0.05, -0.1, 0.0,
                -0.05, 0.1, 0.0, //mitte blaken horizontal
                0.2, 0.1, 0.0,

                -0.05, -0.1, 0.0,
                0.2, -0.1, 0.0,
                0.2, 0.1, 0.0,

                -0.05, 0.1, 0.0,
                -0.05, 0.3, 0.0, //mitte balken vertikal
                0.02, 0.1, 0.0,

                -0.05, 0.3, 0.0,
                0.02, 0.3, 0.0,
                0.02, 0.1, 0.0,


                0.1, 0.5, 0.0,
                0.2, 0.1, 0.0, //unten balken
                0.2, 0.5, 0.0,

                0.1, 0.5, 0.0,
                0.2, 0.1, 0.0,
                0.1, 0.1, 0.0,


                0.2, 0.5, 0.0,
                0.2, 0.6, 0.0, //rechts halber balken
                -0.2, 0.6, 0.0,

                -0.2, 0.6, 0.0,
                -0.2, 0.5, 0.0,
                0.2, 0.5, 0.0,


                //E
                0.8, -0.5, 0.0,
                0.8, -0.6, 0.0, //oben
                0.4, -0.6, 0.0,

                0.4, -0.6, 0.0,
                0.4, -0.5, 0.0,
                0.8, -0.5, 0.0,


                0.4, -0.5, 0.0,
                0.4, 0.5, 0.0, //links
                0.5, 0.5, 0.0,

                0.5, 0.5, 0.0,
                0.5, -0.5, 0.0,
                0.4, -0.5, 0.0,


                0.4, -0.1, 0.0,
                0.4, 0.1, 0.0, //mitte
                0.8, 0.1, 0.0,

                0.4, -0.1, 0.0,
                0.8, -0.1, 0.0,
                0.8, 0.1, 0.0,


                0.8, 0.5, 0.0,
                0.8, 0.6, 0.0, //unten
                0.4, 0.6, 0.0,

                0.4, 0.6, 0.0,
                0.4, 0.5, 0.0,
                0.8, 0.5, 0.0,





                //BACK SIDE

                //S
                -0.4, -0.5, 1.0,
                -0.4, -0.6, 1.0, // Oben balken
                -0.8, -0.6, 1.0,

                -0.8, -0.6, 1.0,
                -0.8, -0.5, 1.0,
                -0.4, -0.5, 1.0,


                -0.8, -0.5, 1.0,
                -0.8, -0.1, 1.0, //links blaken
                -0.7, -0.1, 1.0,

                -0.7, -0.1, 1.0,
                -0.7, -0.5, 1.0,
                -0.8, -0.5, 1.0,


                -0.8, -0.1, 1.0,
                -0.8, 0.1,  1.0, //mittel ding
                -0.4, 0.1,  1.0,

                -0.8, -0.1, 1.0,
                -0.4, -0.1, 1.0,
                -0.4, 0.1,  1.0,


                -0.5, 0.5, 1.0,
                -0.4, 0.1, 1.0, //rechts unten balkne
                -0.4, 0.5, 1.0,

                -0.5, 0.5, 1.0,
                -0.4, 0.1, 1.0,
                -0.5, 0.1, 1.0,


                -0.4, 0.5, 1.0,
                -0.4, 0.6, 1.0, //unten balken
                -0.8, 0.6, 1.0,

                -0.8, 0.6, 1.0,
                -0.8, 0.5, 1.0,
                -0.4, 0.5, 1.0,


                //G
                0.2, -0.5,  1.0,
                0.2, -0.6,  1.0, //oben balken
                -0.2, -0.6, 1.0,

                -0.2, -0.6, 1.0,
                -0.2, -0.5, 1.0,
                0.2, -0.5,  1.0,


                -0.2, -0.5, 1.0,
                -0.2, 0.5,  1.0, //links großer balken
                -0.1, 0.5,  1.0,

                -0.1, -0.5, 1.0,
                -0.1, 0.5,  1.0,
                -0.2, -0.5, 1.0,


                -0.05, -0.1, 1.0,
                -0.05, 0.1,  1.0, //mitte blaken horizontal
                0.2, 0.1,    1.0,

                -0.05, -0.1, 1.0,
                0.2, -0.1,   1.0,
                0.2, 0.1,    1.0,

                -0.05, 0.1, 1.0,
                -0.05, 0.3, 1.0, //mitte balken vertikal
                0.02, 0.1,  1.0,

                -0.05, 0.3, 1.0,
                0.02, 0.3,  1.0,
                0.02, 0.1,  1.0,


                0.1, 0.5, 1.0,
                0.2, 0.1, 1.0, //unten balken
                0.2, 0.5, 1.0,

                0.1, 0.5, 1.0,
                0.2, 0.1, 1.0,
                0.1, 0.1, 1.0,


                0.2, 0.5,  1.0,
                0.2, 0.6,  1.0, //rechts halber balken
                -0.2, 0.6, 1.0,

                -0.2, 0.6, 1.0,
                -0.2, 0.5, 1.0,
                0.2, 0.5,  1.0,


                //E
                0.8, -0.5, 1.0,
                0.8, -0.6, 1.0, //oben
                0.4, -0.6, 1.0,

                0.4, -0.6, 1.0,
                0.4, -0.5, 1.0,
                0.8, -0.5, 1.0,


                0.4, -0.5, 1.0,
                0.4, 0.5,  1.0, //links
                0.5, 0.5,  1.0,

                0.5, 0.5,  1.0,
                0.5, -0.5, 1.0,
                0.4, -0.5, 1.0,


                0.4, -0.1, 1.0,
                0.4, 0.1,  1.0, //mitte
                0.8, 0.1,  1.0,

                0.4, -0.1, 1.0,
                0.8, -0.1, 1.0,
                0.8, 0.1,  1.0,


                0.8, 0.5, 1.0,
                0.8, 0.6, 1.0, //unten
                0.4, 0.6, 1.0,

                0.4, 0.6, 1.0,
                0.4, 0.5, 1.0,
                0.8, 0.5, 1.0,
        };

        size_t num_floats = sizeof(vertices)/sizeof(vertices[0]);
        size_t num_vertices = num_floats / 3;

        sge_vertex_attribute *attribute = allocate_memory(sizeof(sge_vertex_attribute), MEMORY_TAG_RENDERER);
        attribute[0].location = 0;
        attribute[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute[0].offset = 0;

        sge_vertex_format *format = allocate_memory(sizeof(sge_vertex_format), MEMORY_TAG_RENDERER);
        format->stride = 3 * sizeof(float);
        format->attributes = attribute;
        format->attribute_count = 1;

        VkBufferCreateInfo buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(vertices),
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };
        VkBuffer vertex_buffer;
        vkCreateBuffer(vk_context->device, &buffer_info, vk_context->sge_allocator, &vertex_buffer);

        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(vk_context->device, vertex_buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = mem_reqs.size,
            .memoryTypeIndex = find_memory_type(render, mem_reqs.memoryTypeBits,
                                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        };
        VkDeviceMemory vertex_memory;
        vkAllocateMemory(vk_context->device, &alloc_info, vk_context->sge_allocator, &vertex_memory);

        vkBindBufferMemory(vk_context->device, vertex_buffer, vertex_memory, 0);

        void *data;
        vkMapMemory(vk_context->device, vertex_memory, 0, buffer_info.size, 0, &data);
        memcpy(data, vertices, sizeof(vertices));
        vkUnmapMemory(vk_context->device, vertex_memory);

        sge_mesh *mesh = allocate_memory(sizeof(sge_mesh), MEMORY_TAG_RENDERER);
        mesh->vertex_buffer = vertex_buffer;
        mesh->vertex_memory = vertex_memory;
        mesh->index_buffer = VK_NULL_HANDLE;
        mesh->index_memory = VK_NULL_HANDLE;
        mesh->vertex_count = num_vertices;
        mesh->index_count = 0;
        mesh->format = format;

        return mesh;
}

sge_material *create_logo_material(sge_render *render) {
        sge_vulkan_context *vk_context = render->api_context;

        sge_shader *vert_shader = allocate_memory(sizeof(sge_shader), MEMORY_TAG_RENDERER);
        vert_shader->api = render->api;
        vert_shader->api_shader = sge_vulkan_shader_load(render, "simple_shader.vert.spv");
        sge_shader *frag_shader = allocate_memory(sizeof(sge_shader), MEMORY_TAG_RENDERER);
        frag_shader->api = render->api;
        frag_shader->api_shader = sge_vulkan_shader_load(render, "simple_shader.frag.spv");

        sge_material *material = allocate_memory(sizeof(sge_material), MEMORY_TAG_RENDERER);

        material->fragment_shader = frag_shader;
        material->vertex_shader = vert_shader;
        material->color = (vec4){1.0f, 1.0f, 1.0f, 1.0f};

        return material;


}


sge_renderable *create_logo_renderable(sge_render *render) {
        sge_mesh *logo_mesh = create_logo_mesh(render);
        sge_material *logo_material = create_logo_material(render);

        sge_renderable *logo_renderable = allocate_memory(sizeof(sge_renderable), MEMORY_TAG_RENDERER);
        logo_renderable->material = logo_material;
        logo_renderable->mesh = logo_mesh;

        return logo_renderable;
}


//SGE_RESULT sge_render_initializess(sge_render *renderer, render_settings *render_settings) {
//        if (!renderer) {
//                log_event(LOG_LEVEL_FATAL, "Tried to init renderer without passing sge_render struct");
//                return SGE_ERROR;
//        }
//
//        switch (renderer->api) {
//                case RENDER_API_VULKAN: {
//                        vulkan_context *vk_context = allocate_memory(sizeof(vulkan_context), MEMORY_TAG_VULKAN);
//                        if (vk_context == NULL) {
//                                log_event(LOG_LEVEL_FATAL, "Failed to allocate memory for VULKAN context");
//                                return SGE_ERROR;
//                        }
//
//                        renderer->api_context = vk_context;
//
//                        if (initialize_vk(vk_context, renderer->window, "SGE Engine", VK_MAKE_API_VERSION(0, 0, 1, 0)) != 0) {
//                                free_memory(vk_context, MEMORY_TAG_VULKAN);
//                                renderer->api_context = NULL;
//                                return SGE_ERROR;
//                        }
//                } break;
//                case RENDER_API_OPENGL: {
//
//                } break;
//                case RENDER_API_DIRECTX: {
//
//                } break;
//                default: {
//                        return SGE_ERROR;
//                }
//        }
//        return SGE_SUCCESS;
//}
//
//SGE_RESULT sge_render_shutdown(sge_render *render) {
//        switch (render->api) {
//                case RENDER_API_VULKAN: {
//                        if (!shutdown_vulkan(render->api_context)) {
//                             return SGE_ERROR;
//                        }
//                } break;
//                case RENDER_API_OPENGL: {
//
//                } break;
//                case RENDER_API_DIRECTX: {
//
//                } break;
//                default: {
//                        return SGE_ERROR;
//                }
//        }
//        return SGE_SUCCESS;
//}
//
//SGE_RESULT sge_render_initialize(sge_render *renderer, render_settings *render_settings) {
//        if (!renderer || !renderer->sge_interface || !renderer->sge_interface->initialize) {
//                log_event(LOG_LEVEL_FATAL, "Renderer or its interface is not initialized");
//                return SGE_ERROR;
//        }
//        return renderer->sge_interface->initialize(renderer->api_context, renderer->window, render_settings);
//}