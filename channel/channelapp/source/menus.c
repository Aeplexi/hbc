#include "menus.h"

#include "i18n.h"
#include "theme.h"
#include "widgets.h"
#include "wiiinfo.h"

void menu_home(view *m_view, int x, int y, int yadd) {
	yadd = theme_gfx[THEME_BUTTON]->h*2/3;

	widget_button (&m_view->widgets[0], x, y, 0, BTN_NORMAL,
				   _("Settings"));
	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[1], x, y, 0, BTN_NORMAL,
				   _("System Info"));
	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[2], x, y, 0, BTN_NORMAL,
				   _("About"));
	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[3], x, y, 0, BTN_NORMAL,
				   _("Exit"));
}

void menu_exit(view *m_view, int x, int y, int yadd, bool bootmii_ios, bool priiloader) {
	int button_count = 3 + bootmii_ios + priiloader;

	if (button_count == 3) {
		button_count = 5;
		yadd = theme_gfx[THEME_BUTTON]->h/4;
		y += theme_gfx[THEME_BUTTON]->h + yadd;
	} else if (button_count == 2) {
		button_count = 4;
		yadd = theme_gfx[THEME_BUTTON]->h*2/3;
		y += theme_gfx[THEME_BUTTON]->h + yadd;
	} else {
		yadd = ((theme_gfx[THEME_BUTTON]->h*(6-button_count))/(button_count-1));
	}
	// if you need to have 1 button in a menu, something's seriously wrong with either you or your way of thinking

	if (bootmii_ios) {
		widget_button (&m_view->widgets[0], x, y, 0, BTN_NORMAL,
					   _("Launch BootMii IOS"));
		y += theme_gfx[THEME_BUTTON]->h + yadd;
	}

	if (priiloader) {
		widget_button (&m_view->widgets[1], x, y, 0, BTN_NORMAL,
					   _("Launch Priiloader"));
		y += theme_gfx[THEME_BUTTON]->h + yadd;
	}

	widget_button (&m_view->widgets[2], x, y, 0, BTN_NORMAL,
				   _("Exit to System Menu"));
	y += theme_gfx[THEME_BUTTON]->h + yadd;

	if (IS_VWII) {
		widget_button (&m_view->widgets[3], x, y, 0, BTN_NORMAL,
					   _("Reboot to Wii U Menu"));
	} else {
		widget_button (&m_view->widgets[3], x, y, 0, BTN_NORMAL,
					   _("Reboot"));
	}
	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[4], x, y, 0, BTN_NORMAL,
				   _("Shutdown"));
}

void menu_settings(view *m_view, int x, int y, int yadd) {
	yadd = theme_gfx[THEME_BUTTON]->h*2/3;

	widget_button (&m_view->widgets[0], x, y, 0, BTN_NORMAL,
				   _("Reset Theme"));

	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[1], x, y, 0, BTN_NORMAL,
				   _("Sound"));

	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[2], x, y, 0, BTN_NORMAL,
				   _("Miscellaneous"));

	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[3], x, y, 0, BTN_NORMAL,
				   _("Menu Test"));
}
