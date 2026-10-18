#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <menu_theme.h>
#include <lib/config.h>
#include <lib/misc.h>

// Room for two "\e[38;2;255;255;255m" sequences plus a terminator.
#define ESCAPE_BUF_SIZE 48

static bool selected_fg_set = false;
static uint32_t selected_fg_rgb;
static bool selected_bg_set = false;
static uint32_t selected_bg_rgb;

static char escape_buf[ESCAPE_BUF_SIZE];

static bool parse_rgb(const char *str, uint32_t *out) {
    if (str == NULL) {
        return false;
    }
    const char *end;
    uint32_t v = strtoui(str, &end, 16);
    if (end == str) {
        return false;
    }
    *out = v & 0xffffff;
    return true;
}

static bool get_rgb(const char *config, const char *key_uk, const char *key_us, uint32_t *out) {
    char *str = config_get_value(config, 0, key_uk);
    if (str == NULL) {
        str = config_get_value(config, 0, key_us);
    }
    return parse_rgb(str, out);
}

static char *write_dec(char *p, uint8_t v) {
    if (v >= 100) {
        *p++ = '0' + v / 100;
    }
    if (v >= 10) {
        *p++ = '0' + (v / 10) % 10;
    }
    *p++ = '0' + v % 10;
    return p;
}

// SGR 38 sets the foreground, SGR 48 the background.
static char *write_rgb_escape(char *p, char sgr_digit, uint32_t rgb) {
    *p++ = '\e'; *p++ = '['; *p++ = sgr_digit; *p++ = '8'; *p++ = ';'; *p++ = '2'; *p++ = ';';
    p = write_dec(p, (rgb >> 16) & 0xff);
    *p++ = ';';
    p = write_dec(p, (rgb >> 8) & 0xff);
    *p++ = ';';
    p = write_dec(p, rgb & 0xff);
    *p++ = 'm';
    return p;
}

void menu_theme_init(void) {
    selected_fg_set = get_rgb(NULL, "INTERFACE_SELECTED_COLOUR", "INTERFACE_SELECTED_COLOR", &selected_fg_rgb);
    selected_bg_set = get_rgb(NULL, "INTERFACE_SELECTED_BACKGROUND_COLOUR", "INTERFACE_SELECTED_BACKGROUND_COLOR",
                              &selected_bg_rgb);
}

const char *menu_theme_entry_begin(bool selected) {
    if (!selected) {
        return "";
    }
    if (!selected_fg_set && !selected_bg_set) {
        return "\e[7m";
    }

    char *p = escape_buf;
    if (selected_fg_set) {
        p = write_rgb_escape(p, '3', selected_fg_rgb);
    }
    if (selected_bg_set) {
        p = write_rgb_escape(p, '4', selected_bg_rgb);
    }
    *p = 0;
    return escape_buf;
}

const char *menu_theme_entry_end(bool selected) {
    if (!selected) {
        return "";
    }
    return selected_fg_set || selected_bg_set ? "\e[0m" : "\e[27m";
}
