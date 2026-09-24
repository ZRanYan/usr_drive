
insmod nv_imx566.ko modeType=1

## 查询相关设备节点的udev属性
```bash
udevadm info --query=all --name=/dev/video0
udevadm info --query=all --name=/dev/video1

udevadm info --query=all --name=/dev/v4l-subdev0
udevadm info --query=all --name=/dev/v4l-subdev1

udevadm info --query=all --name=/dev/v4l-subdev2
udevadm info --query=all --name=/dev/v4l-subdev3

```

## 替换
```bash
sudo cp -f 99-camera.rules /etc/udev/rules.d/99-camera.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```



