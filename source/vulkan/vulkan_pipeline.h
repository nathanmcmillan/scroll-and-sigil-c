#ifndef VULKAN_PIPELINE_H
#define VULKAN_PIPELINE_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "common/fileio.h"
#include "common/mem.h"

#include "vulkan_base.h"
#include "vulkan_image.h"
#include "vulkan_pipe_data.h"
#include "vulkan_render_buffer.h"
#include "vulkan_state.h"
#include "vulkan_swapchain.h"
#include "vulkan_uniform.h"

struct vulkan_pipeline {
    struct vulkan_pipe_data pipe_data;
    struct vulkan_render_settings render_settings;
    uint32_t swapchain_image_count;
    struct vulkan_uniform_buffer *uniforms;
    struct vulkan_image **images;
    int image_count;
    uint32_t image_descriptors;
    VkDescriptorPool vk_descriptor_pool;
    VkDescriptorSetLayout vk_uniform_buffer_descriptor_set_layout;
    VkDescriptorSetLayout vk_image_descriptor_set_layout;
    VkDescriptorSet *vk_uniform_buffer_descriptor_sets;
    VkDescriptorSet *vk_image_descriptor_sets;
    VkPipeline vk_pipeline;
    VkPipelineLayout vk_pipeline_layout;
    bool include_depth;
    VkFrontFace rasterize_face;
    VkCullModeFlagBits rasterize_cull_mode;
};

struct vulkan_pipeline *create_vulkan_pipeline(struct vulkan_pipe_data pipe_data, struct vulkan_render_settings render_settings);
void vulkan_pipeline_images(struct vulkan_pipeline *self, struct vulkan_image **images, int image_count, uint32_t image_descriptors);
void vulkan_pipeline_settings(struct vulkan_pipeline *self, bool include_depth, VkFrontFace rasterize_face, VkCullModeFlagBits rasterize_cull_mode);
void vulkan_pipeline_compile_graphics(vulkan_state *vk_state, VkExtent2D vk_extent, VkRenderPass vk_render_pass, struct vulkan_pipeline *pipeline);

#endif
