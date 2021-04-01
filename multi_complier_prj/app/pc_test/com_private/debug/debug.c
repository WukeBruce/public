
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "debug.h"

LOG_LEVEL_E g_pri_log_level = COM_LOG_INFO;

static void dump_printf(uint8_t *data,int32_t size,char *str_flag)
{
	int32_t i;
	printf("\n--->[%s]data start:\n",str_flag);
	if ((size != 0)&&(data !=NULL)) {
		for (i = 0; i < (size); i++) {
			printf("0x%02x,", (data)[i]);
			if((i!=0)&&(i%10 == 0))
				printf("\n");
		}
	}
	printf("\n--->[%s]data end:\n",str_flag);

}

static void Log_SetLevel(LOG_LEVEL_E log)
{
	g_pri_log_level = log;
}

static LOG_LEVEL_E Log_GetLevel(void)
{
	return g_pri_log_level;
}

void app_log_init(void)
{
    LOG_LEVEL_E m_level = Log_GetLevel();
    Log_SetLevel(m_level);
    COM_INFO("m_level :%d\n",m_level);

    dump_printf((uint8_t *)"app_log_init",strlen("app_log_init"),"dump_printf");
}




