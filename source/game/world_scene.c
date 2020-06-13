#include "world_scene.h"

static void render_wall(struct vulkan_render_buffer *b, wall *w) {
    uint32_t pos = b->vertex_position;
    float *vertices = b->vertices;

    vertices[pos] = w->va.x;
    vertices[pos + 1] = w->ceiling;
    vertices[pos + 2] = w->va.y;
    vertices[pos + 3] = w->u;
    vertices[pos + 4] = w->t;
    vertices[pos + 5] = w->ld->normal.x;
    vertices[pos + 6] = 0;
    vertices[pos + 7] = w->ld->normal.y;

    vertices[pos + 8] = w->va.x;
    vertices[pos + 9] = w->floor;
    vertices[pos + 10] = w->va.y;
    vertices[pos + 11] = w->u;
    vertices[pos + 12] = w->v;
    vertices[pos + 13] = w->ld->normal.x;
    vertices[pos + 14] = 0;
    vertices[pos + 15] = w->ld->normal.y;

    vertices[pos + 16] = w->vb.x;
    vertices[pos + 17] = w->floor;
    vertices[pos + 18] = w->vb.y;
    vertices[pos + 19] = w->s;
    vertices[pos + 20] = w->v;
    vertices[pos + 21] = w->ld->normal.x;
    vertices[pos + 22] = 0;
    vertices[pos + 23] = w->ld->normal.y;

    vertices[pos + 24] = w->vb.x;
    vertices[pos + 25] = w->ceiling;
    vertices[pos + 26] = w->vb.y;
    vertices[pos + 27] = w->s;
    vertices[pos + 28] = w->t;
    vertices[pos + 29] = w->ld->normal.x;
    vertices[pos + 30] = 0;
    vertices[pos + 31] = w->ld->normal.y;

    b->vertex_position = pos + 32;
    render_index4(b);
}

static void render_triangle(struct vulkan_render_buffer *b, triangle *t) {
    uint32_t pos = b->vertex_position;
    float *vertices = b->vertices;

    vertices[pos] = t->vc.x;
    vertices[pos + 1] = t->height;
    vertices[pos + 2] = t->vc.y;
    vertices[pos + 3] = t->u3;
    vertices[pos + 4] = t->v3;
    vertices[pos + 5] = 0;
    vertices[pos + 6] = t->normal;
    vertices[pos + 7] = 0;

    vertices[pos + 8] = t->vb.x;
    vertices[pos + 9] = t->height;
    vertices[pos + 10] = t->vb.y;
    vertices[pos + 11] = t->u2;
    vertices[pos + 12] = t->v2;
    vertices[pos + 13] = 0;
    vertices[pos + 14] = t->normal;
    vertices[pos + 15] = 0;

    vertices[pos + 16] = t->va.x;
    vertices[pos + 17] = t->height;
    vertices[pos + 18] = t->va.y;
    vertices[pos + 19] = t->u1;
    vertices[pos + 20] = t->v1;
    vertices[pos + 21] = 0;
    vertices[pos + 22] = t->normal;
    vertices[pos + 23] = 0;

    b->vertex_position = pos + 24;
    render_index3(b);
}

// static void render_decal(struct vulkan_render_buffer *b, decal *d) {
//     uint32_t pos = b->vertex_position;
//     float *vertices = b->vertices;

//     vertices[pos] = d->x1;
//     vertices[pos + 1] = d->y1;
//     vertices[pos + 2] = d->z1;
//     vertices[pos + 3] = d->u1;
//     vertices[pos + 4] = d->v1;
//     vertices[pos + 5] = d->nx;
//     vertices[pos + 6] = d->ny;
//     vertices[pos + 7] = d->nz;

//     vertices[pos + 8] = d->x2;
//     vertices[pos + 9] = d->y2;
//     vertices[pos + 10] = d->z2;
//     vertices[pos + 11] = d->u2;
//     vertices[pos + 12] = d->v2;
//     vertices[pos + 13] = d->nx;
//     vertices[pos + 14] = d->ny;
//     vertices[pos + 15] = d->nz;

//     vertices[pos + 16] = d->x3;
//     vertices[pos + 17] = d->y3;
//     vertices[pos + 18] = d->z3;
//     vertices[pos + 19] = d->u3;
//     vertices[pos + 20] = d->v3;
//     vertices[pos + 21] = d->nx;
//     vertices[pos + 22] = d->ny;
//     vertices[pos + 23] = d->nz;

//     vertices[pos + 24] = d->x4;
//     vertices[pos + 25] = d->y4;
//     vertices[pos + 26] = d->z4;
//     vertices[pos + 27] = d->u4;
//     vertices[pos + 28] = d->v4;
//     vertices[pos + 29] = d->nx;
//     vertices[pos + 30] = d->ny;
//     vertices[pos + 31] = d->nz;

//     b->vertex_position = pos + 32;
//     render_index4(b);
// }

static void sector_render(uint_table *cache, sector *s) {
    line **lines = s->lines;
    int line_count = s->line_count;

    for (int i = 0; i < line_count; i++) {
        line *ld = lines[i];

        wall *top = ld->top;
        wall *middle = ld->middle;
        wall *bottom = ld->bottom;

        if (top) {
            struct vulkan_render_buffer *b = uint_table_get(cache, top->texture);
            render_wall(b, top);
        }

        if (ld->middle) {
            struct vulkan_render_buffer *b = uint_table_get(cache, middle->texture);
            render_wall(b, middle);
        }

        if (ld->bottom) {
            struct vulkan_render_buffer *b = uint_table_get(cache, bottom->texture);
            render_wall(b, bottom);
        }
    }

    triangle **triangles = s->triangles;
    int triangle_count = s->triangle_count;

    for (int i = 0; i < triangle_count; i++) {
        triangle *td = triangles[i];
        struct vulkan_render_buffer *b = uint_table_get(cache, td->texture);
        render_triangle(b, td);
    }
}

void world_scene_geometry(struct vulkan_state *vk_state, struct vulkan_base *vk_base, world_scene *self) {

    world *w = self->w;
    uint_table *cache = self->sector_cache;

    uint_table_iterator iter = create_uint_table_iterator(cache);
    while (uint_table_iterator_has_next(&iter)) {
        uint_table_pair pair = uint_table_iterator_next(&iter);
        struct vulkan_render_buffer *b = pair.value;
        vulkan_render_buffer_zero(b);
    }

    sector **sectors = w->sectors;
    int sector_count = w->sector_count;
    for (int i = 0; i < sector_count; i++) {
        sector_render(cache, sectors[i]);
    }

    iter = create_uint_table_iterator(cache);
    while (uint_table_iterator_has_next(&iter)) {
        uint_table_pair pair = uint_table_iterator_next(&iter);
        struct vulkan_render_buffer *b = pair.value;
        vulkan_render_buffer_initialize(vk_state, vk_base->vk_command_pool, b);
    }
}

void world_scene_render(struct vulkan_state *vk_state, struct vulkan_base *vk_base, world_scene *self, VkCommandBuffer command_buffer, uint32_t image_index) {

    {
        struct uniform_buffer_object ubo = {0};

        float view[16];
        float perspective[16];

        static float x = 0.0f;
        x += 0.01f;
        vec3 eye = {0, 0, 0};
        vec3 center = {3 + x, 3, 5};
        matrix_look_at(view, &eye, &center);
        matrix_translate(view, -eye.x, -eye.y, -eye.z);

        float width = (float)vk_base->swapchain->swapchain_extent.width;
        float height = (float)vk_base->swapchain->swapchain_extent.height;
        float ratio = width / height;
        matrix_perspective(perspective, 60.0, 0.01, 100, ratio);

        matrix_multiply(ubo.mvp, perspective, view);

        vk_update_uniform_buffer(vk_state, self->pipeline, image_index, ubo);
    }

    uint_table *cache = self->sector_cache;

    struct vulkan_pipeline *pipeline = self->pipeline;

    vulkan_pipeline_cmd_bind(pipeline, command_buffer);

    uint_table_iterator iter = create_uint_table_iterator(cache);
    while (uint_table_iterator_has_next(&iter)) {
        uint_table_pair pair = uint_table_iterator_next(&iter);

        vulkan_pipeline_cmd_bind_indexed_description(pipeline, command_buffer, pair.key, image_index);

        struct vulkan_render_buffer *b = pair.value;
        vulkan_render_buffer_draw(b, command_buffer);
    }
}

void world_scene_create_buffers(vulkan_state *vk_state, VkCommandPool command_pool, world_scene *self) {

    for (int i = 0; i < TEXTURE_COUNT; i++) {
        struct vulkan_render_settings render_settings = {0};
        vulkan_render_settings_init(&render_settings, 3, 3, 2, 0, 0);
        struct vulkan_render_buffer *b = create_vulkan_renderbuffer(render_settings, 4 * 800, 36 * 800);
        vulkan_render_buffer_initialize(vk_state, command_pool, b);
        uint_table_put(self->sector_cache, i, b);
    }
}

void world_scene_initialize(vulkan_state *vk_state, VkCommandPool command_pool, world_scene *self) {
    world_scene_create_buffers(vk_state, command_pool, self);
}

world_scene *create_world_scene(world *w) {
    world_scene *self = safe_calloc(1, sizeof(world_scene));
    self->w = w;
    self->sector_cache = create_uint_table();
    return self;
}

void delete_world_scene(vulkan_state *vk_state, world_scene *self) {
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        delete_vulkan_renderbuffer(vk_state, uint_table_get(self->sector_cache, i));
    }

    delete_uint_table(self->sector_cache);

    free(self);
}