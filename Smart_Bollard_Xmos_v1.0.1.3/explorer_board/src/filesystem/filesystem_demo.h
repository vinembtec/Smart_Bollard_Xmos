// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef FILESYSTEM_DEMO_H_
#define FILESYSTEM_DEMO_H_

#include "ff.h"
void filesystem_demo_create( UBaseType_t priority );

FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
);

void sl_wfx_app_fw_size(int value);
#endif /* FILESYSTEM_DEMO_H_ */
