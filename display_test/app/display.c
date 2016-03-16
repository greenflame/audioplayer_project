#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#include "display.h"
#include "fonts.h"

void display_set_pin(uint16_t pin, char val)
{
	if (val) GPIO_SetBits(DISPLAY_PORT, pin);
	else GPIO_ResetBits(DISPLAY_PORT, pin);
}

void display_gpio_config()
{
	RCC_APB2PeriphClockCmd(DISPLAY_CLOCK, ENABLE);

	GPIO_InitTypeDef gpio_init;

	gpio_init.GPIO_Pin = RST | CE | DC | DIN | CLK;
	gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_Init(DISPLAY_PORT, &gpio_init);
}

void display_write_byte(unsigned char dat, char mode)
{
	display_set_pin(CE, 0);
	display_set_pin(DC, mode);

	char i;
	for(i = 0; i < 8; i++)
	{
		display_set_pin(DIN, dat & 0x80);
		dat = dat << 1;
		display_set_pin(CLK, 0);
		display_set_pin(CLK, 1);
	}

	display_set_pin(CE, 1);
}

void display_set_XY(int x, int y)	// In char size
{
	display_write_byte(0x40 | y, 0);
	display_write_byte(0x80 | (x * 6), 0);
}

void display_clear()
{
	int i;

	for(i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
	{
		display_write_byte(0, 1);
	}

	display_set_XY(0, 0);
}

void display_init()
{
	display_gpio_config();

	display_set_pin(RST, 0);
	int i = 10 * 4000;	// TODO sleep
	while(i--);
	display_set_pin(RST, 1);

	display_write_byte(0x21, 0);
	display_write_byte(0xc6, 0);
	display_write_byte(0x06, 0);
	display_write_byte(0x13, 0);
	display_write_byte(0x20, 0);
	display_clear();
	display_write_byte(0x0c, 0);
}

void display_write_char(char c)
{
	int i;
	for(i = 0; i < 6; i++)
	{
		display_write_byte(font_6x8[c - ' '][i], 1);
	}
}

void display_write_string(char *s)
{
  	while(*s)
	{
		display_write_char(*(s++));
	}
}

void display_test()
{
	display_write_string("C++ (pronounced as see plus plus) is a general-purpose programming language.");
}
