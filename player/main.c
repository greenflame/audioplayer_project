#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_spi.h"
#include "misc.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "croutine.h"
#include "task.h"
#include "queue.h"

#include "display.h"
#include "ff.h"
#include "codec.h"

#define LED_1 GPIO_Pin_12
#define LED_2 GPIO_Pin_13
#define LED_3 GPIO_Pin_14
#define LED_4 GPIO_Pin_15
#define LED_ALL (LED_1 | LED_2 | LED_3 | LED_4)

void sleep(int n)
{
	while (n > 0) { n--; }
}

void init_leds()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef GPIO_InitDef;
	GPIO_InitDef.GPIO_Pin = LED_ALL;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitDef.GPIO_OType = GPIO_OType_PP;
    GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_100MHz;

    GPIO_Init(GPIOD, &GPIO_InitDef);
}

void init_tim(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 1000000 / 441 - 1; // down to 1 Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 360 / 2 / 2 - 1; // down to 1 MHz
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

#define BUFF_SIZE 102400
FIL file;
UINT nRead;
char buff[BUFF_SIZE];
int ptr = BUFF_SIZE;

void TIM2_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

	SPI_I2S_SendData(SPI3, 0);

//	if (ptr >= BUFF_SIZE)
//	{
//		f_read(&file, &buff, BUFF_SIZE, &nRead);
//		ptr = 0;
//	}
//	else
//	{
//		SPI_I2S_SendData(SPI3, 0xff);
		while(!SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE));
//	}
}

int main(void)
{
	SystemInit();
	while (!RCC_WaitForHSEStartUp());

	// init dac
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	codec_init();
	codec_ctrl_init();
	I2S_Cmd(CODEC_I2S, ENABLE);
	// init dac

    display_init();
    display_write_string("Hola! ");

    // fat
    FATFS FATFS_Obj;
    FRESULT result;

    result = f_mount(&FATFS_Obj, "0", 1);

    if (result == FR_OK) {
    	display_write_string("Fs mounted. ");
    } else {
    	display_write_string("Mount error. ");
    }

    result = f_open(&file, "a.wav", FA_OPEN_EXISTING | FA_READ);

    if (result == FR_OK) {
    	display_write_string("File opened. ");
    } else {
    	display_write_string("Open error. ");
	}
    // fat

    display_write_string("Begin! ");

    while (1)
    {
		if (ptr >= BUFF_SIZE)
		{
			f_read(&file, &buff, BUFF_SIZE, &nRead);
			ptr = 0;
		}
		else
		{
			SPI_I2S_SendData(SPI3, buff[ptr]);
			while(!SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE));
			SPI_I2S_SendData(SPI3, buff[ptr]);
			while(!SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE));
			ptr++;
		}
    }

//    int i = 1000000;
//    while (i--)
//    {
//		SPI_I2S_SendData(SPI3, 0);
//		while(!SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE));
//    }

    display_write_string("End. ");

//    init_tim();

    while(1) {}
}


// Hooks
void vApplicationIdleHook(void)
{
}

void vApplicationMallocFailedHook(void)
{
    for( ;; );
}

void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    for( ;; );
}

void vApplicationTickHook(void)
{
}
