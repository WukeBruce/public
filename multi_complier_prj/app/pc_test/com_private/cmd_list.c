
#include <stdio.h>
#include "debug.h"
#include "cmd_list.h"


static int cmd_mkflash(int argc, char *argv[])
{

	COM_INFO("file: %s,empty\n",__FILE__);
	return 0;
}

static int cmd_list(int argc, char *argv[])
{
	COM_INFO("file: %s enter\n",__FILE__);
	list_test();

	return 0;
}

static int cmd_test_sdl(int argc, char *argv[])
{
	COM_INFO("file: %s enter\n",__FILE__);
	//api_com_key_interface();
    sdl_api_test();

	return 0;
}

static int cmd_test_ffmpeg(int argc, char *argv[])
{
	COM_INFO("file: %s enter\n",__FILE__);

	return 0;
}

static int cmd_test_array_c(int argc, char *argv[])
{
	COM_INFO("file: %s enter\n",__FILE__);
    search_array_max_min_test();
	return 0;
}

static int cmd_test_crc32(int argc, char *argv[])
{
	COM_INFO("file: %s enter\n",__FILE__);
    crc32_algorithm_test();
	return 0;
}
static int cmd_hash_table(int argc, char *argv[])
{
	COM_INFO("file: %s enter\n",__FILE__);
    hash_struct_test();
	return 0;
}

static int cmd_print_size(int argc, char *argv[])
{
	COM_INFO("file: %s enter\n",__FILE__);
    print_size_test();
	return 0;
}

static int cmd_player(int argc, char *argv[])
{
	COM_INFO("file: %s argc: %d argv:%s\n",__FILE__,argc,argv[0]);
    simple_player_test(argc, argv);

    return 0;
}

cmd_node_t cmdtable[] = {
	{"help"         , cmd_help,            "help [cmd]"                                 },
	{"mkflash"      , cmd_mkflash,         "mkflash : mkflash <conf file> <flash file>"           },
	{"list"         , cmd_list,            "list : list kernel test "                          },
	{"sdl"          , cmd_test_sdl,        "sdl : sdl test"                                   },
	{"ffmpeg"       , cmd_test_ffmpeg,     "ffmpeg : ffmpeg test"                                },
	{"sarray"      , cmd_test_array_c,     "sarray : array search max and min test"       },
	{"crc32"        , cmd_test_crc32,      "crc32 : crc32 algorithm test"       },
	{"hash"         , cmd_hash_table,      "hash : hash table test"       },
	{"psize"         , cmd_print_size,     "psize : out the size B K M G T "       },
	{"player"       , cmd_player,          "player : simple play test"              },


	{NULL, NULL, NULL}
};


/**
typedef int (*cmd_func)(int argc, char *argv[]);

static struct cmd_node {
	char *cmd;
	cmd_func func;
	char *helpbrief;
}cmdtable[] = {
	{"help"         , cmd_help,            "help [cmd]"                                 },
	{"mkflash"      , cmd_mkflash,         "mkflash : mkflash <conf file> <flash file>"           },
	{"list"         , cmd_list,            "list : list kernel test "                          },
	{"sdl"          , cmd_test_sdl,        "sdl : sdl test"                                   },
	{"ffmpeg"       , cmd_test_ffmpeg,     "ffmpeg : ffmpeg test"                                },
	{"array_c"      , cmd_test_array_c,    "array_c : array search max and min test"       },

	{NULL, NULL, NULL}
};
*/


