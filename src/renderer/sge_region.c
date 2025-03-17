//
// Created by Geisthardt on 14.03.2025.
//

#include "sge_region.h"

#include <stdio.h>

#include "../core/input.h"
#include "../core/logging.h"
#include "../core/memory_control.h"

extern bool is_hidden;
extern mouse_pos last_visible_pos;


sge_region *sge_region_create(sge_render *render, sge_region_settings *settings) {
        sge_region *region = allocate_memory(sizeof(sge_region), MEMORY_TAG_REGION);
        if (region == NULL) {
                log_event(LOG_LEVEL_FATAL, "failed to allocate for region");
                return NULL;
        }

        region->viewport = allocate_memory(sizeof(sge_viewport), MEMORY_TAG_REGION);
        if (region->viewport == NULL) {
                log_event(LOG_LEVEL_FATAL, "failed to allocate for viewport");
                return NULL;
        }

        region->scissor = allocate_memory(sizeof(sge_scissor), MEMORY_TAG_REGION);
        if (region->scissor == NULL) {
                log_event(LOG_LEVEL_FATAL, "failed to allocate for scissor");
                return NULL;
        }

        region->camera = allocate_memory(sizeof(sge_camera), MEMORY_TAG_REGION);
        if (region->camera == NULL) {
                log_event(LOG_LEVEL_FATAL, "failed to allocate for camera");
                return NULL;
        }


        region->viewport->width = settings->width;
        region->viewport->height = settings->height;
        region->viewport->x = settings->offset_x;
        region->viewport->y = settings->offset_y;
        region->viewport->min_depth = settings->min_depth;
        region->viewport->max_depth = settings->max_depth;

        region->scissor->extent_height = settings->height;
        region->scissor->extent_width = settings->width;
        region->scissor->offset_x = settings->offset_x;
        region->scissor->offset_y = settings->offset_y;


        region->region_index = render->region_count;
        region->auto_scale_on_resize = settings->auto_scale_on_resize;
        region->reposition_on_resize = settings->auto_reposition_on_resize;
        region->auto_update_frames = settings->auto_update;
        region->z_index = settings->z_index;

        region->type = &settings->type;

        log_event(LOG_LEVEL_INFO, "creating vk descriptor pool");
        SGE_RESULT descriptor_pool_result = sge_create_descriptor_pool(render, &region->descriptor_pool);
        if (descriptor_pool_result != SGE_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "failed to create descriptor pool");
                return NULL;
        }

        //todo based on frames in flight
        for (int i = 0; i < 3; ++i) {
                log_event(LOG_LEVEL_INFO, "creating region buffer");
                SGE_RESULT buffer_result = sge_create_buffer(render, &region->uniform_buffers[i].buffer);
                if (buffer_result != SGE_SUCCESS) {
                        log_event(LOG_LEVEL_FATAL, "failed to create buffer for region");
                        return NULL;
                }

                log_event(LOG_LEVEL_INFO, "allocating region buffer");
                SGE_RESULT allocation_result = sge_allocate_buffer(render, &region->uniform_buffers[i].memory, region->uniform_buffers[i].buffer);
                if (allocation_result != SGE_SUCCESS) {
                        log_event(LOG_LEVEL_FATAL, "failed to allocate memory for region buffer");
                        return NULL;
                }

                log_event(LOG_LEVEL_INFO, "creating descriptor set");
                SGE_RESULT descriptor_set_result = sge_allocate_descriptor_set(render, &region->uniform_buffers[i].descriptor, &region->uniform_buffers[i].descriptor_layout, region->descriptor_pool);
                if (descriptor_set_result != SGE_SUCCESS) {
                        log_event(LOG_LEVEL_FATAL, "failed to set descriptor for region");
                        return NULL;
                }

                log_event(LOG_LEVEL_INFO, "writing to descriptor set");
                SGE_RESULT descriptor_write_result = sge_update_descriptor_set(render, &region->uniform_buffers[i]);
                if (descriptor_write_result != SGE_SUCCESS) {
                        log_event(LOG_LEVEL_FATAL, "failed to write descriptor set");
                        return NULL;
                }
        }


        render->region_count++;
        render->regions = reallocate_memory(render->regions, sizeof(sge_region *) * render->region_count, MEMORY_TAG_REGION);
        render->regions[render->region_count -1] = region;


        log_event(LOG_LEVEL_INFO, "created region");

        return region;
}


SGE_RESULT sge_region_add_renderable(sge_region *region, sge_renderable *renderable) {
        region->renderable_count++;
        region->renderables = reallocate_memory(region->renderables, sizeof(sge_renderable *) * region->renderable_count, MEMORY_TAG_RENDERER);
        region->renderables[region->renderable_count - 1] = renderable;

        return SGE_SUCCESS;
}

SGE_RESULT sge_region_resize_auto_resizing_regions(sge_render *render, float old_width, float old_height, float new_width, float new_height) {
        if (new_height == 0 ||new_width == 0) {
                return SGE_SUCCESS;
        }
        float resize_factor_width = new_width / old_width;
        float resize_factor_height = new_height / old_height;
        printf("%f, %f", resize_factor_width, resize_factor_height);

        for (int i = 0; i < render->region_count; ++i) {
                sge_region *region = render->regions[i];
                if (region->auto_scale_on_resize) {
                        if (region->viewport->width == -1000) {
                                region->viewport->width == new_width;
                                region->scissor->extent_width = new_width / 2;
                        } else {
                                region->viewport->width *= resize_factor_width;
                                region->scissor->extent_width *= resize_factor_width;
                        }
                        if (region->viewport->height == -1000) {
                                region->viewport->height == new_height;
                                region->scissor->extent_height = new_height;
                        } else {
                                region->viewport->height *= resize_factor_height;
                                region->scissor->extent_height *= resize_factor_height;
                        }
                }

                if (region->reposition_on_resize) {

                        region->viewport->x *= resize_factor_width;
                        region->scissor->offset_x *= resize_factor_width;

                        if (region->viewport->y == 0) {
                                region->viewport->y *= resize_factor_height;
                                region->scissor->offset_y *= resize_factor_height;
                        }
                        else {
                                float old_bottom_offset = old_height - region->viewport->y;
                                float new_bottom_offset = old_bottom_offset * resize_factor_height;
                                region->viewport->y = new_height - new_bottom_offset;

                                old_bottom_offset = old_height - region->scissor->offset_y;
                                new_bottom_offset = old_bottom_offset * resize_factor_height;
                                region->scissor->offset_y = new_height - new_bottom_offset;
                        }

                }


        }

        return SGE_SUCCESS;
}

sge_region *sge_region_get_active(sge_render *render) {
        mouse_pos current_mouse_pos;
        if (is_hidden) {
                current_mouse_pos = last_visible_pos;
        } else {
                current_mouse_pos = get_mouse_position();
        }

        current_mouse_pos = get_window_mouse_pos(render, current_mouse_pos);

        sge_region *active_region = NULL;
        int max_z_index = INT_MIN;

        for (int i = 0; i < render->region_count; ++i) {
                sge_region *region = render->regions[i];

                int left = region->viewport->x;
                int top = region->viewport->y;
                int right = (region->viewport->width == -1000) ? render->window->width : region->viewport->width + left;
                int bottom = (region->viewport->height == -1000) ? render->window->height : region->viewport->height + top;

                //printf("TOP: %i, LEFT: %i, RIGHT: %i, BOTTOM: %i\nX: %i, Y: %i\n\n", top, left, right, bottom, current_mouse_pos.x, current_mouse_pos.y);

                if (current_mouse_pos.x >= left
                &&  current_mouse_pos.x <= right
                &&  current_mouse_pos.y >= top
                &&  current_mouse_pos.y <= bottom) {
                        if (region->z_index > max_z_index) {
                                active_region = region;
                                max_z_index = region->z_index;
                        }
                }
        }
        return active_region;

}

int compare_z_index(const void *a, const void *b) {
        const sge_region *region_a = *(const sge_region **)a;
        const sge_region *region_b = *(const sge_region **)b;
        if (region_a == NULL && region_b == NULL) {
                return 0;
        }
        if (region_a == NULL) {
                return 1;
        }
        if (region_b == NULL) {
                return -1;
        }
        return region_b->z_index - region_a->z_index;
}

sge_region **sge_region_get_active_list(sge_render *render, int *regions_count) {
        if (render == NULL) {
                return NULL;
        }
        mouse_pos current_mouse_pos;
        sge_region **regions = allocate_memory(sizeof(sge_region*) * render->region_count, MEMORY_TAG_REGION);
        if (regions == NULL) {
                log_event(LOG_LEVEL_FATAL, "Failed to allocate for region list");
                return NULL;
        }
        if (is_hidden) {
                current_mouse_pos = last_visible_pos;
        } else {
                current_mouse_pos = get_mouse_position();
        }

        current_mouse_pos = get_window_mouse_pos(render, current_mouse_pos);

        *regions_count = 0;

        for (int i = 0; i < render->region_count; ++i) {
                sge_region *region = render->regions[i];

                int left = region->viewport->x;
                int top = region->viewport->y;
                int right = (region->viewport->width == SGE_REGION_FULL_DIMENSION) ? render->window->width : region->viewport->width + left;
                int bottom = (region->viewport->height == SGE_REGION_FULL_DIMENSION) ? render->window->height : region->viewport->height + top;

                //printf("TOP: %i, LEFT: %i, RIGHT: %i, BOTTOM: %i\nX: %i, Y: %i\n\n", top, left, right, bottom, current_mouse_pos.x, current_mouse_pos.y);

                if (current_mouse_pos.x >= left
                &&  current_mouse_pos.x <= right
                &&  current_mouse_pos.y >= top
                &&  current_mouse_pos.y <= bottom) {
                        regions[*regions_count] = region;
                        (*regions_count)++;
                }
        }



        if (*regions_count > 1) {
                qsort(regions, *regions_count, sizeof(sge_region*), compare_z_index);
        }

        return regions;
}
