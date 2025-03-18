/**
 * @file     :max31855.h
 * @author   :wangwang105
 * @brief    :
 * @version  :0.1
 * @date     :2025-03-17 11:29:26
 * 
 * @copyright:Copyright (c) 2025 王荣文.AllRightsReserved.
 */

#ifndef __DRV_MAX31855_H__
#define __DRV_MAX31855_H__

#include <rtthread.h>

struct max31855_device
{
    struct rt_spi_device *spi;
    rt_mutex_t            lock;
};
typedef struct max31855_device *max31855_device_t;

void max31855_deinit(max31855_device_t dev);
max31855_device_t max31855_init(const char *spi_device_name);
float max31855_read_internal_temp(max31855_device_t dev);
float max31855_read_thermocouple_temp(max31855_device_t dev);

#endif /* __DRV_MAX31855_H__ */
