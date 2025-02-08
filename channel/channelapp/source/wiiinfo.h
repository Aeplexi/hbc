#ifndef _M_WIIINFO_H_
#define _M_WIIINFO_H_

#include <gctypes.h>

#define IS_VWII ((*(vu32*)(0xCD8005A0) >> 16 ) == 0xCAFE)

char* get_wii_model();
s32 check_connection();
char* get_serial();
int check_setting();
char* get_hardware_region();
char* get_model_number();
bool priiloader_is_installed();
bool bootmii_is_installed(int copy);
bool bootmii_ios_is_installed();

#endif
