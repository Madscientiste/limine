#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <lib/gterm_area.h>
#include <lib/config.h>
#include <lib/misc.h>
#include <lib/fb.h>
#include <mm/pmm.h>

static bool parse_size(const char **str, size_t *out) {
    const char *end;
    *out = strtoui(*str, &end, 10);
    if (end == *str) {
        return false;
    }
    *str = *end == ',' ? end + 1 : end;
    return true;
}

bool gterm_area_get(char *config, struct fb_info *fb, struct gterm_area *area) {
    const char *str = config_get_value(config, 0, "TERM_AREA");
    if (str == NULL) {
        return false;
    }

    if (!parse_size(&str, &area->x) || !parse_size(&str, &area->y)
     || !parse_size(&str, &area->width) || !parse_size(&str, &area->height)) {
        return false;
    }

    if (area->width == 0 || area->height == 0
     || area->x >= fb->framebuffer_width || area->y >= fb->framebuffer_height) {
        return false;
    }

    if (area->width > fb->framebuffer_width - area->x) {
        area->width = fb->framebuffer_width - area->x;
    }
    if (area->height > fb->framebuffer_height - area->y) {
        area->height = fb->framebuffer_height - area->y;
    }

    return true;
}

static uint32_t to_fb_pixel(struct fb_info *fb, uint32_t rgb) {
    uint32_t r = (rgb >> 16) & 0xff;
    uint32_t g = (rgb >> 8) & 0xff;
    uint32_t b = rgb & 0xff;
    return ((r >> (8 - fb->red_mask_size)) << fb->red_mask_shift)
         | ((g >> (8 - fb->green_mask_size)) << fb->green_mask_shift)
         | ((b >> (8 - fb->blue_mask_size)) << fb->blue_mask_shift);
}

// The terminal only draws inside the area, so the rest of the screen is painted here.
void gterm_area_paint_screen(struct fb_info *fb, uint32_t *canvas) {
    volatile uint8_t *base = (volatile uint8_t *)(uintptr_t)fb->framebuffer_addr;

    for (size_t y = 0; y < fb->framebuffer_height; y++) {
        volatile uint32_t *row = (volatile uint32_t *)(base + y * fb->framebuffer_pitch);
        for (size_t x = 0; x < fb->framebuffer_width; x++) {
            row[x] = to_fb_pixel(fb, canvas[y * fb->framebuffer_width + x]);
        }
    }

    fb_flush(base, fb->framebuffer_height * fb->framebuffer_pitch);
}

// Same convention as the terminal colours: the top byte is transparency, 0 meaning opaque.
static uint32_t blend(uint32_t fg, uint32_t bg) {
    unsigned alpha = 255 - (fg >> 24);
    unsigned inv_alpha = (fg >> 24) + 1;

    uint32_t r = (alpha * ((fg >> 16) & 0xff) + inv_alpha * ((bg >> 16) & 0xff)) / 256;
    uint32_t g = (alpha * ((fg >> 8) & 0xff) + inv_alpha * ((bg >> 8) & 0xff)) / 256;
    uint32_t b = (alpha * (fg & 0xff) + inv_alpha * (bg & 0xff)) / 256;

    return (r << 16) | (g << 8) | b;
}

uint32_t *gterm_area_make_canvas(struct fb_info *fb, uint32_t *canvas, struct gterm_area *area,
                                 uint32_t panel_colour, size_t *canvas_size) {
    *canvas_size = area->width * area->height * sizeof(uint32_t);
    uint32_t *area_canvas = ext_mem_alloc(*canvas_size);

    for (size_t y = 0; y < area->height; y++) {
        uint32_t *src = canvas + (area->y + y) * fb->framebuffer_width + area->x;
        uint32_t *dst = area_canvas + y * area->width;
        for (size_t x = 0; x < area->width; x++) {
            dst[x] = blend(panel_colour, src[x]);
        }
    }

    return area_canvas;
}
