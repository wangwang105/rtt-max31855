#include <rtthread.h>
#include <rtdevice.h>

#include "max31855.h"

#define DBG_SECTION_NAME "max31855"
#define DBG_LEVEL        DBG_ERROR
#define DBG_COLOR
#include <rtdbg.h>

#define MAX31855_FAULT_NONE      (0x00)       ///< Disable all fault checks
#define MAX31855_FAULT_OPEN      (0x01)       ///< Enable open circuit fault check
#define MAX31855_FAULT_SHORT_GND (0x02)       ///< Enable short to GND fault check
#define MAX31855_FAULT_SHORT_VCC (0x04)       ///< Enable short to VCC fault check
#define MAX31855_FAULT_ALL       (0x00010000) ///< Enable all fault checks

static rt_err_t read_data(struct rt_spi_device *dev, rt_uint8_t *buf)
{
    rt_uint8_t dummy[4] = {0};
    if (rt_spi_transfer(dev, dummy, buf, 4) != 4)
    {
        LOG_E("SPI transfer failed");
    }
    return RT_EOK;
}

float max31855_read_internal_temp(max31855_device_t dev)
{
    rt_uint8_t  data[4]  = {0};
    rt_uint32_t raw      = 0;
    float       internal = 0.0;
    rt_err_t    result;

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        read_data(dev->spi, (rt_uint8_t *)data);
        LOG_D("data:0x%04x", data);

        raw = ((rt_uint32_t)data[0] << 24) | ((rt_uint32_t)data[1] << 16) | ((rt_uint32_t)data[2] << 8) | ((rt_uint32_t)data[3]);

        rt_int16_t internal_data = (raw >> 4) & 0x0FFF;
        if (internal_data & 0x0800)  // 检查符号位（第 12 位）
        {
            internal_data |= 0xF000; // 进行符号扩展
        }
        internal = internal_data * 0.0625f;
    }
    else
    {
        LOG_E("Failed to reading internal tempurature");
    }
    rt_mutex_release(dev->lock);
    LOG_D("internal:%f", internal);
    return internal;
}

float max31855_read_thermocouple_temp(max31855_device_t dev)
{
    rt_uint8_t  data[4]      = {0};
    rt_uint32_t raw          = 0;
    float       thermocouple = 0.0;
    rt_err_t    result;

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        read_data(dev->spi, (rt_uint8_t *)data);
        LOG_D("data:0x%04x", data);

        raw = ((rt_uint32_t)data[0] << 24) | ((rt_uint32_t)data[1] << 16) | ((rt_uint32_t)data[2] << 8) | ((rt_uint32_t)data[3]);

        if (raw & MAX31855_FAULT_ALL)
        {
            // uh oh, a serious problem!
            LOG_I("Thermocouple Fault: ");
            if (raw & MAX31855_FAULT_OPEN)
            {
                LOG_I("Open Circuit");
                rt_mutex_release(dev->lock);
                return -500;
            }
            if (raw & MAX31855_FAULT_SHORT_GND)
            {
                LOG_I("Short to GND");
                rt_mutex_release(dev->lock);
                return -600;
            }
            if (raw & MAX31855_FAULT_SHORT_VCC)
            {
                LOG_I("Short to VCC");
                rt_mutex_release(dev->lock);
                return -700;
            }

        }

        rt_int16_t temp_data = (raw >> 18) & 0x3FFF;
        /* 温度数据可能为负，需要进行符号扩展 */
        if (temp_data & 0x2000)
        {
            temp_data |= 0xC000;
        }

        thermocouple = temp_data * 0.25f;
    }
    else
    {
        LOG_E("Failed to reading thermocouple tempurature");
    }
    rt_mutex_release(dev->lock);
    LOG_D("thermocouple:%f", thermocouple);
    return thermocouple;
}

void max31855_deinit(max31855_device_t dev)
{
    RT_ASSERT(dev);

    rt_mutex_delete(dev->lock);

    rt_free(dev);
}


max31855_device_t max31855_init(const char *spi_device_name)
{
    max31855_device_t dev;

    RT_ASSERT(spi_device_name);

    dev = rt_calloc(1, sizeof(struct max31855_device));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for max31855 device on '%s'", spi_device_name);
        return RT_NULL;
    }

    dev->spi = SPI_DEVICE(rt_device_find(spi_device_name));
    if (dev->spi == RT_NULL)
    {
        LOG_E("Can't find SPI device '%s' for max31855", spi_device_name);
        rt_free(dev);
        return RT_NULL;
    }


    // ret = rt_spi_configure(spi_device[i], &cfg);
    struct rt_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.mode       = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz     = 4 * 1000 * 1000; /* 20M */

    rt_spi_configure(dev->spi, &cfg);


    /* 为每个设备生成唯一的互斥锁名称 */
    char mutex_name[32] = {0};
    rt_snprintf(mutex_name, sizeof(mutex_name), "mutex_max31855_%s", spi_device_name);
    dev->lock = rt_mutex_create(mutex_name, RT_IPC_FLAG_FIFO);
    if (dev->lock == RT_NULL)
    {
        LOG_E("Can't create mutex for max31855 device on '%s'", spi_device_name);
        rt_free(dev);
        return RT_NULL;
    }

    return dev;
}


// max31855_device_t max31855_init(const char *spi_device_name)
// {
//     max31855_device_t dev;

//     RT_ASSERT(spi_device_name);

//     dev = rt_calloc(1, sizeof(struct max31855_device));
//     if (dev == RT_NULL)
//     {
//         LOG_E("Can't allocate memory for max31855 device on '%s' ", spi_device_name);
//         rt_free(dev);

//         return RT_NULL;
//     }
//     dev->spi = SPI_DEVICE(rt_device_find(spi_device_name));

//     if (dev->spi == RT_NULL)
//     {
//         LOG_E("Can't find max31855 device on '%s'", spi_device_name);
//         rt_free(dev);
//         return RT_NULL;
//     }
//     dev->lock = rt_mutex_create("mutex_max31855", RT_IPC_FLAG_FIFO);
//     if (dev->lock == RT_NULL)
//     {
//         LOG_E("Can't create mutex for max31855 device on '%s'", spi_device_name);
//         rt_free(dev);
//         return RT_NULL;
//     }

//     return dev;
// }

