#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
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
#include "player.h"
#include "ui.h"
#include "controller.h"

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

void test_task()
{
//	vTaskDelay(1000 / portTICK_PERIOD_MS);
//    player_play("onl_s_16.wav");
//	vTaskDelay(5000 / portTICK_PERIOD_MS);
    player_play("bst_s_16.wav");
//	vTaskDelay(5000 / portTICK_PERIOD_MS);
//    player_pause();
//	vTaskDelay(5000 / portTICK_PERIOD_MS);
//    player_resume();
//	vTaskDelay(5000 / portTICK_PERIOD_MS);
//    player_stop();
//	vTaskDelay(5000 / portTICK_PERIOD_MS);
//    player_play("try_s_16.wav");

    vTaskDelete(NULL);
}

FATFS FATFS_Obj;

void init_fs()	//todo
{
	FRESULT result = f_mount(&FATFS_Obj, "0", 1);

//    if (result == FR_OK) {
//    	display_write_string("Fs mounted. ");
//    } else {
//    	display_write_string("Mount error. ");
//    }
}

int main(void)
{
	SystemInit();

	display_init();
//    display_write_string("Hola! ");

    init_fs();

    player_init();
    ui_init();
    controller_init();

    xTaskCreate(player_task, "player_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
    xTaskCreate(controller_task, "ctrl_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ui_task, "ui_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

//    xTaskCreate(test_task, "test_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

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
