#ifndef LIB__GTERM_AREA_H__
#define LIB__GTERM_AREA_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <lib/fb.h>

struct gterm_area {
    size_t x;
    size_t y;
    size_t width;
    size_t height;
};

bool gterm_area_get(char *config, struct fb_info *fb, struct gterm_area *area);
void gterm_area_paint_screen(struct fb_info *fb, uint32_t *canvas);
uint32_t *gterm_area_make_canvas(struct fb_info *fb, uint32_t *canvas, struct gterm_area *area,
                                 uint32_t panel_colour, size_t *canvas_size);

#endif
