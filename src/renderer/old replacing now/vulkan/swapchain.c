//
// Created by Geisthardt on 06.03.2025.
//

#include <stdio.h>

#include "vulkan_renderer.h"
#include "../../core/logging.h"
#include "../../core/memory_control.h"

bool destroy_swapchain(VkDevice device, VkSwapchainKHR swapchain, vulkan_context *vk_context) {
        vkDestroySwapchainKHR(device, swapchain, vk_context->sge_allocator);
        vk_context->swapchain = 0;
        return true;
}


VkSwapchainKHR create_swapchain(VkDevice device, VkSurfaceKHR surface, VkPhysicalDevice physical_device, vulkan_context *vk_context) {
        VkSwapchainKHR swapchain;

        uint32_t surface_format_count;
        VkResult count_result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, NULL);
        if (count_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to receive surface format count");
        }

        if (surface_format_count == 0) {
                log_event(LOG_LEVEL_FATAL, "no surface formats found");
        }

        VkSurfaceFormatKHR *surface_formats = allocate_memory(sizeof(VkSurfaceFormatKHR) * surface_format_count, MEMORY_TAG_VULKAN);
        if (surface_formats == NULL) {
                log_event(LOG_LEVEL_FATAL, "Failed to allocate memory for surface formats");
        }
        VkResult surface_format_result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats);

        //todo choose based on idk
        if (surface_formats == NULL) {
                log_event(LOG_LEVEL_FATAL, "failed to receive surface fomats");
                return NULL;
        }
        vk_context->sc.surface = surface_formats[0];

        if (surface_format_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to get physical device surface formats: %d", surface_format_result);
        }

        printf("Surface format format: %d\n", vk_context->sc.surface.format);

        printf("Surface format colorspace %d\n", vk_context->sc.surface.colorSpace);


        log_event(LOG_LEVEL_INFO, "Got physical device surface formats");

        uint32_t present_mode_count;
        VkPresentModeKHR *present_modes = NULL;

        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);

        present_modes = (VkPresentModeKHR*)allocate_memory(present_mode_count * sizeof(VkPresentModeKHR), MEMORY_TAG_VULKAN);
        VkResult present_mode_result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes);

        if (present_mode_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to receive Present Modes for device");
        }

        log_event(LOG_LEVEL_INFO, "Got physical device present modes");

        VkSurfaceCapabilitiesKHR surface_capabilities;
        VkResult capabilities_result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

        if (capabilities_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "failed to receive surface capabilities: %d", capabilities_result);
        }

        log_event(LOG_LEVEL_INFO, "Got Surface capabilities");

        vk_context->sc.surface_capabilities = surface_capabilities;
        for (uint32_t i = 0; i < present_mode_count; i++) {
                printf("PRESENT MODE: %d\n", present_modes[i]);
        }

        free_memory(present_modes, MEMORY_TAG_VULKAN);

        //todo improve and not hardcode everyting

        VkSwapchainCreateInfoKHR swapchain_create_info = {0};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = surface;
        swapchain_create_info.minImageCount = 3; //tripple buffer
        swapchain_create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        swapchain_create_info.imageFormat =  vk_context->sc.surface.format;
        swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageExtent = surface_capabilities.currentExtent;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchain_create_info.imageColorSpace = vk_context->sc.surface.colorSpace;
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = 0;


        VkResult swapchain_result = vkCreateSwapchainKHR(device, &swapchain_create_info, vk_context->sge_allocator, &swapchain);

        //todo handle out of date
        if (swapchain_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create swapchain: %d", swapchain_result);
        }

        log_event(LOG_LEVEL_INFO, "Created swapchain");

        vk_context->sc.sc_img_count = 0;

        VkResult swapchain_images_result = vkGetSwapchainImagesKHR(device, swapchain, &vk_context->sc.sc_img_count, 0);
        if (swapchain_images_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to retrieve swapchain image data: %d", swapchain_images_result);
        }
        if (vk_context->sc.sc_img_count <= 0) { //kack clion lügt, funktioniert einwandfrei
                log_event(LOG_LEVEL_FATAL, "failed to retrieve image count");
                return NULL;
        }
        if (!vk_context->sc.sc_images) {
                vk_context->sc.sc_images = allocate_memory(sizeof(VkImage) * vk_context->sc.sc_img_count, MEMORY_TAG_VULKAN);
        }
        if (!vk_context->sc.sc_views) {
                vk_context->sc.sc_views = allocate_memory(sizeof(VkImageView) * vk_context->sc.sc_img_count, MEMORY_TAG_VULKAN);
        }
        VkResult swapchain_images_result_2 = vkGetSwapchainImagesKHR(device, swapchain, &vk_context->sc.sc_img_count, vk_context->sc.sc_images);
        if (swapchain_images_result_2 != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to retrieve swapchain image data: %d", swapchain_images_result_2);
        }


        for (uint32_t i = 0; i < vk_context->sc.sc_img_count; i++) {
                VkImageViewCreateInfo view_create_info = {0};
                view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                view_create_info.image = vk_context->sc.sc_images[i];
                view_create_info.format =  vk_context->sc.surface.format;
                view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // todo create based on format
                //todo do gud 👍
                view_create_info.subresourceRange.baseMipLevel = 0;
                view_create_info.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
                view_create_info.subresourceRange.baseArrayLayer = 0;
                view_create_info.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

                VkResult image_view_creation_result = vkCreateImageView(device, &view_create_info, vk_context->sge_allocator, &vk_context->sc.sc_views[i]);

        }

        printf("KACK CLION\n");

        return swapchain;
}