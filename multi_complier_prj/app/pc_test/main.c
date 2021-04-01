#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>

#include "api_interface.h"
#include "list.h"
#include "debug.h"
#include "cmd_list.h"


#ifdef WIN32
char *readline(char *prompt)
{
	static char data[256];
	fputs(prompt, stdout);
	fflush(stdout);

	return fgets(data, 255, stdin);
}

void add_history(char *line)
{

}
#else
#include <readline/readline.h>
#include <readline/history.h>
#endif


static int process_command(char *_str);

#define PRINT_ERR(msg) printf("%s failed at %s %d\n", msg, __FILE__, __LINE__)

#define PRINT_USAGE_AND_RET(ret_value) do {\
	printf("\tusage not right, please see help\n");\
	return ret_value;\
} while(0)

/* return the base name of a file - caller frees */
static char *basecmd(char *s)
{
	char *p;
	p = strrchr(s, '/');
#ifdef _WIN32
	p = strrchr(s, '\\');
#endif
	if (p) s = p + 1;

	return (char *)s;
}

static char *trim(char *str)
{
	char *cp = str;

	while (*cp != 0 && isspace(*cp))
		++cp;

	if (cp == str)
		cp += strlen(str) - 1;
	else {
		char *cpdes;
		for (cpdes = str; *cp != 0; )
			*cpdes++ = *cp++;
		cp = cpdes;
		*cp-- = 0;
	}

	while (cp > str && isspace(*cp))
		*cp-- = 0;

	return str;
}

static int _cmd_help(int argc, char *argv[])
{
	int i;

	if (argc == 2) {
		for (i = 0; cmdtable[i].cmd != NULL; i++)
			if (strcmp(argv[1], cmdtable[i].cmd) == 0) {
				printf("\t%s\n", cmdtable[i].helpbrief);
				break;
			}

		if (cmdtable[i].cmd == NULL)
			printf("no such command\n");

		return 0;
	}

	for (i = 0; cmdtable[i].cmd != NULL; ++i)
		if (cmdtable[i].helpbrief)
			printf("\t%s\n", cmdtable[i].helpbrief);

	printf("\tquit/exit/q\n");

	return 0;
}

int cmd_help(int argc, char *argv[])
{
    _cmd_help(argc, argv);
	return 0;
}

static struct cmd_node *get_cmd_func(char *cmd)
{
	int i, cmdlen;

	if (cmd) {
		char *base = basecmd(cmd);

		// lookup the command table
		cmdlen = strlen(base);
		for (i = 0; cmdtable[i].cmd != NULL; ++i)
			if (strncmp(cmdtable[i].cmd, base, cmdlen) == 0) {
				return cmdtable + i;
			}
	}

	return NULL;
}

static int process_command(char *_str)
{
	int argc;
	char str[256], *cp;
	char *argv[256];
	struct cmd_node *node;

	strcpy(str, _str);
	trim(str);
	if (str[0] != 0) {
		if (strcasecmp(str, "exit") == 0 || strcasecmp(str, "q") == 0 || strcasecmp(str, "quit") == 0) {
			return -2;
		}

		memset(argv, 0, sizeof argv);
		argc = 0;
		for (cp = strtok(str, " "); cp != NULL; cp = strtok(NULL, " "))
			argv[argc++] = cp;

		node = get_cmd_func(argv[0]);
		if (node) {
			if (node->func(argc, argv) == -1)
				printf("\tusage: %s\n", node->helpbrief);

			return 0;
		}
		printf("not found command: %s\n", argv[0]);
	}

	return 0;
}

int main(int argc, char **argv)
{
	char *line;
	struct cmd_node *node = NULL;
	node = get_cmd_func(argv[0]);

	if (node) {
		printf("running %s\n", argv[0]);
		return node->func(argc, argv);
	}

	if (argc > 1) {
		struct cmd_node *node = get_cmd_func(argv[1]);

		if (node)
			return node->func(argc - 1, argv + 1);
	}

    app_log_init();
    process_command("help");//first enter usage

    while( (line = readline(">> ")) != NULL) {
		add_history(line);
		if (process_command(line) == -2)
			break;
	}

	return 0;
}



