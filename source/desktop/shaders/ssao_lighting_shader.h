#ifndef SSAO_LIGHTING_SHADER_H
#define SSAO_LIGHTING_SHADER_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "vulkan/vulkan_base.h"
#include "vulkan/vulkan_base_util.h"
#include "vulkan/vulkan_offscreen_buffer.h"
#include "vulkan/vulkan_pipe_data.h"
#include "vulkan/vulkan_pipeline.h"
#include "vulkan/vulkan_pipeline_util.h"
#include "vulkan/vulkan_render_buffer.h"
#include "vulkan/vulkan_state.h"
#include "vulkan/vulkan_uniform_util.h"

struct uniform_ssao_lighting {
    float light_direction[3];
};

struct ssao_lighting_shader {
    VkDescriptorSetLayout *descriptor_set_layouts;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet *descriptor_sets1;
    VkDescriptorSet *descriptor_sets3;
    struct vulkan_pipeline *pipeline;
    struct vulkan_uniform_buffer *uniforms1;
    struct vulkan_uniform_buffer *uniforms3;
};

struct ssao_lighting_shader *new_ssao_lighting_shader(vulkan_state *vk_state, vulkan_base *vk_base, vulkan_offscreen_buffer *offscreen);
void remake_ssao_lighting_shader(vulkan_state *vk_state, vulkan_base *vk_base, struct ssao_lighting_shader *shader);
void delete_ssao_lighting_shader(vulkan_state *vk_state, struct ssao_lighting_shader *shader);

#endif