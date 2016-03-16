#ifndef DISPLAY_H
#define DISPLAY_H

#define RST GPIO_Pin_0	// Reset
#define CE  GPIO_Pin_1	// Chip enable
#define DC  GPIO_Pin_2	// Data/command
#define DIN GPIO_Pin_3	// MOSI for SPI
#define CLK GPIO_Pin_4	// Clock for SPI

#define DISPLAY_PORT	GPIOA
#define DISPLAY_CLOCK	RCC_APB2Periph_GPIOA

#define DISPLAY_WIDTH	84
#define DISPLAY_HEIGHT	6

void display_init(void);
void display_test(void);	// TODO delet

void display_clear(void);
void display_set_XY(int x, int y);

void display_write_char(char c);
void display_write_string(char *c);

#endif
