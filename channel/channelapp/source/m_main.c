#include <stdio.h>
#include <malloc.h>
#include <ogc/machine/processor.h>
#include <ogc/isfs.h>
#include <ogcsys.h>
#include <network.h>

#include "../config.h"
#include "theme.h"
#include "view.h"
#include "loader.h"
#include "i18n.h"
#include "panic.h"
#include "m_main.h"
#include "title.h"
#include "wiiinfo.h"

#define TITLE_UPPER(x) (u32)(x >> 32)
#define TITLE_LOWER(x) (u32)(x & 0xFFFFFFFF)

static view *v_m_main;

static const char *text_no_ip;
static const char *text_has_ip;
static const char *text_ip;

static bool bootmii_ios = false;
static bool vwii = false;
static bool priiloader = false;

enum menuindex menu_index = EXIT;
enum menuindex parent_menu = HOME;

static bool bootmii_is_installed(u64 title_id) {
	u32 tmd_view_size;
	u8 *tmdbuf;
	bool ret;

	if (ES_GetTMDViewSize(title_id, &tmd_view_size) < 0)
		return false;

	if (tmd_view_size < 90)
		return false;

	if (tmd_view_size > 1024)
		return false;

	tmdbuf = pmemalign(32, 1024);

	if (ES_GetTMDView(title_id, tmdbuf, tmd_view_size) < 0) {
		free(tmdbuf);
		return false;
	}

	if (tmdbuf[50] == 'B' && tmdbuf[51] == 'M')
		ret = true;
	else
		ret = false;

	free(tmdbuf);

	return ret;
}

static bool priiloader_is_installed(u64 title_id) {

	u32 tmd_size;
	static u8 tmd_buf[MAX_SIGNED_TMD_SIZE] ATTRIBUTE_ALIGN(32);
	static signed_blob *mTMD;
	static tmd *rTMD;
	char TMD_Path[ISFS_MAXPATH];
	char TMD_Path2[ISFS_MAXPATH];

	//IOS Shit Done
	//read TMD so we can get the main booting dol
	s32 fd = 0;
	u32 id = 0;
	bool ret = false;
	memset(TMD_Path,0,64);
	memset(TMD_Path2,0,64);
	sprintf(TMD_Path, "/title/%08x/%08x/content/title.tmd",TITLE_UPPER(title_id),TITLE_LOWER(title_id));
	sprintf(TMD_Path2, "/title/%08x/%08x/content/title_or.tmd",TITLE_UPPER(title_id),TITLE_LOWER(title_id));
	fd = ES_GetStoredTMDSize(title_id,&tmd_size);
	if (fd < 0)
	{
		gprintf("Unable to get stored tmd size");
	}
	mTMD = (signed_blob *)tmd_buf;
	fd = ES_GetStoredTMD(title_id,mTMD,tmd_size);
	if (fd < 0)
	{
		gprintf("Unable to get stored tmd");
	}
	rTMD = (tmd*)SIGNATURE_PAYLOAD(mTMD);
	for(u8 i=0; i < rTMD->num_contents; ++i)
	{
		if (rTMD->contents[i].index == rTMD->boot_index)
		{
			id = rTMD->contents[i].cid;
			break;
		}
	}
	if (id == 0)
	{
		gprintf("Unable to retrieve title booting app");
	}

	ret = id;
	return ret;
}

// todo: move to diff file!

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
	{513, "4.3U"}, // technically 4.3 is vWii 1.0.0 but it's gonna be too rare for somebody to be on that anyways probably
	{544, "4.3J (Wii U 4.0.0J)"},
	{545, "4.3U (Wii U 4.0.0U)"},
	{546, "4.3E (Wii U 4.0.0E)"},
	{608, "4.3J (Wii U 5.2.0J)"},
	{609, "4.3U (Wii U 5.2.0U)"},
	{610, "4.3E (Wii U 5.2.0E)"},
	{4609, "4.3U (Wii mini)"},
	{4610, "4.3E (Wii mini)"}
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

static u16 get_tmd_version(u64 title) {
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
	bootmii_ios = bootmii_is_installed(TITLEID_BOOTMII);
	priiloader = priiloader_is_installed(TITLEID_SYSMENU);
	vwii = is_vwii();

	v_m_main = view_new (10, NULL, 0, 0, 0, 0);

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
	u16 x, y, yadd;
	int i;
	char bufferwiinote[55];
	char buffer[50];

	text_no_ip = _("Network not initialized");
	text_has_ip = _("Your Wii's IP is:");
	text_ip = "%u.%u.%u.%u";

	if (inited_widgets)
		for (i = 0; i < v_m_main->widget_count; ++i)
			widget_free(&v_m_main->widgets[i]);

	x = (view_width - theme_gfx[THEME_BUTTON]->w) / 2;
	switch (menu_index) {
		// it's only theoretically possible for one of these to be true: the user is running on a vWii or they have BootMii IOS installed.

		case HOME:
			if (bootmii_ios || vwii)
				if (priiloader)
					yadd = 8;
			else
				yadd = 16;
			else if (priiloader)
				yadd = 16;
			else
				yadd = 32;
			parent_menu = HOME;
			y = 80;
			widget_button (&v_m_main->widgets[0], x, y, 0, BTN_NORMAL,
						   _("Options"));
			y += theme_gfx[THEME_BUTTON]->h + yadd;
			widget_button (&v_m_main->widgets[1], x, y, 0, BTN_NORMAL,
						   _("About"));
			y += theme_gfx[THEME_BUTTON]->h + yadd;

			if (bootmii_ios) {
				widget_button (&v_m_main->widgets[2], x, y, 0, BTN_NORMAL,
							_("Launch BootMii IOS"));
				y += theme_gfx[THEME_BUTTON]->h + yadd;
			}

			if (priiloader) {
				widget_button (&v_m_main->widgets[3], x, y, 0, BTN_NORMAL,
							_("Launch Priiloader"));
				y += theme_gfx[THEME_BUTTON]->h + yadd;
			}

			widget_button (&v_m_main->widgets[4], x, y, 0, BTN_NORMAL,
						   _("Exit"));
			break;
		case EXIT:
			parent_menu = HOME;
			if (vwii) {
				y = 80 + theme_gfx[THEME_BUTTON]->h + yadd;
				yadd = 32;
			} else {
				y = 80 + (theme_gfx[THEME_BUTTON]->h + yadd) / 2;
				yadd = 16;
			}
			widget_button (&v_m_main->widgets[0], x, y, 0, BTN_NORMAL,
						   _("Exit to System Menu"));
			y += theme_gfx[THEME_BUTTON]->h + yadd;

			if (vwii) {
				widget_button (&v_m_main->widgets[1], x, y, 0, BTN_NORMAL,
							   _("Return to Wii U Menu"));
				y += theme_gfx[THEME_BUTTON]->h + yadd;
			} else {
				widget_button (&v_m_main->widgets[1], x, y, 0, BTN_NORMAL,
							   _("Reboot"));
				y += theme_gfx[THEME_BUTTON]->h + yadd;

				widget_button (&v_m_main->widgets[2], x, y, 0, BTN_NORMAL,
							   _("Standby"));
				y += theme_gfx[THEME_BUTTON]->h + yadd;
			}

			widget_button (&v_m_main->widgets[3], x, y, 0, BTN_NORMAL,
						   _("Power Off"));
			break;
	}
	// Wii Menu Version and Model

	u16 system_menu_tmd_version = get_tmd_version(0x0000000100000002ll);
	char* system_menu_version_string = get_system_menu_version_string(system_menu_tmd_version);

	char* wii_revision = get_wii_model();

	// SM/Wii Revision

	sprintf(bufferwiinote, "SM %s (v%d) [%s]", system_menu_version_string, system_menu_tmd_version, wii_revision);

	widget_label (&v_m_main->widgets[6], view_width / 3 * 2 - 32, 32, 0,
				  bufferwiinote, view_width / 3 - 0, FA_RIGHT,
			   FA_ASCENDER, FONT_LABEL);

	// HBC and IOS version

	sprintf(buffer, "HBC v%s, IOS%d v%d.%d", CHANNEL_VERSION_STR, IOS_GetVersion(), IOS_GetRevisionMajor(),
			IOS_GetRevisionMinor());

	widget_label (&v_m_main->widgets[7], view_width / 3 * 2 - 32,
				  32 + font_get_y_spacing(FONT_LABEL), 0, buffer,
				  view_width / 3 - 0, FA_RIGHT, FA_ASCENDER, FONT_LABEL);

	inited_widgets = true;
}

void m_main_update (void) {
	u32 ip;
	char buffer[64];

	if (loader_tcp_initialized ()) {
		ip = net_gethostip ();
		sprintf (buffer, text_ip, (ip >> 24) & 0xff, (ip >> 16) & 0xff,
				 (ip >> 8) & 0xff, ip & 0xff);
		widget_label (&v_m_main->widgets[8], 48, 32, 0, text_has_ip,
					  view_width / 3 * 2 - 32, FA_LEFT, FA_ASCENDER, FONT_LABEL);
		widget_label (&v_m_main->widgets[9], 48, 32 + font_get_y_spacing(FONT_LABEL), 0, buffer, view_width / 3 * 2 - 32, FA_LEFT, FA_ASCENDER, FONT_LABEL);
	} else {
		widget_label (&v_m_main->widgets[8], 48, 32, 0, text_no_ip,
					  view_width / 3 * 2 - 32, FA_LEFT, FA_ASCENDER, FONT_LABEL);
	}

}
