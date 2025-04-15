#ifndef _TITLE_H_
#define _TITLE_H_

#include <gctypes.h>
#include "../config.h"

const char *title_get_path(void);
void title_init(void);
u16 get_tmd_version(u64 title);

#endif

