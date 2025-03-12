//
// Created by Geisthardt on 10.03.2025.
//

#include "sge_vulkan_draw.h"

#include "sge_vulkan_resize.h"
#include "../../core/logging.h"

extern bool is_resize;

SGE_RESULT sge_vulkan_draw_frame(sge_render *render) {
        sge_vulkan_context *vk_context = render->api_context;
        if (is_resize) {
                handle_resize_out_of_date(render);
                is_resize = false;
                return SGE_RESIZE;
        }

        vkWaitForFences(vk_context->device, 1, &vk_context->so.in_flight_fences[vk_context->so.current_frame], VK_TRUE, UINT64_MAX);

        uint32_t image_index;
        VkResult acquire_result = vkAcquireNextImageKHR(vk_context->device, vk_context->swapchain, UINT64_MAX,
                                                        vk_context->so.image_available_semaphores[vk_context->so.current_frame],
                                                        VK_NULL_HANDLE, &image_index);
        if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR) {
            log_event(LOG_LEVEL_ERROR, "Failed to acquire next image");
            return SGE_ERROR;
        }

        vkResetFences(vk_context->device, 1, &vk_context->so.in_flight_fences[vk_context->so.current_frame]);


        VkCommandBuffer command_buffer = vk_context->command_buffer[vk_context->command_buffer_index];
        //log_event(LOG_LEVEL_TRACE, "beginning command buffer recording");
        vkResetCommandBuffer(command_buffer, 0);
        //log_event(LOG_LEVEL_TRACE, "reset cmd buffer");
        VkCommandBufferBeginInfo cmd_buf_begin_info = {0};
        cmd_buf_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        const VkResult rec_begin_result = vkBeginCommandBuffer(command_buffer, &cmd_buf_begin_info);

        if (rec_begin_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to begin cmd buffer recording: %d", rec_begin_result);
        }

        log_event(LOG_LEVEL_INFO, "Can now record into cmd buffer");

        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_context->pipeline_layout, 0, 1, &vk_context->descriptor_set, 0, NULL);

        vk_context->command_buffer_index = (vk_context->command_buffer_index + 1) % 3;

        VkImageMemoryBarrier barrier = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .srcQueueFamilyIndex = vk_context->graphics_queue_family_index,
                .dstQueueFamilyIndex = vk_context->graphics_queue_family_index,
                .image = vk_context->sc.sc_images[image_index],
                .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                },
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        };
        vkCmdPipelineBarrier(command_buffer,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             0, 0, NULL, 0, NULL,
                             1, &barrier);

        VkRenderingInfoKHR render_info = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .renderArea = {
                        .offset = {0, 0},
                        .extent = vk_context->sc.surface_capabilities.currentExtent
                },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &(VkRenderingAttachmentInfoKHR){
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                        .imageView = vk_context->sc.sc_views[image_index],
                        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        .clearValue = {.color = {0.0f, 0.0f, 0.0f, 0.0f}}
                }
        };

        vkCmdBeginRendering(command_buffer, &render_info);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_context->pipeline);

        VkViewport viewport = {
                .x = 0.0f,
                .y = 0.0f,
                .width = (float)vk_context->sc.surface_capabilities.currentExtent.width,
                .height = (float)vk_context->sc.surface_capabilities.currentExtent.height,
                .minDepth = 0.0f,
                .maxDepth = 1.0f
        };
        VkRect2D scissor = {
                .offset = {0, 0},
                .extent = vk_context->sc.surface_capabilities.currentExtent
        };
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        for (int i = 0; i < render->sge_renderables_count; ++i) {
                log_event(LOG_LEVEL_INFO, "rendering renderable");
                sge_renderable *renderable = &render->sge_renderables;

                VkBuffer vertex_buffers[] = { renderable->mesh->vertex_buffer };
                VkDeviceSize offsets[] = { 0 };

                vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
                vkCmdDraw(command_buffer, renderable->mesh->vertex_count, 1, 0, 0);
        }
        vkCmdEndRendering(command_buffer);

        barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;
        vkCmdPipelineBarrier(command_buffer,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             0, 0, NULL, 0, NULL,
                             1, &barrier);

        const VkResult rec_end_result = vkEndCommandBuffer(command_buffer);
        if (rec_end_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to end cmd buffer recording: %d", rec_end_result);
        }

        log_event(LOG_LEVEL_INFO, "Finished cmd buffer recording");

        VkSubmitInfo submit_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &vk_context->so.image_available_semaphores[vk_context->so.current_frame],
                .pWaitDstStageMask = (VkPipelineStageFlags[]){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
                .commandBufferCount = 1,
                .pCommandBuffers = &command_buffer,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &vk_context->so.render_finished_semaphores[vk_context->so.current_frame]
        };

        VkResult submit_result = vkQueueSubmit(vk_context->graphics_queue, 1, &submit_info,
                                               vk_context->so.in_flight_fences[vk_context->so.current_frame]);
        if (submit_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_ERROR, "Failed to submit queue");
                return SGE_ERROR;
        }

        VkPresentInfoKHR present_info = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &vk_context->so.render_finished_semaphores[vk_context->so.current_frame],
                .swapchainCount = 1,
                .pSwapchains = &vk_context->swapchain,
                .pImageIndices = &image_index
        };

        VkResult present_result = vkQueuePresentKHR(vk_context->present_queue, &present_info);
        if (present_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_ERROR, "Failed to present queue");
                return SGE_ERROR;
        }

        vk_context->so.current_frame = (vk_context->so.current_frame + 1) % 3;

        return SGE_SUCCESS;

    //if (!sge_begin_frame(render)) {
    //    log_event(LOG_LEVEL_FATAL, "Failed to begin frame");
    //    return SGE_ERROR;
    //}


    //sge_end_frame(render);
    return SGE_SUCCESS;
}


SGE_RESULT sge_vulkan_begin_frame(sge_render *render) {
    sge_vulkan_context *vk_context = render->api_context;

    return SGE_SUCCESS;
}


SGE_RESULT sge_vulkan_end_frame(sge_render *render) {
    sge_vulkan_context *vk_context = render->api_context;


    return SGE_SUCCESS;
}