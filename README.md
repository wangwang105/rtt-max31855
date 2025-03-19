# max31855 软件包

## 简介

max31855 软件包提供了使用温度传感器 `max31855` 基本功能，并且本软件包新的版本已经对接到了 Sensor 框架，通过 Sensor 框架，开发者可以快速的将此传感器驱动起来。

| 功能 | 热电偶类型 | 量程            | 精度     |
| ---- | ---------- | --------------- | -------- |
| 温度 | K          | `-200℃ - 1350℃` | `±0.25℃` |

## 支持情况

| 包含设备     | 温度 |
| ------------ | ---- |
| **通信接口** |      |
| SPI          | √    |
| **工作模式** |      |
| 轮询         | √    |
| 中断         |      |
| FIFO         |      |

## 使用说明

### 依赖

- RT-Thread 5.0.0+
- Sensor 组件
- SPI 驱动：max31855 设备使用 SPI 进行数据通讯，需要系统 SPI 驱动支持

### 获取软件包

使用 max31855 软件包需要在 RT-Thread 的包管理中选中它，具体路径如下：

```
RT-Thread online packages  --->
  peripheral libraries and drivers  --->
    sensors drivers  --->
          max31855: a package of digital temperature sensor.
               Version (latest)  --->
```

**Version**：软件包版本选择，默认选择最新版本。

### 使用软件包

max31855 软件包初始化函数如下所示：

```c
int rt_hw_max31855_init(const char *name, struct rt_sensor_config *cfg)；
```

该函数需要由用户调用，函数主要完成的功能有，

- 设备配置和初始化（根据传入的配置信息配置接口设备）；
- 注册相应的传感器设备，完成 max31855 设备的注册；
- 使用`rt_device_read`函数读取传感器数据。

#### 初始化示例

```c
#include "maxim_max31855_sensor_v1.h"

#define MAX667_SPI_BUS  "spi30"

int rt_hw_max31855_port(void)
{
    struct rt_sensor_config cfg;
    struct rt_spi_device *spi_device[1];

    spi_device[0] = (struct rt_spi_device *)rt_calloc(1, sizeof(struct rt_spi_device));

    ret = rt_spi_bus_attach_device_cspin(spi_device[0], MAX667_SPI_BUS, "spi3", GET_PIN(E, 7), RT_NULL);

    cfg.intf.dev_name  = MAX667_SPI_BUS;

    rt_hw_max31855_init("max31855", &cfg);

    return RT_EOK;
}
INIT_ENV_EXPORT(rt_hw_max31855_port);
```

#### 使用示例

```c
int main(void)
{
    rt_device_t           dev = RT_NULL;
    struct rt_sensor_data sensor_data;
    rt_size_t             res;
    dev = rt_device_find("temp_max31855");
    if (dev == RT_NULL)
    {
        rt_kprintf("Can't find device:temp_max31855\n");
        return;
    }
    if (rt_device_open(dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("open device failed!\n");
        return;
    }


    while (1)
    {
        res = rt_device_read(dev, 0, &sensor_data, 1);
        if (res != 1)
        {
            LOG_E("read data failed!size is %d\n", res);
        }
        else
        {
            LOG_D("temp:%d(x100) , timestamp:%5d\n", sensor_data.data.temp, sensor_data.timestamp);
        }
        float internal = 0.0;
        res            = rt_device_control(dev, RT_SENSOR_CTRL_SELF_TEST, &internal);
        if (res == RT_EOK)
        {
            LOG_D("internal:%f", internal);
        }
        else
        {
            LOG_E("read internal temp failed!error:%d\n", res);
        }

        rt_thread_mdelay(1000);
    }
}
```

#### 宏定义
```c
#define PKG_USING_MAX31855
```

### 注意事项

暂无。

## 注意事项

暂无。

## 联系人信息

维护人:

- 维护：[wangwang105](https://github.com/wangwang105)
- 主页：https://github.com/wangwang105/rtt-max31855
