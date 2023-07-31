#include "command_parser.h"

static int maxCommandNameLength = 10;
struct cli_command empty_cli_command = {0,NONE,NULL,0,0,0,NULL}; //TODO change to static

//#define prt if(debug) printf
//bool debug = false;
/*
void* my_os_realloc(void* p, size_t s){
	os_printf("memory leak debug - realloc %d from %p",(int)s,p);
	p = os_realloc(p,s);
	os_printf("    -    got %p\r\n", p);
	return(p);
}

void* my_os_malloc(size_t s){
	os_printf("memory leak debug - malloc  %d                ",(int)s);
	void *p = os_malloc(s);
	os_printf("    -    got %p\r\n", p);
	return(p);
}

void my_os_free(void *p){
	os_printf("memory leak debug - freeing %p\r\n",p);
	os_free(p);
}

void my_os_memcpy(void *des, void *src, size_t s){
	os_printf("memory leak debug - copying %d bytes of memory from %p to %p\r\n",(int)s,src,des);
	os_memcpy(des,src,s);
}

#define os_realloc(P,S) my_os_realloc(P,S)
#define os_malloc my_os_malloc
#define os_free my_os_free
#define os_memcpy my_os_memcpy
*/

//TODO remove consumeSpaces from readCommandName, readCommandParameterInt and Color. it should be "readCommand"'s job.
//TODO have readint readcolor and readarray add the parameter they read to cli_cmd themselves


void ICACHE_FLASH_ATTR setMaxCommandNameLength(int max){
	maxCommandNameLength = max;
}

int ICACHE_FLASH_ATTR getMaxCommandNameLength(void){
	return(maxCommandNameLength);
}

static bool isDigit(char c){
	return(c >= '0' && c <= '9');
}

static bool isLetter(char c){
	return((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static bool isHexDigit(char c){
	return((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

static unsigned char hexDigitValue(char c){
	if(c >= '0' && c <= '9') return(c-'0');
	if(c >= 'A' && c <= 'F') return(c-55);
	if(c >= 'a' && c <= 'f') return(c-32-55);
	return(-1);
}

/*
 * int consumeSpaces(char *string);
 *
 * return the number of spaces (' '), newlines ('\n'), or backslash with newline ('\\'+'\n')
 * characters found from the given string
 */
int ICACHE_FLASH_ATTR consumeSpaces(char *str){
	int i = 0;
	while(1){
		if(str[i] == ' ' || str[i] == '\n') i++;
		else if(str[i] == '\\' && str[i+1] == '\n') i+=2;
		else break;
	}
	return(i);
}

/*
 * cli_command readCommandName(char *cmd);
 *
 * reads a commandName from the given $cmd string.
 * returns a commandName of -1 if command name length
 * exceeds the maximum set by setCommandNameMaximum.
 */
void ICACHE_FLASH_ATTR readCommandName(char *cmd, cli_command *cli_cmd){
	int i = cli_cmd->currentIndex;
	cli_cmd->name = NULL;
	while(1){
		if(isLetter(cmd[i]) || (cmd[i] == '_') || (cli_cmd->nameLength != 0 && isDigit(cmd[i]))){
			if(cli_cmd->nameLength % 10 == 0) cli_cmd->name = (char *)os_realloc(cli_cmd->name,sizeof(char)*(cli_cmd->nameLength + 10)); //os_reallocate memory if needed
			if(cli_cmd->nameLength == getMaxCommandNameLength()){
				cli_cmd->error = COMMAND_NAME_LENGTH_EXCEEDS_LIMIT;
				break;
			}
			cli_cmd->name[cli_cmd->nameLength++] = cmd[i++];
		}else if(cmd[i] == '\\' && cmd[i+1] == '\n') i+=2;
		else{
			if(cli_cmd->nameLength == 0){
				cli_cmd->totalLength = (cmd[i] == 0) ? -1 : 0;
				break;
			}
			break;
		}
	}
	if(cli_cmd->nameLength != 0){
		cli_cmd->name = (char *)os_realloc(cli_cmd->name, sizeof(char)*(cli_cmd->nameLength+1));
		cli_cmd->name[cli_cmd->nameLength] = '\0';
	}
	cli_cmd->currentIndex = i;
	return;
}

cli_parameter_int ICACHE_FLASH_ATTR readCommandParameterInt(char *cmd, cli_command *cli_cmd){ /////////////////////
	cli_parameter_int r = 0;
	int i = cli_cmd->currentIndex;
	bool negative = false;
	bool number = false;
	if(cmd[i] == '-' || cmd[i] == '+'){
		negative = (cmd[i] == '-');
		i += 1+consumeSpaces(cmd+i+1); //TODO use  i += consumeSpaces((++i)+cmd);
		number = true;
	}
	int starti = i;
	while(1){
		if(isDigit(cmd[i])) r = (r*10) + (cmd[i++] - '0');
		else if(cmd[i] == '\\' && cmd[i+1] == '\n') i+=2;
		else break;
	}
	if(starti == i){
		if(number) cli_cmd->error = EXPECTING_INTEGER;
		cli_cmd->totalLength = 0;
		return(r);
	}
	cli_cmd->currentIndex = i;
	if(negative) r *= -1;
	return(r);
}

cli_parameter_color ICACHE_FLASH_ATTR readCommandParameterColor(char *cmd, cli_command *cli_cmd){ ////////////////////////
	cli_parameter_color r = 0;
		int i = cli_cmd->currentIndex;
		if(cmd[i] == '#'){ //hex color
			i++;
			int digits = 0;
			while(digits < 6){
				if(isHexDigit(cmd[i])){
					r = (r<<4) | hexDigitValue(cmd[i++]);
					digits++;
				}else if(cmd[i] == '\\' && cmd[i+1] == '\n') i+=2;
				else break;
			}
			if(digits == 0) cli_cmd->error = EXPECTING_COLOR;
			cli_cmd->currentIndex = i;
		}else if(cmd[i] == '{'){ //color triplet
			i++;
			for(char t=0; t<3; t++){
				i += consumeSpaces(cmd+i);
				cli_cmd->currentIndex = i;
				int value = readCommandParameterInt(cmd, cli_cmd);
				i = cli_cmd->currentIndex;
				i += consumeSpaces(cmd+i);
				if(value<0 || value>255) cli_cmd->error = COLOR_VALUE_OUT_OF_RANGE;
				if(cli_cmd->totalLength == 0) cli_cmd->error = EXPECTING_COLOR_VALUE;
				if(t!=2 && cmd[i]!=',') cli_cmd->error = EXPECTING_COMMA;
				if(t==2 && cmd[i]!='}') cli_cmd->error = EXPECTING_RIGHT_CURLYBRACE;
				if(cli_cmd->error != NONE) break;
				i++;
				r = (r<<8) | (unsigned char)value;
			}
			cli_cmd->currentIndex = i;
		}else{
			cli_cmd->totalLength = 0;
		}
		return(r);
}

cli_parameter* ICACHE_FLASH_ATTR readCommandParameterArray(char *cmd, cli_command *cli_cmd){
	//TODO comma handling is clumsy here
	//prt("going to read array!!! from string %s\n",cmd+cli_cmd->currentIndex);
	if(cmd[cli_cmd->currentIndex] != '[') return(NULL);
	cli_parameter *p = (cli_parameter *)os_malloc(sizeof(cli_parameter));
	p->parameter = NULL;
	p->type = NO_PARAMETER;
	p->size = 0;
	int i = cli_cmd->currentIndex+1;
	bool after_comma = false;
	while(1){
		i += consumeSpaces(cmd+i);
		cli_cmd->currentIndex = i;
		cli_cmd->totalLength = 1;
		cli_parameter_int integer = readCommandParameterInt(cmd, cli_cmd);
		if(cli_cmd->error != NONE) break;
		if(cli_cmd->totalLength != 0){
			after_comma = false;
			if(p->type == NO_PARAMETER) p->type = INTEGER_PARAMETER;
			if(p->type == INTEGER_PARAMETER){
				p->size++;
				i = cli_cmd->currentIndex;
				p->parameter = os_realloc(p->parameter,sizeof(cli_parameter_int) * p->size);
				os_memcpy(((cli_parameter_int*)(p->parameter)) + p->size-1, &integer, sizeof(cli_parameter_int));
				continue;
			}else{
				cli_cmd->error = ARRAY_HAS_DIFFERENT_DATA_TYPES;
				cli_cmd->currentIndex = i;
				break;
			}
		}
		cli_cmd->totalLength = 1;
		cli_parameter_color color = readCommandParameterColor(cmd, cli_cmd);
		if(cli_cmd->error != NONE) break;
		if(cli_cmd->totalLength != 0){
			after_comma = false;
			if(p->type == NO_PARAMETER) p->type = COLOR_PARAMETER;
			if(p->type == COLOR_PARAMETER){
				p->size++;
				i = cli_cmd->currentIndex;
				p->parameter = os_realloc(p->parameter,sizeof(cli_parameter_color) * p->size);
				os_memcpy(((cli_parameter_color*)(p->parameter)) + p->size-1, &color, sizeof(cli_parameter_color));
				continue;
			}else{
				os_free(p->parameter);
				p->parameter = NULL;
				cli_cmd->error = ARRAY_HAS_DIFFERENT_DATA_TYPES;
				cli_cmd->currentIndex = i;
				break;
			}
		}
		if(cmd[i] == ']'){
			if(after_comma){
				if(p->type == INTEGER_PARAMETER) cli_cmd->error = EXPECTING_INTEGER;
				else if(p->type == COLOR_PARAMETER) cli_cmd->error = EXPECTING_COLOR;
				break;
			}else if(p->size == 0){
				p->type = ARRAY_PARAMETER;
			}else if(p->size > 0){
				p->type |= ARRAY_PARAMETER;
			}
			cli_cmd->currentIndex = ++i;
			return(p);
		}else if(cmd[i] == ','){
			if(after_comma || p->size==0){
				if(after_comma && p->type == INTEGER_PARAMETER) cli_cmd->error = EXPECTING_INTEGER;
				else if(after_comma && p->type == COLOR_PARAMETER) cli_cmd->error = EXPECTING_COLOR;
				else if(p->size==0) cli_cmd->error = EXPECTING_PARAMETER;
				break;
			}else{
				after_comma = true;
				i++;
				continue;
			}
		}
		if(cli_cmd->totalLength == 0){
			if(after_comma) cli_cmd->error = UNIDENTIFIED_DATA_TYPE; else cli_cmd->error = EXPECTING_COMMA;
			cli_cmd->totalLength = 1;
			cli_cmd->currentIndex = i;
			break;
		}
		break;
	}
	os_free(p->parameter);
	p->parameter = NULL;
	os_free(p);
	p = NULL;
	return(p);
}

cli_parameter* ICACHE_FLASH_ATTR readParameter(char *cmd, cli_command *cli_cmd){ //TODO should not return a pointer! cli_parameter has a fixed size!
	cli_parameter *p = (cli_parameter *)os_malloc(sizeof(cli_parameter));
	cli_cmd->totalLength = 1; cli_cmd->error = NONE;
	cli_parameter_int integer = readCommandParameterInt(cmd, cli_cmd);
	if(cli_cmd->error != NONE){
		os_free(p);
		p = NULL;
		return(NULL);
	}
	if(cli_cmd->totalLength != 0){
		p->type = INTEGER_PARAMETER;
		p->size = 1;
		p->parameter = os_malloc(sizeof(cli_parameter_int));
		*(cli_parameter_int *)(p->parameter) = integer;
		//os_memcpy(p->parameter, &integer, sizeof(cli_parameter_int));
		return(p);
	}
	cli_cmd->totalLength = 1; cli_cmd->error = NONE;
	cli_parameter_color color = readCommandParameterColor(cmd, cli_cmd);
	if(cli_cmd->error != NONE){
		os_free(p);
		p = NULL;
		return(NULL);
	}
	if(cli_cmd->totalLength != 0){
		p->type = COLOR_PARAMETER;
		p->size = 1;
		p->parameter = os_malloc(sizeof(cli_parameter_color));
		*(cli_parameter_color *)(p->parameter) = color;
		//os_memcpy(p->parameter, &color, sizeof(cli_parameter_color));
		return(p);
	}
	os_free(p);
	p = NULL;
	p = readCommandParameterArray(cmd, cli_cmd);
	return(p);
}

void ICACHE_FLASH_ATTR readCommandParameters(char *cmd, cli_command *cli_cmd){
	if(cmd[cli_cmd->currentIndex] != '('){
		cli_cmd->error = EXPECTING_LEFT_PARENTHESIS;
		cli_cmd->totalLength = 0;
		return;
	}
	int i = cli_cmd->currentIndex + 1;
	bool after_comma = false;
	while(1){
		i += consumeSpaces(cmd+i);
		cli_cmd->currentIndex = i;
		//prt("-reading a single parameter at i %d in string \"%s\"\n",i,cmd);
		cli_parameter *p = readParameter(cmd, cli_cmd);
		if(cli_cmd->error != NONE) return;
		//prt("-    no error\n");
		//if(cli_cmd->parametersCount==2) return;
		if(p == NULL){
			//prt("-    p is null\n");
			if(after_comma){
				cli_cmd->error = EXPECTING_PARAMETER;
				cli_cmd->totalLength = 0;
				return;
			}
			break;
		}else{
			after_comma = false;
		}
		i = cli_cmd->currentIndex;
		cli_cmd->parametersCount++;
		cli_cmd->parameters = (cli_parameter *)os_realloc(cli_cmd->parameters, sizeof(cli_parameter) * cli_cmd->parametersCount);
		cli_cmd->parameters[cli_cmd->parametersCount-1] = *p;
		os_free(p);
		//os_memcpy(cli_cmd->parameters[cli_cmd->parametersCount-1],p,sizeof(cli_parameter));
		i += consumeSpaces(cmd + i);
		//prt("-consuming spaces after parameter, i is %d\n",i);
		after_comma = (cmd[i] == ',');
		if(after_comma) i++; else break;
		//prt("-going to read anther parameter from i of %d\n",i);
	}
	//prt("-no more parameters at i %d\n",i);
	if(cmd[i++] != ')'){
		cli_cmd->error = EXPECTING_RIGHT_PARENTHESIS;
		cli_cmd->totalLength = 0;
		return;
	}
	cli_cmd->currentIndex = i;
	cli_cmd->totalLength = i;
}

/*
 * cli_command readCommand(char *cmd);
 *
 * reads a command from the given $cmd string.
 * returns a cli_command
 */
cli_command* ICACHE_FLASH_ATTR readCommand(char *cmd){
	cli_command *r = (cli_command *)os_malloc(sizeof(cli_command));
	os_memcpy(r, &empty_cli_command, sizeof(cli_command)); //clears the command
	r->currentIndex += consumeSpaces(cmd + r->currentIndex);
	//prt("command of length %d is : \"%s\"\n",(int)strlen(cmd),cmd);
	r->totalLength = 1;
	readCommandName(cmd,r);
	if(r->error != NONE) return(r);
	if(r->totalLength < 1){
		r->error = (r->totalLength == -1) ? NO_COMMAND_FOUND : EXPECTING_COMMAND;
		return(r);
	}
	r->totalLength = 1;
	//prt("reading name consumed %d chars as input; left %d: \"%s\"\n",r->currentIndex,(int)strlen(cmd)-r->currentIndex,cmd+r->currentIndex);
	//prt("--------------------------------------------------\n");
	r->currentIndex += consumeSpaces(cmd + r->currentIndex);
	//prt("reading parameters from string \"%s\"\n",cmd+r->currentIndex);
	readCommandParameters(cmd,r);
	if(r->error != NONE) return(r);
	//prt("--------------------------------------------------\n");
	if(r->totalLength == 0){
		r->error = EXPECTING_LEFT_PARENTHESIS;
		return(r);
	}
	//prt("reading parameters consumed %d chars as input; left %d: \"%s\"\n",r->currentIndex,(int)strlen(cmd)-r->currentIndex,cmd+r->currentIndex);
	r->currentIndex += consumeSpaces(cmd + r->currentIndex);
	if(cmd[r->currentIndex] != ';'){
		r->error = EXPECTING_SEMICOLON;
		r->totalLength = 0;
		return(r);
	}
	r->currentIndex++;
	r->totalLength = r->currentIndex;
	//prt("read semicolon. total chars consumed: %d/%d\n",r->currentIndex,(int)strlen(cmd));
	return(r);
}

void ICACHE_FLASH_ATTR free_command(cli_command *cmd){
	for(int i=0; i<cmd->parametersCount; i++) os_free((cmd->parameters)[i].parameter);
	os_free(cmd->parameters);
	os_free(cmd->name);
	os_free(cmd);
}

void ICACHE_FLASH_ATTR printCommandInfo(cli_command *cmd, char *cmd_string, int cmd_string_max_length){
	if(cmd->error != NONE && cmd->error != NO_COMMAND_FOUND){
		int line = 1, letter = 1;
		char *lineStart = &cmd_string[0];
		int length = 0;
		for(int p=0; p < cmd->currentIndex; p++){
			if(cmd_string[p] == '\n'){
				lineStart = &cmd_string[p+1];
				letter = 0;
				line++;
			}else letter++;
		}
		while(lineStart[length] != '\n' && lineStart[length] != '\0' && length < cmd_string_max_length) length++;
		os_printf("Error at line %d char %d\n",line,letter);
		char *line_with_error = (char *)os_malloc(sizeof(char) * length+1);
		os_memcpy(line_with_error,lineStart,sizeof(char) * length+1);
		line_with_error[length] = '\0';
		os_printf("\"%s\"\n ",line_with_error);
		//os_printf("\"%.*s\"\n ",length,lineStart);
		for(int i=0; i<letter-1; i++) os_printf(" ");
		os_printf("^\n");
		switch(cmd->error){
			case 0: break;
			case 1: os_printf("expecting command!\n"); break;
			case 2: os_printf("command name too long!\n"); break;
			case 3: os_printf("expecting integer!\n"); break;
			case 4: os_printf("expecting parameter!\n"); break;
			case 5: os_printf("expecting color!\n"); break;
			case 6: os_printf("expecting color value!\n"); break;
			case 7: os_printf("color value out of range!\n"); break;
			case 8: os_printf("expecting comma!\n"); break;
			case 9: os_printf("expecting semicolon!\n"); break;
			case 10: os_printf("expecting \'(\'!\n"); break;
			case 11: os_printf("expecting \')\'!\n"); break;
			case 12: os_printf("expecting \'[\'!\n"); break;
			case 13: os_printf("expecting \']\'!\n"); break;
			case 14: os_printf("expecting \'{\'!\n"); break;
			case 15: os_printf("expecting \'}\'!\n"); break;
			case 16: os_printf("invalid parameters!\n"); break;
			case 17: os_printf("array with different datatypes!\n"); break;
			case 18: os_printf("unidentified datatype!\n"); break;
			case 19: /*os_printf("no command found...\n");*/ break;
		}
		return;
	}
	os_printf( "command: \"%s\"\n",cmd->name);
	if(cmd->parametersCount > 0) os_printf("parameters:\n");
	for(int i=0; i<cmd->parametersCount; i++){
		cli_parameter p = (cmd->parameters)[i];
		os_printf("\t");
		if(p.size == 0){
			if(p.type == COLOR_PARAMETER) os_printf("color: #%x",*((cli_parameter_color*)p.parameter));
			else if(p.type == INTEGER_PARAMETER) os_printf("int: %d",*((cli_parameter_int*)p.parameter));
			else if(p.type == ARRAY_PARAMETER) os_printf("empty array");
//			os_printf("  -  ('%c' : %d)\n",p.type,p.size);
			os_printf("\n");
		}else{
			if(p.type == COLOR_PARAMETER){
				os_printf("color[%d]: [",p.size);
				for(int i=0; i<p.size-1; i++) os_printf("#%x,",((cli_parameter_color*)p.parameter)[i]);
				os_printf("%x]\n",((cli_parameter_color*)p.parameter)[p.size-1]);
			}else if(p.type == INTEGER_PARAMETER){
				os_printf("integer[%d]: [",p.size);
				for(int i=0; i<p.size-1; i++) os_printf("%d,",((cli_parameter_int*)p.parameter)[i]);
				os_printf("%d]\n",((cli_parameter_int*)p.parameter)[p.size-1]);
			}
		}
	}
	os_printf("Total length: %d.\n",cmd->currentIndex);
}


