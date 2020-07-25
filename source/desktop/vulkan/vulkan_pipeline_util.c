#include "vulkan_pipeline_util.h"

VkPipelineColorBlendAttachmentState create_color_blend_attachment(VkColorComponentFlags mask, VkBool32 blend) {
    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
    color_blend_attachment.colorWriteMask = mask;
    color_blend_attachment.blendEnable = blend;
    return color_blend_attachment;
}

void vulkan_pipeline_cmd_bind(struct vulkan_pipeline *pipeline, VkCommandBuffer command_buffer) {
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_pipeline);
}

void vulkan_pipeline_cmd_bind_set(struct vulkan_pipeline *pipeline, VkCommandBuffer command_buffer, uint32_t set, uint32_t count, VkDescriptorSet *descriptors) {
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_pipeline_layout, set, count, descriptors, 0, NULL);
}

void vulkan_pipeline_cmd_bind_dynamic_set(struct vulkan_pipeline *pipeline, VkCommandBuffer command_buffer, uint32_t set, uint32_t count, VkDescriptorSet *descriptors, uint32_t offset_count, const uint32_t *offsets) {
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_pipeline_layout, set, count, descriptors, offset_count, offsets);
}

static void vulkan_pipeline_clean(vulkan_state *vk_state, struct vulkan_pipeline *pipeline) {

    vkDestroyPipeline(vk_state->vk_device, pipeline->vk_pipeline, NULL);
    vkDestroyPipelineLayout(vk_state->vk_device, pipeline->vk_pipeline_layout, NULL);

    vulkan_pipe_data_clean(vk_state, &pipeline->pipe_data);
}

void vulkan_pipeline_recreate(vulkan_state *vk_state, struct vulkan_base *vk_base, struct vulkan_pipeline *pipeline) {

    vulkan_pipeline_clean(vk_state, pipeline);

    vulkan_pipe_data_initialize_uniforms(vk_state, &pipeline->pipe_data);
    vulkan_pipeline_create_descriptor_pool(vk_state, pipeline);
    vulkan_pipeline_create_descriptor_sets(vk_state, pipeline);

    vulkan_pipeline_compile_graphics(vk_state, vk_base, pipeline);

    vulkan_pipeline_update_descriptor_sets(vk_state, pipeline);
}

void vulkan_pipeline_initialize(vulkan_state *vk_state, struct vulkan_base *vk_base, struct vulkan_pipeline *pipeline) {

    pipeline->swapchain_image_count = vk_base->swapchain->swapchain_image_count;

    vulkan_pipeline_create_descriptor_layouts(vk_state, pipeline);

    vulkan_pipe_data_initialize_uniforms(vk_state, &pipeline->pipe_data);
    vulkan_pipeline_create_descriptor_pool(vk_state, pipeline);
    vulkan_pipeline_create_descriptor_sets(vk_state, pipeline);

    vulkan_pipeline_compile_graphics(vk_state, vk_base, pipeline);
}

void vulkan_pipeline_static_initialize(vulkan_state *vk_state, struct vulkan_base *vk_base, struct vulkan_pipeline *pipeline) {

    vulkan_pipeline_initialize(vk_state, vk_base, pipeline);
    vulkan_pipeline_update_descriptor_sets(vk_state, pipeline);
}

void vulkan_pipeline_basic_initialize(vulkan_state *vk_state, struct vulkan_base *vk_base, struct vulkan_pipeline *pipeline) {

    pipeline->swapchain_image_count = vk_base->swapchain->swapchain_image_count;

    vulkan_pipeline_compile_graphics(vk_state, vk_base, pipeline);
}

void delete_vulkan_pipeline(vulkan_state *vk_state, struct vulkan_pipeline *pipeline) {

    vulkan_pipeline_clean(vk_state, pipeline);
    delete_vulkan_pipe_data(vk_state, &pipeline->pipe_data);

    free(pipeline);
}
