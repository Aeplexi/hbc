#include <stdio.h>
#include <malloc.h>
#include <ogc/machine/processor.h>
#include <ogcsys.h>
#include <network.h>

#include "../config.h"
#include "theme.h"
#include "view.h"
#include "loader.h"
#include "i18n.h"
#include "m_main.h"
#include "wiiinfo.h"
#include "nand.h"
#include "fileops.h"
#include "title.h"

#define TITLE_UPPER(x) (u32)(x >> 32)
#define TITLE_LOWER(x) (u32)(x & 0xFFFFFFFF)

static view *v_m_main;

static const char *text_no_ip;
static const char *text_has_ip;

static bool bootmii_ios = false;
static bool priiloader = false;

enum menuindex menu_index = MENU_HOME;
enum menuindex parent_menu = MENU_HOME;

static bool inited_widgets = false;

view * m_main_init (void) {
	bootmii_ios = bootmii_ios_is_installed(TITLEID_BOOTMII);
	priiloader = priiloader_is_installed();

	v_m_main = view_new (9, NULL, 0, 0, 0, 0);

	m_main_theme_reinit();
	m_main_update();

	view_set_focus(v_m_main, 0);

	return v_m_main;
}

void m_main_deinit(void) {
	view_free (v_m_main);
	inited_widgets = false;
	v_m_main = NULL;
}

void m_main_theme_reinit(void) {
	u16 x, y = 0, yadd = 0, button_count = 0;
	int i;
	char buffer[50];

	text_no_ip = _("Network not initialized");
	text_has_ip = _("Your Wii's IP is %u.%u.%u.%u");

	if (inited_widgets)
		for (i = 0; i < v_m_main->widget_count; ++i)
			widget_free(&v_m_main->widgets[i]);

	x = (view_width - theme_gfx[THEME_BUTTON]->w) / 2;
	y = 80 + (theme_gfx[THEME_BUTTON]->h / 2);

	switch (menu_index) {

		case MENU_HOME:
			parent_menu = MENU_HOME;

			yadd = theme_gfx[THEME_BUTTON]->h*2/3;

			widget_button (&v_m_main->widgets[0], x, y, 0, BTN_NORMAL,
						   _("Settings"));
			y += theme_gfx[THEME_BUTTON]->h + yadd;

			widget_button (&v_m_main->widgets[1], x, y, 0, BTN_NORMAL,
						   _("System Info"));
			y += theme_gfx[THEME_BUTTON]->h + yadd;

			widget_button (&v_m_main->widgets[2], x, y, 0, BTN_NORMAL,
						   _("About"));
			y += theme_gfx[THEME_BUTTON]->h + yadd;

			widget_button (&v_m_main->widgets[3], x, y, 0, BTN_NORMAL,
						   _("Exit"));
			break;
		case MENU_EXIT:
			parent_menu = MENU_HOME;

			button_count = 3 + bootmii_ios + priiloader;

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
				widget_button (&v_m_main->widgets[0], x, y, 0, BTN_NORMAL,
							   _("Launch BootMii IOS"));
				y += theme_gfx[THEME_BUTTON]->h + yadd;
			}

			if (priiloader) {
				widget_button (&v_m_main->widgets[1], x, y, 0, BTN_NORMAL,
							   _("Launch Priiloader"));
				y += theme_gfx[THEME_BUTTON]->h + yadd;
			}

			widget_button (&v_m_main->widgets[2], x, y, 0, BTN_NORMAL,
						   _("Exit to System Menu"));
			y += theme_gfx[THEME_BUTTON]->h + yadd;

			if (IS_VWII) {
				widget_button (&v_m_main->widgets[3], x, y, 0, BTN_NORMAL,
							   _("Reboot to Wii U Menu"));
			} else {
				widget_button (&v_m_main->widgets[3], x, y, 0, BTN_NORMAL,
							   _("Reboot"));
			}
			y += theme_gfx[THEME_BUTTON]->h + yadd;

			widget_button (&v_m_main->widgets[4], x, y, 0, BTN_NORMAL,
						   _("Shutdown"));
			break;
	}

	// HBC and IOS version

	widget_label (&v_m_main->widgets[6], view_width / 3 * 2 - 48,
				  32, 0, CHANNEL_VERSION_STR,
				  view_width / 3 - 0, FA_RIGHT, FA_ASCENDER, FONT_LABEL);

	sprintf(buffer, "IOS%d v%d.%d", IOS_GetVersion(), IOS_GetRevisionMajor(),
			IOS_GetRevisionMinor());

	widget_label (&v_m_main->widgets[7], view_width / 3 * 2 - 48,
				  32 + font_get_y_spacing(FONT_LABEL), 0, buffer,
			   view_width / 3 - 0, FA_RIGHT, FA_ASCENDER, FONT_LABEL);

	inited_widgets = true;
}

void m_main_update (void) {
	u32 ip;
	char buffer[64];

	if (loader_tcp_initialized ()) {
		ip = net_gethostip ();
		sprintf (buffer, text_has_ip, (ip >> 24) & 0xff, (ip >> 16) & 0xff,
				 (ip >> 8) & 0xff, ip & 0xff);
		widget_label (&v_m_main->widgets[8], 48, 32, 0, buffer,
					  view_width / 3 * 2 - 32, FA_LEFT, FA_ASCENDER, FONT_LABEL);
	} else {
		widget_label (&v_m_main->widgets[8], 48, 32, 0, text_no_ip,
					  view_width / 3 * 2 - 32, FA_LEFT, FA_ASCENDER, FONT_LABEL);
	}

}
