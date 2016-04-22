#ifndef DISPLAY_H
#define DISPLAY_H


#define CHAR_PLAY				('z' + 2)
#define CHAR_PAUSE				('z' + 4)
#define CHAR_STOP				('z' + 6)
#define CHAR_VOLUME				('z' + 8)
#define CHAR_VOL_LOW			('z' + 10)
#define CHAR_VOL_HIGH			('z' + 11)

#define DISPLAY_WIDTH	84		// In pixels
#define DISPLAY_HEIGHT	6		// In characters( *8 to convert to pixels)

void display_init(void);

void display_clear(void);
void display_set_XY(int x, int y);

void display_write_char(char c);
void display_write_string(char *c);
void display_write_int(int i);
void display_write_control_char(int c, int is_selected);	// Write defined chars

void display_write_char_inverted(char c);
void display_write_string_inverted(char *s);

#endif
