#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#include "display.h"
#include "ff.h"

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

char    buff[1024];             // буфер для чтения/записи

int main(void)
{
	led_init();

	FRESULT result;
    FATFS FATFS_Obj;

    display_init();
	display_write_string("The cake is a lie. ");


	result = f_mount(&FATFS_Obj, "0", 1);

    if (result != FR_OK)
    {
    	display_write_string("Mount error. ");
    }


    DIR dir;
    FILINFO fileInfo;
    int nFiles = 0;

    result = f_opendir(&dir, "/");
    if (result == FR_OK)
    {
            while (((result = f_readdir(&dir, &fileInfo)) == FR_OK) && fileInfo.fname[0])
            {
                    nFiles++;
            }
    }
    f_closedir(&dir);


    FIL file;
    UINT nRead, nWritten;

    result = f_open(&file, "r.txt", FA_OPEN_EXISTING | FA_READ);
    if (result == FR_OK)
    {
    	display_write_string("File opened. ");
            f_read(&file, &buff, 1024, &nRead);
            f_close(&file);

//            buff[5] = 0;
            display_write_string(buff);
    }

    result = f_open(&file, "write.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (result == FR_OK)
    {
    	display_write_string("File write. ");
            f_write(&file, &buff, nRead, &nWritten);
            f_close(&file);
    }

    while(1)
    {
    	led_set(1);
    	sleep(4000000);
    	led_set(0);
    	sleep(4000000);
    }
}
