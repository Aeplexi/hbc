#ifndef _M_WIIINFO_H_
#define _M_WIIINFO_H_

#include <gctypes.h>

char* get_wii_model();
s32 check_connection();
char* get_serial();
char* check_setting();
char* get_area();
char* is_priiloader_installed();

#endif
