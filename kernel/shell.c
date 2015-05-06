#include <kernel.h>

#define SHELL_WND_WIDTH 61

WINDOW shell_wnd = {0, 9, SHELL_WND_WIDTH, 16, 0, 0, 0xDC};
WINDOW train_wnd = {0, 0, 80, 8, 0, 0, ' '};

//int number_of_chars;
//char buffer[80];

void run_train(WINDOW *wnd)
{
	init_train(wnd);
}

void set_switch_position(char position, char switch_char)
{
	set_switch(position, switch_char);
}

int buffer_has_empty(char *buffer)
{
	int i;
	int has_empty = FALSE;
	int buffer_size = k_strlen(buffer);
	for(i=0; i<buffer_size; i++){
		if(buffer[i]==32){
			has_empty = TRUE;
		}
	}
	return has_empty;
}

/* Get command and next 2 arguments */
void* get_command(char *command, char *arg1, char *arg2, char *buffer)
{
	int i;
	int j;
	int k;
	int command_size;
	int buffer_size = k_strlen(buffer);
	for(i=0; i<buffer_size; i++){
		if(buffer[i]==32){
			command[i] = '\0';
			break;
		}
		else {
			command[i] = buffer[i];
		}
	}
	// Skip the space
	for(j=i; j<buffer_size; j++){
		if(buffer[j]==32){
			continue;
		}
		else {
			break;
		}
	}
	// Get argument 1
	for(i=0, k=j; k<buffer_size; k++, i++){
		if(buffer[k]==32){
			arg1[i] = '\0';
			break;
		}
		else {
			arg1[i] = buffer[k];
		}
	}

	// Skip the space
	for(j=k; j<buffer_size; j++){
		if(buffer[j]==32){
			continue;
		}
		else {
			break;
		}
	}
	// Get argument 2
	for(i=0, k=j; k<buffer_size; k++, i++){

		arg2[i] = buffer[k];
		arg2[i+1] = '\0';
	}
	return command;
}

void run_command(char *buffer, int cmd)
{
	char command[9];
	char arg1[2];
	char arg2[2];

	get_command(command, arg1, arg2, buffer);

	wprintf(&shell_wnd, "\n");

	if(strings_equal(buffer, "")) {
		return;
	}
	if(strings_equal(buffer, "help")) {
		wprintf(&shell_wnd, "Available commands:\n");
		wprintf(&shell_wnd, "clear               =>   Clear window\n");
		wprintf(&shell_wnd, "ps                  =>   Print process table\n");
		wprintf(&shell_wnd, "train               =>   Run train application\n");
		wprintf(&shell_wnd, "stoptrain           =>   Stop train application\n");
		wprintf(&shell_wnd, "starttrain          =>   Start train\n");
		wprintf(&shell_wnd, "slowdown            =>   Slow down train to 2\n");
		wprintf(&shell_wnd, "accelerate          =>   Accelerate to 5\n");
		wprintf(&shell_wnd, "starttrain          =>   Start train\n");
		wprintf(&shell_wnd, "setswitch <n> <R|G> =>   Set switch number n to color R|G\n");
		wprintf(&shell_wnd, "changedir           =>   Change train's direction\n");
		wprintf(&shell_wnd, "help                =>   Print list of all commands\n");
		return;
	}
	if(strings_equal(buffer, "clear")) {
		clear_window(&shell_wnd);
		return;
	}
	if(strings_equal(buffer, "ps")) {
		print_all_processes(&shell_wnd);
		return;
	}
	if(strings_equal(buffer, "train")) {
		run_train(&train_wnd);
		return;
	}
	if(strings_equal(buffer, "stoptrain")) {
		stop_train();
		return;
	}
	if(strings_equal(buffer, "starttrain")) {
		set_speed('5');
		return;
	}
	if(strings_equal(buffer, "slowdown")) {
		set_speed('2');
		return;
	}
	if(strings_equal(buffer, "accelerate")) {
		set_speed('5');
		return;
	}
	if(strings_equal(buffer, "changedir")) {
		change_direction();
		return;
	}
	if(strings_equal(buffer, "setswitch")) {
		wprintf(&shell_wnd, "Usage: setswitch <switch number> <R|G>\n");

		return;
	}
	if(strings_equal(command, "setswitch")) {
		if(arg2[0]!='G' && arg2[0]!='R'){
			wprintf(&shell_wnd, "Usage: setswitch <switch number> <R|G>\n");

			return;
		}
		set_switch_position(arg1[0], arg2[0]);

		return;
	}

	wprintf(&shell_wnd, "\n");
	output_string(&shell_wnd, buffer);
	wprintf(&shell_wnd, ": command not found. Type 'help' for help.");
	wprintf(&shell_wnd, "\n");
	return;
}

void shell_header(WINDOW *wnd)
{
	wprintf(wnd, "-------------------------------------------------------------");
	wprintf(wnd, "------------------------- TOS SHELL -------------------------");
	wprintf(wnd, "-------------------------------------------------------------");

}

void shell_prompt()
{
    wprintf(&shell_wnd, "> ");
}

void shell_process(PROCESS self, PARAM param)
{
	int execute_command;
	int number_of_chars;
	char buffer[80];

	char ch;
	Keyb_Message msg;

	shell_header(&shell_wnd);
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
				case 8:
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
	clear_window(kernel_window);
	create_process(shell_process, 5, 0, "Shell Process");
	resign();
}
