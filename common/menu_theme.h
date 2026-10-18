#ifndef MENU_THEME_H__
#define MENU_THEME_H__

#include <stdbool.h>
#include <lib/config.h>

void menu_theme_init(void);
const char *menu_theme_entry_begin(struct menu_entry *entry, bool selected);
const char *menu_theme_entry_end(struct menu_entry *entry, bool selected);

#endif
