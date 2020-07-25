#include "render_model_shader.h"

#define MODEL_LIMIT 4
#define DESCRIPT_LAYOUT_COUNT 3
#define POOL_SIZE_COUNT 2

struct render_model_shader *new_render_model_shader(vulkan_state *vk_state, vulkan_base *vk_base, struct vulkan_offscreen_buffer *offscreen) {

    struct render_model_shader *shader = safe_calloc(1, sizeof(struct render_model_shader));

    uint32_t swapchain_copies = vk_base->swapchain->swapchain_image_count;

    // uniforms

    shader->uniforms1 = safe_calloc(1, sizeof(struct vulkan_uniform_buffer));
    shader->uniforms1->object_size = sizeof(struct uniform_projection);
    vulkan_uniform_buffer_initialize(vk_state, swapchain_copies, shader->uniforms1);

    shader->uniforms3 = safe_calloc(1, sizeof(struct uniform_bones));
    shader->uniforms3->object_size = sizeof(struct uniform_bones);
    shader->uniforms3->object_instances = MODEL_LIMIT;
    vulkan_uniform_buffer_initialize(vk_state, swapchain_copies, shader->uniforms3);

    // descriptor set layouts

    shader->descriptor_set_layouts = safe_calloc(DESCRIPT_LAYOUT_COUNT, sizeof(VkDescriptorSetLayout));

    {
        VkDescriptorSetLayoutBinding bindings[1];
        memset(bindings, 0, sizeof(VkDescriptorSetLayoutBinding));

        bindings[0].binding = 0;
        bindings[0].descriptorCount = 1;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layout_info = {0};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = 1;
        layout_info.pBindings = bindings;

        VK_RESULT_OK(vkCreateDescriptorSetLayout(vk_state->vk_device, &layout_info, NULL, &shader->descriptor_set_layouts[0]))
    }

    {
        VkDescriptorSetLayoutBinding bindings[1];
        memset(bindings, 0, sizeof(VkDescriptorSetLayoutBinding));

        bindings[0].binding = 0;
        bindings[0].descriptorCount = 1;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layout_info = {0};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = 1;
        layout_info.pBindings = bindings;

        VK_RESULT_OK(vkCreateDescriptorSetLayout(vk_state->vk_device, &layout_info, NULL, &shader->descriptor_set_layouts[1]))
    }

    {
        VkDescriptorSetLayoutBinding bindings[1];
        memset(bindings, 0, sizeof(VkDescriptorSetLayoutBinding));

        bindings[0].binding = 0;
        bindings[0].descriptorCount = 1;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layout_info = {0};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = 1;
        layout_info.pBindings = bindings;

        VK_RESULT_OK(vkCreateDescriptorSetLayout(vk_state->vk_device, &layout_info, NULL, &shader->descriptor_set_layouts[2]))
    }

    // descriptor pool

    {
        uint32_t max_sets = swapchain_copies * 2;
        uint32_t pool_size_count = POOL_SIZE_COUNT;

        VkDescriptorPoolSize pool_sizes[POOL_SIZE_COUNT];
        memset(pool_sizes, 0, POOL_SIZE_COUNT * sizeof(VkDescriptorPoolSize));

        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[0].descriptorCount = swapchain_copies;

        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        pool_sizes[1].descriptorCount = swapchain_copies;

        VkDescriptorPoolCreateInfo pool_info = {0};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.poolSizeCount = pool_size_count;
        pool_info.pPoolSizes = pool_sizes;
        pool_info.maxSets = max_sets;

        VK_RESULT_OK(vkCreateDescriptorPool(vk_state->vk_device, &pool_info, NULL, &shader->descriptor_pool));
    }

    // descriptor sets

    {
        // model view projection uniform buffer

        uint32_t copies = swapchain_copies;

        shader->descriptor_sets1 = safe_calloc(swapchain_copies, sizeof(VkDescriptorSet));

        VkDescriptorSetLayout *layouts = safe_calloc(copies, sizeof(VkDescriptorSetLayout));

        for (uint32_t i = 0; i < copies; i++) {
            memcpy(&layouts[i], &shader->descriptor_set_layouts[0], sizeof(VkDescriptorSetLayout));
        }

        VkDescriptorSetAllocateInfo info = {0};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.descriptorPool = shader->descriptor_pool;
        info.descriptorSetCount = copies;
        info.pSetLayouts = layouts;

        VK_RESULT_OK(vkAllocateDescriptorSets(vk_state->vk_device, &info, shader->descriptor_sets1));

        free(layouts);

        for (uint32_t i = 0; i < copies; i++) {

            VkWriteDescriptorSet write_descriptors[1];
            memset(write_descriptors, 0, sizeof(VkWriteDescriptorSet));

            VkDescriptorBufferInfo buffer_info = {0};
            buffer_info.buffer = shader->uniforms1->vk_uniform_buffers[i];
            buffer_info.offset = 0;
            buffer_info.range = sizeof(struct uniform_projection);

            write_descriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptors[0].dstSet = shader->descriptor_sets1[i];
            write_descriptors[0].dstBinding = 0;
            write_descriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_descriptors[0].descriptorCount = 1;
            write_descriptors[0].pBufferInfo = &buffer_info;

            vkUpdateDescriptorSets(vk_state->vk_device, 1, write_descriptors, 0, NULL);
        }
    }

    {
        // bones uniform buffer

        uint32_t copies = swapchain_copies;

        shader->descriptor_sets3 = safe_calloc(swapchain_copies, sizeof(VkDescriptorSet));

        VkDescriptorSetLayout *layouts = safe_calloc(copies, sizeof(VkDescriptorSetLayout));

        for (uint32_t i = 0; i < copies; i++) {
            memcpy(&layouts[i], &shader->descriptor_set_layouts[2], sizeof(VkDescriptorSetLayout));
        }

        VkDescriptorSetAllocateInfo info = {0};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.descriptorPool = shader->descriptor_pool;
        info.descriptorSetCount = copies;
        info.pSetLayouts = layouts;

        VK_RESULT_OK(vkAllocateDescriptorSets(vk_state->vk_device, &info, shader->descriptor_sets3));

        free(layouts);

        for (uint32_t i = 0; i < copies; i++) {

            VkWriteDescriptorSet write_descriptors[1];
            memset(write_descriptors, 0, sizeof(VkWriteDescriptorSet));

            VkDescriptorBufferInfo buffer_info = {0};
            buffer_info.buffer = shader->uniforms3->vk_uniform_buffers[i];
            buffer_info.offset = 0;
            buffer_info.range = sizeof(struct uniform_projection);

            write_descriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptors[0].dstSet = shader->descriptor_sets3[i];
            write_descriptors[0].dstBinding = 0;
            write_descriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            write_descriptors[0].descriptorCount = 1;
            write_descriptors[0].pBufferInfo = &buffer_info;

            vkUpdateDescriptorSets(vk_state->vk_device, 1, write_descriptors, 0, NULL);
        }
    }

    struct vulkan_pipe_data pipe_settings = {0};

    pipe_settings.vertex = "shaders/spv/model.deferred.vert.spv";
    pipe_settings.fragment = "shaders/spv/model.deferred.frag.spv";

    pipe_settings.number_of_sets = DESCRIPT_LAYOUT_COUNT;

    if (offscreen != NULL) {
        VkPipelineColorBlendAttachmentState color_attach = create_color_blend_attachment(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, VK_FALSE);

        pipe_settings.use_render_pass = true;
        pipe_settings.render_pass = offscreen->render_pass;
        pipe_settings.color_blend_attachments_count = 3;
        pipe_settings.color_blend_attachments = safe_calloc(pipe_settings.color_blend_attachments_count, sizeof(VkPipelineColorBlendAttachmentState));
        pipe_settings.color_blend_attachments[0] = color_attach;
        pipe_settings.color_blend_attachments[1] = color_attach;
        pipe_settings.color_blend_attachments[2] = color_attach;
    }

    struct vulkan_render_settings render_settings = {0};
    vulkan_render_settings_init(&render_settings, 3, 0, 2, 3, 1);

    struct vulkan_pipeline *pipeline = create_vulkan_pipeline(pipe_settings, render_settings);

    vulkan_pipeline_settings(pipeline, true, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_CULL_MODE_BACK_BIT);

    pipeline->descriptor_set_layout_count = DESCRIPT_LAYOUT_COUNT;
    pipeline->descriptor_set_layouts = shader->descriptor_set_layouts;

    vulkan_pipeline_basic_initialize(vk_state, vk_base, pipeline);

    shader->pipeline = pipeline;

    return shader;
}

void delete_render_model_shader(vulkan_state *vk_state, struct render_model_shader *shader) {

    delete_vulkan_pipeline(vk_state, shader->pipeline);

    vulkan_uniform_buffer_clean(vk_state, shader->uniforms1);
    vulkan_uniform_buffer_clean(vk_state, shader->uniforms3);

    vkDestroyDescriptorPool(vk_state->vk_device, shader->descriptor_pool, NULL);

    for (int i = 0; i < DESCRIPT_LAYOUT_COUNT; i++) {
        vkDestroyDescriptorSetLayout(vk_state->vk_device, shader->descriptor_set_layouts[i], NULL);
    }

    free(shader);
}
