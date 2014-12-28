/*
 * LK204-7T-1U LCD panel library
 * Copyright (C) 2015  Matthew Skala
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Matthew Skala
 * http://ansuz.sooke.bc.ca/
 * mskala@ansuz.sooke.bc.ca
 */

#ifndef LK204_H
#define LK204_H

#include <stdarg.h>

void lk204_open(int testmode);
void lk204_close(void);

void lk204_printf(const char *format,...);

void lk204_clear_screen(void);
void lk204_set_cursor_pos(int r,int c);
void lk204_go_home(void);

char lk204_get_key(void);

void lk204_led_red(int ledno);
void lk204_led_yellow(int ledno);
void lk204_led_green(int ledno);
void lk204_led_off(int ledno);

int lk204_horiz_menu(int lineno,int num_entries,const char **entries);
int lk204_horiz_submenu(int lineno,int num_entries,const char **entries);
int lk204_vert_menu(const char *prompt,int num_entries,const char **entries);
int lk204_grid_submenu(int lineno,int num_entries,const char **entries);

#endif /* ndef LK204_H */
