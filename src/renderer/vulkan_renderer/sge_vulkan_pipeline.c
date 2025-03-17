//
// Created by Geisthardt on 07.03.2025.
//

#include "sge_vulkan_pipeline.h"

#include "sge_vulkan_shader.h"
#include "../../core/logging.h"


SGE_RESULT sge_vulkan_pipeline_create(sge_render *render) {
        sge_vulkan_context *vk_context = (sge_vulkan_context*)(render->api_context);

        VkPipelineRenderingCreateInfo rendering_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
                .pNext = 0,
                .viewMask = 0,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &vk_context->sc.surface_format.format,
                .depthAttachmentFormat = 0,
                .stencilAttachmentFormat = 0,
        };

        VkShaderModule vertex_shader_module = sge_vulkan_shader_load(render, "simple_shader.vert.spv");
        VkShaderModule fragment_shader_module = sge_vulkan_shader_load(render, "simple_shader.frag.spv");
        if (!vertex_shader_module || !fragment_shader_module) {
                log_event(LOG_LEVEL_FATAL, "failed getting shaders");
        }

        VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = vertex_shader_module,
                .pName = "main",
                .pSpecializationInfo = NULL,
        };

        VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = fragment_shader_module,
                .pName = "main",
                .pSpecializationInfo = NULL,
        };

        VkPipelineShaderStageCreateInfo shader_stages[] = {
                vertex_shader_stage_create_info,
                fragment_shader_stage_create_info
        };



        VkVertexInputBindingDescription binding_description = {
                .binding = 0,
                .stride = sizeof(float) * 3,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            };

        VkVertexInputAttributeDescription attribute_description = {
                .binding = 0,
                .location = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = 0,
            };

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = &binding_description,
                .vertexAttributeDescriptionCount = 1,
                .pVertexAttributeDescriptions = &attribute_description,
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .primitiveRestartEnable = VK_FALSE,
        };

        VkViewport viewport = {
                .x = 0.0f,
                .y = 0.0f,
                .width = vk_context->sc.surface_capabilities.maxImageExtent.width,
                .height = vk_context->sc.surface_capabilities.maxImageExtent.height,
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
        };

        VkRect2D scissor = {
                .offset = { 0, 0 },
                .extent = vk_context->sc.surface_capabilities.maxImageExtent,
        };

        VkPipelineViewportStateCreateInfo viewport_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .viewportCount = 1,
                .pViewports = &viewport,
                .scissorCount = 1,
                .pScissors = &scissor,
        };

        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = VK_CULL_MODE_NONE,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 0,
                .depthBiasClamp = 0,
                .depthBiasSlopeFactor = 0,
                .lineWidth = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 0,
                .pSampleMask = NULL,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE,
        };

        VkPipelineColorBlendAttachmentState attachment_state = {
                .blendEnable = VK_FALSE,
                .srcColorBlendFactor = 0,
                .dstColorBlendFactor = 0,
                .colorBlendOp = 0,
                .srcAlphaBlendFactor = 0,
                .dstAlphaBlendFactor = 0,
                .alphaBlendOp = 0,
                .colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT,
        };

        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .logicOpEnable = VK_FALSE,
                .logicOp = 0,
                .attachmentCount = 1,
                .pAttachments = &attachment_state,
                .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
        };

        VkDynamicState dynamic_states[] = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .dynamicStateCount = sizeof(dynamic_states)/sizeof(dynamic_states[0]),
                .pDynamicStates = dynamic_states,
        };

        VkPipelineLayoutCreateInfo layout_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .setLayoutCount = 1,
                .pSetLayouts = &vk_context->descriptor_set_layout,
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = NULL,
        };


        VkPipelineLayout layout;

        VkResult layout_result = vkCreatePipelineLayout(vk_context->device, &layout_create_info, vk_context->sge_allocator, &layout);
        if (layout_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "failed creating pipeline layout: %d", layout_result);
                return SGE_ERROR;
        }

        vk_context->pipeline_layout = layout;

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = &rendering_create_info,
                .flags = 0,
                .stageCount = sizeof(shader_stages)/sizeof(shader_stages[0]),
                .pStages = shader_stages,
                .pVertexInputState = &vertex_input_state_create_info,
                .pInputAssemblyState = &input_assembly_create_info,
                .pTessellationState = NULL,
                .pViewportState = &viewport_state_create_info,
                .pRasterizationState = &rasterization_state_create_info,
                .pMultisampleState = &multisample_state_create_info,
                .pDepthStencilState = NULL,
                .pColorBlendState = &color_blend_state_create_info,
                .pDynamicState = &dynamic_state_create_info,
                .layout = layout,
                .renderPass = NULL, //dynamic rendering extension -todo make dynamic if extension not supported
                .subpass = 0,
                .basePipelineHandle = NULL,
                .basePipelineIndex = -1,
        };

        VkPipeline graphics_pipeline;
        VkResult pipeline_result = vkCreateGraphicsPipelines(vk_context->device, VK_NULL_HANDLE, 1,
                                                                &graphics_pipeline_create_info,vk_context->sge_allocator, &graphics_pipeline);
        if (pipeline_result != VK_SUCCESS) {
                log_event(LOG_LEVEL_FATAL, "Failed to create pipeline: %d", pipeline_result);
                return SGE_ERROR;
        }

        vk_context->pipeline = graphics_pipeline;
        log_event(LOG_LEVEL_INFO, "Created pipeline");
        return SGE_SUCCESS;
}
