//
// Created by Geisthardt on 07.03.2025.
//

#include "sge_vulkan_swapchain.h"
#include "../../core/logging.h"
#include "../../core/memory_control.h"


SGE_RESULT sge_vulkan_swapchain_create(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context*)render->api_context;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_context->physical_device, vk_context->surface, &vk_context->sc.surface_capabilities);
        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_context->physical_device, vk_context->surface, &format_count, NULL);
        VkSurfaceFormatKHR *formats = allocate_memory(format_count * sizeof(VkSurfaceFormatKHR), MEMORY_TAG_VULKAN);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_context->physical_device, vk_context->surface, &format_count, formats);
        vk_context->sc.surface_format = formats[0];
        free_memory(formats, MEMORY_TAG_VULKAN);

        VkSwapchainCreateInfoKHR create_info = {0};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = vk_context->surface;
        create_info.minImageCount = 2;
        create_info.imageFormat = vk_context->sc.surface_format.format;
        create_info.imageColorSpace = vk_context->sc.surface_format.colorSpace;
        create_info.imageExtent = vk_context->sc.surface_capabilities.currentExtent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.preTransform = vk_context->sc.surface_capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        create_info.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(vk_context->device, &create_info, NULL, &vk_context->swapchain) != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "failed to create swapchain");
                return SGE_ERROR;
        }

        vkGetSwapchainImagesKHR(vk_context->device, vk_context->swapchain, &vk_context->sc.sc_img_count, NULL);
        vk_context->sc.sc_images = allocate_memory(vk_context->sc.sc_img_count * sizeof(VkImage), MEMORY_TAG_VULKAN);
        vkGetSwapchainImagesKHR(vk_context->device, vk_context->swapchain, &vk_context->sc.sc_img_count, vk_context->sc.sc_images);

        vk_context->sc.sc_views = allocate_memory(vk_context->sc.sc_img_count * sizeof(VkImageView), MEMORY_TAG_VULKAN);
        for (uint32_t i = 0; i < vk_context->sc.sc_img_count; i++) {
                VkImageViewCreateInfo view_info = {0};
                view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                view_info.image = vk_context->sc.sc_images[i];
                view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                view_info.format = vk_context->sc.surface_format.format;
                view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                view_info.subresourceRange.levelCount = 1;
                view_info.subresourceRange.layerCount = 1;
                if (vkCreateImageView(vk_context->device, &view_info, vk_context->sge_allocator, &vk_context->sc.sc_views[i]) != VK_SUCCESS) {
                        log_event(LOG_LEVEL_FATAL, "failed creating image view");
                        return SGE_ERROR;
                }
        }
        return SGE_SUCCESS;
}

SGE_RESULT sge_vulkan_swapchain_destroy(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context*)render->api_context;

        vkDeviceWaitIdle(vk_context->device);

        for (uint32_t i = 0; i < vk_context->sc.sc_img_count; i++) {
                if (vk_context->sc.sc_views[i] != VK_NULL_HANDLE) {
                        vkDestroyImageView(vk_context->device, vk_context->sc.sc_views[i], vk_context->sge_allocator);
                }
        }
        free_memory(vk_context->sc.sc_views, MEMORY_TAG_VULKAN);
        vk_context->sc.sc_views = NULL;

        free_memory(vk_context->sc.sc_images, MEMORY_TAG_VULKAN);
        vk_context->sc.sc_images = NULL;

        if (vk_context->swapchain != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(vk_context->device, vk_context->swapchain, vk_context->sge_allocator);
                vk_context->swapchain = VK_NULL_HANDLE;
        }

        vk_context->sc.sc_img_count = 0;

        return SGE_SUCCESS;
}
