//
// Created by Geisthardt on 07.03.2025.
//

#include "sge_vulkan_surface.h"
#include "../../core/logging.h"

SGE_RESULT sge_vulkan_surface_create(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context*)render->api_context;

        VkSurfaceKHR surface;
        char *os;
#ifdef WIN32
        os = "Win32";
        VkWin32SurfaceCreateInfoKHR win32_surface_create_info = {0};
        win32_surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32_surface_create_info.hwnd = render->window->handle.hwnd;
        win32_surface_create_info.hinstance = render->window->handle.hinstance;
        VkResult surface_result = vkCreateWin32SurfaceKHR(vk_context->instance, &win32_surface_create_info, NULL, &surface);
#else
        log_event(LOG_LEVEL_FATAL, "cant create surface for unsupported operating system");
        return SGE_UNSUPPORTED_SYSTEM;
#endif
        if (surface_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create surface for given handle");
                return false;
        }
        log_event(LOG_LEVEL_INFO, "Created surface for %s", os);

        vk_context->surface = surface;


        return SGE_SUCCESS;
}


SGE_RESULT sge_vulkan_surface_destroy(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context*)render->api_context;
        if (vk_context->surface != VK_NULL_HANDLE) {
                vkDestroySurfaceKHR(vk_context->instance, vk_context->surface, vk_context->sge_allocator);
                vk_context->surface = VK_NULL_HANDLE;
                return SGE_SUCCESS;
        }
        return SGE_ERROR;
}