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

// TODO: maybe use an enum later? needs revamp
int get_wii_model(void) {
	char code[4];
	char model[13];

	if (is_dolphin())
		return MODEL_DOLPHIN;
	if (IS_VWII)
		return MODEL_WIIU;

	__CONF_GetTxt("CODE", code, 4);
	__CONF_GetTxt("MODEL", model, 13);

	if (model[2] == 'T') {
		return MODEL_NDEV;
	}

	if ((code[0] == 'K') | (model[4] == '1')) {
		return MODEL_FAMILY;
	} else if ((code[0] == 'H') | (model[4] == '2')) {
		return MODEL_MINI;
	}

	return MODEL_WII;
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
