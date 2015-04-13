#include <kernel.h>

#define SHELL_WND_WIDTH 61

WINDOW shell_wnd = {0, 9, SHELL_WND_WIDTH, 16, 0, 0, 0xDC};

void run_command(char* buffer, int cmd)
{

	wprintf(&shell_wnd, "\n%d\n", cmd);

	while (*buffer != '\0') {
		wprintf(&shell_wnd, "%c", *buffer);
		buffer++;
	}

	wprintf(&shell_wnd, "\n");
	return;
}

void shell_prompt()
{
    wprintf(&shell_wnd, "> ");
}

void shell_process(PROCESS self, PARAM param)
{
	int execute_command;
	int number_of_chars;
	char buffer[SHELL_WND_WIDTH];
	clear_window(&shell_wnd);

	char ch;
	Keyb_Message msg;

	while (1) {
		execute_command = FALSE;
		number_of_chars = 0;
		shell_prompt();
		while (!execute_command) {

			msg.key_buffer = &ch;
			send(keyb_port, &msg);

			switch(ch) {
				case 13:
					execute_command = TRUE;
					wprintf(&shell_wnd, '\n');
					buffer[number_of_chars] = '\0';

					break;
				case '\b':
					if(number_of_chars != 0){
						number_of_chars--;
						wprintf(&shell_wnd, "%c", ch);
					}
					else {
						continue;
					}

					break;
				default:
					buffer[number_of_chars] = ch;
					number_of_chars++;
					wprintf(&shell_wnd, "%c", ch);

					break;
			}

		}
		run_command(buffer, number_of_chars);
	}
}


void init_shell()
{
	create_process(shell_process, 5, 0, "Shell Process");
	resign();
}
