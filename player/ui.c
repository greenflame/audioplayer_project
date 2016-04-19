#include "ui.h"
#include "player.h"
#include "ff.h"
#include "display.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "croutine.h"
#include "task.h"

#include <string.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))


// Track list
char	track_list[20][20];		// List of file names
int		track_list_size = 0;	// List size
int		track_ptr = 0;			// Active track

int		disp_ln_cnt = 6;		// Display lines count
int		disp_list_shift = 0;	// List shift

void ui_down_handler()
{
	track_ptr--;

	if (track_ptr < 0)	// Pointer adjust
	{
		track_ptr = 0;
	}

	if (track_ptr < disp_list_shift)	// List shift adjust
	{
		disp_list_shift = track_ptr;
	}
}

void ui_up_handler()
{
	track_ptr++;

	if (track_ptr >= track_list_size)	// Pointer adjust
	{
		track_ptr = track_list_size - 1;
	}

	if (track_ptr >= disp_list_shift + disp_ln_cnt)	// List shift adjust
	{
		disp_list_shift = track_ptr - disp_ln_cnt + 1;
	}
}

void ui_press_handler()
{
	player_play(track_list[track_ptr]);
}

void ui_scan_files()
{
    FRESULT res;
    DIR dir;
    FILINFO fno;

    track_list_size = 0;
    res = f_opendir(&dir, "/");
    if (res == FR_OK)
    {
        while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0)	// No errors, not last
        {
        	if (!(fno.fattrib & AM_DIR))	// File, not folder
            {
            	strcpy(track_list[track_list_size++], fno.fname);
            }
        }
        f_closedir(&dir);
    }
}

void int_to_str(char *str, int i)
{
	int ptr = 1;
	while (ptr * 10 < i)
	{
		ptr *= 10;
	}

	while (ptr > 0)
	{
		*(str++) = '0' + i / ptr;
		i %= ptr;
		ptr /= 10;
	}

	*str = 0;	// eol
}

void ui_display_update()
{
	int i;
	for (i = disp_list_shift; i < MIN(disp_list_shift + disp_ln_cnt, track_list_size); i++)
	{
		char str[20];
		int_to_str(str, i + 1);
		strcat(str, ".");
		strcat(str, track_list[i]);

		while (strlen(str) < 14)
		{
			strcat(str, " ");
		}

		display_set_XY(0, i - disp_list_shift);
		if (i == track_ptr)
		{
			display_write_string_inverted(str);
		}
		else
		{
			display_write_string(str);
		}
	}
}

void ui_init()
{
	ui_scan_files("/");
}

void ui_task()
{
	while (1)
	{
		vTaskDelay(100);
		ui_display_update();
	}
}
