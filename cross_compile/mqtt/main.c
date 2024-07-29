/*******************************************************************************
 * Copyright (c) 2009, 2020 IBM Corp. and others
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    https://www.eclipse.org/legal/epl-2.0/
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - Test program utilities
 *    Milan Tucic - Session present test1
 *******************************************************************************/

#include "MQTTClient.h"
#include <string.h>
#include <stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<malloc.h>
#include <time.h>

#if !defined(_WINDOWS)
  #include <sys/time.h>
  #include <sys/socket.h>
  #include <unistd.h>
  #include <errno.h>
#else
  #include <windows.h>
  #define setenv(a, b, c) _putenv_s(a, b)
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define     MAX_LOOP_TIMES      3
#define     OTA_MSG_LEN      900
#define     OTA_MSG_CNT      1000
#define     PAYLOAD_FULL_LENGTH   (OTA_MSG_LEN - 8-4)

//ota data formate
//head+len+now+last+data+crc
//2+2+2+2+data+4

#define INPUT_FILE  "gd32_app_src.bin"
FILE *ota_file=NULL;
int total_section = 0;
int cur_section = 0;
int hw_version = 1;
int sw_version = 1;
int chipid = 1;
typedef struct mqtt_msg_bin{
    char bin[OTA_MSG_CNT][OTA_MSG_LEN];
    int section_cnt;
}mqtt_msg_bin_t;

void usage(void)
{
    printf("help!!\n");
    exit(EXIT_FAILURE);
}
#define  USE_SSL  1
#if USE_SSL
struct Options
{
    char* connection;         /**< connection to system under test. */
    char** haconnections;
    char* proxy_connection;
    char* client_id;
    char* username;
    char* password;
    char* log;
    int hacount;
    int verbose;
    int test_no;
    int MQTTVersion;
    int iterations;
    int reconnect_period;
} options =
{
    "ssl://k23b1411.ala.cn-hangzhou.emqxsl.cn:8883",// "ssl://broker.emqx.io:8883", "ssl://k23b1411.ala.cn-hangzhou.emqxsl.cn:8883"
    NULL,
    "tcp://localhost:1884",//
    "MQTT_test_ubuntu_tool_0001",//"c-client" client_id
    "test1",// test1 username
    "123456",// 123456 password
    NULL,
    0,
    0,
    0,
    MQTTVERSION_DEFAULT,
    1,
    3
};
//test1
//123456
#else
struct Options
{
    char* connection;         /**< connection to system under test. */
    char** haconnections;
    char* proxy_connection;
    char* client_id;
    char* username;
    char* password;
    char* log;
    int hacount;
    int verbose;
    int test_no;
    int MQTTVersion;
    int iterations;
    int reconnect_period;
} options =
{
    "tcp://broker.emqx.io:1883",// "tcp://localhost:1883",
    NULL,
    "tcp://localhost:1884",
    "MQTT_test_ubuntu_tool_0001",
    NULL,
    NULL,
    NULL,
    0,
    0,
    0,
    MQTTVERSION_DEFAULT,
    1,
    3
};

#endif

void getopts(int argc, char** argv)
{
    int count = 1;

    while (count < argc)
    {
        if (strcmp(argv[count], "--test_no") == 0)
        {
            if (++count < argc)
                options.test_no = atoi(argv[count]);
            else
                usage();
        }
        else if (strcmp(argv[count], "--connection") == 0)
        {
            if (++count < argc)
            {
                options.connection = argv[count];
                printf("\nSetting connection to %s\n", options.connection);
            }
            else
                usage();
        }
        else if (strcmp(argv[count], "--haconnections") == 0)
        {
            if (++count < argc)
            {
                char* tok = strtok(argv[count], " ");
                options.hacount = 0;
                options.haconnections = malloc(sizeof(char*) * 5);
                while (tok)
                {
                    options.haconnections[options.hacount] = malloc(strlen(tok) + 1);
                    strcpy(options.haconnections[options.hacount], tok);
                    options.hacount++;
                    tok = strtok(NULL, " ");
                }
            }
            else
                usage();
        }
        else if (strcmp(argv[count], "--proxy_connection") == 0)
        {
            if (++count < argc)
                options.proxy_connection = argv[count];
            else
                usage();
        }
        else if (strcmp(argv[count], "--client_id") == 0)
        {
            if (++count < argc)
                options.client_id = argv[count];
            else
                usage();
        }
        else if (strcmp(argv[count], "--username") == 0)
        {
            if (++count < argc)
                options.username = argv[count];
            else
                usage();
        }
        else if (strcmp(argv[count], "--password") == 0)
        {
            if (++count < argc)
                options.password = argv[count];
            else
                usage();
        }
        else if (strcmp(argv[count], "--log") == 0)
        {
            if (++count < argc)
                options.log = argv[count];
            else
                usage();
        }
        else if (strcmp(argv[count], "--MQTTversion") == 0)
        {
            if (++count < argc)
            {
                options.MQTTVersion = atoi(argv[count]);
                printf("setting MQTT version to %d\n", options.MQTTVersion);
            }
            else
                usage();
        }
        else if (strcmp(argv[count], "--iterations") == 0)
        {
            if (++count < argc)
                options.iterations = atoi(argv[count]);
            else
                usage();
        }
        else if (strcmp(argv[count], "--reconnection_period") == 0)
        {
            if (++count < argc)
                options.reconnect_period = atoi(argv[count]);
            else
                usage();
        }
        else if (strcmp(argv[count], "--verbose") == 0)
        {
            options.verbose = 1;
            printf("\nSetting verbose on\n");
        }
        count++;
    }
}


#define LOGA_DEBUG 0
#define LOGA_INFO 1
#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>
void MyLog(int LOGA_level, char* format, ...)
{
	static char msg_buf[256];
	va_list args;
#if defined(_WIN32) || defined(_WINDOWS)
	struct timeb ts;
#else
	struct timeval ts;
#endif
	struct tm timeinfo;

	if (LOGA_level == LOGA_DEBUG && options.verbose == 0)
	  return;

#if defined(_WIN32) || defined(_WINDOWS)
	ftime(&ts);
	localtime_s(&timeinfo, &ts.time);
#else
	gettimeofday(&ts, NULL);
	localtime_r(&ts.tv_sec, &timeinfo);
#endif
	strftime(msg_buf, 80, "%Y%m%d %H%M%S", &timeinfo);

#if defined(_WIN32) || defined(_WINDOWS)
	sprintf(&msg_buf[strlen(msg_buf)], ".%.3hu ", ts.millitm);
#else
	sprintf(&msg_buf[strlen(msg_buf)], ".%.3lu ", ts.tv_usec / 1000L);
#endif

	va_start(args, format);
	vsnprintf(&msg_buf[strlen(msg_buf)], sizeof(msg_buf) - strlen(msg_buf), format, args);
	va_end(args);

	printf("%s\n", msg_buf);
	fflush(stdout);
}


#if defined(_WIN32) || defined(_WINDOWS)
#define mqsleep(A) Sleep(1000*A)
#define START_TIME_TYPE DWORD
static DWORD start_time = 0;
START_TIME_TYPE start_clock(void)
{
    return GetTickCount();
}
#elif defined(AIX)
#define mqsleep sleep
#define START_TIME_TYPE struct timespec
START_TIME_TYPE start_clock(void)
{
    static struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    return start;
}
#else
#define mqsleep sleep
#define START_TIME_TYPE struct timeval
/* TODO - unused - remove? static struct timeval start_time; */
START_TIME_TYPE start_clock(void)
{
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    return start_time;
}
#endif


#if defined(_WIN32)
long elapsed(START_TIME_TYPE start_time)
{
    return GetTickCount() - start_time;
}
#elif defined(AIX)
#define assert(a)
long elapsed(struct timespec start)
{
    struct timespec now, res;

    clock_gettime(CLOCK_REALTIME, &now);
    ntimersub(now, start, res);
    return (res.tv_sec)*1000L + (res.tv_nsec)/1000000L;
}
#else
long elapsed(START_TIME_TYPE start_time)
{
    struct timeval now, res;

    gettimeofday(&now, NULL);
    timersub(&now, &start_time, &res);
    return (res.tv_sec)*1000 + (res.tv_usec)/1000;
}
#endif


#define assert(a, b, c, d) myassert(__FILE__, __LINE__, a, b, c, d)
#define assert1(a, b, c, d, e) myassert(__FILE__, __LINE__, a, b, c, d, e)

int tests = 0;
int failures = 0;
FILE* xml;
START_TIME_TYPE global_start_time;
char output[3000];
char* cur_output = output;


void write_test_result(void)
{
    long duration = elapsed(global_start_time);

    fprintf(xml, " time=\"%ld.%.3ld\" >\n", duration / 1000, duration % 1000);
    if (cur_output != output)
    {
        fprintf(xml, "%s", output);
        cur_output = output;
    }
    fprintf(xml, "</testcase>\n");
}


void myassert(char* filename, int lineno, char* description, int value, char* format, ...)
{
    ++tests;
    if (!value)
    {
        va_list args;

        ++failures;
        MyLog(LOGA_INFO, "Assertion failed, file %s, line %d, description: %s\n", filename, lineno, description);

        va_start(args, format);
        vprintf(format, args);
        va_end(args);

        cur_output += sprintf(cur_output, "<failure type=\"%s\">file %s, line %d </failure>\n",
                        description, filename, lineno);
    }
    else
        MyLog(LOGA_DEBUG, "Assertion succeeded, file %s, line %d, description: %s", filename, lineno, description);
}

void mysleep(int seconds)
{
#if defined(_WIN32)
    Sleep(1000L*seconds);
#else
    sleep(seconds);
#endif
}

/*********************************************************************

Test 1: clean session and reconnect with session present

*********************************************************************/
MQTTClient test1_c1;

void test1_connectionLost(void* context, char* cause)
{
    printf("Callback: connection lost\n");
}

void test1_deliveryComplete(void* context, MQTTClient_deliveryToken token)
{
    printf("Callback: publish complete for token %d\n", token);
}

int test1_messageArrived(void* context, char* topicName, int topicLen, MQTTClient_message* m)
{
    //MQTTClient c = (MQTTClient)context;
    printf("Callback: message received on topic '%s' is '%.*s'.\n", topicName, m->payloadlen, (char*)(m->payload));
    MQTTClient_free(topicName);
    MQTTClient_freeMessage(&m);
    return 1;
}

static unsigned int s_nTable[256] =
{
0x00000000, 0x04C11DB7, 0x9823B6E, 0xD4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2,
0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3,
0x3C8EA00A, 0x384FBDBD, 0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC,
0x5BD4B01B, 0x569796C2, 0x52568B75,  0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,  0x9823B6E0, 0x9CE2AB57, 0x91A18D8E,
0x95609039,0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,  0xBE2B5B58, 0xBAEA46EF,
0xB7A96036,0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,  0xD4326D90,
0xD0F37027,0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
0xF23A8028,0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A,
0xEC7DD02D,0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C,
0x2E003DC5,0x2AC12072,  0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x18AEB13,
0x54BF6A4,0x808D07D, 0xCC9CDCA,  0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB,
0x6F52C06C, 0x6211E6B5, 0x66D0FB02,  0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,  0xACA5C697, 0xA864DB20, 0xA527FDF9,
0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,  0x8AAD2B2F, 0x8E6C3698,
0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,  0xE0B41DE7,
0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED,
0xD8FBA05A,  0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85,
0x738AAD5C, 0x774BB0EB,  0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A,
0x58C1663D, 0x558240E4, 0x51435D53,  0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,  0x315D626, 0x7D4CB91, 0xA97ED48,
0xE56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,  0xF12F560E, 0xF5EE4BB9,
0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,  0xD727BBB6,
0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC,
0xA379DD7B,  0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD,
0x81B02D74, 0x857130C3,  0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645,
0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,  0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,  0x119B4BE9, 0x155A565E, 0x18197087,
0x1CD86D30, 0x29F3D35, 0x65E2082, 0xB1D065B, 0xFDC1BEC,  0x3793A651, 0x3352BBE6,
0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,  0xC5A92679,
0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673,
0xFDE69BC4,  0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
0x933EB0BB, 0x97FFAD0C,  0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,0xBCB4666D,
0xB8757BDA, 0xB5365D03, 0xB1F740B4,
};

/**国芯协议的CRC32,注意CRC32所用的方式可能不同*/
static unsigned int _crc32_value(unsigned char * pBuffer, unsigned int nSize)
{
    unsigned int nResult = 0xFFFFFFFF;
    while (nSize--) nResult = (nResult << 8) ^ s_nTable[(nResult >> 24) ^ *pBuffer++];
    return nResult;
}

static int running;

static mqtt_msg_bin_t s_mqtt_msg_data;

static void mqtt_msg_data_init(void)
{
    memset(&s_mqtt_msg_data,0x0,sizeof(mqtt_msg_bin_t));
}

unsigned int htoi(const char *str)
{
	const char *cp;
	unsigned int data, bdata;
	if(str && (strlen(str) > 2))
	{
		if((str[1]=='x') || (str[1]=='X'))
		{
			cp = str + 2;
		}
		else
		{
			cp = str;
		}
	}
	else
	{
		cp = str;
	}

	for (data = 0; *cp != 0; ++cp) {
		if (*cp >= '0' && *cp <= '9')
			bdata = *cp - '0';
		else if (*cp >= 'A' && *cp <= 'F')
			bdata = *cp - 'A' + 10;
		else if (*cp >= 'a' && *cp <= 'f')
			bdata = *cp - 'a' + 10;
		else
			break;
		data = (data << 4) | bdata;
	}

	return data;
}

//2byte
void make_ota_head(unsigned char *buf)
{
	if(buf == NULL)
	{
		printf("buf null\n");
		return;
	}
	buf[0]=0x77;//w
	buf[1]=0x6b;//k
}

int make_ota_section(unsigned char*p_buf,int length,int section_number)
{
	unsigned char buf[OTA_MSG_LEN];
	int i = 0;
	int write_len = 0;
	int crc = 0;
	int section_length = (length + 8+4);
	memset(buf,0xff,OTA_MSG_LEN);
	make_ota_head(buf);
	i = 2;
	//len
    	buf[i++] = (section_length & 0xff00)>>8;
	buf[i++] = (section_length & 0xff);// 4

	buf[i++] = ((section_number & 0xff00) >> 8);
	buf[i++] = ((section_number & 0xff));
	buf[i++] = ((total_section & 0xff00) >> 8);
	buf[i++] = ((total_section & 0xff));

	memcpy(&buf[8],p_buf,length);

	crc = _crc32_value(&buf[4],section_length-4-4);// rm crc and head and len
	buf[length + 8] = ((crc & 0xff000000) >> 24);
	buf[length + 8+1] = ((crc & 0x00ff0000) >> 16);
	buf[length + 8+2] = ((crc & 0x0000ff00) >> 8);
	buf[length + 8+3] = ((crc & 0x000000ff) >> 0);

        memcpy(s_mqtt_msg_data.bin[s_mqtt_msg_data.section_cnt],buf,OTA_MSG_LEN);
        s_mqtt_msg_data.section_cnt++;

	write_len = fwrite(buf,1,sizeof(buf),ota_file);
	if(section_number == total_section)
	{
		printf("finish file make write_len  %d\n",write_len);
		fclose(ota_file);
	}
    return 0;
}


void make_ota_section_0(int hw_version,int chipid,int sw_version)
{
	int section_0_length = 20+4;
	unsigned char buf[OTA_MSG_LEN];
	int i = 0;
	int crc32 = 0;
         cur_section=0;
	memset(buf,0xff,OTA_MSG_LEN);

	make_ota_head(buf);
	i = 2;
	buf[i++] = (section_0_length & 0xff00)>>8;
	buf[i++] = (section_0_length & 0xff);// 4

	buf[i++] = ((cur_section & 0xff00) >> 8);
	buf[i++] = ((cur_section & 0xff));
	buf[i++] = ((total_section & 0xff00) >> 8);
	buf[i++] = ((total_section & 0xff));

	buf[i++]=((chipid & 0xff000000) >> 24);
	buf[i++]=((chipid & 0x00ff0000) >> 16);
	buf[i++]=((chipid & 0x0000ff00) >> 8);
	buf[i++]=((chipid & 0x000000ff) >> 0);

	buf[i++]=((hw_version & 0xff000000) >> 24);
	buf[i++]=((hw_version & 0x00ff0000) >> 16);
	buf[i++]=((hw_version & 0x0000ff00) >> 8);
	buf[i++]=((hw_version & 0x000000ff) >> 0);

	buf[i++]=((sw_version & 0xff000000) >> 24);
	buf[i++]=((sw_version & 0x00ff0000) >> 16);
	buf[i++]=((sw_version & 0x0000ff00) >> 8);
	buf[i++]=((sw_version & 0x000000ff) >> 0);

	crc32 = _crc32_value(&buf[4],section_0_length-4-4);// rm crc and head and len

	buf[i++]=((crc32 & 0xff000000) >> 24);
	buf[i++]=((crc32 & 0x00ff0000) >> 16);
	buf[i++]=((crc32 & 0x0000ff00) >> 8);
	buf[i++]=((crc32 & 0x000000ff) >> 0);
         memcpy(s_mqtt_msg_data.bin[s_mqtt_msg_data.section_cnt],buf,OTA_MSG_LEN);
         s_mqtt_msg_data.section_cnt++;

	printf("\ ndata start \n");
	for(i=0;i<OTA_MSG_LEN;i++)
	{
		if(i%16==0 && i!=0)
			printf("\n");
		printf("%02x\t",buf[i]);
	}
	printf("\n data end \n");

	fwrite(buf,1,sizeof(buf),ota_file);
}

void make_file_to_ota_bin(unsigned char *buf,int length)
{
	int i = 0;
	unsigned char *p_data = buf;
	int rest_length = length;
	int finish_length = 0;

	total_section = (length / PAYLOAD_FULL_LENGTH) + 1;// because insert sec 0
	make_ota_section_0(hw_version,chipid,sw_version);

	for(i=1;i<=total_section;i++)
	{
		if(i == total_section)
		{
			if(rest_length > PAYLOAD_FULL_LENGTH)
			{
				printf("error ..length to much\n");
			}
			make_ota_section(p_data,rest_length,i);
		}
		else
		{
			make_ota_section(p_data,PAYLOAD_FULL_LENGTH,i);
			rest_length -= PAYLOAD_FULL_LENGTH;
			p_data += PAYLOAD_FULL_LENGTH;
			finish_length += PAYLOAD_FULL_LENGTH;
		}
	}
}


static char *get_date(void)
{
	static char time_buf[256]={0};
	time_t rawtime;
	int time_year;
	int time_month;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	time_year=(timeinfo->tm_year + 1900);
	time_month=(timeinfo->tm_mon + 1);

         memset(time_buf,0x0,sizeof(time_buf));
	sprintf(time_buf,"%04d%02d%02d_%02d%02d%02d",time_year,time_month,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
        return time_buf;
}
int ota_bin_create_package(void)
{
	FILE *read_fd = NULL;
         char *src_file= INPUT_FILE;
        unsigned char *src_data = NULL;
	int src_read_len = 0;
	int file_crc = 0;
         char buf[1024] ={0};
         char *p_time=NULL;

         unsigned int  filesize = 0;
 #if !defined(_WINDOWS)
         struct stat statinfo;
#endif
	read_fd = fopen(src_file,"r");
	if(read_fd == NULL)
	{
		printf("file %s  open failed\n",INPUT_FILE);
		return -1;
	}
        p_time = get_date();
        // gd32_chipid_hw_sw_time_ota.bin
	sprintf(buf,"gd32_chipid%s_hw%s_sw%s_%s_ota.bin","0001","0001","0001",p_time);
	printf("%s\n",buf);

	ota_file = fopen(buf,"w");
	if(ota_file == NULL)
	{
		printf("output file open failed\n");
		fclose(read_fd);
		return -1;
	}
#if !defined(_WINDOWS)
        if(stat(src_file,&statinfo) != 0)
        {
            printf("stat failed for  file %s\n",INPUT_FILE);
            goto out;

        }

        filesize = statinfo.st_size;
#else
        HANDLE file = CreateFile(src_file, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (file == INVALID_HANDLE_VALUE) {
            printf("Cannot open %s\n", src_file);
            return 1;
        }

        LARGE_INTEGER size;
        if (GetFileSizeEx(file, &size)) {
            printf("Size of %s: %lld bytes\n", src_file, size.QuadPart);
        } else {
            printf("Cannot get size of %s\n", src_file);
        }

        CloseHandle(file);
        filesize = size.QuadPart;
#endif
        src_data = malloc(filesize+4); //4字节CRC
        src_read_len=fread(src_data,1,filesize,read_fd);
        if(src_read_len != filesize)
        {
            printf("input data read failed\n");
            goto out;
        }
        file_crc = _crc32_value(src_data,src_read_len);
        src_data[src_read_len]=((file_crc & 0xff000000) >> 24);
        src_data[src_read_len+1]=((file_crc & 0x00ff0000) >> 16);
        src_data[src_read_len+2]=((file_crc & 0x0000ff00) >> 8);
        src_data[src_read_len+3]=((file_crc & 0x000000ff) >> 0);

        printf("file_crc = %x\n",file_crc);
        make_file_to_ota_bin(src_data,src_read_len+4);
        out:
        if(src_data)
        {
            	free(src_data);
        }
	fclose(read_fd);
	return 0;

}
void mqtt_sendMsage(MQTTClient c, int qos, char* test_topic)
{
	MQTTClient_deliveryToken dt;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	//MQTTClient_message* m = NULL;
	int i = 0;
	int rc;
	int iterations = 1;// send times

	MyLog(LOGA_INFO, "pub_topic= %s,loop times = %d messages at QoS %d",test_topic, iterations, qos);
	pubmsg.qos = qos;
	pubmsg.retained = 0;
	for (i = 0; i< iterations; ++i)
	{
                int j=0;
                for (j = 0; j< s_mqtt_msg_data.section_cnt; ++j)
                {
                    pubmsg.payload = s_mqtt_msg_data.bin[j];
                    pubmsg.payloadlen = OTA_MSG_LEN;
                    //or
                    rc = MQTTClient_publish(c, test_topic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
                   //rc = MQTTClient_publishMessage(c, test_topic, &pubmsg, &dt);
                    assert("Good rc from publish", rc == MQTTCLIENT_SUCCESS, "rc was %d", rc);
                    usleep(1000*1000);//300 ms

                    char *p_time=NULL;
                    p_time = get_date();
                    printf("--->iterations %d ,cnt %d, time %s send \n",i , j,p_time);
                }
	}

}

void mqtt_subMsage(MQTTClient c, int qos, char* test_topic)
{
        int rc;
        MQTTClient_message* m = NULL;
        int topicLen;
        char* subtopic = "MQ/gd32/wendu/0001/reserved/topic/test";
        char* topicName = NULL;
        int i =0;
        int iterations = (5000/50);//5s *60*60*24*1

        MyLog(LOGA_INFO, "subtopic= %s messages at QoS %d sub ok !!!", subtopic, qos);
        rc = MQTTClient_subscribe(c, subtopic, qos);
        assert("Good rc from subscribe", rc == MQTTCLIENT_SUCCESS, "rc was %d", rc);

        for (i = 0; i< iterations; ++i)
        {
                rc = MQTTClient_receive(c, &topicName, &topicLen, &m, 2000);
                if (topicName)
                {
                    char *p_time=NULL;
                    p_time = get_date();
                    printf("--->iterations %d  time %s Recv \n",i ,p_time);
#if 0
                    printf("Message received on topic %s is %.*s.\n", topicName, m->payloadlen, (char*)(m->payload));
                    MQTTClient_free(topicName);
                    MQTTClient_freeMessage(&m);
#endif
                }


                usleep(50*1000);//10 ms
        }

        rc = MQTTClient_unsubscribe(c, subtopic);
        assert("Unsubscribe successful", rc == MQTTCLIENT_SUCCESS, "rc was %d", rc);

        return ;
}

void mqtt_SendandRecvMsage(MQTTClient c, int qos, char* test_topic)
{
	int i = 0;
	int iterations = MAX_LOOP_TIMES;
	for (i = 0; i< iterations; ++i)
	{
                mqtt_sendMsage(c, qos, test_topic);
                mqtt_subMsage(c, qos, test_topic);
	}

        running = 0;
}


int send_topic_bin(struct Options options)
{
    char* testname = "send_topic_bin";
    char* testtopic = "MQ/gd32/ota/0001/reserved/topic/test";
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;

    failures = 0;
    MyLog(LOGA_INFO, "Starting test 1 - clean session and reconnect with session present");
    fprintf(xml, "<testcase classname=\"send_topic_bin\" name=\"connectionLost and will messages\"");
    global_start_time = start_clock();

    opts.keepAliveInterval = 60;
    opts.cleansession = 1;
    opts.MQTTVersion = MQTTVERSION_3_1_1;
    if (options.haconnections != NULL)
    {
        opts.serverURIs = options.haconnections;
        opts.serverURIcount = options.hacount;
    }
    if (options.username)
    {
        opts.username = options.username;
    }
    if (options.password)
    {
        opts.password = options.password;
    }
    #if USE_SSL
	MQTTClient_SSLOptions sslopts = MQTTClient_SSLOptions_initializer;
	opts.ssl = &sslopts;
	opts.ssl->enableServerCertAuth = 0;
    #endif
    MyLog(LOGA_INFO, "connection:%s,u %s,p %s,options.client_id %s",options.connection,opts.username,opts.password,options.client_id);

    rc = MQTTClient_create(&test1_c1, options.connection, options.client_id, MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);
    assert("good rc from create", rc == MQTTCLIENT_SUCCESS, "rc was %d\n", rc);
    if (rc != MQTTCLIENT_SUCCESS)
        goto exit;

    rc = MQTTClient_setCallbacks(test1_c1, (void*)test1_c1, test1_connectionLost, test1_messageArrived, test1_deliveryComplete);
    assert("good rc from setCallbacks",  rc == MQTTCLIENT_SUCCESS, "rc was %d\n", rc);
    if (rc != MQTTCLIENT_SUCCESS)
        goto exit;
    /* Connect to the broker with clean session = true */
    rc = MQTTClient_connect(test1_c1, &opts);
    assert("good rc from connect with clean session = true",  rc == MQTTCLIENT_SUCCESS, "rc was %d\n", rc);
    if (rc != MQTTCLIENT_SUCCESS)
        goto exit;

    assert("connected, session cleaned", opts.returned.sessionPresent == 0,
                     "opts.returned.sessionPresent = %d\n", opts.returned.sessionPresent);
    //pub
    MyLog(LOGA_INFO, "pub msage ...");

    mqtt_msg_data_init();
    ota_bin_create_package();
    //mqtt_sendMsage(test1_c1, 0,testtopic);
    mqtt_SendandRecvMsage(test1_c1,0, testtopic);
    //sub

   //MyLog(LOGA_INFO, "Sleeping after session cleaned %d s ...", options.reconnect_period);

    /* Disconnect */
    rc = MQTTClient_disconnect(test1_c1, 1000);
    assert("good rc from disconnect",  rc == MQTTCLIENT_SUCCESS, "rc was %d\n", rc);
    if (rc != MQTTCLIENT_SUCCESS)
        goto exit;

    MQTTClient_destroy(&test1_c1);
exit:
    MyLog(LOGA_INFO, "%s: test %s. %d tests run, %d failures.\n",
            (failures == 0) ? "passed" : "failed", testname, tests, failures);
    write_test_result();
    return failures;
}

#ifndef WIN32

#include <signal.h>
static void signal_handler(int sig)
{
	switch (sig) {
	case SIGINT:
	case SIGTERM:
		running = 0;
		break;
	}
}
static void init_signals(void)
{
	struct sigaction sigact;

	sigact.sa_handler = signal_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
}
#endif

int main(int argc, char** argv)
{
//value
#ifndef WIN32
	init_signals();
#endif

        int rc = 0;
        int (*tests[])() = {NULL, send_topic_bin};
        int i;
        unsigned test_i;

        xml = fopen("TEST-test1.xml", "w");
        fprintf(xml, "<testsuite name=\"send_topic_bin\" tests=\"%d\">\n", (int)(ARRAY_SIZE(tests) - 1));

        getopts(argc, argv);

        if (options.log)
        {
            setenv("MQTT_C_CLIENT_TRACE", "ON", 1);
            setenv("MQTT_C_CLIENT_TRACE_LEVEL", options.log, 1);
        }


        MyLog(LOGA_INFO, "Running %d iteration(s)", options.iterations);

        for (i = 0; i < options.iterations; ++i)
        {
            if (options.test_no == 0)
            { /* run all the tests */
                for (test_i = 1; test_i < ARRAY_SIZE(tests); ++test_i)
                    rc += tests[test_i](options); /* return number of failures.  0 = test succeeded */
            }
            else
                rc = tests[options.test_no](options); /* run just the selected test */
        }

//runing
	while (running) {
                mysleep(1);
	}

        if (rc == 0)
            MyLog(LOGA_INFO, "verdict pass");
        else
            MyLog(LOGA_INFO, "verdict fail");

        fprintf(xml, "</testsuite>\n");
        fclose(xml);
        return rc;
}

