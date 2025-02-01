#ifndef _M_WIIINFO_H_
#define _M_WIIINFO_H_

#include <gctypes.h>

char* get_wii_model();
s32 check_connection();
char* check_connection_str();
char* get_serial();
char* check_setting();
char* get_area();
char* is_priiloader_installed();
char* is_bootmii_installed();

#endif
