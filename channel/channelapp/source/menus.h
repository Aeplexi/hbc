#ifndef _MENUS_H_
#define _MENUS_H_

#include "../config.h"
#include "loader.h"
#include "view.h"

void menu_home(view *m_view, int x, int y, int yadd);
void menu_exit(view *m_view, int x, int y, int yadd, bool bootmii_ios, bool priiloader);
void menu_settings(view *m_view, int x, int y, int yadd);
void submenu_menutest(view *v);

#endif
