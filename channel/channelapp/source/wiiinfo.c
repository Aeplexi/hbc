#include <ogc/ipc.h>
#include <ogc/conf.h>
#include <gccore.h>

#include "title.h"
#include "string.h"
#include "m_main.h"

static const char* devDolphin [[gnu::aligned(0x20)]] = "/dev/dolphin";
static const char* netConfig [[gnu::aligned(0x20)]] = "/shared2/sys/net/02/config.dat";
static u8 netBuffer[7004] [[gnu::aligned(0x20)]];
static int dolphin_fd = ~0;

extern int __CONF_GetTxt(const char *name, char *buf, int length);

// int connection1_selectpos = 8, connection2_selectpos = 8+2332, connection3_selectpos = 8+2332+2332;
// thanks Naim2000/thepikachugamer for this method of detecting dolphin

bool is_dolphin() {
	if (!~dolphin_fd) {
		dolphin_fd = IOS_Open(devDolphin, 0);
		if (dolphin_fd > 0)
			IOS_Close(dolphin_fd);
	}
	return dolphin_fd > 0;
}

s32 check_network() {
	int fd;
	int ret;
	fd = IOS_Open( netConfig, 1);
	if (fd < 0) return fd;

	ret = IOS_Read(fd, netBuffer, 7004);
	IOS_Close(fd);
	return ret;
}

char* get_serial(char* code) {
	char serno[10];
	s32 ret;
	ret = __CONF_GetTxt("CODE", code, 4);
	if (ret < 0)
		return "err";
	ret = __CONF_GetTxt("SERNO", serno, 10);
	if (ret < 0)
		return "err";
	strcat(code, serno);
	return code;
}

int check_setting() {
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

char* get_area()
{
	// there definitely is a better way to do this, i know this kinda sucks
	s32 area = CONF_GetArea();
	switch (area)
	{
		case CONF_AREA_JPN:
			return "Japan";
		case CONF_AREA_USA:
			return "USA/Canada";
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

// TODO: maybe use an enum later?  needs revamp
char* get_wii_model() {
	int setting = check_setting();

    if (is_dolphin()) {
        return "Dolphin";
	}

	if (setting == 1) {
		return "Wii Family Edition";
	}

	if (setting == 2) {
		return "Wii Mini";
	}

	if (setting == 3) {
		return "NDEV";
	}

    if (is_vwii()) {
        return "Wii U";
	}

	if (setting < 0) {
		return "Unknown model";
	}

	return "Wii";
}

// TODO: Don't use m_main.h for this, bad practice imo - aep

char* is_priiloader_installed()
{
	bool is_installed = priiloader_is_installed();
	if (is_installed)
	{
		return "Yes";
	}
	return "No";
}