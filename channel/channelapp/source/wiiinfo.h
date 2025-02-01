#ifndef _M_WIIINFO_H_
#define _M_WIIINFO_H_

#define IS_VWII ((*(vu32*)(0xCD8005A0) >> 16 ) == 0xCAFE)

char* get_wii_model();

#endif
