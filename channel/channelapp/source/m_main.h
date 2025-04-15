#ifndef _M_MAIN_H_
#define _M_MAIN_H_

#include <gctypes.h>

#include "view.h"

enum menuindex {
	MENU_HOME = 0,
	MENU_SETTINGS,
	MENU_EXIT
};

extern enum menuindex menu_index;
extern enum menuindex parent_menu;

view * m_main_init (void);
void m_main_deinit (void);
void m_main_gen_view (void);
void m_main_theme_reinit (void);
void m_main_fade (bool fade_in);

void m_main_update (void);

#endif

