#include "maxim_max31855_sensor_v1.h"
#include "max31855.h"

#define DBG_SECTION_NAME "sensor.maxim.max31855"
#define DBG_LEVEL        DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

// K type thermocouple temperature range
#define SENSOR_TEMP_RANGE_MAX (1350)
#define SENSOR_TEMP_RANGE_MIN (-200)

#ifndef RT_SENSOR_VENDOR_MAXIM
#define RT_SENSOR_VENDOR_MAXIM (8u)
#endif

static rt_size_t _max31855_polling_get_data(rt_sensor_t sensor, struct rt_sensor_data *data)
{
    float                   temperature;
    struct max31855_device *max31855_dev = sensor->parent.user_data;

    if (sensor->info.type == RT_SENSOR_CLASS_TEMP)
    {
        temperature     = max31855_read_thermocouple_temp(max31855_dev);
        data->data.temp = (rt_int32_t)(temperature * 100);
        data->timestamp = rt_sensor_get_ts();
    }
    else
        return 0;
    return 1;
}

static RT_SIZE_TYPE max31855_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    RT_ASSERT(buf);

    if (sensor->config.mode == RT_SENSOR_MODE_POLLING)
    {
        return _max31855_polling_get_data(sensor, buf);
    }
    else
        return 0;
}


static rt_err_t max31855_control(struct rt_sensor_device *sensor, int cmd, void *arg)
{
    rt_err_t ret = RT_EOK;

    switch (cmd)
    {
    case RT_SENSOR_CTRL_GET_ID: {
        LOG_E("Unsupported sensor control command: %d\n", cmd);
        // /* 假设 arg 为 int 类型指针，用来存放传感器的ID */
        // int *id = (int *)arg;
        // if (id == RT_NULL)
        // {
        //     ret = -RT_EINVAL;
        //     break;
        // }
        // *id = sensor->info.id;  // 这里假设 sensor->info.id 已经初始化
        // rt_kprintf("Sensor ID: %d\n", *id);
        break;
    }
    case RT_SENSOR_CTRL_SET_RANGE: {
        LOG_E("Unsupported sensor control command: %d\n", cmd);
        /* arg 指向量程参数 */
        // int range = *(int *)arg;
        // rt_kprintf("Setting sensor range to: %d\n", range);
        // 在此处根据传感器的硬件寄存器或内部变量设置量程
        // sensor->config.range = range;
        break;
    }
    case RT_SENSOR_CTRL_SET_ODR: {
        LOG_E("Unsupported sensor control command: %d\n", cmd);
        /* arg 指向采样率参数 */
        // int odr = *(int *)arg;
        // rt_kprintf("Setting sensor ODR to: %d Hz\n", odr);
        // 设置采样率，例如修改传感器内部寄存器
        // sensor->config.odr = odr;
        break;
    }
    case RT_SENSOR_CTRL_SET_MODE: {
        LOG_E("Unsupported sensor control command: %d\n", cmd);
        /* arg 指向工作模式参数 */
        // int mode = *(int *)arg;
        // rt_kprintf("Setting sensor mode to: %d\n", mode);
        // 根据不同模式配置传感器，如连续工作或单次测量模式
        // sensor->config.mode = mode;
        break;
    }
    case RT_SENSOR_CTRL_SELF_TEST: {
        // LOG_E("Unsupported sensor control command: %d\n",cmd);
        struct max31855_device *max31855_dev = sensor->parent.user_data;
        float                  *temperature  = (float *)arg;
        *temperature                         = max31855_read_internal_temp(max31855_dev);

        /* 传感器自检：假设自检结果通过 arg 返回 */
        // 示例：* (int *)arg = sensor_self_test();
        // rt_kprintf("Sensor self-test executed\n");
        break;
    }
    default: {
        rt_kprintf("Unsupported sensor control command: %d\n", cmd);
        ret = -RT_EINVAL;
        break;
    }
    }

    return ret;
}

static struct rt_sensor_ops sensor_ops =
    {
        max31855_fetch_data,
        max31855_control};

int rt_hw_max31855_init(const char *name, struct rt_sensor_config *cfg)
{
    rt_int8_t               result;
    rt_sensor_t             sensor_temp = RT_NULL;
    struct max31855_device *max31855_dev;

    max31855_dev = max31855_init(cfg->intf.dev_name);
    if (max31855_dev == RT_NULL)
    {
        LOG_E("max31855 init failed on '%s'", cfg->intf.dev_name);
        return -RT_ERROR;
    }
    sensor_temp = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_temp == RT_NULL)
    {
        LOG_E("Can't allocate memory for max31855 sensor device on '%s'", cfg->intf.dev_name);
        max31855_deinit(max31855_dev);
        return -RT_ERROR;
    }
    sensor_temp->info.type      = RT_SENSOR_CLASS_TEMP;
    sensor_temp->info.vendor    = RT_SENSOR_VENDOR_MAXIM;
    sensor_temp->info.model     = "max31855";
    sensor_temp->info.unit      = RT_SENSOR_UNIT_DCELSIUS;
    sensor_temp->info.intf_type = RT_SENSOR_INTF_SPI;
    sensor_temp->info.range_max = SENSOR_TEMP_RANGE_MAX;
    sensor_temp->info.range_min = SENSOR_TEMP_RANGE_MIN;

    rt_memcpy(&sensor_temp->config, cfg, sizeof(struct rt_sensor_config));
    sensor_temp->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_temp, name, RT_DEVICE_FLAG_RDONLY, max31855_dev);

    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    return RT_EOK;

__exit:
    if (sensor_temp)
        rt_free(sensor_temp);
    return -RT_ERROR;
}
