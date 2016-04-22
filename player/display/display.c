#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"

#include "display.h"
#include "fonts.h"

#define RST GPIO_Pin_0	// Reset
#define CE  GPIO_Pin_1	// Chip enable
#define DC  GPIO_Pin_2	// Data/command
#define DIN GPIO_Pin_7	// MOSI for SPI
#define CLK GPIO_Pin_5	// Clock for SPI

#define GPIO_DISPLAY					GPIOA
#define RCC_AHB2Periph_GPIO_DISPLAY		RCC_AHB1Periph_GPIOA
#define RCC_APB2Periph_SPI_DISPLAY		RCC_APB2Periph_SPI1
#define SPI_DISPLAY						SPI1

#define GPIO_PinSource_MOSI				GPIO_PinSource7
#define GPIO_PinSource_CLOCK			GPIO_PinSource5

void display_set_pin(uint16_t pin, char val)
{
	if (val) GPIO_SetBits(GPIO_DISPLAY, pin);
	else GPIO_ResetBits(GPIO_DISPLAY, pin);
}

void display_gpio_spi_config()
{
	// Configure GPIO(RST, CE, DC)
	RCC_AHB1PeriphClockCmd(RCC_AHB2Periph_GPIO_DISPLAY, ENABLE);

	GPIO_InitTypeDef gpio_init_struct;
	gpio_init_struct.GPIO_Pin = RST | CE | DC;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_100MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_OUT;
	gpio_init_struct.GPIO_OType = GPIO_OType_PP;
	gpio_init_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIO_DISPLAY, &gpio_init_struct);

	// Configure SPI(DIN, CLK)
	gpio_init_struct.GPIO_Pin = DIN | CLK;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_100MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_AF;
	gpio_init_struct.GPIO_OType = GPIO_OType_PP;
	gpio_init_struct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIO_DISPLAY, &gpio_init_struct);

	GPIO_PinAFConfig(GPIO_DISPLAY, GPIO_PinSource_MOSI, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIO_DISPLAY, GPIO_PinSource_CLOCK, GPIO_AF_SPI1);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI_DISPLAY, ENABLE);

	SPI_InitTypeDef spi_ini_struct;
	spi_ini_struct.SPI_Direction = SPI_Direction_1Line_Tx;
	spi_ini_struct.SPI_DataSize = SPI_DataSize_8b;
	spi_ini_struct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	spi_ini_struct.SPI_FirstBit = SPI_FirstBit_MSB;
	spi_ini_struct.SPI_Mode = SPI_Mode_Master;
	spi_ini_struct.SPI_CPHA = SPI_CPHA_1Edge;
	spi_ini_struct.SPI_CPOL = SPI_CPOL_Low;
	spi_ini_struct.SPI_NSS = SPI_NSS_Soft;
	spi_ini_struct.SPI_CRCPolynomial = SPI_CRC_Tx;

	SPI_Init(SPI_DISPLAY, &spi_ini_struct);
	SPI_Cmd(SPI_DISPLAY, ENABLE);
}

void display_write_byte(unsigned char data, char mode)
{
	display_set_pin(CE, 0);
	display_set_pin(DC, mode);

	SPI_I2S_SendData(SPI_DISPLAY, data);
	while(SPI_I2S_GetFlagStatus(SPI_DISPLAY, SPI_I2S_FLAG_BSY) == SET);

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
	display_gpio_spi_config();

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

void display_write_char_inverted(char c)
{
	int i;
	for(i = 0; i < 6; i++)
	{
		display_write_byte(~font_6x8[c - ' '][i], 1);
	}
}

void display_write_string_inverted(char *s)
{
  	while(*s)
	{
		display_write_char_inverted(*(s++));
	}
}

void display_write_control_char(int c, int is_selected)
{
	int length;
	const int char_select_interval = 10;

	if (c != CHAR_VOL_LOW && c != CHAR_VOL_HIGH)
	{
		length = 2;
	}
	else
	{
		length = 1;
	}

	int i;
	for (i = 0; i < length; i++)
	{
		if (is_selected)
		{
			display_write_char_inverted(c + char_select_interval + i);
		}
		else
		{
			display_write_char(c + i);
		}
	}
}

void display_write_int(int i)
{
	int ptr = 1;
	while (ptr * 10 < i)
	{
		ptr *= 10;
	}

	while (ptr != 0)
	{
		display_write_char('0' + i / ptr);
		i = i % ptr;
		ptr /= 10;
	}
}
