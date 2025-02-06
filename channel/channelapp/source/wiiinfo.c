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

#define BLOCK_SIZE 64
#define NAND_ECC_DATA_SIZE	0x40
#define PAGE_SIZE_NO_ECC	0x800
#define NAND_PAGE_SIZE		(PAGE_SIZE_NO_ECC + NAND_ECC_DATA_SIZE)
#define NAND_BLOCK_SIZE		(NAND_PAGE_SIZE * BLOCK_SIZE)

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
			IOS_Close(fd);
			return 0;
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

char* get_hardware_region(char* region) {
	s32 ret;
	char model[13];
	ret = __CONF_GetTxt("MODEL", model, 13);
	if (ret < 0)
		return "Error";
	region[0] = model[8];
	region[1] = model[9];
	region[2] = model[10];
	region[3] = '\0';
	return region;
}

char* get_model_number(char* model_setting)
{
	s32 ret;
	char model[13];
	ret = __CONF_GetTxt("MODEL", model, 13);
	if (ret < 0)
		return "Failed to get the model number!";
	model_setting[0] = model[0];
	model_setting[1] = model[1];
	model_setting[2] = model[2];
	model_setting[3] = model[3];
	model_setting[4] = model[4];
	model_setting[5] = model[5];
	model_setting[6] = model[6];
	model_setting[7] = '\0';
	return model_setting;
}

// TODO: maybe use an enum later? needs revamp
char* get_wii_model() {
	int setting = check_setting();

    if (is_dolphin()) {
        return "Dolphin";
	} else if (IS_VWII) {
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

static s32 __isBlockBad(s32 fd) {
	s32 rv;

	rv = IOS_Ioctl(fd, 4, NULL, 0, NULL, 0); /* Is this block bad? */

	return (rv == -13);
}

static s32 __readPage(u8* data, int pageno, void *ecc, s32 fd) {
	static unsigned char buffer[NAND_PAGE_SIZE] __attribute__((aligned(0x20)));
	int rv;

	rv = IOS_Seek(fd, pageno, 0);

	if (rv < 0)
		return -3;

	rv = IOS_Read(fd, buffer, (u32) NAND_PAGE_SIZE);

	gprintf("IOS_Read returned %d\n", rv);

	if (ecc) {
		memcpy(data, buffer, PAGE_SIZE_NO_ECC);
		memcpy(ecc, buffer + PAGE_SIZE_NO_ECC, NAND_ECC_DATA_SIZE);
	} else
		memcpy(data, buffer, NAND_PAGE_SIZE);


	return 0;
}

bool bootmii_is_installed(int copy) {
	s32 fd = IOS_Open("/dev/flash", IPC_OPEN_RW);

	u8* nandPage;

	u16 blocksToTest[3] = {1,192,448};
	u16 block = blocksToTest[copy];

	bool result;

	IOS_Seek(fd, block, 0);

	/* Is this block bad? */
	if(__isBlockBad(fd)) {
		IOS_Close(fd);
		return -1;
	}

	/* [nitr8]: moved down here */
	nandPage = (u8*)malloc(NAND_PAGE_SIZE);
	__readPage(nandPage, block + 1, NULL, fd);
	hexdump(nandPage, block + 1);
	result = (nandPage[18] == 0x42 && nandPage[19] == 0x4D);
	gprintf("%p", &nandPage[1654]);
	free(nandPage);
	IOS_Close(fd);
	return result;
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

	if (tmdbuf[50] == 'B' && tmdbuf[51] == 'M')
		ret = true;
	else
		ret = false;

	free(tmdbuf);

	return ret;
}

static u32 GetSysMenuBootContent(void)
{
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

bool GetSysMenuExecPath(char path[ISFS_MAXPATH], bool mainDOL)
{
	u32 cid = GetSysMenuBootContent();
	if (!cid) return false;

	if (mainDOL) cid |= 0x10000000;
	sprintf(path, "/title/00000001/00000002/content/%08x.app", cid);

	return true;
}

bool priiloader_is_installed()
{
	char path[ISFS_MAXPATH] ATTRIBUTE_ALIGN(0x20);

	if (!GetSysMenuExecPath(path, true))
		return false;

	u32 size = 0;
	NANDGetFileSize(path, &size);

	return (size > 0);
}

char* bootmii_is_installed_text() {
	u8 i;
	if (IS_VWII)
		return "N/A";
	for (i = 0; i < 3; i++) {
		if (bootmii_is_installed(i))
			return "Yes";
	}
	return "No";
}

char* bootmii_ios_is_installed_text() {
	if (bootmii_ios_is_installed(TITLEID_BOOTMII))
		return "Yes";
	return "No";
}

char* priiloader_is_installed_text() {
	if (priiloader_is_installed(TITLEID_SYSMENU))
		return "Yes";
	return "No";
}
