#include "player.h"
#include "codec.h"
#include "ff.h"

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


unsigned char	buff[2][BUFF_SIZE];		// Buffers
int 			cur_buff_ind = 0;		// Index of current buffer (0, 1)
int				is_next_buff_ready = 0;	// Is next buffer ready
int				buff_ptr = 0;			// Current sample pointer

int 			cur_data[2];			// Left, right
int 			cur_side_ind = 0;		// Index of current ear (0, 1)

int				volume = 160;
const int		vol_step = 1;
const int		vol_low_bound = 52;
const int		vol_up_bound = 255;

player_status_t player_status = READY;	// Player status
FIL file;								// Current file


void player_timer_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 1000000 / 44100 - 1; // down to 44100 Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 360 / 2 / 2 - 1; // down to 1 MHz
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void player_i2s_it_init()
{
    NVIC_InitTypeDef   NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = SPI3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_TXE, ENABLE);
}

void player_init()
{
	codec_init();
	codec_ctrl_init();

	player_timer_init();
	player_i2s_it_init();

	player_volume_set(volume);
}

void player_task()	//todo ...
{
	while(1)
	{
		if (player_status == PLAYING && !is_next_buff_ready)
		{
			UINT nRead;
			f_read(&file, &buff[!cur_buff_ind], BUFF_SIZE, &nRead);
			is_next_buff_ready = 1;

			if (nRead == 0)
			{
				player_stop();
			}
		}
		else
		{
			vTaskDelay(1);
		}
	}
}

player_status_t player_get_status()
{
	return player_status;
}

void player_enable_periphery()
{
	TIM_Cmd(TIM2, ENABLE);
	I2S_Cmd(SPI3, ENABLE);
}

void player_disable_periphery()
{
	TIM_Cmd(TIM2, DISABLE);
//	I2S_Cmd(SPI3, DISABLE);		//todo
}

void player_volume_add()
{
	volume += vol_step;
	player_volume_set(volume);
}

void player_volume_sub()
{
	volume -= vol_step;
	player_volume_set(volume);
}

int player_volume_get()
{
	return volume;
}

int player_volume_set(int vol)
{
	if (vol > vol_up_bound || vol < vol_low_bound)
	{
		return 0;	// Wrong value
	}

	unsigned char commandBuff[2];

	commandBuff[0] = CODEC_MAP_MASTER_A_VOL;
	commandBuff[1] = vol;
	codec_send_ctrl(commandBuff, 2);

	return 1;
}


// Interrupts

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

        if (buff_ptr >= BUFF_SIZE)
        {
        	buff_ptr = 0;
        	cur_buff_ind = !cur_buff_ind;
        	is_next_buff_ready = 0;
        }

        char smallBit = buff[cur_buff_ind][buff_ptr++];
        char bigBit = buff[cur_buff_ind][buff_ptr++];
        cur_data[0] = bigBit * 256 + smallBit;	// Left data

        smallBit = buff[cur_buff_ind][buff_ptr++];
        bigBit = buff[cur_buff_ind][buff_ptr++];
        cur_data[1] = bigBit * 256 + smallBit;	// Right data
    }
}

void SPI3_IRQHandler(void)
{
    if (SPI_I2S_GetITStatus(SPI3, SPI_I2S_IT_TXE) != RESET)
    {
        SPI_I2S_ClearITPendingBit(SPI3, SPI_I2S_IT_TXE);

        SPI_I2S_SendData(SPI3, cur_data[cur_side_ind]);
        cur_side_ind = !cur_side_ind;
    }
}

// End of interrupts

int player_play(char *file_name)
{
	player_stop();

	// Load file
	FRESULT result = f_open(&file, file_name, FA_OPEN_EXISTING | FA_READ);

    if (result != FR_OK)
    {
    	return 0;	// File load error
	}

    player_enable_periphery();
	player_status = PLAYING;

	return 1;	// Ok
}

void player_pause()
{
	if (player_status == PLAYING)
	{
		player_disable_periphery();
		player_status = PAUSED;
	}
}

void player_resume()
{
	if (player_status == PAUSED)
	{
		player_enable_periphery();
		player_status = PLAYING;
	}
}

void player_stop()
{
	if (player_status == PLAYING || player_status == PAUSED)
	{
		f_close(&file);
		player_disable_periphery();
		player_status = READY;
	}
}
