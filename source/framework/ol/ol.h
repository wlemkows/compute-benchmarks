/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "framework/configuration.h"

#include <OffloadAPI.h>

struct OlState {
    OlState();
    ~OlState();

    ol_device_handle_t device;
};
