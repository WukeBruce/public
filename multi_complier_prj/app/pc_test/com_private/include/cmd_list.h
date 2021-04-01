#ifndef _CMD_LIST_H_
#define _CMD_LIST_H_


#include "api_interface.h"


typedef int (*cmd_func)(int argc, char *argv[]);

typedef struct cmd_node {
	char *cmd;
	cmd_func func;
	char *helpbrief;
}cmd_node_t;

extern cmd_node_t cmdtable[];

extern int search_array_max_min_test(void);
extern 	void list_test(void);
int cmd_help(int argc, char *argv[]);


#endif /*_CMD_LIST_H_*/




