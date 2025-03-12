//
// Created by Geisthardt on 07.03.2025.
//

#include "sge_vulkan_instance.h"

#include "../../core/logging.h"
#include "../../core/memory_control.h"

const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};

int checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, NULL);

        VkLayerProperties* availableLayers = malloc(layerCount * sizeof(VkLayerProperties));
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

        int found = 1;
        for (size_t i = 0; i < sizeof(validation_layers) / sizeof(validation_layers[0]); i++) {
                int layerFound = 0;
                for (uint32_t j = 0; j < layerCount; j++) {
                        if (strcmp(validation_layers[i], availableLayers[j].layerName) == 0) {
                                layerFound = 1;
                                break;
                        }
                }
                if (!layerFound) {
                        found = 0;
                        break;
                }
        }

        free(availableLayers);
        return found;
}

SGE_RESULT sge_vulkan_instance_create(sge_render *render, sge_render_settings *render_settings) {
        VkInstance instance;
        VkApplicationInfo app_info = {0};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = render_settings->vulkan.app_info.application_name;
        app_info.applicationVersion = render_settings->vulkan.app_info.application_version;
        app_info.pEngineName = "SGE";
        app_info.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
        app_info.apiVersion = VK_MAKE_API_VERSION(1,4,304,1);

        const char *extensions[] = {
                VK_KHR_SURFACE_EXTENSION_NAME
#ifdef WIN32
                ,VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif UNIX
                //todo gucken dann ob xlib oder xcb
                VK_USE_PLATFORM_XCB_KHR - X Window System, using the XCB library
                VK_USE_PLATFORM_XLIB_KHR - X Window System, using the Xlib library
#endif

        };


        VkInstanceCreateInfo create_info = {0};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = sizeof(extensions)/sizeof(extensions[0]);
        create_info.ppEnabledExtensionNames = extensions;

        if (render_settings->vulkan.enable_validation_layers) {
                create_info.enabledLayerCount = sizeof(validation_layers)/sizeof(validation_layers[0]);
                create_info.ppEnabledLayerNames = validation_layers;
        }


        sge_vulkan_context *vk_context = (sge_vulkan_context*)render->api_context;

        const VkResult result = vkCreateInstance(&create_info, vk_context->sge_allocator, &instance);
        if (result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to initialize vk_instance: %d", result);
                return SGE_ERROR;
        }
        log_event(LOG_LEVEL_INFO, "Created Vk Instance");

        vk_context->instance = instance;

        return SGE_SUCCESS;
}

SGE_RESULT sge_vulkan_instance_destroy(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context*)render->api_context;

        sge_vulkan_logical_device_destroy(render);
        sge_vulkan_surface_destroy(render);
        if (vk_context->instance != VK_NULL_HANDLE) {
                vkDestroyInstance(vk_context->instance, vk_context->sge_allocator);
                vk_context->instance = VK_NULL_HANDLE;
        }

        free_memory(render->api_context, MEMORY_TAG_VULKAN);
        render->api_context = NULL;
        return SGE_SUCCESS;
}