#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

#include "inc/core/defs.h"
#include "inc/listeners.h"
#include "inc/utils.h"

extern struct wl_registry_listener registry_listener;
extern struct wl_surface_listener surface_listener;
extern struct wl_shell_surface_listener shell_surface_listener;
extern struct wl_shm_listener shm_listener;
extern struct wl_seat_listener seat_listener;
extern struct wl_pointer_listener pointer_listener;
extern struct wl_keyboard_listener keyboard_listener;
extern struct wl_touch_listener touch_listener;
extern struct wl_callback_listener callback_listener;

static uint32_t pixel_rgb;

struct wl_display* connect_to_server(const char* name) {
  struct wl_display* dpy = wl_display_connect(name);
  ERROR_CHECK(dpy, "Can't open wayland compositor", "Connect OK");
  return dpy;
}

/* ----------------------------------- 注册表 ----------------------------------
 */

void on_global_registry_added(void* data, struct wl_registry* registry,
                              uint32_t id, const char* interface,
                              uint32_t version) {
  printf("Global add: %s , version: %u, name: %u\n", interface, version, id);

  if (strcmp(interface, "wl_compositor") == 0) {
    compositor =
        wl_registry_bind(registry, id, &wl_compositor_interface, version);
    ERROR_CHECK(compositor, "Can't find wl_compositor", "Found wl_compositor");
  }
  // else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
  //   xdg_shell = wl_registry_bind(registry, id, &xdg_wm_base_interface,
  //   version); ERROR_CHECK(xdg_shell, "Can't find xdg_shell", "Found
  //   xdg_shell");
  // }
  else if (strcmp(interface, "wl_shell") == 0) {
    shell = wl_registry_bind(registry, id, &wl_shell_interface, version);
    ERROR_CHECK(shell, "Can't find wl_shell", "Found wl_shell");
  } else if (strcmp(interface, "wl_shm") == 0) {
    shm = wl_registry_bind(registry, id, &wl_shm_interface, version);
    ERROR_CHECK(shm, "Can't find wl_shm", "Found wl_shm");
    wl_shm_add_listener(shm, &shm_listener, NULL);
    cursor_theme = wl_cursor_theme_load(NULL, 32, shm);
    default_cursor = wl_cursor_theme_get_cursor(cursor_theme, "left_ptr");
  } else if (strcmp(interface, "wl_seat") == 0) {
    seat = wl_registry_bind(registry, id, &wl_seat_interface, version);
    ERROR_CHECK(seat, "Can't find wl_seat", "Found wl_seat");
    wl_seat_add_listener(seat, &seat_listener, NULL);
  }
}

void on_global_registry_removed(void* data, struct wl_registry* registry,
                                uint32_t name) {
  printf("Global remove: name: %u\n", name);
}

struct wl_registry* init_registry(struct wl_display* dpy) {
  struct wl_registry* reg = wl_display_get_registry(dpy);
  ERROR_CHECK(reg, "Can't get registry", "Get registry OK");
  static struct wl_registry_listener reg_listener = {
      .global = on_global_registry_added,
      .global_remove = on_global_registry_removed};
  wl_registry_add_listener(reg, &reg_listener, NULL);
  return reg;
}

void paint_pixels_into_shm_data(int width, int height, enum color_enum color1,
                                enum color_enum color2) {
  if (color1 > color2) {
    perror("color2 should bigger than color1\n");
    return;
  }
  static bool flag = false;
  if (!flag) {
    pixel_rgb = color1;
    flag = true;
  }
  uint32_t* pixel =
      (uint32_t*)shm_data;  // 这里强转一下，因为后面用到了指针运算
  for (int i = 0; i < width * height; ++i) {
    *pixel++ = (pixel_rgb | transpranet << 24);  // 向共享内存中写入像素值
  }
  // 每个RGB分量增加1
  pixel_rgb += 0x010101;
  if (pixel_rgb >= color2) {
    pixel_rgb = color1;
  }
}

struct wl_buffer* create_buffer(uint width, uint height) {
  int stride = width * 4;  // R G B Alpha，每个像素4B
  int size = stride * height;
  int fd = os_create_anonymous_file(size);  // 返回一个匿名文件的文件描述符
  ASSERT_MSG(fd >= 0, "creating a buffer file for %d B failed: %m", size);
  // 将创建的匿名文件映射到内存中并设置文件保护标志，使之成为共享内存（可读可写）
  shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm_data == MAP_FAILED) {
    fprintf(stderr, "mmap failed: %m\n");
    close(fd);
    exit(EXIT_FAILURE);
  }
  // 创建一个客户端使用的共享内存池对象，用来在共享内存上分配缓冲区内存
  struct wl_shm_pool* pool = wl_shm_create_pool(shm, fd, size);
  // 通过共享内存创建缓冲区
  struct wl_buffer* buff =
      wl_shm_pool_create_buffer(pool, 0, width, height, stride, shm_format);
  wl_buffer_add_listener(buff, &buffer_listener, NULL);
  wl_shm_pool_destroy(pool);  // 这个共享内存池用不到了，可以释放
  return buff;
}

void create_window(int x, int y, int width, int height) {
  buffer = create_buffer(width, height);
  // 1. 将缓冲区对应到窗口的特定位置
  wl_surface_attach(surface, buffer, x, y);
  // 2. 标记窗口失效需要重绘的区域
  // wl_surface_damage(surface, x, y, width, height);
  // 3. 提交挂起的缓冲区内容到合成器
  wl_surface_commit(surface);
}

int main() {
  // Wayland 根据XDG_RUNTIME_DIR来确定
  fprintf(stderr, "XDG_RUNTIME_DIR= %s\n", getenv("XDG_RUNTIME_DIR"));
  fprintf(stderr, "WAYLAND_DISPLAY= %s\n", getenv("WAYLAND_DISPLAY"));
  struct wl_display* dpy = connect_to_server(NULL);
  struct wl_registry* reg = init_registry(dpy);

  wl_display_roundtrip(dpy);  // 等待服务器完成发送过去的全部请求的响应
  wl_display_dispatch(dpy);  // 在事件队列中读取事件并对其进行排队

  surface = wl_compositor_create_surface(compositor);
  ERROR_CHECK(surface, "Can't create surface", "Created surface");
  wl_surface_add_listener(surface, &surface_listener, NULL);
  cursor_surface = wl_compositor_create_surface(compositor);
  ERROR_CHECK(surface, "Can't create cursor_surface", "Created cursor_surface");

  // struct xdg_surface* shell_surface =
  //     xdg_wm_base_get_xdg_surface(xdg_shell, surface);
  // ERROR_CHECK(shell_surface, "Can't create shell surface",
  //             "Created shell surface");
  // struct xdg_toplevel* toplevel = xdg_surface_get_toplevel(shell_surface);
  // xdg_surface_add_listener(shell_surface, &surface_listener, NULL);

  shell_surface = wl_shell_get_shell_surface(shell, surface);
  ERROR_CHECK(surface, "Can't get shell_surface", "Got shell_surface");
  wl_shell_surface_set_toplevel(shell_surface);
  wl_shell_surface_add_listener(shell_surface, &shell_surface_listener, NULL);

  frame_callback = wl_surface_frame(surface);
  wl_callback_add_listener(frame_callback, &frame_callback_listener, NULL);

  xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

  create_window(0, 0, 480, 360);

  // on_frame_redraw(NULL, NULL, 0);

  int num = 0;
  while ((num = wl_display_dispatch(dpy)) != -1) {
    // printf("Client receive %d event!\n", num);
  }

  // xdg_surface_destroy(shell_surface);
  // xdg_wm_base_destro
  // 在服务器端，当相关的 wl_surface

  xkb_context_unref(xkb_context);
  // 被销毁时，wl_shell_surface 对象会自动销毁。在客户端，必须在销毁 wl_surface
  // 对象之前调用 wl_shell_surface_destroy()
  wl_shell_surface_destroy(shell_surface);
  wl_shell_destroy(shell);
  wl_surface_destroy(surface);
  wl_compositor_destroy(compositor);
  wl_buffer_destroy(buffer);
  wl_registry_destroy(reg);
  wl_display_disconnect(dpy);
  printf("disconnected from server\n");
  return 0;
}