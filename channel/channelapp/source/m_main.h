#ifndef _M_MAIN_H_
#define _M_MAIN_H_

#include <gctypes.h>

#include "view.h"

enum menuindex {
	MENU_HOME = 0,
	MENU_OPTIONS_1,
	MENU_OPTIONS_2,
	MENU_EXIT,
};

extern enum menuindex menu_index;
extern enum menuindex parent_menu;

view * m_main_init (void);
void m_main_deinit (void);
void m_main_theme_reinit (void);

void m_main_update (void);

// I very much think these should not be here and they should be in a different file, wiiinfo.c or nand.c maybe? - aep
u16 get_tmd_version(u64 title);
char* get_system_menu_version_string(u16 number);
bool priiloader_is_installed();

#endif

