#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#include "display.h"

void sleep(int n)
{
	while(n > 0) n--;
}

void led_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIOC_Init;

	GPIOC_Init.GPIO_Pin = GPIO_Pin_13;
	GPIOC_Init.GPIO_Speed = GPIO_Speed_10MHz;
	GPIOC_Init.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_Init(GPIOC, &GPIOC_Init);
}

void led_set(char c)
{
	if (c) GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	else GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

int main(void)
{
	display_init();
	display_test();

	led_init();

    while(1)
    {
    	led_set(1);
    	sleep(2000000);
    	led_set(0);
    	sleep(1000000);
    }
}
