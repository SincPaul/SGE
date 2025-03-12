//
// Created by Geisthardt on 07.03.2025.
//

#include "sge_vulkan_device.h"
#include "../../core/logging.h"

SGE_RESULT sge_vulkan_physical_device_select(sge_render *render) {
        uint32_t device_count = 0;
        sge_vulkan_context *vk_context = (sge_vulkan_context*)render->api_context;

        vkEnumeratePhysicalDevices(vk_context->instance, &device_count, NULL);

        if (device_count == 0) {
                log_event(LOG_LEVEL_FATAL, "No Vulkan compatible GPUs found");
        }

        VkPhysicalDevice devices[device_count];
        vkEnumeratePhysicalDevices(vk_context->instance, &device_count, devices);

        struct highest_score_gpu {
                int score;
                int device_index;
                bool is_dedicated_gpu;
        };

        struct highest_score_gpu using_gpu = { .score = -1, .device_index = -1};

        for (int i = 0; i < device_count; i++) {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(devices[i], &properties);

                VkPhysicalDeviceMemoryProperties memory_properties;
                vkGetPhysicalDeviceMemoryProperties(devices[i], &memory_properties);

                const bool is_dedicated = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                int score = 0;
                if (is_dedicated) {score += 100; }
                score += (properties.limits.maxComputeSharedMemorySize / 1024) / 2;
                score += (properties.limits.maxImageDimension2D / 1024) * 5;
                score += (memory_properties.memoryHeaps[0].size / (1024 * 1024)) / 100;

                if (score == using_gpu.score) {
                        if (using_gpu.is_dedicated_gpu) { continue; }
                        using_gpu.device_index = i;
                        using_gpu.is_dedicated_gpu = is_dedicated;
                }
                if (score > using_gpu.score) {
                        using_gpu.score = score;
                        using_gpu.is_dedicated_gpu = is_dedicated;
                        using_gpu.device_index = i;
                }
        }

        if (using_gpu.device_index == -1) {
                log_event(LOG_LEVEL_FATAL, "Failed to get gpu for Vulkan");
                return SGE_ERROR;
        }

        VkPhysicalDevice best_device = devices[using_gpu.device_index];

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(best_device, &properties);

        vk_context->physical_device = best_device;
        vk_context->physical_device_properties = properties;

        return SGE_SUCCESS;
}

SGE_RESULT sge_vulkan_physical_device_remove(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context*)render->api_context;

        if (!vk_context->physical_device) {
                return SGE_ERROR;
        }
        vk_context->physical_device = 0;

        return SGE_SUCCESS;
}

SGE_RESULT sge_vulkan_logical_device_create(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context*)render->api_context;

        if (!vk_context->physical_device) {
                log_event(LOG_LEVEL_FATAL, "Tried creating logical device without having physical device");
                return SGE_ERROR;
        }

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vk_context->physical_device, &queue_family_count, NULL);

        if (queue_family_count == 0) {
                log_event(LOG_LEVEL_FATAL, "No queue Families found");
        }

        VkQueueFamilyProperties queue_family_properties[queue_family_count];
        vkGetPhysicalDeviceQueueFamilyProperties(vk_context->physical_device, &queue_family_count, queue_family_properties);

        uint32_t graphics_queue_family_index = UINT32_MAX;
        for (int i = 0; i < queue_family_count; i++) {
                //Queue flags: 15
                //VK_QUEUE_GRAPHICS_BIT = 0x00000001,
                //VK_QUEUE_COMPUTE_BIT = 0x00000002,
                //VK_QUEUE_TRANSFER_BIT = 0x00000004,
                //VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008
                const uint32_t graphics_bit = queue_family_properties[i].queueFlags & 1;
                const uint32_t compute_bit = queue_family_properties[i].queueFlags & (1 << 1);
                if (graphics_bit && compute_bit) {
                        graphics_queue_family_index = i;
                        break;
                }
        }

        if (graphics_queue_family_index == UINT32_MAX) {
                log_event(LOG_LEVEL_FATAL, "Couldnt find working queue family");
                return false;
        }

        vk_context->graphics_queue_family_index;

        log_event(LOG_LEVEL_INFO, "Found %d queue families", queue_family_count);

        float queue_priorities = 1.0f;
        VkDeviceQueueCreateInfo queue_create_info = {0};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = graphics_queue_family_index;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priorities;

        const char *extensions[] = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        };

        VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
                .dynamicRendering = VK_TRUE,
                .pNext = 0
        };

        VkDeviceCreateInfo device_create_info = {0};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = 1;
        device_create_info.pQueueCreateInfos = &queue_create_info;
        device_create_info.enabledExtensionCount = sizeof(extensions)/sizeof(extensions[0]);
        device_create_info.ppEnabledExtensionNames = extensions;
        device_create_info.pNext = &dynamic_rendering_features;

        VkDevice vk_device;

        VkResult result2 =  vkCreateDevice(vk_context->physical_device, &device_create_info, vk_context->sge_allocator, &vk_device);

        if (result2 != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create Device");
                return SGE_ERROR;
        }

        log_event(LOG_LEVEL_INFO, "Created logical device for GPU: %s", vk_context->physical_device_properties.deviceName);

        vk_context->device = vk_device;

        vkGetDeviceQueue(vk_device, graphics_queue_family_index, 0, &vk_context->graphics_queue);
        vkGetDeviceQueue(vk_device, graphics_queue_family_index, 0, &vk_context->present_queue);
        vkGetDeviceQueue(vk_device, graphics_queue_family_index, 0, &vk_context->transfer_queue);

        return SGE_SUCCESS;
}

SGE_RESULT sge_vulkan_logical_device_destroy(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context*)render->api_context;
        if (vk_context->device != VK_NULL_HANDLE) {
                vkDestroyDevice(vk_context->device, vk_context->sge_allocator);
                vk_context->device = VK_NULL_HANDLE;
                return SGE_SUCCESS;
        }
        return SGE_ERROR;
}