### 使用wayland-scanner生成协议的C文件

```shell
wayland-scanner private-code ~/dev/source_code/Wayland/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.c
wayland-scanner client-header ~/dev/source_code/Wayland/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
```