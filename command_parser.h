#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "stdbool.h"
#include "string.h"
#include "mem.h"

// if command is "  set (12,{255,0,23},0);  ", then cli_command is:
//    nameLength = 3, name = "set"
//    length = 24 //(it stop when it reaches ';' - it does not consume spaces after that)

//set(int place, color c, int millis);
//set(color[] colors, int millis);
//setAll(color color, int millis);
//delay(int millis);

//TODO seperate "expecting command" from "no command found"
//TODO when parsing color - expecting comma and expecting color value is not 100%

enum cli_parameter_type{
	NO_PARAMETER = 0x00,
	INTEGER_PARAMETER,
	COLOR_PARAMETER,
	ARRAY_PARAMETER = 0x80 //this is a flags only! (NO_PARAMETER | ARRAY_PARAMETER) is an empty array
};
typedef enum cli_parameter_type cli_parameter_type;

struct cli_parameter{
	char type; //see cli_parameter_type
	int size; // 0 if its a single element, > 0 if its an array
	void *parameter;
};
typedef struct cli_parameter cli_parameter;

enum cli_error{
	NONE = 0,
	EXPECTING_COMMAND, COMMAND_NAME_LENGTH_EXCEEDS_LIMIT,
	EXPECTING_INTEGER,
	EXPECTING_PARAMETER,
	EXPECTING_COLOR = 5,
	EXPECTING_COLOR_VALUE, COLOR_VALUE_OUT_OF_RANGE,
	EXPECTING_COMMA,
	EXPECTING_SEMICOLON,
	EXPECTING_LEFT_PARENTHESIS = 10,
	EXPECTING_RIGHT_PARENTHESIS,
	EXPECTING_LEFT_BRACKET,
	EXPECTING_RIGHT_BRACKET,
	EXPECTING_LEFT_CURLYBRACE,
	EXPECTING_RIGHT_CURLYBRACE = 15,
	INVALID_PARAMETERS,
	ARRAY_HAS_DIFFERENT_DATA_TYPES,
	UNIDENTIFIED_DATA_TYPE,
	NO_COMMAND_FOUND
};

struct cli_command{
	int currentIndex;
	enum cli_error error;

	char *name;
	int nameLength;
	int totalLength;
	int parametersCount;
	cli_parameter *parameters;
};
typedef struct cli_command cli_command;

typedef unsigned int cli_parameter_color; //TODO use a 4 byte data type instead of int (8 bytes in windows 64bit). uint32_t etc...
typedef int cli_parameter_int;

int consumeSpaces(char *string);
void ICACHE_FLASH_ATTR setMaxCommandNameLength(int max); //TODO add max parameters and max array length, so users cant cause a crash by entering long string
int ICACHE_FLASH_ATTR getMaxCommandNameLength(void);

cli_parameter_int ICACHE_FLASH_ATTR readCommandParameterInt(char *cmd, cli_command *cli_cmd);
cli_parameter_color ICACHE_FLASH_ATTR readCommandParameterColor(char *cmd, cli_command *cli_cmd);
cli_parameter* ICACHE_FLASH_ATTR readCommandParameterArray(char *cmd, cli_command *cli_cmd);
cli_parameter ICACHE_FLASH_ATTR readCommandParameter(char *cmd, cli_command *cli_cmd);
void ICACHE_FLASH_ATTR readCommandName(char *cmd, cli_command *cli_cmd); //adds command name information to $cli_command
void ICACHE_FLASH_ATTR readCommandParameters(char *cmd, cli_command *cli_cmd); //adds command parameters information to $cli_command

cli_command* readCommand(char *cmd); //return a complete cli_command
void ICACHE_FLASH_ATTR free_command(cli_command *cmd);

void ICACHE_FLASH_ATTR printCommandInfo(cli_command *cmd, char *cmd_string, int cmd_string_max_length);

#endif
