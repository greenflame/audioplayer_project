#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "misc.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "croutine.h"
#include "task.h"
#include "queue.h"

int pins1 = GPIO_Pin_12 | GPIO_Pin_13;
int pins2 = GPIO_Pin_14 | GPIO_Pin_15;

void sleep(int n)
{
	while (n > 0) { n--; }
}

void init()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef GPIO_InitDef;


	GPIO_InitDef.GPIO_Pin = pins1 | pins2;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitDef.GPIO_OType = GPIO_OType_PP;
    GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_100MHz;

    GPIO_Init(GPIOD, &GPIO_InitDef);
}

void task1(void *pvParameters)
{
	int d = 100 / portTICK_RATE_MS;
    while (1) {
//            GPIO_SetBits(GPIOD, pins1);
//            vTaskDelay(d);
//            sleep(1800000);
//            GPIO_ResetBits(GPIOD, pins1);
//            vTaskDelay(d);
//            sleep(1800000);
    }
    vTaskDelete(NULL);
}

void task2(void *pvParameters)
{
//    while (1) {
//            GPIO_SetBits(GPIOD, pins2);
//            sleep(300000);
//            GPIO_ResetBits(GPIOD, pins2);
//            sleep(300000);
//            vTaskDelay(1);
//    }
    vTaskDelete(NULL);
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
	}
}

void INTTIM_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable the TIM2 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	/* Time base configuration */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 1000000 - 1; // down to 1 Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 336 / 2 - 1; // down to 1 MHz
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	/* TIM IT enable */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);
}

int main(void)
{
	SystemInit();
	while (!RCC_WaitForHSEStartUp());

    init();
    INTTIM_Config();
    xTaskCreate(task1, "LedTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
//    xTaskCreate(task2, "LedTask2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskStartScheduler();
}

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
