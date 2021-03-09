/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include "tapi/t.h"
#include <time.h>
#include <math.h>
#include <stdio.h>

#define GET_DEVICE_FUNCTION_PTR(name) \
    PFN_vk##name name = (PFN_vk##name)vkGetDeviceProcAddr(t_device, "vk"#name)

#define GET_INSTANCE_FUNCTION_PTR(name) \
    PFN_vk##name name = (PFN_vk##name)vkGetInstanceProcAddr(t_instance, "vk"#name)

/* Make sure the function pointers promised by the extension are valid. */
static void
test_funcs(void)
{
    t_require_ext("VK_EXT_calibrated_timestamps");

    GET_INSTANCE_FUNCTION_PTR(GetPhysicalDeviceCalibrateableTimeDomainsEXT);
    GET_DEVICE_FUNCTION_PTR(GetCalibratedTimestampsEXT);

    t_assert(GetPhysicalDeviceCalibrateableTimeDomainsEXT != NULL);
    t_assert(GetCalibratedTimestampsEXT != NULL);
}

test_define {
    .name = "func.calibrated-timestamps.funcs",
    .start = test_funcs,
    .no_image = true,
};

/* Make sure all of the domains offered by the driver are in range. */
static void
test_domains(void)
{
    t_require_ext("VK_EXT_calibrated_timestamps");

    GET_INSTANCE_FUNCTION_PTR(GetPhysicalDeviceCalibrateableTimeDomainsEXT);
    GET_DEVICE_FUNCTION_PTR(GetCalibratedTimestampsEXT);

    t_assert(GetPhysicalDeviceCalibrateableTimeDomainsEXT != NULL);
    t_assert(GetCalibratedTimestampsEXT != NULL);

    VkResult result;

    uint32_t timeDomainCount;
    result = GetPhysicalDeviceCalibrateableTimeDomainsEXT(
        t_physical_dev,
        &timeDomainCount,
        NULL);
    t_assert(result == VK_SUCCESS);
    t_assert(timeDomainCount > 0);

    VkTimeDomainEXT *timeDomains = calloc(timeDomainCount, sizeof (VkTimeDomainEXT));
    t_assert(timeDomains != NULL);

    result = GetPhysicalDeviceCalibrateableTimeDomainsEXT(
        t_physical_dev,
        &timeDomainCount,
        timeDomains);

    t_assert(result == VK_SUCCESS);

    /* Make sure all reported domains are valid */
    for (uint32_t d = 0; d < timeDomainCount; d++) {
        t_assert(VK_TIME_DOMAIN_BEGIN_RANGE_EXT <= timeDomains[d] &&
                 timeDomains[d] <= VK_TIME_DOMAIN_END_RANGE_EXT);
    }
}

test_define {
    .name = "func.calibrated-timestamps.domains",
    .start = test_domains,
    .no_image = true,
};

static uint64_t
crucible_clock_gettime(VkTimeDomainEXT domain)
{
    struct timespec current;
    int ret;
    clockid_t clock_id;

    switch (domain) {
    case VK_TIME_DOMAIN_CLOCK_MONOTONIC_EXT:
        clock_id = CLOCK_MONOTONIC;
        break;
    case VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT:
        clock_id = CLOCK_MONOTONIC_RAW;
        break;
    default:
        t_assert(0);
        return 0;
    }

    ret = clock_gettime(clock_id, &current);
    t_assert (ret >= 0);
    if (ret < 0)
        return 0;

    return (uint64_t) current.tv_sec * 1000000000ULL + current.tv_nsec;
}

/* Make sure any monotonic domains return accurate data. */
static void
test_monotonic(void)
{
    t_require_ext("VK_EXT_calibrated_timestamps");

    GET_INSTANCE_FUNCTION_PTR(GetPhysicalDeviceCalibrateableTimeDomainsEXT);
    GET_DEVICE_FUNCTION_PTR(GetCalibratedTimestampsEXT);

    t_assert(GetPhysicalDeviceCalibrateableTimeDomainsEXT != NULL);
    t_assert(GetCalibratedTimestampsEXT != NULL);

    VkResult result;

    uint32_t timeDomainCount;
    result = GetPhysicalDeviceCalibrateableTimeDomainsEXT(
        t_physical_dev,
        &timeDomainCount,
        NULL);
    t_assert(result == VK_SUCCESS);
    t_assert(timeDomainCount > 0);

    VkTimeDomainEXT *timeDomains = calloc(timeDomainCount, sizeof (VkTimeDomainEXT));
    t_assert(timeDomains != NULL);

    result = GetPhysicalDeviceCalibrateableTimeDomainsEXT(
        t_physical_dev,
        &timeDomainCount,
        timeDomains);

    t_assert(result == VK_SUCCESS);

    /* Test all monotonic clocks */

    VkCalibratedTimestampInfoEXT        *timestampInfo =
        calloc(timeDomainCount, sizeof (VkCalibratedTimestampInfoEXT));
    t_assert(timestampInfo != NULL);

    uint64_t                            *timestamps =
        calloc(timeDomainCount, sizeof (uint64_t));
    t_assert(timestamps != NULL);

    uint64_t                            maxDeviation;

    for (uint32_t i = 0; i < 10; i++) {
        for (uint32_t d = 0; d < timeDomainCount; d++) {
            uint64_t        before, after;

            switch (timeDomains[d]) {
            case VK_TIME_DOMAIN_CLOCK_MONOTONIC_EXT:
            case VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT:
                timestampInfo[0] = (VkCalibratedTimestampInfoEXT) {
                    .sType = VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT,
                    .pNext = NULL,
                    .timeDomain = timeDomains[d]
                };
                before = crucible_clock_gettime(timeDomains[d]);
                result = GetCalibratedTimestampsEXT(
                    t_device,
                    1,
                    timestampInfo,
                    timestamps,
                    &maxDeviation
                    );
                t_assert(result == VK_SUCCESS);
                after = crucible_clock_gettime(timeDomains[d]);
                t_assert(before <= timestamps[0]);
                t_assert(timestamps[0] <= after);
                break;
            default:
                break;
            }
        }

        struct timespec req = {
            .tv_sec = 0,
            .tv_nsec = 100000000UL
        };
        nanosleep(&req, NULL);
    }
}

test_define {
    .name = "func.calibrated-timestamps.monotonic",
    .start = test_monotonic,
    .no_image = true,
};


static uint64_t
device_time_to_ns(uint64_t device_time)
{
    float timestamp_period = t_physical_dev_props->limits.timestampPeriod;
    long double dt_ld = (long double) device_time;
    long double tp_ld = (long double) timestamp_period;
    long double ns = dt_ld * tp_ld;

    while (ns >= UINT64_MAX)
        ns -= UINT64_MAX;

    return (uint64_t) floorl(ns + 0.5);
}

/* Make sure the Device domain doesn't drift relative to a monotonic domain. */
static void
test_device(void)
{
    t_require_ext("VK_EXT_calibrated_timestamps");

    GET_INSTANCE_FUNCTION_PTR(GetPhysicalDeviceCalibrateableTimeDomainsEXT);
    GET_DEVICE_FUNCTION_PTR(GetCalibratedTimestampsEXT);

    t_assert(GetPhysicalDeviceCalibrateableTimeDomainsEXT != NULL);
    t_assert(GetCalibratedTimestampsEXT != NULL);

    VkResult result;

    uint32_t timeDomainCount;
    result = GetPhysicalDeviceCalibrateableTimeDomainsEXT(
        t_physical_dev,
        &timeDomainCount,
        NULL);
    t_assert(result == VK_SUCCESS);
    t_assert(timeDomainCount > 0);

    VkTimeDomainEXT *timeDomains = calloc(timeDomainCount, sizeof (VkTimeDomainEXT));
    t_assert(timeDomains != NULL);

    result = GetPhysicalDeviceCalibrateableTimeDomainsEXT(
        t_physical_dev,
        &timeDomainCount,
        timeDomains);

    t_assert(result == VK_SUCCESS);

    /* Test device clock */


    /* Pick a monotonic domain to test against. Prefer MONOTONIC_RAW */
    VkTimeDomainEXT     monotonicDomain = VK_TIME_DOMAIN_MAX_ENUM_EXT;
    bool                foundDeviceDomain = false;

    for (uint32_t d = 0; d < timeDomainCount; d++) {
        switch (timeDomains[d]) {
        case VK_TIME_DOMAIN_CLOCK_MONOTONIC_EXT:
            if (monotonicDomain == VK_TIME_DOMAIN_MAX_ENUM_EXT)
                monotonicDomain = timeDomains[d];
            break;
        case VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT:
            monotonicDomain = timeDomains[d];
            break;
        case VK_TIME_DOMAIN_DEVICE_EXT:
            foundDeviceDomain = true;
            break;
        default:
            break;
        }
    }

    t_assert(foundDeviceDomain);
    t_assert(monotonicDomain != VK_TIME_DOMAIN_MAX_ENUM_EXT);

    VkCalibratedTimestampInfoEXT        timestampInfo[2] = {
        {
            .sType = VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT,
            .pNext = NULL,
            .timeDomain = VK_TIME_DOMAIN_DEVICE_EXT
        },
        {
            .sType = VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT,
            .pNext = NULL,
            .timeDomain = monotonicDomain,
        }
    };
    uint64_t                            timestamps_start[2];
    uint64_t                            timestamps[2];
    uint64_t                            max_deviation_start;
    uint64_t                            max_deviation;

    result = GetCalibratedTimestampsEXT(
        t_device,
        2,
        timestampInfo,
        timestamps_start,
        &max_deviation_start
        );
    t_assert(result == VK_SUCCESS);

    /* Make sure device time doesn't drift relative to monotonic time by more than
     * that promised by the driver plus a
     */
    for (uint32_t i = 0; i < 10; i++) {
        result = GetCalibratedTimestampsEXT(
            t_device,
            2,
            timestampInfo,
            timestamps,
            &max_deviation
            );

        t_assert(result == VK_SUCCESS);

        uint64_t        device_delta = device_time_to_ns(timestamps[0] - timestamps_start[0]);
        uint64_t        mono_delta = timestamps[1] - timestamps_start[1];

        uint64_t        difference = device_delta > mono_delta ? device_delta - mono_delta : mono_delta - device_delta;

        uint64_t        allowed_clock_drift = mono_delta / 1000;        /* require clocks to be within 0.1% */
        uint64_t        max_difference = max_deviation_start + max_deviation + allowed_clock_drift;

        t_assert (difference <= max_difference);

        struct timespec req = {
            .tv_sec = 0,
            .tv_nsec = 100000000UL
        };
        nanosleep(&req, NULL);
    }
}

test_define {
    .name = "func.calibrated-timestamps.device",
    .start = test_device,
    .no_image = true,
};

static uint64_t
get_timestamps(void)
{
    VkQueryPool pool = qoCreateQueryPool(t_device,
                                         .queryType = VK_QUERY_TYPE_TIMESTAMP,
                                         .queryCount = 1);

    VkCommandBuffer cmdBuffer = qoAllocateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmdBuffer);
    vkCmdWriteTimestamp(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, pool, 0);
    qoEndCommandBuffer(cmdBuffer);

    qoQueueSubmit(t_queue, 1, &cmdBuffer, VK_NULL_HANDLE);

    qoQueueWaitIdle(t_queue);

    uint64_t results[1];
    vkGetQueryPoolResults(t_device, pool, 0, 1,
                          sizeof(results), results, sizeof (results[0]),
                          VK_QUERY_RESULT_64_BIT);
    return results[0];
}

static void
test_command(void)
{
    t_require_ext("VK_EXT_calibrated_timestamps");

    GET_INSTANCE_FUNCTION_PTR(GetPhysicalDeviceCalibrateableTimeDomainsEXT);
    GET_DEVICE_FUNCTION_PTR(GetCalibratedTimestampsEXT);

    t_assert(GetPhysicalDeviceCalibrateableTimeDomainsEXT != NULL);
    t_assert(GetCalibratedTimestampsEXT != NULL);

    VkResult result;

    VkCalibratedTimestampInfoEXT        timestampInfo[1] = {
        {
            .sType = VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT,
            .pNext = NULL,
            .timeDomain = VK_TIME_DOMAIN_DEVICE_EXT
        },
    };

    for (uint32_t t = 0; t < 10; t++) {
        uint64_t    device_time_before;
        uint64_t    device_time_after;
        uint64_t    max_deviation;

        result = GetCalibratedTimestampsEXT(
            t_device,
            1,
            timestampInfo,
            &device_time_before,
            &max_deviation);
        t_assert (result == VK_SUCCESS);

        uint64_t    queue_time;

        queue_time = get_timestamps();

        result = GetCalibratedTimestampsEXT(
            t_device,
            1,
            timestampInfo,
            &device_time_after,
            &max_deviation);
        t_assert (result == VK_SUCCESS);

        printf("before %lu queue %lu (%ld) after %lu (%ld)\n",
               device_time_to_ns(device_time_before),
               device_time_to_ns(queue_time),
               (int64_t) (device_time_to_ns(queue_time) - device_time_to_ns(device_time_before)),
               device_time_to_ns(device_time_after),
               (int64_t) (device_time_to_ns(device_time_after) - device_time_to_ns(device_time_before)));

        t_assert(device_time_before <= queue_time);
        t_assert(queue_time <= device_time_after);
        struct timespec req = {
            .tv_sec = 0,
            .tv_nsec = 100000000UL
        };
        nanosleep(&req, NULL);
    }
}

test_define {
    .name = "func.calibrated-timestamps.command",
    .start = test_command,
    .no_image = true,
};
