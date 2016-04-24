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

FATFS FATFS_Obj;

int main(void)
{
	// Initializing
	SystemInit();

	display_init();

	FRESULT result = f_mount(&FATFS_Obj, "0", 1);
    if (result != FR_OK) {
    	display_write_string("Flash mount error.");
    	while (1) {}
    }

    controller_init();
    player_init();
    ui_init();


    // Creating tasks
    xTaskCreate(player_task, "player_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
    xTaskCreate(controller_task, "ctrl_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ui_task, "ui_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    // Run
    vTaskStartScheduler();
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
