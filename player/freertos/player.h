#ifndef PLAYER_H
#define PLAYER_H


#define BUFF_SIZE 10240

typedef enum {
	READY,
	PLAYING,
	PAUSED
} player_status_t;

void player_init();
void player_task();
player_status_t player_get_status();

int player_play(char *file_name);
void player_pause();
void player_resume();
void player_stop();

//void player_volume_inc();	//todo


#endif
