#include <ogc/ipc.h>
#include <ogc/conf.h>
#include <ogc/isfs.h>
#include <ogc/es.h>
#include <stdio.h>
#include <malloc.h>

#include "wiiinfo.h"
#include "../config.h"
#include "nand.h"
#include "panic.h"
#include "string.h"
#include "title.h"

static const char* devDolphin [[gnu::aligned(0x20)]] = "/dev/dolphin";
static const char* netConfig [[gnu::aligned(0x20)]] = "/shared2/sys/net/02/config.dat";
static u8 netBuffer[7004] [[gnu::aligned(0x20)]];
static int dolphin_fd = ~0;

extern int __CONF_GetTxt(const char *name, char *buf, int length);

struct system_menu_version {
	u16 number;
	char* string;
};

static const struct system_menu_version version_table[] = {
	// sorted by version
	{0, "Prelaunch 0!?"},
	{1, "Startup Disc Menu"},
	{33, "1.0U"},
	{34, "1.0E"},
	{64, "1.0J"},
	{66, "1.0E?"}, // not listed in wiibrew??
	{97, "2.0U"},
	{128, "2.0J"},
	{130, "2.0E"},
	{162, "2.1E"},
	{192, "2.2J"},
	{193, "2.2U"},
	{194, "2.2E"},
	{224, "3.0J"},
	{225, "3.0U"},
	{226, "3.0E"},
	{256, "3.1J"},
	{257, "3.1U"},
	{258, "3.1E"},
	{288, "3.2J"},
	{289, "3.2U"},
	{290, "3.2E"},
	{326, "3.3K"},
	{352, "3.3J"},
	{353, "3.3U"},
	{354, "3.3E"},
	{384, "3.4J"},
	{385, "3.4U"},
	{386, "3.4E"},
	{390, "3.5K"},
	{416, "4.0J"},
	{417, "4.0U"},
	{418, "4.0E"},
	{448, "4.1J"},
	{449, "4.1U"},
	{450, "4.1E"},
	{454, "4.1K"},
	{480, "4.2J"},
	{481, "4.2U"},
	{482, "4.2E"},
	{486, "4.2K"},
	{512, "4.3J"},
	{513, "4.3U"},
	{514, "4.3E"},
	{518, "4.3K"},
	{544, "4.3J (4.0.0)"},
	{545, "4.3U (4.0.0)"},
	{546, "4.3E (4.0.0)"},
	{608, "4.3J (5.2.0)"},
	{609, "4.3U (5.2.0)"},
	{610, "4.3E (5.2.0)"},
	{4609, "4.3U (Mini)"},
	{4610, "4.3E (Mini)"}
};

char* get_system_menu_version_string(u16 number) {
	u8 table_size = sizeof(version_table) / sizeof(version_table[0]);

	for (u8 i = 0; i < table_size; i++) {
		if (version_table[i].number == number) {
			return version_table[i].string;
		}
	}

	return "????"; // no system menu version installed, or weird patched one?
}

// thanks Naim2000/thepikachugamer for this method of detecting dolphin
bool is_dolphin(void) {
	if (!~dolphin_fd) { // has this already been checked for? if not:
		// /dev/dolphin would only exist on Dolphin, so try to open it
		dolphin_fd = IOS_Open(devDolphin, 0);
		if (dolphin_fd > 0) // close if it actually worked, it is in fact Dolphin
			IOS_Close(dolphin_fd);
	}
	return dolphin_fd > 0; // if opening returns an error, /dev/dolphin doesn't exist, and therefore, isn't Dolphin
}

s32 check_connection(void) {
	int fd;
	s32 ret;
	u8 i;
	fd = IOS_Open(netConfig, 1);
	if (fd < 0) return fd;

	ret = IOS_Read(fd, netBuffer, 7004);
	if (ret < 0)
		return ret;
	for (i = 0; i < 4; i++) {
		if (i == 3) { // are we out of possible connections?
			// No connection is selected
			IOS_Close(fd);
			return 0;
		}
		// is connection #i selected?
		if (netBuffer[8 + 2332 * i] & 0b10000000)
			break; // connection #i is selected, stop cycling
	}
	if (netBuffer[8 + 2332 * i] & 1) { // the earlier break out of the for loop kept i at the selected connection
		ret = 1; // wired connection
	} else
		ret = 2; // wireless connection
	IOS_Close(fd);
	return ret;
}

void get_serial(char* code) {
	char serno[10];
	s32 ret;
	ret = __CONF_GetTxt("CODE", code, 4);
	if (ret < 0)
		strcpy(code, "???");
	ret = __CONF_GetTxt("SERNO", serno, 10);
	if (ret < 0)
		strcpy(serno, "?????????");
	strcat(code, serno);
}

int check_setting(void) {
	char code[4];
	char model[13];
	s32 ret;

	ret = __CONF_GetTxt("CODE", code, 4);
	if (ret < 0)
		return ret;

	ret = __CONF_GetTxt("MODEL", model, 13);
	if (ret < 0)
		return ret;

	if (model[2] == 'T') {
		ret = 3;
	}

	if ((code[0] == 'K') | (model[4] == '1')) {
		ret = 1;
	} else if ((code[0] == 'H') | (model[4] == '2')) {
		ret = 2;
	}
	return ret;
}

void get_hardware_region(char* region) {
	s32 ret;
	char model[13];
	ret = __CONF_GetTxt("MODEL", model, 13);
	if (ret < 0)
		strcpy(region, "???");
	region[0] = model[8];
	region[1] = model[9];
	region[2] = model[10];
	region[3] = '\0';
}

void get_model_number(char* model_setting) {
	s32 ret;
	char model[13];
	ret = __CONF_GetTxt("MODEL", model, 13);
	if (ret < 0)
		strcpy(model_setting, "MODEL error");
	strncpy(model_setting, model, 7);
	model_setting[7] = '\0';
}

char* get_area() {
	// TODO: there definitely is a better way to do this, i know this kinda sucks
	s32 area = CONF_GetArea();
	switch (area) {
		case CONF_AREA_JPN:
			return "Japan";
		case CONF_AREA_USA:
			return "United States";
		case CONF_AREA_EUR:
			return "Europe";
		case CONF_AREA_AUS:
			return "Australia";
		case CONF_AREA_BRA:
			return "Brazil";
		case CONF_AREA_TWN:
			return "Taiwan";
		case CONF_AREA_ROC:
			return "Republic of China";
		case CONF_AREA_KOR:
			return "Korea";
		case CONF_AREA_HKG:
			return "Hong Kong";
		case CONF_AREA_ASI:
			return "Asia";
		case CONF_AREA_LTN:
			return "Latin America";
		case CONF_AREA_SAF:
			return "South Africa";
		case CONF_AREA_CHN:
			return "China (ique wtf?)";
		default:
			return "unknown";
	}
}

// TODO: maybe use an enum later? needs revamp
char* get_wii_model(void) {
	int setting = check_setting();

	if (is_dolphin())
		return "Dolphin";
	if (IS_VWII)
		return "Wii U";

	switch (setting) {
		case 1:
			return "Wii Family Edition";

		case 2:
			return "Wii Mini";

		case 3:
			return "NDEV";

		default:
			return "Wii";

	}
}

bool bootmii_ios_is_installed(u64 title_id) {
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

	if (tmdbuf[50] == 'B' && tmdbuf[51] == 'M') {
		ret = true;
	} else
		ret = false;

	free(tmdbuf);

	return ret;
}

void bootmii_ios_version(u64 title_id, char* version) {
	u32 tmd_view_size;
	u8 *tmdbuf;

	if (ES_GetTMDViewSize(title_id, &tmd_view_size) < 0)
		return;

	if (tmd_view_size < 90)
		return;

	if (tmd_view_size > 1024)
		return;

	tmdbuf = pmemalign(32, 1024);

	if (ES_GetTMDView(title_id, tmdbuf, tmd_view_size) < 0) {
		free(tmdbuf);
		return;
	}
	sprintf(version, "v%.4s", (char*)tmdbuf + 52);

	free(tmdbuf);
}

static u32 GetSysMenuBootContent(void) {
	s32 ret;
	u32 cid = 0;
	u32 size = 0;
	signed_blob *s_tmd = NULL;

	ret = ES_GetStoredTMDSize(0x100000002LL, &size);
	if (!size)
	{
		printf("Error! ES_GetStoredTMDSize failed (ret=%i)\n", ret);
		return 0;
	}

	s_tmd = memalign(32, size);
	if (!s_tmd)
	{
		printf("Error! Memory allocation failed!\n");
		return 0;
	}

	ret = ES_GetStoredTMD(0x100000002LL, s_tmd, size);
	if (ret < 0)
	{
		printf("Error! ES_GetStoredTMD failed (ret=%i)\n", ret);
		free(s_tmd);
		return 0;
	}

	tmd *p_tmd = SIGNATURE_PAYLOAD(s_tmd);

	for (int i = 0; i < p_tmd->num_contents; i++)
	{
		tmd_content* content = &p_tmd->contents[i];
		if (content->index == p_tmd->boot_index)
		{
			cid = content->cid;
			break;
		}
	}

	free(s_tmd);
	if (!cid) printf("Error! Cannot find system menu boot content!\n");

	return cid;
}

bool GetSysMenuExecPath(char path[ISFS_MAXPATH], bool mainDOL) {
	u32 cid = GetSysMenuBootContent();
	if (!cid) return false;

	if (mainDOL) cid |= 0x10000000;
	sprintf(path, "/title/00000001/00000002/content/%08x.app", cid);

	return true;
}

bool priiloader_is_installed(void) {
	char path[ISFS_MAXPATH] ATTRIBUTE_ALIGN(0x20);

	if (!GetSysMenuExecPath(path, true))
		return false;

	u32 size = 0;
	NANDGetFileSize(path, &size);

	return (size > 0);
}
