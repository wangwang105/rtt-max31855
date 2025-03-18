/**
 * @file     :max31855_sensor_v1.h
 * @author   :wangwang105
 * @brief    :
 * @version  :0.1
 * @date     :2025-03-17 11:43:26
 * 
 * @copyright:Copyright (c) 2025 王荣文.AllRightsReserved.
 */

#ifndef __SENSOR_MAX31855_H__
#define __SENSOR_MAX31855_H__

#include <rtthread.h>
#include <rtdevice.h>

#if defined(RT_VERSION_CHECK)
    #if (RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2))
        #define RT_SIZE_TYPE   rt_ssize_t
    #else
        #define RT_SIZE_TYPE   rt_size_t
    #endif
#endif

#include "max31855.h"

/*CMD*/
#define CMD_SENSOR_READ_SELF_TEMP 0x01

int rt_hw_max31855_init(const char *name, struct rt_sensor_config *cfg);

#endif /* __SENSOR_MAX31855_H__ */
