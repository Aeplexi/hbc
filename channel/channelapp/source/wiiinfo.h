#ifndef _M_WIIINFO_H_
#define _M_WIIINFO_H_

#include <gctypes.h>

#define IS_VWII ((*(vu32*)(0xCD8005A0) >> 16 ) == 0xCAFE)

typedef enum {
	MODEL_WII = 0,
	MODEL_FAMILY,
	MODEL_MINI,
	MODEL_WIIU,
	MODEL_DOLPHIN,
	MODEL_NDEV
} wii_models;

int get_wii_model(void);
s32 check_connection(void);
void get_serial(char* code);
void get_hardware_region(char* region);
void get_model_number(char* model_setting);
bool priiloader_is_installed(void);
bool bootmii_ios_is_installed(u64 title_id);
void bootmii_ios_version(u64 title_id, char* outbuf);
#endif
