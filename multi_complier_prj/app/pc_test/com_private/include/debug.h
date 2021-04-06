#ifndef _DEBUG_H_
#define _DEBUG_H_

#define PRI_DEBUG

/// log system

typedef enum
{
    COM_LOG_ERROR = (1 << 0),
    COM_LOG_INFO  = (1 << 1),
    COM_LOG_DEBUG = (1 << 2),
    COM_LOG_MAX   = (1 << 3),
}LOG_LEVEL_E;


extern LOG_LEVEL_E g_pri_log_level;

#ifdef PRI_DEBUG
#define COM_ERROR(...)	do{if( COM_LOG_ERROR <=  g_pri_log_level){\
								printf("\033[031m ---ERROR![%s,%d] \033[0m ",__FUNCTION__,__LINE__);\
								printf(__VA_ARGS__);}\
							}while(0)

#define COM_DEBUG(...)	do{if( COM_LOG_DEBUG <=  g_pri_log_level){\
								printf("\033[034m -DEBUG-[%s,%d] \033[0m ",__FUNCTION__,__LINE__);\
								printf(__VA_ARGS__);}\
							}while(0)
#define COM_INFO(...)	do{if( COM_LOG_INFO <=  g_pri_log_level){\
								printf("\033[032m --INFO-[%s,%d] \033[0m ",__FUNCTION__,__LINE__);\
								printf(__VA_ARGS__);}\
							}while(0)

#define COM_DUMP(mark, data, size)\
                    do {\
                    	int i;\
                    	if (size != 0) {  \
                    		printf("[API-COM]\t\%s Start: len=%d\n[API-COM]\t", (mark), (size));\
                    		for (i = 0; i < (size); i++) {\
                    			if (i != 0 && (i%16) == 0) { \
                    				printf("\n[API-COM]\t"); \
                    			}  \
                    			printf("\033[034m 0x%02x,\033[0m ", (data)[i]); \
                    		}\
                    		printf("\n[API-COM]\t%s End\n", (mark)); \
                    	} \
                   	} while (0)
#else
#define COM_ERROR(...)\
			do{ \
			}while(0)

#define COM_DEBUG(...)\
			do{ \
			}while(0)
#define COM_INFO(...)\
			do{ \
			}while(0)
#define COM_DUMP(mark, data, size)\
			do{ \
			}while(0)
#endif

#ifndef ARRAY_NUM
#define ARRAY_NUM(x) ((sizeof(x)/sizeof(x[0])))
#endif


void app_log_init(void);

#endif




