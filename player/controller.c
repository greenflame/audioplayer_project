#include "controller.h"
#include "player.h"
#include "ui.h"

#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

// Old states
int		sw_old_state = 1;		// Button, GPIOE PIN0
int		clk_old_state = 1;		// Rotation, GPIOE PIN3, DT - E2

void controller_init()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_2;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
}

void controller_tick()
{
	int sw_new_state = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0);
	int clk_new_state = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3);

	// Button press
	if (sw_old_state && !sw_new_state)
	{
		ui_press_handler();
	}

	// Rotation
	if (clk_old_state && !clk_new_state)
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
