#include "sector.h"

int sector_unique_index = 0;

sector *sector_init(vec **vecs, int vec_count, line **lines, int line_count) {
    int index = sector_unique_index;
    sector_unique_index++;
    sector *s = safe_calloc(1, sizeof(sector));
    s->index = index;
    s->vecs = vecs;
    s->vec_count = vec_count;
    s->lines = lines;
    s->line_count = line_count;
    return s;
}

bool sector_contains(sector *self, float x, float y) {
    bool odd = false;
    vec **vecs = self->vecs;
    int count = self->vec_count;
    int j = count - 1;
    for (int i = 0; i < count; i++) {
        vec *vi = vecs[i];
        vec *vj = vecs[j];

        if ((vi->y > y) != (vj->y > y)) {
            float value = (vj->x - vi->x) * (y - vi->y) / (vj->y - vi->y) + vi->x;
            if (x < value) {
                odd = !odd;
            }
        }

        j = i;
    }
    return odd;
}

sector *sector_find(sector *self, float x, float y) {
    sector **inside = self->inside;
    int count = self->inside_count;
    for (int i = 0; i < count; i++) {
        sector *s = inside[i];
        if (sector_contains(s, x, y)) {
            return sector_find(s, x, y);
        }
    }
    return self;
}