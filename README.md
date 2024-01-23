# Wayland_learn

## 安装环境

安装Wayland合成器

```shell
sudo apt install weston
```

安装Wayland依赖和开发库

```shell
sudo apt install libxml2 libxml2-dev xdot xmlto libjpeg-dev libwebp-dev libsystemd-dev liblcms2-dev libegl-mesa0 libgbm-dev freerdp2-dev 
libx11-xcb-dev libxcb-composite0 libcolord-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libpipewire-0.3-dev
sudo apt install libwayland-dev libwayland-server0 libwayland-cursor0 libwayland-egl1
```

安装Wayland调试器

```shell
git clone https://github.com/mchalupa/wldbg.git
```

自行编译安装

## 使用wayland-scanner生成协议的C文件

```shell
wayland-scanner private-code ~/dev/source_code/Wayland/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.c
wayland-scanner client-header ~/dev/source_code/Wayland/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
```

## 参考资料

1. [Wayland开发入门系列索引](https://zhuanlan.zhihu.com/p/423462310)

2. [Wayland编程](https://www.zhihu.com/column/c_1670440875476942848)