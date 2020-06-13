#ifndef HUD_H
#define HUD_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "core/log.h"
#include "core/mem.h"
#include "render/render.h"
#include "vulkan/vulkan_base.h"
#include "vulkan/vulkan_base_util.h"
#include "vulkan/vulkan_pipeline.h"
#include "vulkan/vulkan_pipeline_util.h"
#include "vulkan/vulkan_render_buffer.h"
#include "vulkan/vulkan_state.h"
#include "vulkan/vulkan_uniforms.h"

struct hud {
    struct vulkan_pipeline *pipeline;
    struct vulkan_render_buffer *render;
};

struct hud *create_hud(struct vulkan_pipeline *pipeline, struct vulkan_render_buffer *render);

void hud_render(struct vulkan_state *vk_state, struct vulkan_base *vk_base, struct hud *self, VkCommandBuffer command_buffer, uint32_t image_index);

void delete_hud(struct vulkan_state *vk_state, struct hud *self);

#endif
