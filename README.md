# Wayland_learn

## 安装环境

安装Wayland合成器

```shell
sudo apt install weston
```

安装Wayland依赖和开发库

```shell
sudo apt install libxml2 libxml2-dev xdot xmlto libjpeg-dev libwebp-dev libsystemd-dev liblcms2-dev libegl-mesa0 libgbm-dev freerdp2-dev libx11-xcb-dev libxcb-composite0 libcolord-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libpipewire-0.3-dev
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

## 编译运行

如果出现段错误，debug看一下这些全局对象的代理是否申请成功。我设的初值都是 `NULL` ，因此如果没有申请到，就会是 `0X00000` 。据此判断当前安装的 Wayland 协议是否支持。或者直接在 `wl_display_get_registry` 中打印目前合成器广播的全局对象有哪些。

此外，段错误常见的还有 `listener function for opcode X of XXX is NULL` 这种，这是因为 `XXX` 对象的回调没有写全，导致RPC调用对应偏移量的回调时发现是空指针。还可以通过 `wl_registry_bind` 的 `version` 字段来指定更低的协议版本，这样就可以免去一些回调。

## 参考资料

1. [Wayland开发入门系列索引](https://zhuanlan.zhihu.com/p/423462310)

2. [Wayland编程](https://www.zhihu.com/column/c_1670440875476942848)