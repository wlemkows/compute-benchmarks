/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "ol.h"

#include "error.h"

#include <OffloadAPI.h>
#include <stdexcept>
#include <vector>

OlState::OlState() {

    olInit();

    size_t device_count = 0;
    std::vector<ol_device_handle_t> Devices;

    EXPECT_OL_RESULT_SUCCESS(olIterateDevices(
        [](ol_device_handle_t Device, void *UserData) {
            reinterpret_cast<decltype(Devices) *>(UserData)->push_back(Device);
            return true;
        },
        &Devices));
    device_count = Devices.size();

    if (device_count == 0) {
        FATAL_ERROR("No device found");
    }

    if (Configuration::get().olDeviceIndex >= device_count) {
        FATAL_ERROR("Invalid OL device index. deviceIndex=",
                    Configuration::get().olDeviceIndex, " deviceCount=",
                    device_count);
    }
    this->device = Devices[Configuration::get().olDeviceIndex];
}

OlState::~OlState() {

    olShutDown();
}
