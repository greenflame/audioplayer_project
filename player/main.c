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

void leds_init()
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

void timer_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 1000000 / 44100 - 1; // down to 1 Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 360 / 2 / 2 - 1; // down to 1 MHz
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

void i2s_it_init()
{
    NVIC_InitTypeDef   NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = SPI3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_TXE, ENABLE);
}

#define BUFF_SIZE 10240

int current_data[2];	// Left, right
int current_side = 0;	// Left, right (0, 1)

char buff[2][BUFF_SIZE];
int curBuffInd = 0;		// Index of current buffer (0, 1)
int nextBuffReady = 0;	// Is next buffer ready
int curBuffPtr = 0;		// Current sample pointer

FATFS FATFS_Obj;
FIL file;				// Current file
TaskHandle_t readerTask_handler;

void open_file(char *file_name)
{
    FRESULT result;

    result = f_mount(&FATFS_Obj, "0", 1);

    if (result == FR_OK) {
    	display_write_string("Fs mounted. ");
    } else {
    	display_write_string("Mount error. ");
    }

    result = f_open(&file, file_name, FA_OPEN_EXISTING | FA_READ);

    if (result == FR_OK) {
    	display_write_string("File opened. ");
    } else {
    	display_write_string("Open error. ");
	}
}

void readerTask(void *pvParameters)
{
    timer_init();
    while(1) {
    	if (!nextBuffReady)
    	{
    		UINT nRead;
			f_read(&file, &buff[!curBuffInd], BUFF_SIZE, &nRead);
			nextBuffReady = 1;
    	} else {
    		vTaskDelay(1);
    	}
    }

    vTaskDelete(NULL);
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

        if (curBuffPtr >= BUFF_SIZE)
        {
        	curBuffPtr = 0;
        	curBuffInd = !curBuffInd;
        	nextBuffReady = 0;
        }

        char smallBit = buff[curBuffInd][curBuffPtr++];
        char bigBit = buff[curBuffInd][curBuffPtr++];
        current_data[0] = bigBit * 256 + smallBit;	// Left data

        smallBit = buff[curBuffInd][curBuffPtr++];
        bigBit = buff[curBuffInd][curBuffPtr++];
        current_data[1] = bigBit * 256 + smallBit;	// Right data
    }
}

void SPI3_IRQHandler(void)
{
    if (SPI_I2S_GetITStatus(SPI3, SPI_I2S_IT_TXE) != RESET)
    {
        SPI_I2S_ClearITPendingBit(SPI3, SPI_I2S_IT_TXE);

        SPI_I2S_SendData(SPI3, current_data[current_side]);
        current_side = !current_side;
    }
}

void set_vol()
{
	unsigned char commandBuff[2];

	commandBuff[0] = CODEC_MAP_MASTER_A_VOL;
	commandBuff[1] = 0b10100000;

	send_codec_ctrl(commandBuff, 2);
}

int main(void)
{
	SystemInit();

	display_init();
    display_write_string("Loading! ");

	codec_init();
	codec_ctrl_init();

    i2s_it_init();

    open_file("bst_s_16.wav");

    set_vol();

    xTaskCreate(readerTask, "readerTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &readerTask_handler);
    vTaskStartScheduler();

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
