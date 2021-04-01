#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <libgen.h>

#include "flash.h"
#ifdef WIN32
#include <io.h>
#endif

static char *table_file_name = NULL;

static char *get_size_str(char *size_str, size_t size)
{
	if (size >= 1024 * 1024)
		sprintf(size_str, "%6.2f MB", size * 1.0 / 1024.0 / 1024.0);
	else if (size >= 1024)
		sprintf(size_str, "%6.2f KB", size * 1.0 / 1024.0);
	else
		sprintf(size_str, "%6d  B", size);

	return size_str;
}

static void int32tochar(uint8_t *data, uint32_t a)
{
	data[0]  = (a & 0xFF000000) >> 24;
	data[1]  = (a & 0x00FF0000) >> 16;
	data[2]  = (a & 0x0000FF00) >>  8;
	data[3]  = (a & 0x000000FF) >>  0;
}

static void int16tochar(uint8_t *data, uint16_t a)
{
	data[0]  = (a & 0xFF00) >> 8;
	data[1]  = (a & 0x00FF) >> 0;
}

static int char2int32(uint8_t *data)
{
	return (data[0] << 24) + (data[1] << 16) + (data[2]  << 8) + data[3];
}

static int char2int16(uint8_t *data)
{
	return (data[0]  << 8) + data[1];
}

static char *get_fstype(int type)
{
	switch (type) {
		case RAW:    return "RAW"     ;
		case CRAMFS: return "CRAMFS"  ;
		case ROMFS:  return "ROMFS"   ;
		case JFFS2:  return "JFFS2"   ;
		case YAFFS2: return "YAFFS2"  ;
		case MINIFS: return "MINIFS"  ;
		case HIDE:   return "HIDE"    ;
		default:     return "UNKNOWN" ;
	}
}

static char *crc32str(uint32_t crc)
{
	static char buf[9] = {0, };

	if (crc == 0)
		memset(buf, ' ', 8);
	else
		sprintf(buf, "%08X", crc);

	return buf;
}

struct table_ver_adapt {
	unsigned int version;
	unsigned int magic;
	unsigned int size;
	unsigned int part_num;
} table_sect = {
	.version = 1,
	.magic = PARTITION_MAGIC,
	.size = FLASH_PARTITION_SIZE,
	.part_num = FLASH_PARTITION_NUM,
};

static void init_table_sect(int version)
{
	if (version == 2) {
		table_sect.version = 2;
		table_sect.magic = PARTITION_V2_MAGIC;
		table_sect.size = FLASH_PARTITION_V2_SIZE;
		table_sect.part_num = FLASH_PARTITION_V2_NUM;
	} else {
		table_sect.version = 1;
		table_sect.magic = PARTITION_MAGIC;
		table_sect.size = FLASH_PARTITION_SIZE;
		table_sect.part_num = FLASH_PARTITION_NUM;
	}
}

void flash_printf(flash* entry)
{
	int i;
	char size_str1[32], size_str2[32], size_str3[32], size_str4[32];

	printf("\n");
	printf("Partition Version   :  %d\n", entry->flash_table.version);
	printf("Flase Size          :  %s\n", get_size_str(size_str1, entry->flash_size));
	printf("Partition Count     :  %d\n", entry->flash_table.count);
	printf("Partition MAX Count :  %d\n", table_sect.part_num);
	printf("Table Enable        :  %s\n", entry->table_enable ? "TRUE": "FALSE");
	printf("Write Protect       :  %s\n", entry->flash_table.write_protect ? "TRUE": "FALSE");
	printf("CRC32 Enable        :  %s\n", entry->flash_table.crc32_enable ? "TRUE": "FALSE");
	printf("Table CRC32         :  %X\n", entry->flash_table.crc_verify);

	printf("\n");

#define T_ID     "%-3s"
#define T_NAME   "%-8s"
#define T_MODE   "%-3s"
#define T_FS     "%-8s"
#define T_CRC    "%8s"
#define T_UPDATE "%8s"
#define T_VER    "%7s"
#define T_START  "%8s"
#define T_TSIZE  "%11s"
#define T_PSIZE  "%11s"
#define T_USIZE  "%11s"
#define T_PER    "%7s"
#define T_RSIZE  "%10s"
#define T_PASS   "%3s"
#define T_STR_FORMAT T_ID T_NAME T_MODE T_FS T_CRC T_UPDATE T_VER T_START T_TSIZE T_PSIZE T_USIZE T_PER T_RSIZE T_PASS"\n"

#define _ID     "%-3d"
#define _NAME   "%-8s"
#define _MODE   "%-3s"
#define _FS     "%-8s"
#define _CRC    "%-8s"
#define _UPDATE "%8u"
#define _VER    "%7u"
#define _START  " %08x"
#define _TSIZE  " %-10s"
#define _PSIZE  " %-10s"
#define _USIZE  "%-10s"
#define _PER    "%5.1f%%  "
#define _RSIZE  "%-7s"
#define _PASS   "%3s"
#define STR_FORMAT _ID _NAME _MODE _FS _CRC _UPDATE _VER _START _TSIZE _PSIZE _USIZE _PER _RSIZE _PASS"\n"

	printf("==========================================================================================================\n");
	printf(T_STR_FORMAT,
			"ID", "NAME", "", "FS", "CRC32", "UPDATE", "VERION", "START", "TOTAL_SIZE", "MAIN_SIZE", "USED_SIZE", "USE%", "RES_SIZE", "OK");
	printf("==========================================================================================================\n");
	for (i=0; i < entry->count; i++) {
		printf(STR_FORMAT,
				entry->list[i].info->id,
				entry->list[i].info->name,
				entry->list[i].info->mode == 0 ? "RO" : "RW",
				get_fstype(entry->list[i].info->file_system_type),
				crc32str(entry->list[i].info->crc32),
				entry->list[i].info->update_tags,
				entry->list[i].info->version,
				entry->list[i].info->start_addr,
				get_size_str(size_str1, entry->list[i].info->total_size),
				get_size_str(size_str2, entry->list[i].info->partition_size),
				get_size_str(size_str3, entry->list[i].info->used_size),
				entry->list[i].info->used_size * 100.0 / entry->list[i].info->partition_size,
				get_size_str(size_str4, entry->list[i].info->reserved_size),
				entry->list[i].crc32_passed ? "*" : " "
				);
	}
	printf("----------------------------------------------------------------------------------------------------------\n\n");
}

int flash_compr(const void *a, const void *b)
{
	struct partition_data *a1 = (struct partition_data*)a;
	struct partition_data *b1 = (struct partition_data*)b;

	return a1->info->start_addr - b1->info->start_addr;
}

void flash_sort(flash *entry)
{
	qsort(entry->list, entry->count, sizeof(struct partition_data), flash_compr);
}

static int check_table_crc(struct partition_table *tbl, uint8_t *data, int len)
{
	uint32_t new_crc;

	if (tbl->version >= PARTITION_VERSION) {
		new_crc = crc32(0, data, len - 4);
		return new_crc != tbl->crc_verify;
	}

	return 0;
}

static int flash_table_read(struct partition_table *tbl, uint8_t *data, int len)
{
	int i;
	uint8_t *part;
	uint8_t *crc32_table  = data + FLASH_PARTITION_CRC_ADDR;
	uint8_t *part_version = data + FLASH_PARTITION_VERSION_ADDR;

	if (len < FLASH_PARTITION_SIZE)
		return -1;

	tbl->magic         = char2int32(data);
	tbl->count         = data[4];
	tbl->crc_verify    = char2int32(data + FLASH_PARTITION_SIZE - 4);
	tbl->version       = data[FLASH_PARTITION_SIZE - 5];
	tbl->crc32_enable  = data[FLASH_PARTITION_SIZE - 6];
	tbl->write_protect = data[FLASH_PARTITION_SIZE - 7];

	if (tbl->count > FLASH_PARTITION_NUM) {
		printf("%s,%d partition_num=%d error\n", __func__, __LINE__, tbl->count);

		return -1;
	}

	part = data + 5;
	for (i = 0; i < tbl->count; i++, part += FLASH_PARTITION_HEAD_SIZE) {
		memcpy(tbl->tables[i].name, part, FLASH_PARTITION_NAME_SIZE);
		tbl->tables[i].total_size       = char2int32(part +  8);
		tbl->tables[i].used_size        = char2int32(part + 12);
		tbl->tables[i].partition_size   = tbl->tables[i].total_size;
		tbl->tables[i].start_addr       = char2int32(part + 16);
		tbl->tables[i].file_system_type = part[20];
		tbl->tables[i].mode             = part[21] & 0x7F;
		tbl->tables[i].crc32_enable     = part[21] >> 7;
		if (tbl->version >= PARTITION_VERSION) {
			tbl->tables[i].id               = part[22] & 0x7F;
			tbl->tables[i].write_protect    = (part[22] >> 7) & 0x01;
		}
		else {
			tbl->tables[i].id = i;
			tbl->tables[i].write_protect = 0;
		}
		tbl->tables[i].update_tags = part[23] & 0x03;

		tbl->tables[i].crc32   = char2int32(crc32_table  + i * sizeof(uint32_t));
		tbl->tables[i].version = char2int16(part_version + i * sizeof(uint16_t));
	}

	return check_table_crc(tbl, data, len);
}

static int flash_table_write(struct partition_table *tbl, uint8_t *data, int len)
{
	int i;
	uint8_t *part;
	uint8_t *crc32_table  = data + FLASH_PARTITION_CRC_ADDR;
	uint8_t *part_version = data + FLASH_PARTITION_VERSION_ADDR;

	if (len < FLASH_PARTITION_SIZE)
		return -1;

	memset(data, 0xFF, FLASH_PARTITION_SIZE);  // read table

	int32tochar(data, tbl->magic);
	data[4] = tbl->count;

	part = data + 5;
	for (i = 0; i < tbl->count; i++, part+= FLASH_PARTITION_HEAD_SIZE) {
		memcpy(part, tbl->tables[i].name, FLASH_PARTITION_NAME_SIZE + 1);
		int32tochar(part +  8, tbl->tables[i].total_size);
		int32tochar(part + 12, tbl->tables[i].used_size);
		int32tochar(part + 16, tbl->tables[i].start_addr);
		part[20] = tbl->tables[i].file_system_type;
		part[21] = tbl->tables[i].mode    | tbl->tables[i].crc32_enable << 7;
		part[22] = tbl->tables[i].id      | (tbl->tables[i].write_protect << 7);
		part[23] = tbl->tables[i].update_tags & 0x03;

		// 分区的校验码区域
		int32tochar(crc32_table  + i * sizeof(uint32_t), tbl->tables[i].crc32);
		// 分区的版本区域
		int16tochar(part_version + i * sizeof(uint16_t), tbl->tables[i].version);
	}

	for (; i < FLASH_PARTITION_NUM; i++, part += FLASH_PARTITION_HEAD_SIZE) {
		memset(part, 0xFF, FLASH_PARTITION_HEAD_SIZE);
		int32tochar(crc32_table + i * 4, 0xFFFFFFFF);
	}

	data[FLASH_PARTITION_SIZE - 5] = tbl->version;
	data[FLASH_PARTITION_SIZE - 6] = tbl->crc32_enable;
	data[FLASH_PARTITION_SIZE - 7] = tbl->write_protect;

	tbl->crc_verify = crc32(0, data, FLASH_PARTITION_SIZE - 4);
	int32tochar(data + FLASH_PARTITION_SIZE - 4, tbl->crc_verify);

	return 0;
}

static int flash_table_v2_read(struct partition_table *tbl, uint8_t *data, int len)
{
	int i;
	uint8_t *part;
	uint8_t *crc32_table  = data + FLASH_PARTITION_V2_CRC_ADDR;
	uint8_t *part_version = data + FLASH_PARTITION_V2_VERSION_ADDR;

	if (len < FLASH_PARTITION_V2_SIZE)
		return -1;

	tbl->magic         = char2int32(data);
	tbl->count         = data[4];
	tbl->crc_verify    = char2int32(data + FLASH_PARTITION_V2_SIZE - 4);
	tbl->version       = data[FLASH_PARTITION_V2_SIZE - 5];
	tbl->crc32_enable  = data[FLASH_PARTITION_V2_SIZE - 6];
	tbl->write_protect = data[FLASH_PARTITION_V2_SIZE - 7];

	if (tbl->count > FLASH_PARTITION_V2_NUM) {
		printf("%s,%d partition_num=%d error\n", __func__, __LINE__, tbl->count);

		return -1;
	}

	part = data + 5;
	for (i = 0; i < tbl->count; i++, part += FLASH_PARTITION_V2_HEAD_SIZE) {
		memcpy(tbl->tables[i].name, part, FLASH_PARTITION_NAME_SIZE);
		tbl->tables[i].total_size       = char2int32(part +  8);
		tbl->tables[i].used_size        = char2int32(part + 12);
		tbl->tables[i].reserved_size    = char2int32(part + 16);
		tbl->tables[i].partition_size   = tbl->tables[i].total_size - tbl->tables[i].reserved_size;
		tbl->tables[i].start_addr       = char2int32(part + 20);
		tbl->tables[i].file_system_type = part[24];
		tbl->tables[i].mode             = part[25] & 0x7F;
		tbl->tables[i].crc32_enable     = part[25] >> 7;
		if (tbl->version >= PARTITION_VERSION) {
			tbl->tables[i].id               = part[26] & 0x7F;
			tbl->tables[i].write_protect    = (part[26] >> 7) & 0x01;
		}
		else {
			tbl->tables[i].id = i;
			tbl->tables[i].write_protect = 0;
		}
		tbl->tables[i].update_tags = part[27] & 0x03;

		tbl->tables[i].crc32   = char2int32(crc32_table  + i * sizeof(uint32_t));
		tbl->tables[i].version = char2int16(part_version + i * sizeof(uint16_t));
	}

	return check_table_crc(tbl, data, len);
}

static int flash_table_v2_write(struct partition_table *tbl, uint8_t *data, int len)
{
	int i;
	uint8_t *part;
	uint8_t *crc32_table  = data + FLASH_PARTITION_V2_CRC_ADDR;
	uint8_t *part_version = data + FLASH_PARTITION_V2_VERSION_ADDR;

	if (len < FLASH_PARTITION_V2_SIZE)
		return -1;

	memset(data, 0xFF, FLASH_PARTITION_V2_SIZE);  // read table

	int32tochar(data, tbl->magic);
	data[4] = tbl->count;

	part = data + 5;
	for (i = 0; i < tbl->count; i++, part+= FLASH_PARTITION_V2_HEAD_SIZE) {
		memcpy(part, tbl->tables[i].name, FLASH_PARTITION_NAME_SIZE + 1);
		int32tochar(part +  8, tbl->tables[i].total_size);
		int32tochar(part + 12, tbl->tables[i].used_size);
		int32tochar(part + 16, tbl->tables[i].reserved_size);
		int32tochar(part + 20, tbl->tables[i].start_addr);
		part[24] = tbl->tables[i].file_system_type;
		part[25] = tbl->tables[i].mode    | tbl->tables[i].crc32_enable << 7;
		part[26] = tbl->tables[i].id      | (tbl->tables[i].write_protect << 7);
		part[27] = tbl->tables[i].update_tags & 0x03;

		// 分区的校验码区域
		int32tochar(crc32_table  + i * sizeof(uint32_t), tbl->tables[i].crc32);
		// 分区的版本区域
		int16tochar(part_version + i * sizeof(uint16_t), tbl->tables[i].version);
	}

	for (; i < FLASH_PARTITION_V2_NUM; i++, part += FLASH_PARTITION_V2_HEAD_SIZE) {
		memset(part, 0xFF, FLASH_PARTITION_V2_HEAD_SIZE);
		int32tochar(crc32_table + i * 4, 0xFFFFFFFF);
	}

	data[FLASH_PARTITION_V2_SIZE - 5] = tbl->version;
	data[FLASH_PARTITION_V2_SIZE - 6] = tbl->crc32_enable;
	data[FLASH_PARTITION_V2_SIZE - 7] = tbl->write_protect;

	tbl->crc_verify = crc32(0, data, FLASH_PARTITION_V2_SIZE - 4);
	int32tochar(data + FLASH_PARTITION_V2_SIZE - 4, tbl->crc_verify);

	return 0;
}


static int str2bool(const char *value)
{
	return (strcasecmp(value, "true") == 0) ? 1: 0;
}

#define keycmp(s1, s2) strncasecmp(s1, s2, strlen(s2))

#define ID_NAME    0
#define ID_FILE    1
#define ID_CRC     2
#define ID_FS      3
#define ID_MODE    4
#define ID_UPDATE  5
#define ID_VERSION 6
#define ID_START   7
#define ID_SIZE    8
#define ID_RES_SIZE  9

char *basedir = ".";

int flash_save(flash *entry, const char *filename)
{

	return 0;
#if 0
	int i;
	FILE *fp;

	// 更新 Cramfs 分区数据
	for (i=0; i < entry->count; i++) {
		if (entry->list[i].info->file_system_type == CRAMFS) {
			struct entry *cramfs;
			void *data;
			size_t size = 0;
			struct partition_data *p;

			p = entry->list + i;
			if (p->fs != NULL) {
				cramfs = (struct entry*)p->fs;
				data = cramfs_mmap(cramfs, &size);

				if (data) {
					if (partition_update(p, data, size) < 0) {
						free(data);
						return -1;
					}
					free(data);
				}
			}
		}
	}

	for(i = 0; i < entry->count; i++){
		if(strcasecmp(entry->list[i].info->name, "TABLE") == 0){
			if(entry->list[i].buf == NULL){
				entry->list[i].buf = malloc(table_sect.size);
			}
			if (entry->table_version == 2)
				flash_table_v2_write(&entry->flash_table, (uint8_t*)entry->list[i].buf, table_sect.size);
			else
				flash_table_write(&entry->flash_table, (uint8_t*)entry->list[i].buf, table_sect.size);
		}
	}

	if ((fp = fopen(filename, "wb+")) == NULL) {
		printf("error: %s\n", strerror(errno));

		return -1;
	}

	if (entry->new_style){
		write_flash_zlibmode(entry, fp);
	} else {
		for (i= 0; i <entry->count; i++) {
			if (entry->list[i].buf != NULL) {
				fseek(fp, entry->list[i].info->start_addr, SEEK_SET);
				fwrite(entry->list[i].buf, 1, entry->list[i].info->total_size, fp);
			}
		}
	}
	fclose(fp);

	/* generate table.bin */
	char table_dst_file[255];
	if (table_file_name == NULL)
		snprintf(table_dst_file, 255, "%s/table.bin", basedir);
	else {
		snprintf(table_dst_file, 255, "%s/%s", basedir, table_file_name);
		free(table_file_name);
		table_file_name = NULL;
	}

	if((fp = fopen(table_dst_file, "wb+")) == NULL){
		printf("error: %s\n", strerror(errno));
		return -1;
	}
	for (i= 0; i < entry->count; i++) {
		if(strcasecmp(entry->list[i].info->name, "TABLE") == 0){
			fwrite((uint8_t*)entry->list[i].buf, 1, table_sect.size, fp);
		}
	}
	fclose(fp);
#endif
	return 0;
}



int flash_free(flash *entry)
{
	int i;

	for (i=0; i <entry->count; i++) {
		if (entry->list[i].buf != NULL){
			free(entry->list[i].buf);
		}

		if (entry->list[i].fs != NULL) {
			//if (entry->list[i].info->file_system_type == MINIFS)
				//minifs_umount(entry->list[i].fs);
		}
	}
	free(entry);

	return 0;
}

static int parse_filename(const char *filename, char *partition, char *file)
{
	char *p;

	p = strchr(filename, '/');
	if (p) {
		memcpy(partition, filename, p - filename);
		partition[p - filename] = '\0';
        // +1 : minifs needn't /   but cramfs need /
		strcpy(file, p+1);

		return 0;
	}

	return -1;
}


flash *flash_load_conf(const char *conf)
{
	FILE *fp = NULL;
	char str[1024], *key = NULL, *value = NULL;
	flash *entry = NULL;
	int i;
	unsigned int count = 0;
	unsigned int total_size = 0;
	char *conf_file = strdup(conf);

	if ((fp = fopen(conf, "r")) == NULL) {
		printf("error: can't open file \"%s\"!\n", conf);
		return NULL;
	}

	entry = (flash*)calloc(1, sizeof(flash));
	memset(entry, 0, sizeof(flash));

	entry->table_enable = 0;
	entry->flash_table.write_protect = 0;
	entry->flash_table.crc32_enable = 0;

	basedir = dirname(conf_file);
	while (!feof(fp)) {
		memset(str, 0, 1024);
		fgets(str, 1023, fp);
		trim(str);
		if (strcmp(str, "") == 0 || str[0] == '#')
			continue;

		if (keycmp(str, "flash_size") == 0) {
			key = strtok(str, " \t");
			value = strtok(NULL, " \t");
			trim(value);
			entry->flash_size = str2int(value);
		}
		else if (keycmp(str, "block_size") == 0) {
			key = strtok(str, " \t");
			value = strtok(NULL, " \t");
			trim(value);
			entry->block_size = str2int(value);
		}
		else if (keycmp(str, "table_enable") == 0) {
			key = strtok(str, " \t");
			value = strtok(NULL, " \t");
			trim(value);

			entry->table_enable = str2bool(value);
		}
		else if (keycmp(str, "write_protect") == 0) {
			key = strtok(str, " \t");
			value = strtok(NULL, " \t");
			trim(value);

			entry->flash_table.write_protect = str2bool(value);
		}
		else if (keycmp(str, "crc32") == 0) {
			key = strtok(str, " \t");
			value = strtok(NULL, " \t");
			trim(value);

			entry->flash_table.crc32_enable = str2bool(value);
		}
		else if (keycmp(str, "zlibmode") == 0) {
			key = strtok(str, " \t");
			value = strtok(NULL, " \t");
			trim(value);

			entry->new_style = str2bool(value);
		}
		else if (keycmp(str, "table_version") == 0) {
			key = strtok(str, " \t");
			value = strtok(NULL, " \t");
			trim(value);

			entry->table_version = str2int(value);
			init_table_sect(entry->table_version);
		} else {
			int id = 0;
			struct partition_info *info = &entry->flash_table.tables[count];

			entry->list[count].info = info;
			memset(info, 0, sizeof(struct partition_info));
			id = 0;
			key = strtok(str, " \t");
			while (key) {
				char filename[256];
				void *buf;
				size_t size;
//				printf("key = %s, id = %d\n", key , id);
				switch(id) {
					case ID_NAME:
						strncpy(info->name, key, 7);
						break;
					case ID_FILE:
						if (keycmp(info->name, "table") == 0)
							info->used_size = table_sect.size;

						if (strcasecmp(key, "NULL") != 0) {
							if (keycmp(info->name, "table") == 0) {
								table_file_name = strdup(key);
								if (table_file_name == NULL) {
									printf("dup filename %s error\n", key);
									flash_free(entry);
									return NULL;
								}
							} else {
								snprintf(filename, 255, "%s/%s", basedir, key);
								buf = x_fmmap(filename, &size);
								if (buf) {
									info->used_size = size;
									info->crc32 = crc32(0, buf, size);
									entry->list[count].file_name = strdup(key);
									entry->list[count].buf = malloc(size);
									memcpy(entry->list[count].buf, buf, size);
									x_munmap(buf, size);
								}
							}
						}
						break;
					case ID_CRC:
						info->crc32_enable = str2bool(key);
						break;
					case ID_FS:
						if (strcasecmp(key, "RAW") == 0)
							info->file_system_type = RAW;
						else if (strcasecmp(key, "CRAMFS") == 0)
							info->file_system_type = CRAMFS;
						else if (strcasecmp(key, "ROMFS") == 0)
							info->file_system_type = ROMFS;
						else if (strcasecmp(key, "JFFS2") == 0)
							info->file_system_type = JFFS2;
						else if (strcasecmp(key, "YAFFS2") == 0 || strcasecmp(key, "YAFFS") == 0)
							info->file_system_type = YAFFS2;
						else if (strcasecmp(key, "MINIFS") == 0)
							info->file_system_type = MINIFS;
						else if (strcasecmp(key, "RAMFS") == 0)
							info->file_system_type = RAMFS;
						else if (strcasecmp(key, "HIDE") == 0)
							info->file_system_type = HIDE;
						break;
					case ID_MODE:
						if (strcasecmp(key, "ro") == 0)
							info->mode = 0;
						else if (strcasecmp(key, "rw") == 0)
							info->mode = 1;
						break;
					case ID_UPDATE:
						info->update_tags = str2int(key);
						break;
					case ID_VERSION:
						info->version = str2int(key);
						break;
					case ID_START:
						if (strcasecmp(key, "auto") == 0)
							info->start_addr = -1;
						else
							info->start_addr = str2int(key);
						break;
					case ID_SIZE:
						if (strcasecmp(key, "auto") == 0) {
							info->total_size = -1;
							info->partition_size = -1;
						} else {
							info->total_size = str2int(key);
							info->partition_size = str2int(key);
						}
						break;
					case ID_RES_SIZE:
						if (table_sect.version == 2) {
							if (strcasecmp(key, "auto") == 0)
								info->reserved_size = -1;
							else
								info->reserved_size = str2int(key);
						} else {
							info->reserved_size = 0;
						}
						break;
				}

				id++;
				key = strtok(NULL, " \t");
				if (key)
					trim(key);
			}

			count++;

			if (count > table_sect.part_num) {
				printf("No more than %d the number of partitions.\n", table_sect.part_num);
				exit(1);
			}
		}
	}

	printf("\n");
	entry->count = count;
	fclose(fp);

	if (entry->list[0].info->start_addr == -1)
		entry->list[0].info->start_addr = 0;
	if (entry->list[0].info->total_size == -1)
		entry->list[0].info->total_size = entry->list[0].info->used_size;
	if (entry->list[0].info->reserved_size == -1)
		entry->list[0].info->reserved_size = 0;

	for (i=1; i < entry->count; i++) {
		if (entry->list[i].info->total_size == -1)
			entry->list[i].info->total_size = entry->list[i].info->used_size;
		if (entry->list[i].info->reserved_size == -1)
			entry->list[i].info->reserved_size = 0;
		if (entry->list[i].info->start_addr == -1) {
			entry->list[i].info->start_addr = entry->list[i-1].info->total_size + entry->list[i-1].info->start_addr;
			entry->list[i].info->start_addr = ((entry->list[i].info->start_addr + 0x10000 - 1) / 0x10000) * 0x10000;
		}
		if (strcasecmp(entry->list[i].info->name, "TABLE") == 0) {
			entry->table_enable = 1;
			entry->table_addr = entry->list[i].info->start_addr;
		}
		if (entry->list[i].info->file_system_type == MINIFS) {
			if (entry->list[i].info->start_addr % (64 * 1024) != 0) {
				printf("minifs start address must be aligned 64K\n");
				//free(conf_file);
				flash_free(entry);

				return NULL;
			}
		}
	}

	for (i=0; i < entry->count; i++) {
		if (i == entry->count - 1)
			entry->list[i].info->total_size = entry->flash_size - entry->list[i].info->start_addr;
		else
			entry->list[i].info->total_size = entry->list[i + 1].info->start_addr - entry->list[i].info->start_addr;
		total_size += entry->list[i].info->total_size;
		if (total_size > entry->flash_size) {
			printf("Total partition size is %d, out flash size %d, please check.\n", total_size, entry->flash_size);
			exit(1);
		}
		if (entry->list[i].info->reserved_size > entry->list[i].info->total_size) {
			printf("error: partition %s (RESERVED size = 0x%x) > (TOTAL size = 0x%x)\n",
					entry->list[i].info->name,
					entry->list[i].info->reserved_size,
					entry->list[i].info->total_size);
			printf("please check the config of %s\n\n", conf);
			exit(1);
		}
		entry->list[i].info->partition_size = entry->list[i].info->total_size - entry->list[i].info->reserved_size;
		entry->list[i].crc32_passed = 1;
	}

	for (i=0; i < entry->count; i++) {
		/* if used_size > partition_size, printf error info */
		if(entry->list[i].info->used_size > entry->list[i].info->partition_size){
			printf("error: partition %s (FILE size = 0x%x) > (PART size = 0x%x)\n",
					entry->list[i].info->name,
					entry->list[i].info->used_size,
					entry->list[i].info->partition_size);
			printf("please check the config of %s\n\n", conf);
			exit(1);
		}
	}

	entry->flash_table.magic = table_sect.magic;
	entry->flash_table.count = entry->count;

	flash_sort(entry);

	if(entry->new_style){
		for (i=0; i < entry->count; i++) {
			entry->list[i].info->id = i;
			if(entry->list[i].buf){
				entry->list[i].buf = realloc(entry->list[i].buf, entry->list[i].info->total_size);
				memset((char*)entry->list[i].buf + entry->list[i].info->used_size, 0xFF, \
						entry->list[i].info->total_size - entry->list[i].info->used_size);
			}
		}
	}else{
		for (i=0; i < entry->count; i++) {
			entry->list[i].info->id = i;
			entry->list[i].buf = realloc(entry->list[i].buf, entry->list[i].info->total_size);
			memset((char*)entry->list[i].buf + entry->list[i].info->used_size, 0xFF, \
					entry->list[i].info->total_size - entry->list[i].info->used_size);
		}
	}

	entry->flash_table.version = PARTITION_VERSION;

	/*
	 * 保存 fs 的原因是有文件系统的分区的需要，主要是 cramfs，当前工程中 cramfs 文件系统的数据结构同标准工具 mkfs.cramfs
	 * 数据结构相比做了一定程度改动，这里的修改可统一从制作 bin 文件到解析 bin 文件中对 cramfs 文件系统分区的处理
	 *
	 */
	for (i = 0; i < entry->count; i++)
		if (entry->list[i].info->file_system_type == CRAMFS) {
			if (entry->list[i].buf != NULL)
				entry->list[i].fs = NULL;
		}

	//update partitions crc32
	for(i = 0; i < entry->count; i++){
		unsigned int new_crc32;
		if(strcasecmp(entry->list[i].info->name, "TABLE") != 0){
			if(entry->list[i].info->crc32_enable){
				new_crc32 = crc32(0, entry->list[i].buf, entry->list[i].info->used_size);
				if(new_crc32 != entry->list[i].info->crc32){
					printf("update partition %s crc32, old_crc32=0x%x, new_crc32=0x%x\n",entry->list[i].info->name, entry->list[i].info->crc32,new_crc32);
					entry->list[i].info->crc32 = new_crc32;
				}
			}
		}
	}

#ifndef WIN32
	flash_printf(entry);
#endif

	//free(conf_file);
	return entry;
}

static int flash_save_conf(flash *entry, const char *filename)
{
	FILE *fp;
	int i;

	if ((fp = fopen(filename, "w")) == NULL)
		return -1;

	fprintf(fp, "flash_size\t= 0x%x\n",    entry->flash_size);
	fprintf(fp, "block_size\t= 0x%x\n",    entry->block_size);
	fprintf(fp, "write_protect\t= %s\n",   entry->flash_table.write_protect ? "TRUE" : "FALSE");
	fprintf(fp, "write_protect\t= %s\n\n", entry->flash_table.crc32_enable ? "TRUE" : "FALSE");

	printf("%-3s%-8s%-3s%-8s%-10s%-9s%10s%11s%9s%5s\n",
			"ID", "NAME", "", "FS", "CRC32", "START", "SIZE", "USED", "USE%", "PASS");
	for (i=0; i < entry->count; i++) {
		fprintf(fp, "%-10sBIN.%-16s%-8s%-8s%-3s0x%08x 0x%08x\n",
				entry->list[i].info->name,                            // partition name
				entry->list[i].info->name,                            // local file name
				entry->list[i].info->crc32_enable ? "TRUE" : "FALSE", // CRC
				get_fstype(entry->list[i].info->file_system_type),    // file system
				entry->list[i].info->mode == 0 ? "RO" : "RW",         // read/write mode
				entry->list[i].info->start_addr,                      // flash address
				entry->list[i].info->total_size                       // partition size
		       );
	}
	fclose(fp);

	return 0;
}


#if 0
[Main]
SegNumber=9
Seg0=All
Seg1=Bootload
Seg2=Main Code
Seg3=Logo
Seg4=Common Data
Seg5=Default UsrDB
Seg6=UsrDB
Seg7=SysDB
Seg8=Key
[All]
StartAddr=0x00000000
SegSize   =0x00200000
[Bootload]
StartAddr=0x00000000
SegSize   =0x00100000
[Main Code]
StartAddr=0x00010000
SegSize   =0x00120000
[Logo]
StartAddr=0x00130000
SegSize   =0x00008000
[Common Data]
StartAddr=0x00138000
SegSize   =0x00000800
[Default UsrDB]
StartAddr=0x00138800
SegSize   =0x00007800
[UsrDB]
StartAddr=0x00140000
SegSize   =0x00080000
[SysDB]
StartAddr=0x001C0000
SegSize   =0x00020000
[Key]
StartAddr=0x001E0000
SegSize   =0x00020000
#endif
void save_main_head_style(FILE *fp,flash *entry)
{
	unsigned char i;
	//head
	
	fprintf(fp, "[%s]\r\r\n","Main");
	fprintf(fp, "SegNumber=%d\r\r\n",entry->count+1);
	
	fprintf(fp, "Seg%d=%s\r\r\n",0,"ALL");
	for (i=0; i < entry->count; i++){
		fprintf(fp, "Seg%d=%s\r\r\n",i+1,entry->list[i].info->name);
	}

	// 1
	fprintf(fp, "[%s]\r\r\n","ALL");
	fprintf(fp, "StartAddr=0x%08x\r\r\n",0);
	fprintf(fp, "SegSize=0x%08x\r\r\n",entry->flash_size);
	
	//end
	for (i=0; i < entry->count; i++){
		
		fprintf(fp, "[%s]\r\r\n",entry->list[i].info->name);
		fprintf(fp, "StartAddr=0x%08x\r\r\n",entry->list[i].info->start_addr);
		fprintf(fp, "SegSize=0x%08x\r\r\n",entry->list[i].info->total_size);
	}
	
}
	
int flash_pctool_conf_ini_save(flash *entry, const char *filename)
{
	FILE *fp;
	int i;

	if((fp = fopen(filename, "w")) == NULL)
		return -1;
	save_main_head_style(fp,entry);
	fclose(fp);

	return 0;
}

int flash_load_free(flash *entry)
{
	int i;
	for (i=0; i <entry->count; i++) {
		if (entry->list[i].buf != NULL){
			free(entry->list[i].buf);
		}

		if (entry->list[i].fs != NULL) {
			//if (entry->list[i].info->file_system_type == MINIFS)
				//minifs_umount(entry->list[i].fs);
		}
	}
	free(entry);
	
	return 0;
}
