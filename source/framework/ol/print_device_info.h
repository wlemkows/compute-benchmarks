/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "error.h"
#include "ol.h"

#include <iomanip>

namespace OL {

// Unified template function to get info for any handle type
template <typename Type, typename Handle, typename InfoType>
static Type getInfo(Handle handle,
                    InfoType info,
                    ol_result_t (*getSizeFn)(Handle, InfoType, size_t *),
                    ol_result_t (*getInfoFn)(Handle, InfoType, size_t, void *)) {
    if constexpr (std::is_same_v<Type, std::string>) {
        // Special handling for strings - need to query size first
        size_t size = 0;
        if (getSizeFn(handle, info, &size) != OL_SUCCESS || size == 0) {
            return "<unknown>";
        }

        std::string result(size, '\0');
        if (getInfoFn(handle, info, size, result.data()) != OL_SUCCESS) {
            return "<error>";
        }
        return result;
    } else {
        // Simple handling for numeric types and handles
        Type result{};
        if (getInfoFn(handle, info, sizeof(Type), &result) != OL_SUCCESS) {
            return Type{};
        }
        return result;
    }
}

// Wrapper for device info
template <typename Type>
static Type getDeviceInfo(ol_device_handle_t device, ol_device_info_t info) {
    return getInfo<Type>(device, info, olGetDeviceInfoSize, olGetDeviceInfo);
}

// Wrapper for platform info
template <typename Type>
static Type getPlatformInfo(ol_platform_handle_t platform, ol_platform_info_t info) {
    return getInfo<Type>(platform, info, olGetPlatformInfoSize, olGetPlatformInfo);
}

// Convenient wrapper functions using templates

// Get device type as string
static std::string getDeviceType(ol_device_handle_t device) {
    ol_device_type_t type = getDeviceInfo<ol_device_type_t>(device, OL_DEVICE_INFO_TYPE);

    switch (type) {
    case OL_DEVICE_TYPE_GPU:
        return "GPU";
    case OL_DEVICE_TYPE_CPU:
        return "CPU";
    case OL_DEVICE_TYPE_HOST:
        return "HOST";
    default:
        return "Unknown";
    }
}

// Basic device info
static std::string getDeviceName(ol_device_handle_t device) {
    return getDeviceInfo<std::string>(device, OL_DEVICE_INFO_NAME);
}

static std::string getDeviceVendor(ol_device_handle_t device) {
    return getDeviceInfo<std::string>(device, OL_DEVICE_INFO_VENDOR);
}

static uint32_t getDeviceVendorId(ol_device_handle_t device) {
    return getDeviceInfo<uint32_t>(device, OL_DEVICE_INFO_VENDOR_ID);
}

// Compute capabilities
static uint32_t getDeviceNumComputeUnits(ol_device_handle_t device) {
    return getDeviceInfo<uint32_t>(device, OL_DEVICE_INFO_NUM_COMPUTE_UNITS);
}

static uint32_t getDeviceMaxWorkGroupSize(ol_device_handle_t device) {
    return getDeviceInfo<uint32_t>(device, OL_DEVICE_INFO_MAX_WORK_GROUP_SIZE);
}

// Platform
static ol_platform_handle_t getDevicePlatform(ol_device_handle_t device) {
    return getDeviceInfo<ol_platform_handle_t>(device, OL_DEVICE_INFO_PLATFORM);
}

static std::string getPlatformName(ol_platform_handle_t platform) {
    return getPlatformInfo<std::string>(platform, OL_PLATFORM_INFO_NAME);
}

static std::string getPlatformVersion(ol_platform_handle_t platform) {
    return getPlatformInfo<std::string>(platform, OL_PLATFORM_INFO_VERSION);
}

void printDeviceInfo() {
    std::cout << "========== OL Device Information ==========" << std::endl;
    OlState ol;

    auto platform = getDevicePlatform(ol.device);

    // Platform information
    std::cout << "\nPlatform Information:" << std::endl;
    std::cout << "  Name:    " << getPlatformName(platform) << std::endl;
    std::cout << "  Version: " << getPlatformVersion(platform) << std::endl;

    // Device identification
    std::cout << "\nDevice Identification:" << std::endl;
    std::cout << "  Name:           " << getDeviceName(ol.device) << std::endl;
    std::cout << "  Type:           " << getDeviceType(ol.device) << std::endl;
    std::cout << "  Vendor:         " << getDeviceVendor(ol.device) << std::endl;
    std::cout << "  Vendor ID:      0x" << std::hex << getDeviceVendorId(ol.device) << std::dec << std::endl;

    // Compute capabilities
    std::cout << "\nCompute Capabilities:" << std::endl;
    std::cout << "  Num Compute Units:      " << getDeviceNumComputeUnits(ol.device) << std::endl;
    std::cout << "  Max Work Group Size:    " << getDeviceMaxWorkGroupSize(ol.device) << std::endl;
    std::cout << "\n===========================================" << std::endl;
    std::cout << std::endl;
}

static void printAvailableDevices() {
    std::cout << "Available devices:" << std::endl;

    OlState ol;

    // Get all devices
    std::vector<ol_device_handle_t> Devices;
    EXPECT_OL_RESULT_SUCCESS(olIterateDevices(
        [](ol_device_handle_t Device, void *UserData) {
            reinterpret_cast<decltype(Devices) *>(UserData)->push_back(Device);
            return true;
        },
        &Devices));

    std::cout << "OL devices: " << Devices.size() << std::endl;

    for (size_t device_index = 0; device_index < Devices.size(); device_index++) {
        auto &device = Devices[device_index];

        std::cout << "  Device " << device_index << ": " << getDeviceName(device) << std::endl;
        std::cout << "    Type: " << getDeviceType(device) << std::endl;

        auto platform = getDevicePlatform(device);
        std::cout << "    Platform: " << getPlatformName(platform) << std::endl;
        std::cout << "    select this device with --olDeviceIndex=" << device_index << std::endl;
        std::cout << std::endl;
    }
}

} // namespace OL
