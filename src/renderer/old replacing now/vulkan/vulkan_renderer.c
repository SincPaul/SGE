//
// Created by Geisthardt on 03.03.2025.
//

#include "vulkan_renderer.h"

#include <assert.h>

#include "../../core/logging.h"
#include "../../core/os_specific/sge_window.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include "../../core/memory_control.h"
#include "../../core/os_specific/os_utils.h"
#include "../../utils/steinfile.h"

bool is_debug = false;

extern bool is_resize;

const char *validation_layers[] = {
        "VK_LAYER_KHRONOS_validation"
};

//todo create allocator
//todo destroy stuff
VkShaderModule create_shader_module(VkDevice device, const char *filepath, vulkan_context *vk_context);
char* get_shader_path(const char* shader_name);

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

int initialize_vk(vulkan_context *vk_context, sge_window *window, const char *application_name, uint32_t application_version) {
        if (is_debug && !checkValidationLayerSupport()) {
                log_event(LOG_LEVEL_FATAL, "failed to check validation layer support, disable debug to ignore");
        }

        get_sge_allocator(vk_context);

        create_vk_instance(application_name, application_version, vk_context);

        create_surface(window, vk_context);

        select_physical_device(vk_context);

        uint32_t graphics_queue_family_index = UINT32_MAX;
        vk_context->device = create_logical_device(vk_context->physical_device, &graphics_queue_family_index, vk_context);

        vk_context->graphics_queue = get_graphics_queue(vk_context->device, graphics_queue_family_index);

        vk_context->present_queue = get_present_queue(vk_context->device, graphics_queue_family_index);

        vk_context->transfer_queue = get_transfer_queue(vk_context->device, graphics_queue_family_index);

        vk_context->swapchain = create_swapchain(vk_context->device, vk_context->surface, vk_context->physical_device, vk_context);

        vk_context->command_pool = create_command_pool(vk_context->device, graphics_queue_family_index, vk_context);

        allocate_command_pools(vk_context->device, vk_context->command_pool, vk_context);

        //vk_context->render_pass = create_render_pass(vk_context->device, vk_context);

        //vk_context->sc.framebuffer = create_framebuffer(vk_context->device, vk_context);

        //vk_context->so.image_available_semaphore = create_semaphore(vk_context->device, vk_context);

        //vk_context->so.render_finished_semaphore = create_semaphore(vk_context->device, vk_context);

        //vk_context->so.in_flight_fence = create_fence(vk_context->device, vk_context);

        create_sync_objects(vk_context->device, vk_context);

        vk_context->pipeline = create_graphics_pipeline(vk_context->device, vk_context);

        return 0;
}

bool create_vk_instance(const char *application_name, uint32_t application_version, vulkan_context *vk_context) {
        VkInstance instance;
        VkApplicationInfo app_info = {0};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = application_name;
        app_info.applicationVersion = application_version;
        app_info.pEngineName = "SGE";
        app_info.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
        app_info.apiVersion = VK_MAKE_API_VERSION(1,4,304,1);

        const char *extensions[] = {
                VK_KHR_SURFACE_EXTENSION_NAME,
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME
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

        if (is_debug) {
                create_info.enabledLayerCount = sizeof(validation_layers)/sizeof(validation_layers[0]);
                create_info.ppEnabledLayerNames = validation_layers;
        }


        const VkResult result = vkCreateInstance(&create_info, vk_context->sge_allocator, &instance);
        printf("Result: %d\n", result);
        if (result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to initialize vk_instance: %d", result);
                return false;
        }
        log_event(LOG_LEVEL_INFO, "Created Vk Instance");

        vk_context->instance = instance;

        return true;
}

bool create_surface(sge_window *window, vulkan_context *vk_context) {

        VkSurfaceKHR surface;
        char *os;
#ifdef WIN32
        os = "Win32";
        VkWin32SurfaceCreateInfoKHR win32_surface_create_info = {0};
        win32_surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32_surface_create_info.hwnd = window->handle.hwnd;
        win32_surface_create_info.hinstance = window->handle.hinstance;
        VkResult surface_result = vkCreateWin32SurfaceKHR(vk_context->instance, &win32_surface_create_info, NULL, &surface);
#else
        log_event(LOG_LEVEL_FATAL, "cant create surface for unsupported operating system");
        return false;
#endif
        if (surface_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create surface for given handle");
                return false;
        }
        log_event(LOG_LEVEL_INFO, "Created surface for %s", os);

        vk_context->surface = surface;
        return true;
}



bool select_physical_device(vulkan_context *vk_context) {
        uint32_t device_count = 0;
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
                printf("i: %d", i);
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

                //printf("GPU %d: %s (%d)\n", i, properties.deviceName, properties.deviceType);
                //printf("   Device ID: %d, Vendor ID: %d\n", properties.deviceID, properties.vendorID);
                //printf("   Max Compute Shared Memory: %d KB\n", properties.limits.maxComputeSharedMemorySize / 1024);
                //printf("   Max Image Dimension 2D: %d\n", properties.limits.maxImageDimension2D);
                //printf("   Max Memory Alloc: %llu MB\n", memory_properties.memoryHeaps[0].size / (1024 * 1024));
                //printf("   Score: %d\n", score);

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
                return false;
        }

        VkPhysicalDevice best_device = devices[using_gpu.device_index];

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(best_device, &properties);

        printf("Best GPU Selected: %s\n", properties.deviceName);

        vk_context->physical_device = best_device;
        return true;
}

VkDevice create_logical_device(VkPhysicalDevice physical_device, uint32_t *graphics_queue_index, vulkan_context *vk_context) {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

        printf("Queue Family Count: %u\n", queue_family_count);

        if (queue_family_count == 0) {
                log_event(LOG_LEVEL_FATAL, "No queue Families found");
        }

        VkQueueFamilyProperties queue_family_properties[queue_family_count];
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties);

        *graphics_queue_index = UINT32_MAX;
        for (int i = 0; i < queue_family_count; i++) {
                //Queue flags: 15
                //VK_QUEUE_GRAPHICS_BIT = 0x00000001,
                //VK_QUEUE_COMPUTE_BIT = 0x00000002,
                //VK_QUEUE_TRANSFER_BIT = 0x00000004,
                //VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008
                const uint32_t graphics_bit = queue_family_properties[i].queueFlags & 1;
                const uint32_t compute_bit = queue_family_properties[i].queueFlags & (1 << 1);
                if (graphics_bit && compute_bit) {
                        *graphics_queue_index = i;
                        break;
                }
        }

        if (*graphics_queue_index == UINT32_MAX) {
                log_event(LOG_LEVEL_FATAL, "Couldnt find working queue family");
                return false;
        }

        log_event(LOG_LEVEL_INFO, "Found %d queue families", queue_family_count);

        float queue_priorities = 1.0f;
        VkDeviceQueueCreateInfo queue_create_info = {0};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = *graphics_queue_index;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priorities;

        const char *extensions[] = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        };

        printf("ALIVE\n");

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

        printf("ALIVE\n");
        VkDevice vk_device;

        VkResult result2 =  vkCreateDevice(physical_device, &device_create_info, vk_context->sge_allocator, &vk_device);

        if (result2 != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create Device");
        }

        log_event(LOG_LEVEL_INFO, "Created logical device for GPU: %s", physical_device);




        return vk_device;
}

VkQueue get_graphics_queue(VkDevice device, uint32_t graphic_queue_family_index) {
        VkQueue graphics_queue;
        vkGetDeviceQueue(device, graphic_queue_family_index, 0, &graphics_queue);

        return graphics_queue;
}

VkQueue get_present_queue(VkDevice device, uint32_t graphics_family_queue_index) {
        VkQueue present_queue;
        vkGetDeviceQueue(device, graphics_family_queue_index, 0, &present_queue);

        return present_queue;
}

VkQueue get_transfer_queue(VkDevice device, uint32_t graphics_family_queue_index) {
        VkQueue transfer_queue;
        vkGetDeviceQueue(device, graphics_family_queue_index, 0, &transfer_queue);

        return transfer_queue;
}

VkCommandPool create_command_pool(VkDevice device, uint32_t graphics_queue_family_index, vulkan_context *vk_context) {
        VkCommandPoolCreateInfo pool_create_info = {0};
        pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_create_info.queueFamilyIndex = graphics_queue_family_index;
        VkCommandPool command_pool = {0};
        const VkResult vk_result = vkCreateCommandPool(device, &pool_create_info, vk_context->sge_allocator, &command_pool);

        if (vk_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create command pool for the queue %d", vk_result);
        }

        log_event(LOG_LEVEL_INFO, "Created command pool");

        return command_pool;
}

bool allocate_command_pools(VkDevice device, VkCommandPool command_pool, vulkan_context *vk_context) {
        VkCommandBufferAllocateInfo cmd_buffer_allocate_info = {0};
        cmd_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_buffer_allocate_info.commandPool = command_pool;
        cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmd_buffer_allocate_info.commandBufferCount = 1;

        for (int i = 0; i < 3; i++) {
                VkCommandBuffer cmd_buffer = {0};

                const VkResult alloc_result = vkAllocateCommandBuffers(device, &cmd_buffer_allocate_info, &cmd_buffer);

                if (alloc_result != VK_SUCCESS) {
                        log_event(LOG_LEVEL_FATAL, "Failed to allocated Command Buffers: %d", alloc_result);
                }

                log_event(LOG_LEVEL_INFO, "Allocated Command Buffers");

                vk_context->command_buffer[i] = cmd_buffer;
        }

        return true;
}

VkRenderPass create_render_pass(VkDevice device, vulkan_context *vk_context) {
        VkRenderPass render_pass;

        VkSubpassDescription subpass = {0};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        //todo more attachments "depth, input, resolve, preserve"

        uint32_t attachment_count = 1;
        VkAttachmentDescription attachment_description[attachment_count];

        VkAttachmentDescription color_attachment;
        color_attachment.format = vk_context->sc.surface.format;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        color_attachment.flags = 0;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

        attachment_description[0] = color_attachment;

        VkAttachmentReference color_attachment_reference;
        color_attachment_reference.attachment = 0;
        color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        //subpass.colorAttachmentCount = 1;
        //subpass.pColorAttachments = &color_attachment_reference;

        VkRenderPassCreateInfo render_pass_create_info = {0};
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        //render_pass_create_info.attachmentCount = attachment_count;
        //render_pass_create_info.pAttachments = attachment_description;
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;

        VkResult render_pass_result = vkCreateRenderPass(device, &render_pass_create_info, vk_context->sge_allocator, &render_pass);
        if (render_pass_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create render pass");
        }

        log_event(LOG_LEVEL_INFO, "Created render pass");

        return render_pass;
}

VkFramebuffer create_framebuffer(VkDevice device, vulkan_context *vk_context) {
        VkFramebuffer framebuffer;

        VkFramebufferCreateInfo framebuffer_create_info = {0};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = vk_context->render_pass;
        framebuffer_create_info.width = vk_context->sc.surface_capabilities.currentExtent.width;
        framebuffer_create_info.height = vk_context->sc.surface_capabilities.currentExtent.height;
        framebuffer_create_info.layers = 1;

        VkResult framebuffer_result = vkCreateFramebuffer(device, &framebuffer_create_info, vk_context->sge_allocator, &framebuffer);

        if (framebuffer_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "failed to create framebuffer: %d", framebuffer_result);
        }

        log_event(LOG_LEVEL_INFO, "created framebuffer");

        return framebuffer;
}

void create_sync_objects(VkDevice device, vulkan_context *vk_context) {
        for (int i = 0; i < 3; i++) {
                vk_context->so.image_available_semaphore[i] = create_semaphore(device, vk_context);
                vk_context->so.render_finished_semaphore[i] = create_semaphore(device, vk_context);
                vk_context->so.in_flight_fence[i] = create_fence(device, vk_context);
        }
}

VkSemaphore create_semaphore(VkDevice device, vulkan_context *vk_context) {
        VkSemaphore semaphore;

        VkSemaphoreCreateInfo semaphore_create_info = {0};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkResult result;
        result = vkCreateSemaphore(device, &semaphore_create_info, vk_context->sge_allocator, &semaphore);
        if (result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "failed to create img available semaphores");
        }

        return semaphore;
}

VkFence create_fence(VkDevice device, vulkan_context *vk_context) {
        VkFence fence;

        VkFenceCreateInfo fence_create_info = {0};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkResult result = vkCreateFence(device, &fence_create_info, vk_context->sge_allocator, &fence);
        if (result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "failed to create fence");
        }

        return fence;
}


bool begin_command_buffer_rec(VkCommandBuffer command_buffer, vulkan_context *vk_context) {
        log_event(LOG_LEVEL_TRACE, "beginning command buffer recording");
        vkResetCommandBuffer(command_buffer, 0);
        log_event(LOG_LEVEL_TRACE, "reset cmd buffer");
        VkCommandBufferBeginInfo cmd_buf_begin_info = {0};
        cmd_buf_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        const VkResult rec_begin_result = vkBeginCommandBuffer(command_buffer, &cmd_buf_begin_info);

        if (rec_begin_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to begin cmd buffer recording: %d", rec_begin_result);
        }

        log_event(LOG_LEVEL_INFO, "Can now record into cmd buffer");
        return true;
}

bool submit_command_buffer_queue(VkCommandBuffer command_buffer,VkQueue queue, vulkan_context *vk_context) {
        VkSubmitInfo submit_info = {0};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        VkResult queue_submit_result = vkQueueSubmit(queue, 1, &submit_info, 0);

        if (queue_submit_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to submit command buffer to queue");
        }

        VkResult wait_result = vkQueueWaitIdle(queue);

        if (wait_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to wait for Queue Submit finish");
        }

        return true;
}

bool end_command_buffer_rec(VkCommandBuffer command_buffer, vulkan_context *vk_context) {
        const VkResult rec_end_result = vkEndCommandBuffer(command_buffer);
        if (rec_end_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to end cmd buffer recording: %d", rec_end_result);
        }

        log_event(LOG_LEVEL_INFO, "Finished cmd buffer recording");

        return true;
}

VkPipeline create_graphics_pipeline(VkDevice device, vulkan_context *vk_context) {
        VkShaderModule vert_shader = create_shader_module(device, "simple_shader.vert.spv", vk_context);
        VkShaderModule frag_shader = create_shader_module(device, "simple_shader.frag.spv", vk_context);

        VkPipelineShaderStageCreateInfo stages[2] = {
                {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, NULL, 0,
                     VK_SHADER_STAGE_VERTEX_BIT, vert_shader, "main"},
                {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, NULL, 0,
                     VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader, "main"}
        };

        VkFormat color_format = vk_context->sc.surface.format;
        VkPipelineRenderingCreateInfoKHR rendering_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &color_format
        };

        VkPipelineVertexInputStateCreateInfo vertex_input = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
        };

        VkPipelineViewportStateCreateInfo viewport_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .scissorCount = 1
        };

        VkPipelineRasterizationStateCreateInfo rasterizer = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = VK_CULL_MODE_NONE,
                .frontFace = VK_FRONT_FACE_CLOCKWISE,
                .lineWidth = 1.0f
        };

        VkPipelineColorBlendAttachmentState color_blend_attachment = {
                .blendEnable = VK_FALSE,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo color_blending = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .attachmentCount = 1,
                .pAttachments = &color_blend_attachment
        };

        VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamic_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .dynamicStateCount = 2,
                .pDynamicStates = dynamic_states
        };

        VkPipelineLayoutCreateInfo pipeline_layout_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
        };
        VkPipelineLayout pipeline_layout;
        vkCreatePipelineLayout(device, &pipeline_layout_info, vk_context->sge_allocator, &pipeline_layout);

        VkPipelineMultisampleStateCreateInfo multisample_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,  // No multisampling
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 1.0f,
                .pSampleMask = NULL,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE
        };

        VkGraphicsPipelineCreateInfo pipeline_create_info = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = &rendering_info,
                .stageCount = 2,
                .pStages = stages,
                .pVertexInputState = &vertex_input,
                .pInputAssemblyState = &input_assembly,
                .pViewportState = &viewport_state,
                .pRasterizationState = &rasterizer,
                .pColorBlendState = &color_blending,
                .pDynamicState = &dynamic_state,
                .layout = pipeline_layout,
                .pMultisampleState = &multisample_state
        };

        VkPipeline pipeline;
        VkResult pipline_result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, vk_context->sge_allocator, &pipeline);

        if (pipline_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create graphics pipeline");
        }

        vkDestroyShaderModule(device, vert_shader, vk_context->sge_allocator);
        vkDestroyShaderModule(device, frag_shader, vk_context->sge_allocator);

        return pipeline;
}

bool draw_frame(vulkan_context *vk_context) {
        if (is_resize) {
                handle_resize_out_of_date(vk_context);
                is_resize = false;
                return true;
        }
        vkWaitForFences(vk_context->device, 1, &vk_context->so.in_flight_fence[vk_context->so.current_frame], VK_TRUE, UINT64_MAX);
        uint32_t image_index;
        vkAcquireNextImageKHR(vk_context->device, vk_context->swapchain, UINT64_MAX,
                            vk_context->so.image_available_semaphore[vk_context->so.current_frame], VK_NULL_HANDLE, &image_index);

        log_event(LOG_LEVEL_TRACE, "Command buffer index: %d", vk_context->command_buffer_index);

        vkResetFences(vk_context->device, 1, &vk_context->so.in_flight_fence[vk_context->so.current_frame]);

        VkCommandBuffer command_buffer = vk_context->command_buffer[vk_context->command_buffer_index];
        begin_command_buffer_rec(command_buffer, vk_context);

        if (vk_context->command_buffer_index < 2) {
                vk_context->command_buffer_index++;
        } else {
                vk_context->command_buffer_index = 0;
        }

        VkImageMemoryBarrier barrier = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = vk_context->sc.sc_images[image_index],
                .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                }
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
                        .clearValue = {.color = {0.0f, 0.0f, 0.0f, 1.0f}}
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

        vkCmdDraw(command_buffer, 6, 1, 0, 0);
        vkCmdEndRendering(command_buffer);

        barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        vkCmdPipelineBarrier(command_buffer,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                0, 0, NULL, 0, NULL, 1, &barrier);

        end_command_buffer_rec(command_buffer, vk_context);

        VkSubmitInfo submit_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &vk_context->so.image_available_semaphore[vk_context->so.current_frame],
                .pWaitDstStageMask = (VkPipelineStageFlags[]){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
                .commandBufferCount = 1,
                .pCommandBuffers = &command_buffer,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &vk_context->so.render_finished_semaphore[vk_context->so.current_frame]};

        VkResult submit_result = vkQueueSubmit(vk_context->graphics_queue, 1, &submit_info, vk_context->so.in_flight_fence[vk_context->so.current_frame]);
        if (submit_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to submit queue");
        }

        log_event(LOG_LEVEL_INFO, "Submitted Queue");

        VkPresentInfoKHR present_info = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &vk_context->so.render_finished_semaphore[vk_context->so.current_frame],
                .swapchainCount = 1,
                .pSwapchains = &vk_context->swapchain,
                .pImageIndices = &image_index};

        VkResult present_result = vkQueuePresentKHR(vk_context->present_queue, &present_info);
        if (present_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "failed to present queue");
        }

        log_event(LOG_LEVEL_INFO, "presented queue");

        vk_context->so.current_frame = (vk_context->so.current_frame) % 3;

        return true;
}

VkShaderModule create_shader_module(VkDevice device, const char *shader_name, vulkan_context *vk_context) {
        char *full_filepath = get_shader_path(shader_name);
        size_t codeSize;
        uint32_t *code = read_file_as_binary(full_filepath, &codeSize);

        free_memory(full_filepath, MEMORY_TAG_INPUT);

        VkShaderModuleCreateInfo createInfo = {0};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = codeSize;
        createInfo.pCode    = code;

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, vk_context->sge_allocator, &shaderModule) != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create shader module from file: %s", full_filepath);
        }

        free_memory(code, MEMORY_TAG_INPUT);
        return shaderModule;
}

char* get_shader_path(const char* shader_name) {
        char *base_dir = get_cwd();
        if (base_dir == NULL) {
                log_event(LOG_LEVEL_FATAL, "Failed to get base dir");
        }

        char *shader_path = allocate_memory(512, MEMORY_TAG_INPUT);
        if (!shader_path) {
                log_event(LOG_LEVEL_FATAL, "failed to allocated memory");
        }

        sprintf(shader_path, "%s/../src/renderer/shaders/%s", base_dir, shader_name);
        free_memory(base_dir, MEMORY_TAG_INPUT);
        return shader_path;
}

bool handle_resize_out_of_date(vulkan_context *vk_context) {
        destroy_swapchain(vk_context->device, vk_context->swapchain, vk_context);
        vk_context->swapchain = create_swapchain(vk_context->device, vk_context->surface, vk_context->physical_device, vk_context);

        return true;
}
//int initialize_vk_instance(HWND hwnd, const char *application_name, uint32_t application_version) {
//
//
//
//
//
//        //VkResult vkCreateCommandPool(
//        //VkDevice device,
//        //const VkCommandPoolCreateInfo* pCreateInfo,
//        //const VkAllocationCallbacks* pAllocator,
//        //VkCommandPool* pCommandPoo
//
//
//
//
//
//
//        VkCommandBufferBeginInfo cmd_buf_begin_info = {0};
//        cmd_buf_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//
//        const VkResult rec_begin_result = vkBeginCommandBuffer(cmd_buffer, &cmd_buf_begin_info);
//
//        if (rec_begin_result != VK_SUCCESS) {
//                log_event(LOG_LEVEL_FATAL, "Failed to begin cmd buffer recording: %d", rec_begin_result);
//        }
//
//        log_event(LOG_LEVEL_INFO, "Can now record into cmd buffer");
//
//
//
//        const VkResult rec_end_result = vkEndCommandBuffer(cmd_buffer);
//        if (rec_end_result != VK_SUCCESS) {
//                log_event(LOG_LEVEL_FATAL, "Failed to end cmd buffer recording: %d", rec_end_result);
//        }
//
//        log_event(LOG_LEVEL_INFO, "Finished cmd buffer recording");
//
//
//
//        vkFreeCommandBuffers(vk_device, command_pool, 1, &cmd_buffer);
//
//        log_event(LOG_LEVEL_INFO, "Freed Command Buffer");
//
//        return 0;
//}
