#ifndef COMMAND_EXECUTER_H_
#define COMMAND_EXECUTER_H_

#include "stdbool.h"
#include "mem.h"
#include "ets_sys.h"
#include "osapi.h"
#include "command_parser.h"
#include "ws2812_i2s.h"

typedef uint32 color;

enum functions_id{
	FUNCTION_ID_NO_FUNCTION,
	FUNCTION_ID_set,
	FUNCTION_ID_setAll,
	FUNCTION_ID_delay,
	FUNCTION_ID_shiftOut,
	FUNCTION_ID_shiftIn,
	FUNCTION_ID_script,
	FUNCTION_ID_loop
};

struct final_command{
	enum functions_id function;
	void* parameters; //parameters, the function will know how to handle the void pointer
};
typedef struct final_command final_command;

struct final_commands_queue{
	int size;
	int next_command;
	final_command *commands;
};

struct function_profile{
	char *name; //for example, "set";
	int number_of_parameters;
	cli_parameter_type *params; //for example, {COLOR_PARAMETER | ARRAY_PARAMETER, INTEGER_PARAMETER, INTEGER_PARAMETER};
	void *function; //real function address to call!
};

void ICACHE_FLASH_ATTR initalize_command_executor(void);
void ICACHE_FLASH_ATTR execute_commands_from_string(char *string, int length);
void ICACHE_FLASH_ATTR stop_executing_queue(void);

#endif //COMMAND_EXECUTER_H_
