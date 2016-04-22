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
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))


// Track list
char	track_list[20][20];		// List of file names
int		track_list_size = 0;	// List size
int		track_ptr = 0;			// Active track

// Track list representation
int		disp_ln_cnt = 5;		// Display lines count
int		disp_list_shift = 0;	// List shift

// Top menu
int		menu_items_cnt = 4;		// Top menu items count
int		menu_ptr = 0;			// Selected menu item

int		is_menu_active = 0;		// Menu / track list active
int		is_in_vol_ctrl = 0;		// Is user controlling volume

// Volume indicator
const int	vol_mid = 160;		// Default volume value
const int	vol_ind_step = 5;
const int	vol_ind_size = 6;

void ui_down_handler()
{
	if (is_in_vol_ctrl)
	{
		player_volume_sub();
	}
	else
	{
		if (is_menu_active)	// Menu active
		{
			if (menu_ptr != 0)
			{
				menu_ptr--;
			}
		}
		else					// Track list active
		{
			if (track_ptr == 0)	// Switch to menu
			{
				is_menu_active = 1;
				menu_ptr = menu_items_cnt - 1;
			}
			else
			{
				track_ptr--;

				if (track_ptr < disp_list_shift)	// List shift adjust
				{
					disp_list_shift = track_ptr;
				}
			}
		}
	}
}

void ui_up_handler()
{
	if (is_in_vol_ctrl)
	{
		player_volume_add();
	}
	else
	{
		if (is_menu_active)	// Top menu active
		{
			if (menu_ptr == menu_items_cnt - 1)	// Switch to track list
			{
				is_menu_active = 0;
				track_ptr = 0;
			}
			else
			{
				menu_ptr++;
			}
		}
		else				// Track list active
		{
			if (track_ptr < track_list_size - 1)	// Not last item
			{
				track_ptr++;
			}

			if (track_ptr >= disp_list_shift + disp_ln_cnt)	// List shift adjust
			{
				disp_list_shift = track_ptr - disp_ln_cnt + 1;
			}
		}
	}
}

void ui_press_handler()
{
	if (is_menu_active)
	{
		switch (menu_ptr)
		{
		case 0:	// Play
			player_resume();
			break;
		case 1:	// Pause
			player_pause();
			break;
		case 2:	// Stop
			player_stop();
			break;
		case 3:	// Volume in / out
			is_in_vol_ctrl = !is_in_vol_ctrl;
			break;
		}
	}
	else
	{
		player_play(track_list[track_ptr]);
	}
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
        	int is_dir = fno.fattrib & AM_DIR;
        	int is_wav = strcmp(fno.fname + strlen(fno.fname) - 3, "WAV") == 0;
        	if (!is_dir && is_wav)
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

void ui_render_menu()
{
	// Play
	int is_char_selected = is_menu_active && menu_ptr == 0;
	display_write_control_char(CHAR_PLAY, is_char_selected);

	// Pause
	is_char_selected = is_menu_active && menu_ptr == 1;
	display_write_control_char(CHAR_PAUSE, is_char_selected);

	// Stop
	is_char_selected = is_menu_active && menu_ptr == 2;
	display_write_control_char(CHAR_STOP, is_char_selected);

	//Volume
	is_char_selected = is_menu_active && menu_ptr == 3;
	display_write_control_char(CHAR_VOLUME, is_char_selected);

	int vol_ind_val = MAX(0, (player_volume_get() - vol_mid) / vol_ind_step + vol_ind_size / 2);

	int i;
	for (i = 0; i < vol_ind_val; i++)
	{
		display_write_control_char(CHAR_VOL_HIGH, is_char_selected);
	}

	for (i = 0; i < vol_ind_size - vol_ind_val; i++)
	{
		display_write_control_char(CHAR_VOL_LOW, is_char_selected);
	}
}

void ui_render_track_list()
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

		display_set_XY(0, i - disp_list_shift + 1);
		if (!is_menu_active && i == track_ptr)
		{
			display_write_string_inverted(str);
		}
		else
		{
			display_write_string(str);
		}
	}
}

void ui_display_update()
{
	display_set_XY(0, 0);

	ui_render_menu();
	ui_render_track_list();
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
