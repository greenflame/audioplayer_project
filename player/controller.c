#include "controller.h"
#include "player.h"
#include "ui.h"
#include "display.h"

#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "croutine.h"
#include "task.h"


// Old states
int		sw_old_state = 1;		// Button, GPIOE PIN0
int		clk_old_state = 1;		// Rotation, GPIOE PIN0, DT - E2

void controller_init()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
}

void controller_tick()
{
	int sw_new_state = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1);
	int clk_new_state = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0);

	if (clk_old_state && !clk_new_state)	// Rotation
	{
		// Direction
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2))
		{
			ui_down_handler();
		}
		else
		{
			ui_up_handler();
		}
	}
	else if (sw_old_state && !sw_new_state)	// Button press
	{
		ui_press_handler();
	}

	sw_old_state = sw_new_state;
	clk_old_state = clk_new_state;
}

void controller_task()
{
	while (1)
	{
		controller_tick();
		vTaskDelay(1);
	}
}
