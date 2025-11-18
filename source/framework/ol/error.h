/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "framework/utility/error.h"

#include <sstream>
#include <string>

#define ASSERT_OL_RESULT_SUCCESS(retVal)                                                                                      \
    {                                                                                                                         \
        const ol_result_t tempVarForDefine = (retVal);                                                                        \
        if (tempVarForDefine != OL_SUCCESS) {                                                                                 \
            std::ostringstream ss;                                                                                            \
            ss << "0x" << std::hex << reinterpret_cast<uintptr_t>(tempVarForDefine);                                          \
            const char *details = (tempVarForDefine && tempVarForDefine->Details) ? tempVarForDefine->Details : "No details"; \
            NON_FATAL_ERROR("ASSERT_OL_RESULT_SUCCESS", #retVal, details, ss.str().c_str());                                  \
            return TestResult::Error;                                                                                         \
        }                                                                                                                     \
    }

#define EXPECT_OL_RESULT_SUCCESS(retVal)                                                                                      \
    {                                                                                                                         \
        const ol_result_t tempVarForDefine = (retVal);                                                                        \
        if (tempVarForDefine != OL_SUCCESS) {                                                                                 \
            std::ostringstream ss;                                                                                            \
            ss << "0x" << std::hex << reinterpret_cast<uintptr_t>(tempVarForDefine);                                          \
            const char *details = (tempVarForDefine && tempVarForDefine->Details) ? tempVarForDefine->Details : "No details"; \
            NON_FATAL_ERROR("EXPECT_OL_RESULT_SUCCESS", #retVal, details, ss.str().c_str());                                  \
        }                                                                                                                     \
    }

#define OL_RESULT_SUCCESS_OR_RETURN_VALUE(retVal, value) \
    if ((retVal) != OL_SUCCESS) {                        \
        return (value);                                  \
    }

#define OL_RESULT_SUCCESS_OR_RETURN_FALSE(retVal) \
    OL_RESULT_SUCCESS_OR_RETURN_VALUE((retVal), false)

#define OL_RESULT_SUCCESS_OR_RETURN_ERROR(retVal) \
    OL_RESULT_SUCCESS_OR_RETURN_VALUE((retVal), TestResult::Error)

#define OL_RESULT_SUCCESS_OR_RETURN(retVal)                                    \
    {                                                                          \
        const auto tempVarForDefine = (retVal);                                \
        OL_RESULT_SUCCESS_OR_RETURN_VALUE(tempVarForDefine, tempVarForDefine); \
    }

#define OL_RESULT_SUCCESS_OR_ERROR(retVal)                                                                           \
    {                                                                                                                \
        const auto tempVarForDefine = (retVal);                                                                      \
        if (tempVarForDefine != OL_SUCCESS) {                                                                        \
            std::ostringstream ss;                                                                                   \
            ss << "Fatal OL error occurred, retVal=0x" << std::hex << reinterpret_cast<uintptr_t>(tempVarForDefine); \
            if (tempVarForDefine && tempVarForDefine->Details) {                                                     \
                ss << ", details: " << tempVarForDefine->Details;                                                    \
            }                                                                                                        \
            FATAL_ERROR(ss.str());                                                                                   \
        }                                                                                                            \
    }
