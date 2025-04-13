#include <gccore.h>

#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <ogc/machine/processor.h>
#include "title.h"

u32 ng_id = 0;
u32 ms_id = 0;

static u64 title_id = 0;
static char title_path[ISFS_MAXPATH] __attribute__((aligned(0x20)));

static u8 buf[0x1000] __attribute__((aligned(0x20)));

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

static void title_get_ngid(void) {
	s32 res;

	res = ES_GetDeviceID(&ng_id);
	if (res < 0) {
		gprintf("ES_GetDeviceID failed: %d\n", res);
	}
}

static void title_get_msid(void) {
	s32 ret;

	// check for dpki
	ret = ES_GetDeviceCert(buf);
	if (ret < 0) {
		gprintf("ES_GetDeviceCert failed: %d\n", ret);
		return;
	}

	ms_id = buf[0x99] - '0';

	if (ms_id == 3) {
		gprintf("We're on dpki\n");
	} else if (ms_id == 2) {
		gprintf("We're on retail\n");
	} else {
		gprintf("Unknown ms-id %d?\n", ms_id);
	}
}

static void title_get_title_path(void) {
	s32 res;

	res = ES_GetTitleID(&title_id);
	if (res < 0) {
		gprintf("ES_GetTitleID failed: %d\n", res);
		return;
	}

	res = ES_GetDataDir(title_id, title_path);
	if (res < 0) {
		gprintf("ES_GetDataDir failed: %d\n", res);
		return;
	}

	gprintf("data path is '%s'\n", title_path);
}

const char *title_get_path(void) {
	return title_path;
}

static bool title_is_installed(u64 title_id) {
	s32 ret;
	u32 x;

	ret = ES_GetTitleContentsCount(title_id, &x);

	if (ret < 0)
		return false; // title was never installed

	if (x <= 0)
		return false; // title was installed but deleted via Channel Management

	return true;
}

void title_init(void) {
	memset(title_path, 0, sizeof(title_path));
	title_get_msid();
	title_get_ngid();
	title_get_title_path();
}
