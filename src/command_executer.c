#include "command_executer.h"
#include "user_config.h"

#define NUMBER_OF_FUNCTIONS 1

static struct final_command ICACHE_FLASH_ATTR finalize_command(cli_command *cmd);
static int execute_final_command(final_command c);
static void ICACHE_FLASH_ATTR free_final_command(final_command fc);
static bool ICACHE_FLASH_ATTR cli_command_matches_profile(cli_command *cmd, const struct function_profile *profile);

os_timer_t command_queue_executor_timer;
bool command_queue_executor_is_running = false;
struct final_commands_queue commands_queue = {0,0,NULL};
static void ICACHE_FLASH_ATTR execute_queue(void);

static int set(void *p);
//static ICACHE_FLASH_ATTR void setTrial(void *p);
//static void ICACHE_FLASH_ATTR setAtIndex(void *p);
static int setAll(void *p);
static int delay(void *p);
static int shiftOut(void *p);
static int shiftIn(void *p);
static int script(void *p);
static int loop();

static cli_parameter_type function_set_parameters_profile[] = {COLOR_PARAMETER | ARRAY_PARAMETER, INTEGER_PARAMETER};
static const struct function_profile function_set_profile = {"set",2,function_set_parameters_profile};

static cli_parameter_type function_setAll_parameters_profile[] = {COLOR_PARAMETER, INTEGER_PARAMETER};
static const struct function_profile function_setAll_profile = {"setAll",2,function_setAll_parameters_profile};

static cli_parameter_type function_delay_parameters_profile[] = {INTEGER_PARAMETER};
static const struct function_profile function_delay_profile = {"delay",1,function_delay_parameters_profile};

static cli_parameter_type function_shiftOut_parameters_profile[] = {COLOR_PARAMETER, INTEGER_PARAMETER};
static const struct function_profile function_shiftOut_profile = {"shiftOut",2,function_shiftOut_parameters_profile};

static cli_parameter_type function_shiftIn_parameters_profile[] = {COLOR_PARAMETER, INTEGER_PARAMETER};
static const struct function_profile function_shiftIn_profile = {"shiftIn",2,function_shiftIn_parameters_profile};

static cli_parameter_type function_script_parameters_profile[] = {INTEGER_PARAMETER};
static const struct function_profile function_script_profile = {"script",1,function_script_parameters_profile};

static const struct function_profile function_loop_profile = {"loop",0,NULL};
//static struct function_profile all_functions_profiles[NUMBER_OF_FUNCTIONS];

typedef uint32 color;
static uint8_t leds_current[NUMBER_OF_LEDS*3] = {0};
static uint8_t leds_start[NUMBER_OF_LEDS*3] = {0}, leds_end[NUMBER_OF_LEDS*3] = {0};
static uint32 leds_start_timestamp, leds_end_timestamp;
static os_timer_t leds_timer;

static void ICACHE_FLASH_ATTR set_leds_to_goal(void){
	os_memcpy(leds_current,leds_end,sizeof(uint8_t) * NUMBER_OF_LEDS*3);
	ws2812_push(leds_current, NUMBER_OF_LEDS*3);
}

static void leds_timer_function(void){
	int32 time_current = system_get_time()/1000;
	int32 time_left = leds_end_timestamp - time_current;
	if(time_left <= 20){
		set_leds_to_goal();
		os_timer_disarm(&leds_timer);
	}else{
		float precent_through = (time_current - leds_start_timestamp) / (float)(leds_end_timestamp - leds_start_timestamp);
		for(int l=0; l<NUMBER_OF_LEDS; l++){
			leds_current[l*3 + 0] = leds_start[l*3 + 0] + (uint8_t)(precent_through * (leds_end[l*3 + 0] - leds_start[l*3 + 0]));
			leds_current[l*3 + 1] = leds_start[l*3 + 1] + (uint8_t)(precent_through * (leds_end[l*3 + 1] - leds_start[l*3 + 1]));
			leds_current[l*3 + 2] = leds_start[l*3 + 2] + (uint8_t)(precent_through * (leds_end[l*3 + 2] - leds_start[l*3 + 2]));
		}
		ws2812_push(leds_current, NUMBER_OF_LEDS*3);
	}
}

static void ICACHE_FLASH_ATTR set_leds_to_goal_within(int millis){
	if(millis < 20){
		set_leds_to_goal();
	}else{
		leds_start_timestamp = system_get_time()/1000;
		leds_end_timestamp = leds_start_timestamp + millis;
		os_memcpy(leds_start,leds_current,sizeof(uint8_t) * NUMBER_OF_LEDS*3);
		os_timer_disarm(&leds_timer);
		os_timer_setfn(&leds_timer, (os_timer_func_t *)leds_timer_function, NULL);
		os_timer_arm(&leds_timer, 20, 1);
	}
}

void ICACHE_FLASH_ATTR initalize_command_executor(void){
	//all_functions_profiles[0] = function_set_profile;
	os_timer_disarm(&leds_timer);
	os_timer_setfn(&leds_timer, (os_timer_func_t *)leds_timer_function, NULL);
	os_timer_disarm(&command_queue_executor_timer);
	os_timer_setfn(&command_queue_executor_timer, (os_timer_func_t *)execute_queue, NULL);
}

static void setLed(int pos, color clr){
	if(pos > NUMBER_OF_LEDS) return;
	int i = pos*3;
	leds_end[i+2] = (uint8_t)clr;
	leds_end[i+0] = (uint8_t)(clr >> 8);
	leds_end[i+1] = (uint8_t)(clr >> 16);
}

static int set(void *p){
	int numberOfColors = *(int *)p;
	p += sizeof(int);
	int millis = *(int *)(p);
	p += sizeof(int);
	color *colors = (color *)p;
//	os_printf("set: %d clrs,%dms\r\n",numberOfColors,millis);
	if(numberOfColors > NUMBER_OF_LEDS) numberOfColors = NUMBER_OF_LEDS;
	for(int i=0; i<numberOfColors; i++) setLed(i,colors[i]);
	set_leds_to_goal_within(millis);
	return(0);
}

/*static ICACHE_FLASH_ATTR void setTrial(void *p){
	color clrs[] = (int *)(p);
	int numberOfColors = *(int *)p;
	p += sizeof(int);
	int millis = *(int *)(p);
	p += sizeof(int);
	color *colors = (color *)p;
	os_printf("set: %d clrs,%dms\r\n",numberOfColors,millis);
	if(numberOfColors > NUMBER_OF_LEDS) numberOfColors = NUMBER_OF_LEDS;
	for(int i=0; i<numberOfColors; i++) setLed(i,colors[i]);
	ws2812_push(leds, NUMBER_OF_LEDS*3);
}
*/

static int ICACHE_FLASH_ATTR setAll(void *p){
	int clr = *(int *)p;
	p += sizeof(int);
	int millis = *(int *)(p);
	os_printf("setAll color: %d,delay: %d\r\n",clr,millis);
	for(int i=0; i<NUMBER_OF_LEDS; i++) setLed(i,clr);
	set_leds_to_goal_within(millis);
	return(0);
}

static int ICACHE_FLASH_ATTR delay(void *p){
	int millis = *(int *)(p);
	os_printf("inside delay command! with time of %d\r\n",millis);
	return(millis);
}

static int ICACHE_FLASH_ATTR shiftOut(void *p){
	color clr = *(color *)p;
	int millis = *((int *)p + 1);
	os_printf("inside shiftOut command! with color of %d and time of %d\r\n",clr,millis);
	for(int l = NUMBER_OF_LEDS-1; l > 0; l--){
		leds_end[l*3 + 0] = leds_end[l*3-3 + 0];
		leds_end[l*3 + 1] = leds_end[l*3-3 + 1];
		leds_end[l*3 + 2] = leds_end[l*3-3 + 2];
	}
	setLed(0, clr);
	set_leds_to_goal_within(millis);
	return(0);
}

static int ICACHE_FLASH_ATTR shiftIn(void *p){
	color clr = *(color *)p;
	int millis = *((int *)p + 1);
	os_printf("inside shiftIn command! with color of %d and time of %d\r\n",clr,millis);
	for(int l=0; l<NUMBER_OF_LEDS-1; l++){
		leds_end[l*3 + 0] = leds_end[l*3+3 + 0];
		leds_end[l*3 + 1] = leds_end[l*3+3 + 1];
		leds_end[l*3 + 2] = leds_end[l*3+3 + 2];
	}
	setLed(NUMBER_OF_LEDS-1, clr);
	set_leds_to_goal_within(millis);
	return(0);
}

os_timer_t script_timer;
int active_script = 0;
int script_1_stage = 0;
int script_1_colors[] = {0x000000,0x0000ff,0x00ffff,0x00ff00,0xffff00,0xff0000,0xff00ff,0xffffff};
int script_2_stage = 0;
struct{
	int led_number;
	int stage;
	int color_number;
} script_3_stage;

int script_3_colors[] = {0x0000ff,0x00ffff,0xffff00,0xff0000,0xff00ff};

static void execute_script(){
	os_timer_disarm(&script_timer);
	os_timer_setfn(&script_timer, (os_timer_func_t *)execute_script, NULL);
	if(active_script == 1){
			int p[] = {0, 3000};
			p[0] = script_1_colors[script_1_stage++];
			if(script_1_stage == 8) script_1_stage = 0;
			setAll(p);
			os_timer_arm(&script_timer, p[1], 0);
	}else if(active_script == 2){
		int p[] = {0, 1000};
		p[0] = script_1_colors[script_2_stage/20];
		script_2_stage++;
		if(script_2_stage == 160) script_2_stage = 0;
		shiftOut(p);
		os_timer_arm(&script_timer, 800, 0);
	}else if(active_script == 3){
		const int lit_amount = 7;
		const int step_delay_ms = 30;
		//stages:
		// -shift out n from color
		// -shift out NUMBER_OF_LEDS-lit_amount blacks
		// -shift in NUMBER_OF_LEDS-lit_amount blacks
		// -shift out NUMBER_OF_LEDS-lit_amount from color
		// -shift out NUMBER_OF_LEDS-lit_amount black
		// -repeat for each color
		int p[] = {0, step_delay_ms};
		// int nextStage = 200;
		if(script_3_stage.stage == 0){
			p[0] = script_3_colors[script_3_stage.color_number];
			shiftOut(p);
			script_3_stage.led_number++;
			if(script_3_stage.led_number == lit_amount){
				script_3_stage.stage++;
				script_3_stage.led_number = 0;
			}
		}else if(script_3_stage.stage == 1 || script_3_stage.stage == 2 || script_3_stage.stage == 4){
			p[0] = 0;
			if(script_3_stage.stage == 1 || script_3_stage.stage == 4) shiftOut(p); else shiftIn(p);
			script_3_stage.led_number++;
			if(script_3_stage.led_number == NUMBER_OF_LEDS-lit_amount){
				script_3_stage.stage++;
				script_3_stage.led_number = 0;
				if(script_3_stage.stage == 5){
					script_3_stage.color_number++;
					if(script_3_stage.color_number == 5) script_3_stage.color_number = 0;
					script_3_stage.stage = 0;
				}
			}
		}else if(script_3_stage.stage == 3){
			p[0] = script_3_colors[script_3_stage.color_number];
			shiftOut(p);
			script_3_stage.led_number++;
			if(script_3_stage.led_number == NUMBER_OF_LEDS-lit_amount){
				script_3_stage.stage++;
				script_3_stage.led_number = 0;
			}
		}
		os_timer_arm(&script_timer, step_delay_ms, 0);
		return;
		/*p[0] = script_3_colors[script_3_stage.color_number];
		if(script_3_stage.direction) shiftIn(p); else shiftOut(p);
		script_3_stage.led_number++;
		if(script_3_stage.led_number == NUMBER_OF_LEDS){ script_3_stage.color_number++; script_3_stage.led_number = 0;}
		if(script_3_stage.color_number == 5){ script_3_stage.color_number = 0; script_3_stage.direction = !script_3_stage.direction;};
		os_timer_arm(&script_timer, 150, 0);*/
	}
}

static int ICACHE_FLASH_ATTR script(void *p){
	int number = *(int *)(p);
	os_printf("inside script command! with number %d\r\n", number);
	active_script = number;
	if(number == 1){
		script_1_stage = 0;
	}else if(number == 2){
		script_2_stage = 0;
	}else if(number == 3){
		script_3_stage.stage = 0;
		script_3_stage.color_number = 0;
		script_3_stage.led_number = 0;
	}
	os_timer_disarm(&script_timer);
	os_timer_setfn(&script_timer, (os_timer_func_t *)execute_script, NULL);
	os_timer_arm(&script_timer, 0, 0);
	return(0);
}

static int ICACHE_FLASH_ATTR loop(){
	return(-2);
}

/*static void ICACHE_FLASH_ATTR setAtIndex(color color, int position, int millis){
	os_printf("set: %d color at position %d, delay: %d\r\n", color, position, millis);
	setLed(position-1,color);
	ws2812_push(leds, NUMBER_OF_LEDS*3);
}*/

void ICACHE_FLASH_ATTR add_final_command_to_queue(final_command cmd){
	os_printf("adding to queue sized %d command id %d\r\n", commands_queue.size, cmd.function);
	int size = commands_queue.size;
	commands_queue.commands = (final_command *)os_realloc(commands_queue.commands, sizeof(final_command) * (size + 1));
	commands_queue.commands[size] = cmd;
	commands_queue.size = size+1;
}

void ICACHE_FLASH_ATTR free_final_command_queue(){
	for(int i=0; i<commands_queue.size; i++) free_final_command(commands_queue.commands[i]);
	os_free(commands_queue.commands);
	commands_queue.commands = NULL;
	commands_queue.size = 0;
	commands_queue.next_command = 0;
}

void ICACHE_FLASH_ATTR execute_queue_if_not_running(){
	if(command_queue_executor_is_running) return;
	os_timer_disarm(&command_queue_executor_timer);
	os_timer_setfn(&command_queue_executor_timer, (os_timer_func_t *)execute_queue, NULL);
	command_queue_executor_is_running = true;
	os_timer_arm(&command_queue_executor_timer, 0, 0);
}

void ICACHE_FLASH_ATTR stop_executing_queue(){
	os_timer_disarm(&command_queue_executor_timer);
	command_queue_executor_is_running = false;
	active_script = 0;
}

void execute_queue(){
	os_timer_disarm(&command_queue_executor_timer);
	if(commands_queue.size == 0 || commands_queue.next_command == commands_queue.size){
		os_printf("command queue size is 0 or next command doesnt exists...\r\n");
		command_queue_executor_is_running = false;
		free_final_command_queue();
	}
	int delay = 0;
	while(delay == 0){
		os_printf("-   executing command #%d, ",commands_queue.next_command);
		delay = execute_final_command(commands_queue.commands[commands_queue.next_command]);
		commands_queue.next_command++;
		if(delay == -2){
			os_printf("looping queue!\r\n");
			commands_queue.next_command = 0;
			delay = 0;
		}
		if(commands_queue.next_command == commands_queue.size){ //done executing the entire queue
			os_printf("done executing entire queue!\r\n");
			command_queue_executor_is_running = false;
			free_final_command_queue();
			return;
		}
	}
	if(delay<10) delay = 10;
	if(commands_queue.next_command == commands_queue.size){ //done executing the entire queue
		os_printf("done executing entire queue!\r\n");
		command_queue_executor_is_running = false;
		free_final_command_queue();
	}else{
		os_printf("will execute next command in queue within %d millis!\r\n",delay);
		os_timer_setfn(&command_queue_executor_timer, (os_timer_func_t *)execute_queue, NULL);
		os_timer_arm(&command_queue_executor_timer, delay, 0);
	}
}

void ICACHE_FLASH_ATTR free_final_command(final_command fc){
	os_free(fc.parameters);
}

final_command ICACHE_FLASH_ATTR finalize_command(cli_command *cmd){
	/*final_command c = {FUNCTION_ID_NO_FUNCTION, NULL};
	for(int i=0; i<NUMBER_OF_FUNCTIONS; i++){
		if(cli_command_matches_profile(cmd, all_functions_profiles[i])){
			struct function_profile fp = all_functions_profiles[i];
			c.function = fp.function; //TODO change c.function to a void pointer.
			int size = 0;
			for(int p=0; p<fp.number_of_parameters; p++){
				switch(cmd->parameters[p].type & ~ARRAY_PARAMETER){
				case INTEGER_PARAMETER:
				case COLOR_PARAMETER:
					size += sizeof(int) * cmd->parameters[p].size; break;
				default: break;
				}
				if(cmd->parameters[p].type & ARRAY_PARAMETER) size += sizeof(int); //after every array pointer comes its length.
			}
			c.parameters = os_malloc((size_t)size);
			void *p = c.parameters;
			for(int p=0; p<fp.number_of_parameters; p++){
				switch(cmd->parameters[p].type){
				case INTEGER_PARAMETER:
				case COLOR_PARAMETER:
					*(int *)p = (int)cmd->parameters[p].parameter;
					p += sizeof(int);
					break;
				case INTEGER_PARAMETER | ARRAY_PARAMETER:
				case COLOR_PARAMETER | ARRAY_PARAMETER:
					void *param = os_malloc(sizeof(int)*cmd->parameters[p].size);
					os_memcpy(param,cmd->parameters[p].parameter,sizeof(int)*cmd->parameters[p].size);
					*(int *)p = (int *)param;
					p += sizeof(int *);
					*(int *)p = (int)cmd->parameters[p].size;
					break;
				default: break;
				}
			}
			c.parameters = p;
			p = NULL;
			return(c);
		}
	}
	return(c);*/
	final_command c = {FUNCTION_ID_NO_FUNCTION, NULL};
	c.function = FUNCTION_ID_NO_FUNCTION;
	c.parameters = NULL;
	if(cmd->error != NONE) return(c);
	if(cli_command_matches_profile(cmd, &function_set_profile)){
		//set(color[] colors, int millis);
		c.function = FUNCTION_ID_set;
		c.parameters = os_malloc(sizeof(int)*2 + sizeof(color)*cmd->parameters[0].size);
		*((int *)c.parameters) = cmd->parameters[0].size;
		*(((int *)c.parameters) + 1) = *(cli_parameter_color *)cmd->parameters[1].parameter;
		os_memcpy(c.parameters + sizeof(int)*2,cmd->parameters[0].parameter,sizeof(color)*cmd->parameters[0].size);
	}else if(cli_command_matches_profile(cmd, &function_setAll_profile)){
		//setAll(color c, int millis);
		c.function = FUNCTION_ID_setAll;
		c.parameters = os_malloc(sizeof(int)*2);
		*((int *)c.parameters) = *(int *)cmd->parameters[0].parameter;
		*((int *)c.parameters + 1) = *(int *)cmd->parameters[1].parameter;
	}else if(cli_command_matches_profile(cmd, &function_delay_profile)){
		//delay(int millis);
		c.function = FUNCTION_ID_delay;
		c.parameters = os_malloc(sizeof(int));
		*(int *)c.parameters = *(int *)(cmd->parameters[0].parameter);
	}else if(cli_command_matches_profile(cmd, &function_shiftOut_profile)){
		c.function = FUNCTION_ID_shiftOut;
		c.parameters = os_malloc(sizeof(int)*2);
		*(int *)c.parameters = *(int *)(cmd->parameters[0].parameter);
		*((int *)c.parameters + 1) = *(int *)(cmd->parameters[1].parameter);
	}else if(cli_command_matches_profile(cmd, &function_shiftIn_profile)){
		c.function = FUNCTION_ID_shiftIn;
		c.parameters = os_malloc(sizeof(int)*2);
		*(int *)c.parameters = *(int *)(cmd->parameters[0].parameter);
		*((int *)c.parameters + 1) = *(int *)(cmd->parameters[1].parameter);
	}else if(cli_command_matches_profile(cmd, &function_script_profile)){
		c.function = FUNCTION_ID_script;
		c.parameters = os_malloc(sizeof(int));
		*(int *)c.parameters = *(int *)(cmd->parameters[0].parameter);
	}else if(cli_command_matches_profile(cmd, &function_loop_profile)){
		c.function = FUNCTION_ID_loop;
	}else{
		os_printf("command not found!\r\n");
		return(c);
	}
	return(c);
}

int ICACHE_FLASH_ATTR execute_final_command(final_command c){
	if(c.function == FUNCTION_ID_NO_FUNCTION) return(false);
	switch(c.function){
	case FUNCTION_ID_set: return(set(c.parameters));
	case FUNCTION_ID_setAll: return(setAll(c.parameters));
	case FUNCTION_ID_delay: return(delay(c.parameters));
	case FUNCTION_ID_shiftOut: return(shiftOut(c.parameters));
	case FUNCTION_ID_shiftIn: return(shiftIn(c.parameters));
	case FUNCTION_ID_script: return(script(c.parameters));
	case FUNCTION_ID_loop: return(loop());
	default: return(-1);
	}
}

bool ICACHE_FLASH_ATTR cli_command_matches_profile(cli_command *cmd, const struct function_profile *profile){
	if(cmd->nameLength == os_strlen(profile->name) && os_memcmp(cmd->name,profile->name,cmd->nameLength) == 0 && cmd->parametersCount == profile->number_of_parameters){
		for(int i=0; i<profile->number_of_parameters; i++){
			if(cmd->parameters[i].type == ARRAY_PARAMETER){ //is empty array
				if((profile->params[i] & ARRAY_PARAMETER) == 0) return(false);
			}else{
				if(cmd->parameters[i].type != profile->params[i]) return(false);
			}
		}
		return(true);
	}else{
		return(false);
	}
}

void ICACHE_FLASH_ATTR execute_commands_from_string(char *string, int length){
	//	uint32_t t1 = system_get_time(), t2;
	//	os_printf("leds rx %d;\r\n", (int)length);
		int count = 0, index=0;
		cli_command *cmd;
		final_command fc;
		uint32_t timeoutStartTime = system_get_time();
		stop_executing_queue();
		free_final_command_queue();
		while(index < length && system_get_time() - timeoutStartTime < 1000000){
	//		t1 = system_get_time();
			cmd = readCommand(string+index);
	//		t2 = system_get_time(); os_printf("timing: #1 - %d\r\n",t2-t1);
			index += cmd->totalLength;
	//		printCommandInfo(cmd, (char *)data, length);
			if(cmd->error != NONE){
				os_printf("error parsing command #%d!\r\n",count);
				free_command(cmd);
				break;
			}
	//		t1 = system_get_time();
			fc = finalize_command(cmd);
	//		t2 = system_get_time(); os_printf("timing: #2 - %d\r\n",t2-t1); t1 = system_get_time();
			free_command(cmd);
			add_final_command_to_queue(fc);
	//		t2 = system_get_time(); os_printf("timing: #3 - %d\r\n",t2-t1); t1 = system_get_time();
			//execute_final_command(fc);
	//		t2 = system_get_time(); os_printf("timing: #4 - %d\r\n",t2-t1); t1 = system_get_time();
			//free_final_command(fc);
	//		t2 = system_get_time(); os_printf("timing: #5 - %d\r\n",t2-t1);
	//		os_printf("cmd #%d!\r\n",count);
			count++;
			//os_printf("this message here is to make a single command executino take much much much much much much much longer..... it is used as a delay!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
		}
		if(system_get_time() - timeoutStartTime >= 50000) os_printf("timed out!!! \r\n");
		execute_queue_if_not_running(); //TODO can just call execute_queue() since im stopping queue before because of "loop" function
	//	t2 = system_get_time(); os_printf("t: %d\r\n",t2-t1);
}


/*
char *command_name_set = "set";
char *command_name_setAll = "setAll";
char *command_name_delay = "delay";

bool b = cli_command_matches_profile(cmd, function_set_profile);
	os_printf("is mathing profile? %c \r\n",'0'+b);
	final_command c = {FUNCTION_ID_NO_FUNCTION, NULL};
	if(cmd->error != NONE) return(c);
	if(cmd->nameLength == strlen(command_name_set) && os_memcmp(cmd->name,command_name_set,cmd->nameLength) == 0){
		if( cmd->parametersCount != 2 ||
				!(cmd->parameters[0].type == INTEGER_PARAMETER || (cmd->parameters[0].type == COLOR_PARAMETER && cmd->parameters[0].size > 0)) ||
				(cmd->parameters[1].type != INTEGER_PARAMETER)
		){}else{
			//set(color[] colors, int millis);
			c.function = FUNCTION_ID_set;
			c.parameters = os_malloc(sizeof(int)*2 + sizeof(color)*cmd->parameters[0].size);
			*((int *)c.parameters) = cmd->parameters[0].size;
			*(((int *)c.parameters) + 1) = *(cli_parameter_color *)cmd->parameters[1].parameter;
			os_memcpy(c.parameters + sizeof(int)*2,cmd->parameters[0].parameter,sizeof(color)*cmd->parameters[0].size);
			return(c);
		}
	}else if(cmd->nameLength == strlen(command_name_setAll) && os_memcmp(cmd->name,command_name_setAll,cmd->nameLength) == 0){
		if( cmd->parametersCount == 2 && cmd->parameters[0].type == COLOR_PARAMETER && cmd->parameters[1].type == INTEGER_PARAMETER){
			//setAll(color c, int millis);
			c.function = FUNCTION_ID_setAll;
			c.parameters = os_malloc(sizeof(int)*2);
			*((int *)c.parameters) = *(int *)cmd->parameters[0].parameter;
			*((int *)c.parameters + 1) = *(int *)cmd->parameters[1].parameter;
			return(c);
		}
	}else if(cmd->nameLength == strlen(command_name_delay) && os_memcmp(cmd->name,command_name_delay,cmd->nameLength) == 0){
		if( cmd->parametersCount == 1 && cmd->parameters[0].type == INTEGER_PARAMETER){
			//delay(int millis);
			c.function = FUNCTION_ID_setAll;
			c.parameters = os_malloc(sizeof(int));
			*(int *)c.parameters = *(int *)(cmd->parameters[0].parameter);
			return(c);
		}
	}else{
		os_printf("command not found!\r\n");
		return(c);
	}
	return(c);*/
