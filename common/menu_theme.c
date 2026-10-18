#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <menu_theme.h>
#include <lib/config.h>
#include <lib/misc.h>

// Room for two "\e[38;2;255;255;255m" sequences plus a terminator.
#define ESCAPE_BUF_SIZE 48

static bool entry_rgb_set = false;
static uint32_t entry_rgb;
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
    entry_rgb_set = get_rgb(NULL, "INTERFACE_ENTRY_COLOUR", "INTERFACE_ENTRY_COLOR", &entry_rgb);
    selected_fg_set = get_rgb(NULL, "INTERFACE_SELECTED_COLOUR", "INTERFACE_SELECTED_COLOR", &selected_fg_rgb);
    selected_bg_set = get_rgb(NULL, "INTERFACE_SELECTED_BACKGROUND_COLOUR", "INTERFACE_SELECTED_BACKGROUND_COLOR",
                              &selected_bg_rgb);
}

static bool get_entry_rgb(struct menu_entry *entry, uint32_t *out) {
    if (get_rgb(entry->body, "COLOUR", "COLOR", out)) {
        return true;
    }
    if (entry_rgb_set) {
        *out = entry_rgb;
        return true;
    }
    return false;
}

const char *menu_theme_entry_begin(struct menu_entry *entry, bool selected) {
    char *p = escape_buf;
    uint32_t rgb;

    if (selected && (selected_fg_set || selected_bg_set)) {
        if (selected_fg_set) {
            p = write_rgb_escape(p, '3', selected_fg_rgb);
        } else if (get_entry_rgb(entry, &rgb)) {
            p = write_rgb_escape(p, '3', rgb);
        }
        if (selected_bg_set) {
            p = write_rgb_escape(p, '4', selected_bg_rgb);
        }
    } else {
        if (get_entry_rgb(entry, &rgb)) {
            p = write_rgb_escape(p, '3', rgb);
        }
        if (selected) {
            *p++ = '\e'; *p++ = '['; *p++ = '7'; *p++ = 'm';
        }
    }

    *p = 0;
    return escape_buf;
}

const char *menu_theme_entry_end(struct menu_entry *entry, bool selected) {
    uint32_t rgb;
    if ((selected && (selected_fg_set || selected_bg_set)) || get_entry_rgb(entry, &rgb)) {
        return "\e[0m";
    }
    return selected ? "\e[27m" : "";
}
