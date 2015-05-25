#include <kernel.h>

WINDOW *train_window;

int sleep_tick = 15;
int time_tick = 15;
int short_sleep_tick = 3;
int long_sleep_tick = 60;

void send_signal_to_com1(char *cmd, int len_input_buffer, char* input_buffer);
void clear_buffer();
void set_speed(char speed);
void stop_train();
void change_direction();
char get_status_of_contact(char *contact_id);
int is_train8_car2();
int is_train5_car11();
int is_train5_car16();
void set_switch(char position, char switch_char);
void run_train_configuration_no_zamboni(int position_code);
void run_train_configuration_with_zamboni(int position_code, int zamboni_position_code);
void train_process(PROCESS self, PARAM param);
void initialize_switches();

/**
 * Send message to COM1 port
 * Called in all train commands
 */
void send_signal_to_com1(char *cmd, int len_input_buffer, char* input_buffer)
{
	COM_Message msg;

	msg.output_buffer = cmd;
	msg.len_input_buffer = len_input_buffer;
	msg.input_buffer = input_buffer;

	send(com_port, &msg);
	sleep(sleep_tick);
}

/**
 * Clear buffer
 * R\015
 */
void clear_buffer()
{
	char empty;

	char cmd[3];
	cmd[0] = 'R';
	cmd[1] = '\015';
	cmd[2] = '\0';

	send_signal_to_com1(cmd, 0, empty);
}

/**
 * Set speed
 * L20S4\015
 */
void set_speed(char speed)
{
	char empty;

	char cmd[7];
	cmd[0] = 'L';
	cmd[1] = '2';
	cmd[2] = '0';
	cmd[3] = 'S';
	cmd[4] = speed;
	cmd[5] = '\015';
	cmd[6] = '\0';

	send_signal_to_com1(cmd, 0, empty);
}

/**
 * Stop train
 * L20S0\015
 */
void stop_train()
{
	char empty;

	char cmd[7];
	cmd[0] = 'L';
	cmd[1] = '2';
	cmd[2] = '0';
	cmd[3] = 'S';
	cmd[4] = '0';
	cmd[5] = '\015';
	cmd[6] = '\0';

	send_signal_to_com1(cmd, 0, empty);
}

/**
 * Change train's direction
 * L20D\015
 */
void change_direction()
{
	char empty;

	char cmd[6];
	cmd[0] = 'L';
	cmd[1] = '2';
	cmd[2] = '0';
	cmd[3] = 'D';
	cmd[4] = '\015';
	cmd[5] = '\0';

	send_signal_to_com1(cmd, 0, empty);
}

/**
 * Probe the contact for the train
 * C3\015
 * *1\015 - returned if train on the contact
 * *0\015 - returned if no train on the contact
 */
char get_status_of_contact(char *contact_id)
{
	char result[3];
	clear_buffer();

	if(k_strlen(contact_id)==1){
		char cmd[4];
		cmd[0] = 'C';
		cmd[1] = contact_id[0];
		cmd[2] = '\015';
		cmd[3] = '\0';
		send_signal_to_com1(cmd, 3, result);
	}
	else{
		char cmd[5];
		cmd[0] = 'C';
		cmd[1] = contact_id[0];
		cmd[2] = contact_id[1];
		cmd[3] = '\015';
		cmd[4] = '\0';
		send_signal_to_com1(cmd, 3, result);
	}

	return result[1];
}

/**
 *  Find train and car is on position 8 and 2
 */
int is_train8_car2()
{
	int train = get_status_of_contact("8");
	int car = get_status_of_contact("2");
	if(train=='1' && car=='1'){
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/**
 *  Find train and car is on position 5 and 11
 */
int is_train5_car11()
{
	int train = get_status_of_contact("5");
	int car = get_status_of_contact("11");
	if(train=='1' && car=='1'){
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/**
 *  Find train and car is on position 5 and 16
 */
int is_train5_car16()
{
	int train = get_status_of_contact("5");
	int car = get_status_of_contact("16");
	if(train=='1' && car=='1'){
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/**
 * Find Zamboni on the rail
 * Clockwise direction status = 2
 * Counterclockwise direction status = 3
 * Direction unknown status = 1
 * No Zamboni status = 0
 */
int find_zamboni()
{
	int direction_status = 0;
	int is_zamboni = 0;
	int is_clockwise = 0;
	int is_counter_clockwise = 0;
	long i;
	for(i=0; i < 30; i++) {
		wprintf(train_window, "%d,", i);
		int status = get_status_of_contact("4");
		if(status=='1'){
			wprintf(train_window, "\nZamboni found\n");
			is_zamboni = 1;
			break;
		}
		sleep(time_tick);
	}

	// If zamboni is present - check direction
	if(is_zamboni) {
		for(i=0; i<15; i++) {
			int status3  = get_status_of_contact("3");
			int status15 = get_status_of_contact("15");
			int status14 = get_status_of_contact("14");

			int status6  = get_status_of_contact("6");
			int status7  = get_status_of_contact("7");

			if(status14=='1' || status15=='1' || status3=='1'){
				is_counter_clockwise = 1;
				break;
			}

			else if(status7=='1' || status6=='1'){
				is_clockwise = 1;
				break;
			}
			sleep(time_tick);
		}
	}

	if(is_zamboni) {
		if(is_clockwise){
			direction_status = 2;
		}
		else if(is_counter_clockwise){
			direction_status = 3;
		}
		else {
			direction_status = 1;
		}
	}

	return direction_status;
}

/**
 * Set switch position n to color <G|R>
 * M5R\015
 */
void set_switch(char position, char switch_char)
{
	char empty;

	char cmd[5];
	cmd[0] = 'M';
	cmd[1] = position;
	cmd[2] = switch_char;
	cmd[3] = '\015';
	cmd[4] = '\0';

	send_signal_to_com1(cmd, 0, empty);
}

/**
 * Run train configuration when
 * Zamboni is not present
 */
void run_train_configuration_no_zamboni(int position_code)
{
	char speed;
	int i;

	if(position_code==82){
		set_switch('3','G');
		set_switch('4','R');
		speed = '5';
		set_speed(speed);
		for(i=0; i < 30; i++) {
			int status2 = get_status_of_contact("6");
			if(status2=='1'){
				wprintf(train_window, "Close to car, decreasing speed\n");
				speed = '2';
				set_speed(speed);
				break;
			}
			sleep(time_tick);
		}
		for(i=0; i < 30; i++) {
			int status1 = get_status_of_contact("1");
			if(status1=='1'){
				wprintf(train_window, "Hooked up the car, going back\n");
				stop_train();
				change_direction();
				speed = '5';
				set_speed(speed);

				// Change switch so train can go back
				set_switch('5','R');
				set_switch('6','R');
				break;
			}
			sleep(time_tick);
		}
		for(i=0; i < 30; i++) {
			int status8 = get_status_of_contact("8");
			if(status8=='1'){
				wprintf(train_window, "Came back, stop the train\n");
				stop_train();
				break;
			}
			sleep(time_tick);
		}
	}
	else if(position_code==511){
		set_switch('3','R');
		set_switch('4','G');
		set_switch('5','R');
		set_switch('6','G');
		speed = '5';
		set_speed(speed);
		for(i=0; i < 30; i++) {
			int status12 = get_status_of_contact("12");
			if(status12=='1'){
				wprintf(train_window, "On track 12, Stop and reverse\n");
				speed = '2';
				set_speed(speed);
				stop_train();
				set_switch('7','R');
				set_switch('9','R');
				set_switch('1','G');
				change_direction();
				speed = '5';
				set_speed(speed);
				wprintf(train_window, "Hooked up the car, going back\n");
				break;
			}
			sleep(time_tick);
		}
		for(i=0; i < 30; i++) {
			int status6 = get_status_of_contact("6");
			if(status6=='1'){
				wprintf(train_window, "On track 6, stop and reverse\n");
				stop_train();
				change_direction();
				set_switch('4','R');
				speed = '5';
				set_speed(speed);
				break;
			}
			sleep(time_tick);
		}
		for(i=0; i < 30; i++) {
			int status5 = get_status_of_contact("5");
			if(status5=='1'){
				wprintf(train_window, "Came back, stop the train\n");
				stop_train();
				break;
			}
			sleep(time_tick);
		}
	}
	else if(position_code==516){
		set_switch('3','R');
		set_switch('4','G');
		set_switch('9','G');
		speed = '5';
		set_speed(speed);
		for(i=0; i < 30; i++) {
			int status14 = get_status_of_contact("14");
			if(status14=='1'){
				wprintf(train_window, "On track 14, Slow down\n");
				wprintf(train_window, "Sleeping\n");
				for(i=0; i<5; i++){
					wprintf(train_window, "%d,", i);
					sleep(long_sleep_tick);
				}
				wprintf(train_window, "\nSlowing down\n");
				speed = '2';
				set_speed(speed);
				for(i=0; i<5; i++){
					wprintf(train_window, "%d,", i);
					sleep(long_sleep_tick);
				}

				wprintf(train_window, "\nOn track 16, Stop\n");
				stop_train();
				set_switch('9','R');
				change_direction();
				speed = '5';
				set_speed(speed);
				wprintf(train_window, "Hooked up the car, going back\n");
				break;
			}
			sleep(short_sleep_tick);
		}
		for(i=0; i < 30; i++) {
			int status14 = get_status_of_contact("14");
			if(status14=='1'){
				wprintf(train_window, "On track 14, stop and reverse\n");
				stop_train();
				change_direction();
				set_switch('9','R');
				speed = '5';
				set_speed(speed);
				break;
			}
			sleep(time_tick);
		}
		for(i=0; i < 30; i++) {
			int status6 = get_status_of_contact("6");
			if(status6=='1'){
				wprintf(train_window, "On track 6, stop and reverse\n");
				stop_train();
				change_direction();
				set_switch('4','R');
				speed = '5';
				set_speed(speed);
				break;
			}
			sleep(time_tick);
		}
		for(i=0; i < 30; i++) {
			int status5 = get_status_of_contact("5");
			if(status5=='1'){
				wprintf(train_window, "Came back, stop the train\n");
				stop_train();
				break;
			}
			sleep(time_tick);
		}
	}
	else{
		wprintf(train_window, "Train position unknown!\n");
	}
}

/**
 * Run train configuration when
 * Zamboni is present
 * Zamboni Position:
 * 2 => clockwise
 * 3 => counter clockwise
 */

void run_train_configuration_with_zamboni(int position_code, int zamboni_position_code)
{
	char speed;
	int i;
	int zamboni_passed = FALSE;

	if(position_code==82){
		if(zamboni_position_code == 2){
			set_switch('3','G');
			set_switch('4','R');
			// Look for Zamboni
			for(i=0; i < 30; i++) {
				int status10 = get_status_of_contact("10");
				if(status10=='1'){
					wprintf(train_window, "Zamboni just passed\n");
					speed = '5';
					set_speed(speed);
					break;
				}
				sleep(time_tick);
			}
		}
		if(zamboni_position_code == 3){
			// Look for Zamboni
			for(i=0; i < 30; i++) {
				int status4 = get_status_of_contact("4");
				if(status4=='1'){
					wprintf(train_window, "Zamboni just passed\n");
					speed = '5';
					set_speed(speed);
					set_switch('3','G');
					set_switch('4','R');
					break;
				}
				sleep(time_tick);
			}
		}
		for(i=0; i < 30; i++) {
			int status2 = get_status_of_contact("6");
			if(status2=='1'){
				wprintf(train_window, "Close to car, decreasing speed\n");
				speed = '3';
				set_speed(speed);
				break;
			}
			sleep(time_tick);
		}
		if(zamboni_position_code == 3){
			// Close Zamboni in small loop
			set_switch('8','R');
		}
		for(i=0; i < 30; i++) {
			int status1 = get_status_of_contact("1");
			if(status1=='1'){
				wprintf(train_window, "Hooked up the car, go home\n");
				stop_train();
				change_direction();
				speed = '5';
				set_speed(speed);
				break;
			}
			sleep(time_tick);
		}
		if(zamboni_position_code == 2){
			// Look for Zamboni
			for(i=0; i < 30; i++) {
				int status10 = get_status_of_contact("10");
				if(status10=='1'){
					wprintf(train_window, "Zamboni passed home\n");
					// Change switch so train can go back
					set_switch('5','R');
					set_switch('6','R');
					speed = '5';
					set_speed(speed);
					break;
				}
				sleep(time_tick);
			}
		}
		if(zamboni_position_code == 3) {
			set_switch('5','R');
			set_switch('6','R');
		}
		for(i=0; i < 30; i++) {
			int status8 = get_status_of_contact("8");
			if(status8=='1'){
				wprintf(train_window, "Came back, stop the train\n");
				stop_train();
				// Switch the switch so Zamboni doesn't come here
				set_switch('5','G');
				break;
			}
			sleep(time_tick);
		}
	}
	else if(position_code==511){
		if(zamboni_position_code == 2){
			// Look for Zamboni
			for(i=0; i < 30; i++) {
				int status10 = get_status_of_contact("10");
				if(status10=='1'){
					wprintf(train_window, "Zamboni just passed\n");
					speed = '5';
					set_speed(speed);
					break;
				}
				sleep(time_tick);
			}
		}
		set_switch('3','R');
		set_switch('4','G');
		set_switch('5','R');
		set_switch('6','G');
		for(i=0; i < 30; i++) {
			int status9 = get_status_of_contact("9");
			if(status9=='1'){
				wprintf(train_window, "On track 9, close the switch\n");
				set_switch('5','G');
				break;
			}
			sleep(time_tick);
		}
		for(i=0; i < 30; i++) {
			int status12 = get_status_of_contact("12");
			if(status12=='1'){
				wprintf(train_window, "On track 12, Stop and reverse\n");
				speed = '2';
				set_speed(speed);
				stop_train();
				set_switch('7','R');
				set_switch('9','R');
				set_switch('1','G');
				change_direction();
				break;
			}
			sleep(time_tick);
		}
		if(zamboni_position_code == 2){
			// Look for Zamboni
			for(i=0; i < 30; i++) {
				int status13 = get_status_of_contact("13");
				if(status13=='1'){
					wprintf(train_window, "Zamboni just passed\n");
					speed = '5';
					set_speed(speed);
					break;
				}
				sleep(time_tick);
			}
		}

		for(i=0; i < 30; i++) {
			int status7 = get_status_of_contact("7");
			if(status7=='1'){
				wprintf(train_window, "Zamboni passed track 7\n");
				zamboni_passed = TRUE;
				break;
			}
			sleep(time_tick);
		}
		if(zamboni_passed) {
			for(i=0; i < 30; i++) {
				int status6 = get_status_of_contact("6");
				if(status6=='1'){
					wprintf(train_window, "On track 6, stop and reverse\n");
					stop_train();
					change_direction();
					set_switch('4','R');
					speed = '5';
					set_speed(speed);
					break;
				}
				sleep(time_tick);
			}
			for(i=0; i < 30; i++) {
				int status5 = get_status_of_contact("5");
				if(status5=='1'){
					wprintf(train_window, "Came back, stop the train\n");
					stop_train();
					break;
				}
				sleep(time_tick);
			}
		}
	}
	else if(position_code==516){
		if(zamboni_position_code == 3){
			set_switch('5','R');
			set_switch('6','G');
			// Look for Zamboni
			for(i=0; i < 30; i++) {
				int status4 = get_status_of_contact("4");
				if(status4=='1'){
					wprintf(train_window, "Zamboni passed track 4\n");
					speed = '5';
					set_speed(speed);
					break;
				}
				sleep(time_tick);
			}

			for(i=0; i < 30; i++) {
				int status12 = get_status_of_contact("12");
				if(status12=='1'){
					wprintf(train_window, "On track 12, stop\n");
					stop_train();
					break;
				}
				sleep(time_tick);
			}
			for(i=0; i < 30; i++) {
				int status13 = get_status_of_contact("13");
				if(status13=='1'){
					wprintf(train_window, "Zamboni passed track 13\n");
					zamboni_passed = TRUE;
					break;
				}
				sleep(time_tick);
			}
			if(zamboni_passed){
				speed = '5';
				set_speed(speed);
				set_switch('9','G');
				for(i=0; i < 30; i++) {
					int status14 = get_status_of_contact("14");
					if(status14=='1'){
						wprintf(train_window, "On track 14, Slow down\n");
						stop_train();
						change_direction();

						speed = '5';
						set_speed(speed);
						for(i=0; i<5; i++){
							wprintf(train_window, "%d,", i);
							sleep(long_sleep_tick);
						}

						speed = '2';
						set_speed(speed);
						for(i=0; i<5; i++){
							wprintf(train_window, "%d,", i);
							sleep(long_sleep_tick);
						}
						wprintf(train_window, "\nOn track 16, Stop\n");
						stop_train();
						set_switch('9','R');
						change_direction();
						wprintf(train_window, "Hooked up the car,waiting\n");
						zamboni_passed = FALSE;
						break;
					}
					sleep(short_sleep_tick);
				}
				for(i=0; i < 30; i++) {
					int status13 = get_status_of_contact("13");
					if(status13=='1'){
						wprintf(train_window, "Zamboni passed track 13\n");
						zamboni_passed = TRUE;
						break;
					}
					sleep(time_tick);
				}
			}
			if(zamboni_passed){
				speed = '5';
				set_speed(speed);
				zamboni_passed = FALSE;
			}
			for(i=0; i < 30; i++) {
				int status4 = get_status_of_contact("4");
				if(status4=='1'){
					wprintf(train_window, "Zamboni passed track 4\n");
					zamboni_passed = TRUE;
					set_switch('4','R');
					set_switch('3','R');
					break;
				}
				sleep(time_tick);
			}

		}
		for(i=0; i < 30; i++) {
			int status5 = get_status_of_contact("5");
			if(status5=='1'){
				wprintf(train_window, "Came back, stop the train\n");
				stop_train();
				set_switch('4','G');
				break;
			}
			sleep(time_tick);
		}
	}
	else{
		wprintf(train_window, "Train position unknown!\n");
	}
}

/**
 * Train process
 */
void train_process(PROCESS self, PARAM param)
{
	train_window = (WINDOW*) param;

	initialize_switches();
	int pos8_2 = is_train8_car2();
	int pos5_11 = is_train5_car11();
	int pos5_16 = is_train5_car16();
	int zamboni_found = 0;
	int position_code = 0;

	if(pos8_2){
		wprintf(train_window, "Train on track 8, car on track 2\n");
		position_code = 82;
	}
	else if(pos5_11){
		wprintf(train_window, "Train on track 5, car on track 11\n");
		position_code = 511;
	}
	else if(pos5_16){
		wprintf(train_window, "Train on track 5, car on track 16\n");
		position_code = 516;
	}
	else{
		wprintf(train_window, "Unknown train position!\n");
	}

	zamboni_found = find_zamboni();

	if(zamboni_found){
		if(zamboni_found==2){
			wprintf(train_window, "Zamboni is present and is running clockwise\n");
		}
		else if(zamboni_found==3){
			wprintf(train_window, "Zamboni is present and is running counter clockwise\n");
		}
		else if(zamboni_found==1){
			wprintf(train_window, "Zamboni is present but direction is unknown\n");
		}
		else {
			wprintf(train_window, "Zamboni is NOT present!\n");
		}
	}
	else {
		wprintf(train_window, "No Zamboni!\n");
	}

	if(!zamboni_found) {
		run_train_configuration_no_zamboni(position_code);
	}
	else {
		run_train_configuration_with_zamboni(position_code, zamboni_found);
	}

	remove_ready_queue(active_proc);
	resign();

}

/**
 * Init all switches
 */
void initialize_switches()
{
	wprintf(train_window, "Initializing rail switches.\n");
	set_switch('1','G');
	set_switch('2','G');
	set_switch('3','G');
	set_switch('4','G');
	set_switch('5','G');
	set_switch('6','R');
	set_switch('7','R');
	set_switch('8','G');
	set_switch('9','R');
}

/**
 * Init train
 */
void init_train(WINDOW* wnd)
{
	wprintf(wnd, "Initializing TOS Train...\n");
	create_process(train_process, 5, (PARAM) wnd, "Train process");
}
