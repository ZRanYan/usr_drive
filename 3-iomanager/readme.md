# GPIO独立测试命令

测试命令

```bash
gpioinfo /dev/gpiochip0 | grep PH.00
```

自定义配置管脚参数

```bash
gpioset /dev/gpiochip0 43=1
```

获取管脚的电平参数

```bash
gpioget /dev/gpiochip0 43
```

# 驱动的版本信息读取
```bash
cat /sys/module/iomanager/version  #驱动代码里面的BLIOMANAGER_VER宏定义
V0.1.0 (e5279ce on master 2025-12-22 11:57:48)
```

# debugfs的文件夹

 在位置`/sys/kernel/debug/iomanager_driver`
```bash
cat /sys/kernel/debug/iomanager_driver/irq_count #获取中断的计数
rgb_mode:1          #rgb的灯模式，0-代表仅仅DLP亮灯，1-代表DLP+RGB联动闪烁出图
xtrig_num:10        #统计xtrig的数量
tou_num:10          #统计tou0的数量
rgbTrigIndex:17     #开始触发rgb的开始，等xtrig管脚到达这个值的时候开始rgb灯闪烁
totalTrigIndex:21   #当达到tou0管脚达到这个数的时候，唤醒队列检查这个xtrig_num和tou_num，并更新状态值
cat /sys/kernel/debug/iomanager_driver/io_index #获取io驱动加载状态
rgbPwm:ok           #代表设备树里面有rgb的pwm配置节点
num_dlp:4           #设备树里面的dlp数量
num_usb:1           #设备树里面的usb控制管脚数量
num_inputs:4        #输入管脚的数量
num_outputs:4       #输出管脚的数量
num_flash:3         #led闪烁的数量
cat /sys/kernel/debug/iomanager_driver/2D_irq_count #获取2d管脚中断输入数量，写1清零:echo 1 > /sys/kernel/debug/iomanager_driver/2D_irq_count
2d input count:0
cat /sys/kernel/debug/iomanager_driver/3D_irq_count #获取2d管脚中断输入数量，写1清零
3d input count:0

cat /sys/kernel/debug/iomanager_driver/irq_set # 获取中断的开关的和配置参数
2D irq num:254 isEnable:0   #2d的开关
3D irq num:255 isEnable:0   
xtrig irq num:256 isEnable:0
tout0 irq num:257 isEnable:0

```






