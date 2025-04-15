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

extern bool view_fakelogo;

// TODO: move to diff file!

struct system_menu_version {
	u16 number;
	char* string;
};

struct system_menu_version version_table[] = {
	// no particular order here
	{33, "1.0U"},
	{64, "1.0J"},
	{66, "1.0E"},
	{97, "2.0U"},
	{128, "2.0J"},
	{130, "2.0E"},
	{162, "2.1E"},
	{192, "2.2J"},
	{194, "2.2E"},
	{193, "2.2U"},
	{224, "3.0J"},
	{226, "3.0E"},
	{256, "3.1J"},
	{258, "3.1E"},
	{288, "3.2J"},
	{290, "3.2E"},
	{352, "3.3J"},
	{354, "3.3E"},
	{384, "3.4J"},
	{386, "3.4E"},
	{390, "3.5K"},
	{416, "4.0J"},
	{418, "4.0E"},
	{448, "4.1J"},
	{450, "4.1E"},
	{454, "4.1K"},
	{480, "4.2J"},
	{482, "4.2E"},
	{486, "4.2K"},
	{518, "4.3K"},
	{514, "4.3E"},
	{225, "3.0U"},
	{257, "3.1U"},
	{289, "3.2U"},
	{353, "3.3U"},
	{385, "3.4U"},
	{417, "4.0U"},
	{449, "4.1U"},
	{481, "4.2U"},
	{512, "4.3J"},
	{513, "4.3U"},
	{544, "4.3J"},
	{545, "4.3U"},
	{546, "4.3E"},
	{608, "4.3J"},
	{609, "4.3U"},
	{610, "4.3E"},
	{4609, "4.3U"},
	{4610, "4.3E"}
};

char* get_system_menu_version_string(u16 number) {
	u8 table_size = sizeof(version_table) / sizeof(version_table[0]);

	for (u8 i = 0; i < table_size; i++) {
		if (version_table[i].number == number) {
			return version_table[i].string;
		}
	}

	return "Unknown"; // no system menu version installed, or weird patched one?
}

u16 get_tmd_version(u64 title) {
	STACK_ALIGN(u8, tmdbuf, 1024, 32);
	u32 tmd_view_size = 0;
	s32 res;

	res = ES_GetTMDViewSize(title, &tmd_view_size);
	if (res < 0)
		return 0;

	if (tmd_view_size > 1024)
		return 0;

	res = ES_GetTMDView(title, tmdbuf, tmd_view_size);
	if (res < 0)
		return 0;

	return (tmdbuf[88] << 8) | tmdbuf[89];
}

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
