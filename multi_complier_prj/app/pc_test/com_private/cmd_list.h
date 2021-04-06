#ifndef _CMD_LIST_H_
#define _CMD_LIST_H_


#include "api_interface.h"
#include "hash.h"
#include "sdl_api.h"

typedef int (*cmd_func)(int argc, char *argv[]);

typedef struct cmd_node {
	char *cmd;
	cmd_func func;
	char *helpbrief;
}cmd_node_t;

extern cmd_node_t cmdtable[];

extern int search_array_max_min_test(void);
extern 	void list_test(void);
extern int print_size_test(void);
extern int sdl_api_test(void);
extern int simple_player_test(int argc, char* argv[]);


int cmd_help(int argc, char *argv[]);
int crc32_algorithm_test(void);



#endif /*_CMD_LIST_H_*/




