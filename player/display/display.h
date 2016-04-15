#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_WIDTH	84		// In pixels
#define DISPLAY_HEIGHT	6		// In characters( *8 to convert to pixels)

void display_init(void);

void display_clear(void);
void display_set_XY(int x, int y);

void display_write_char(char c);
void display_write_string(char *c);
void display_write_int(int i);

#endif
