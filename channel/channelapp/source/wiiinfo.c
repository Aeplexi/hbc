#include <ogc/ipc.h>

#include "title.h"

static const char* devDolphin [[gnu::aligned(0x20)]] = "/dev/dolphin";
static int dolphin_fd = ~0;
extern int __CONF_GetTxt(const char *name, char *buf, int length);

// thanks Naim2000/thepikachugamer for this method of detecting dolphin
bool is_dolphin() {
	if (!~dolphin_fd) {
		dolphin_fd = IOS_Open(devDolphin, 0);
		if (dolphin_fd > 0)
			IOS_Close(dolphin_fd);
	}
	return dolphin_fd > 0;
}

int check_setting() {
	char code[4];
	char model[13];
	char ret;

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
