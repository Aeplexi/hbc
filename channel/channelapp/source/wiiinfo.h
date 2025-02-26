#ifndef _M_WIIINFO_H_
#define _M_WIIINFO_H_

#include <gctypes.h>

#define IS_VWII ((*(vu32*)(0xCD8005A0) >> 16 ) == 0xCAFE)

char* get_wii_model(void);
s32 check_connection(void);
void get_serial(char* code);
int check_setting(void);
void get_hardware_region(char* region);
char* get_area(void);
void get_model_number(char* model_setting);
bool priiloader_is_installed(void);
bool bootmii_ios_is_installed(u64 title_id);
void bootmii_ios_version(u64 title_id, char* outbuf);
#endif
