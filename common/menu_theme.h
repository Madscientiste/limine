#ifndef MENU_THEME_H__
#define MENU_THEME_H__

#include <stdbool.h>

void menu_theme_init(void);
const char *menu_theme_entry_begin(bool selected);
const char *menu_theme_entry_end(bool selected);

#endif
