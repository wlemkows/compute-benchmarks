/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "framework/ol/error.h"
#include "framework/ol/ol.h"
#include "framework/test_case/register_test_case.h"
#include "framework/utility/combo_profiler.h"
#include "framework/utility/file_helper.h"
#include "framework/utility/timer.h"

#include "definitions/submit_kernel.h"

#include <OffloadAPI.h>
#include <gtest/gtest.h>

static constexpr size_t n_dimensions = 3;
static constexpr size_t global_size[] = {1, 1, 1};
static constexpr size_t local_size[] = {1, 1, 1};

static TestResult run(const SubmitKernelArguments &arguments, Statistics &statistics) {
    ComboProfilerWithStats profiler(Configuration::get().profilerType);

    if (isNoopRun()) {
        profiler.pushNoop(statistics);
        return TestResult::Nooped;
    }

    int kernelExecutionTime = arguments.kernelExecutionTime;

    // Setup
    OlState ol;

    // Create program
    auto spirvModule = FileHelper::loadBinaryFile("api_overhead_benchmark_eat_time.spv");
    if (spirvModule.size() == 0)
        return TestResult::KernelNotFound;

    ol_program_handle_t program;
    EXPECT_OL_RESULT_SUCCESS(olCreateProgram(ol.device, spirvModule.data(),
                                             spirvModule.size(), &program));

    // Get kernel symbol
    ol_symbol_handle_t kernel = nullptr;
    EXPECT_OL_RESULT_SUCCESS(olGetSymbol(program, "eat_time", OL_SYMBOL_KIND_KERNEL, &kernel));

    // Create queue
    ol_queue_handle_t queue;
    EXPECT_OL_RESULT_SUCCESS(olCreateQueue(ol.device, &queue));

    // Setup launch parameters
    ol_kernel_launch_size_args_t LaunchArgs{};
    LaunchArgs.Dimensions = n_dimensions;
    LaunchArgs.GroupSize = {local_size[0], local_size[1], local_size[2]};
    LaunchArgs.NumGroups = {global_size[0], global_size[1], global_size[2]};
    LaunchArgs.DynSharedMemory = 0;

    std::vector<ol_event_handle_t> events(arguments.numKernels);

    // Warmup - no timing
    for (auto iteration = 0u; iteration < arguments.numKernels; iteration++) {
        ol_event_handle_t *signalEvent = nullptr;
        if (arguments.useEvents) {
            signalEvent = &events[iteration];
            EXPECT_OL_RESULT_SUCCESS(olCreateEvent(queue, signalEvent));
        }

        EXPECT_OL_RESULT_SUCCESS(olLaunchKernel(
            queue, ol.device, kernel,
            reinterpret_cast<void *>(&kernelExecutionTime), sizeof(int),
            &LaunchArgs));
    }

    // Cleanup warmup events
    for (auto &event : events) {
        if (event) {
            EXPECT_OL_RESULT_SUCCESS(olDestroyEvent(event));
        }
    }

    // Benchmark
    for (auto i = 0u; i < arguments.iterations; i++) {
        profiler.measureStart();

        for (auto iteration = 0u; iteration < arguments.numKernels; iteration++) {
            ol_event_handle_t *signalEvent = nullptr;
            if (arguments.useEvents) {
                signalEvent = &events[iteration];
                EXPECT_OL_RESULT_SUCCESS(olCreateEvent(queue, signalEvent));
            }

            EXPECT_OL_RESULT_SUCCESS(olLaunchKernel(
                queue, ol.device, kernel,
                reinterpret_cast<void *>(&kernelExecutionTime), sizeof(int),
                &LaunchArgs));
        }

        if (!arguments.measureCompletionTime) {
            profiler.measureEnd();
        }

        EXPECT_OL_RESULT_SUCCESS(olSyncQueue(queue));

        if (arguments.measureCompletionTime) {
            profiler.measureEnd();
        }

        profiler.pushStats(statistics);

        for (auto &event : events) {
            if (event) {
                EXPECT_OL_RESULT_SUCCESS(olDestroyEvent(event));
            }
        }
    }

    // Cleanup
    EXPECT_OL_RESULT_SUCCESS(olDestroyQueue(queue));
    EXPECT_OL_RESULT_SUCCESS(olDestroyProgram(program));

    return TestResult::Success;
}

[[maybe_unused]] static RegisterTestCaseImplementation<SubmitKernel> registerTestCase(run, Api::OL);
