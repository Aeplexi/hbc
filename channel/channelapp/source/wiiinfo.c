#include <ogc/ipc.h>
#include <ogc/conf.h>

#include "../config.h"
#include "title.h"
#include "string.h"
#include "m_main.h"

static const char* devDolphin [[gnu::aligned(0x20)]] = "/dev/dolphin";
static const char* netConfig [[gnu::aligned(0x20)]] = "/shared2/sys/net/02/config.dat";
static u8 netBuffer[7004] [[gnu::aligned(0x20)]];
static int dolphin_fd = ~0;

extern int __CONF_GetTxt(const char *name, char *buf, int length);

// thanks Naim2000/thepikachugamer for this method of detecting dolphin
bool is_dolphin() {
	if (!~dolphin_fd) { // has this already been checked for? if not:
		// /dev/dolphin would only exist on Dolphin, so try to open it
		dolphin_fd = IOS_Open(devDolphin, 0);
		if (dolphin_fd > 0) // close if it actually worked, it is in fact Dolphin
			IOS_Close(dolphin_fd);
	}
	return dolphin_fd > 0; // if opening returns an error, /dev/dolphin doesn't exist, and therefore, isn't Dolphin
}

s32 check_connection() {
	int fd;
	s32 ret;
	u8 i;
	fd = IOS_Open(netConfig, 1);
	if (fd < 0) return fd;

	ret = IOS_Read(fd, netBuffer, 7004);
	if (ret < 0)
		return ret;
	for (i = 0; i < 4; i++) {
		if (i == 3) {
			// No connection is selected
			ret = 0;
			break;
		}
		// is connection #i selected?
		if (netBuffer[8 + 2332 * i] & 0b10000000)
			break;
	}
	if (netBuffer[8 + 2332 * i] & 1) {
		ret = 1; // wired connection
	} else
		ret = 2; // wireless connection
	IOS_Close(fd);
	return ret;
}

char* get_serial(char* code) {
	char serno[10];
	s32 ret;
	ret = __CONF_GetTxt("CODE", code, 4);
	if (ret < 0)
		return "CODE err";
	ret = __CONF_GetTxt("SERNO", serno, 10);
	if (ret < 0)
		return "SERNO err";
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

char* get_area() {
	// TODO: there definitely is a better way to do this, i know this kinda sucks
	s32 area = CONF_GetArea();
	switch (area) {
		case CONF_AREA_JPN:
			return "Japan";
		case CONF_AREA_USA:
			return "North America";
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
char* get_wii_model() {
	int setting = check_setting();

    if (is_dolphin()) {
        return "Dolphin";
	} else if (is_vwii()) {
		return "Wii U";
	}

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

// TODO: Don't use m_main.h for this, bad practice imo - aep

char* is_priiloader_installed() {
	if (priiloader_is_installed(TITLEID_SYSMENU))
		return "Yes";
	return "No";
}

char* is_bootmii_installed() {
	if (bootmii_is_installed(TITLEID_BOOTMII))
		return "Yes";
	return "No";
}
