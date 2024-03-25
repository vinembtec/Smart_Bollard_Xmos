// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef INTERTILE_COMM_TILE1_H_
#define INTERTILE_COMM_TILE1_H_

#include <xcore/channel.h>
#include <xcore/parallel.h>
#include "Camera_Main/host_mcu/host_mcu_types.h"

void InterTileCommTile1_sendResponse(chanend_t txCmd, unsigned char packetType, unsigned char status, unsigned char *data);

#endif /* INTERTILE_COMM_TILE1_H_ */
