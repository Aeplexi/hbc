#include <stdio.h>
#include <malloc.h>
#include <ogc/machine/processor.h>
#include <ogcsys.h>
#include <network.h>
#include <math.h>

#include "../config.h"
#include "dialogs.h"
#include "theme.h"
#include "loader.h"
#include "i18n.h"
#include "m_main.h"
#include "wiiinfo.h"
#include "nand.h"
#include "fileops.h"
#include "menus.h"

#define TITLE_UPPER(x) (u32)(x >> 32)
#define TITLE_LOWER(x) (u32)(x & 0xFFFFFFFF)

#define TRANS_STEPS 15
#define MAX_ROWS 5

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
	text_no_ip = _("Network not initialized");
	text_has_ip = _("Your Wii's IP is %u.%u.%u.%u");
}

void m_main_fade(bool fade_in) {
	s8 focus;

	u32 i, j;

	u8 o;
	s16 x;
	float xm;
	float vala = 0;
	float val[MAX_ROWS];
	float stepa = M_TWOPI / TRANS_STEPS;
	float step = M_PI / (TRANS_STEPS - 6);
	// s16 s;
	float f;

	memset(val, 0, sizeof(float) * MAX_ROWS);

	o = MAX_ROWS;

	focus = o;

	// xal = v_m_main->widgets[0].coords.x;
	// xar = v_m_main->widgets[1].coords.x;

	if (fade_in) {
		x = (3 * view_width - theme_gfx[THEME_BUTTON]->w) / 2;
		xm = -view_width / 2;
		// x2 = view_width + x2;
	} else {
		x = (view_width - theme_gfx[THEME_BUTTON]->w) / 2;
		xm = view_width / 2;
		// x2 = -view_width + x2;
	}

	for (i = 0; i < TRANS_STEPS; ++i) {
		vala += stepa;
		// s = roundf (156.0 * (cosf (vala) - 1));

		// adjust L/R button positions
		// v_m_main->widgets[0].coords.x = xal + s;
		// v_m_main->widgets[1].coords.x = xar - s;

		for (j = 0; j < MAX_ROWS; ++j) {
			if ((i > j) &&
				(i < TRANS_STEPS - MAX_ROWS + j))
				val[j] += step;
			if (fade_in) {
				f = roundf (xm * (cosf (val[j]) - 1));
			} else {
				f = -roundf (xm * (cosf (val[j]) - 1));
			}

			v_m_main->widgets[j].coords.x = x - f;
			v_m_main->widgets[j + 1].coords.x = x - f;
		}

		view_plot (v_m_main, 0, NULL, NULL, NULL);

		if (i == TRANS_STEPS / 2) {
		// 	widget_set_flag (&v_m_main->widgets[0], WF_VISIBLE, true);
		// 	widget_set_flag (&v_m_main->widgets[1], WF_VISIBLE, true);
		// 	widget_set_flag (&v_m_main->widgets[0], WF_VISIBLE, less);
		// 	widget_set_flag (&v_m_main->widgets[1], WF_VISIBLE, more);
			view_set_focus (v_m_main, focus);
		}
	}
}

void m_main_gen_view(void) {
	u16 x, y = 0, yadd = 0;
	int i;
	char buffer[50];

	view_fakelogo = true;
	if (inited_widgets)
		for (i = 0; i < v_m_main->widget_count; ++i)
			widget_free(&v_m_main->widgets[i]);

	x = (view_width - theme_gfx[THEME_BUTTON]->w) / 2;
	y = 80 + (theme_gfx[THEME_BUTTON]->h / 2);

	switch (menu_index) {
		case MENU_HOME: // Home
			parent_menu = MENU_HOME;
			menu_home(v_m_main, x, y, yadd);
			break;
		case MENU_EXIT: // Exit
			parent_menu = MENU_HOME;
			menu_exit(v_m_main, x, y, yadd, bootmii_ios, priiloader);
			break;
		case MENU_SETTINGS:	// Settings
			parent_menu = MENU_HOME;
			menu_settings(v_m_main, x, y, yadd);
	}

	// HBC and IOS version

	widget_label (&v_m_main->widgets[6], view_width / 3 * 2 - 48,
				  32, 0, CHANNEL_VERSION_STR,
				  view_width / 3, FA_RIGHT, FA_ASCENDER, FONT_LABEL);

	sprintf(buffer, "IOS%d v%d.%d", IOS_GetVersion(), IOS_GetRevisionMajor(),
			IOS_GetRevisionMinor());

	widget_label (&v_m_main->widgets[7], view_width / 3 * 2 - 48,
				  32 + font_get_y_spacing(FONT_LABEL), 0, buffer,
			   view_width / 3, FA_RIGHT, FA_ASCENDER, FONT_LABEL);

	inited_widgets = true;

	m_main_update();
	m_main_fade(true);
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
