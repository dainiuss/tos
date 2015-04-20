#include <kernel.h>

WINDOW* train_wnd;
#define TRAIN_SLEEP_TICK 15;

int sleep_tick = TRAIN_SLEEP_TICK;

void send_signal_to_com1(char *cmd, int len_input_buffer, char* input_buffer);
void set_speed(char* speed);
void set_switch(char* switch_char);
void train_process(PROCESS self, PARAM param);
void initialize_switches();

void send_signal_to_com1(char *cmd, int len_input_buffer, char* input_buffer)
{
	COM_Message msg;

	msg.output_buffer = cmd;
	msg.len_input_buffer = len_input_buffer;
	msg.input_buffer = input_buffer;

	send(com_port, &msg);
	sleep(sleep_tick);
}

// L20S4\015
void set_speed(char* speed)
{
	char empty;

	char cmd[6];
	cmd[0] = 'L';
	cmd[1] = '2';
	cmd[2] = '0';
	cmd[3] = 'S';
	cmd[4] = speed;
	cmd[5] = '\015';

	send_signal_to_com1(cmd, 0, empty);
}
// ‘M5R\015
void set_switch(char* switch_char)
{
	char empty;

	char cmd[4];
	cmd[0] = 'M';
	cmd[1] = '5';
	cmd[2] = switch_char;
	cmd[3] = '\015';

	send_signal_to_com1(cmd, 0, empty);
}

void train_process(PROCESS self, PARAM param)
{
	train_wnd = (WINDOW*) param;

	initialize_switches();

	wprintf(train_wnd, ".");
	char *speed = '5';
	set_speed(speed);
	remove_ready_queue(active_proc);
	resign();

}

void initialize_switches()
{
	wprintf(train_wnd, "Initializing the railroad.");
}


void init_train(WINDOW* wnd)
{
	wprintf(wnd, "Initializing TOS Train...\n");
	create_process(train_process, 5, (PARAM) wnd, "Train process");
}
