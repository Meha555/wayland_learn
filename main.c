#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

#include "inc/core/defs.h"
#include "inc/listeners.h"
#include "inc/utils.h"
#include "xdg-shell-client-protocol.h"

bool running = true;

static uint32_t pixel_rgb;

/**
 * @description: 在共享内存中填充像素
 * @return {*}
 */
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
      (uint32_t*)g_window.shm.shm_data;  // 这里强转一下，因为后面用到了指针运算
  for (int i = 0; i < width * height; ++i) {
    *pixel++ = (pixel_rgb);  // | (transpranet << 24) 向共享内存中写入像素值
  }
  // 每个RGB分量增加1
  pixel_rgb += 0x010101;
  if (pixel_rgb >= color2) {
    pixel_rgb = color1;
  }
}

// void draw_track(int x, int y) {
//   uint32_t* pixel = (uint32_t*)g_window.shm.shm_data + (x + y * WIDTH);
//   int bursh = 10;
//   if(cursor_buffer == NULL) cursor_buffer = create_buffer(bursh, bursh);
//   if ((x >= 0 && x <= WIDTH) && (y >= 0 && y <= HEIGHT - bursh)) {
//     for (int yi = 0; yi < bursh; ++yi) {
//       for (int xi = 0; xi < bursh; ++xi) {
//         *pixel++ = (0xC0 << 24 | 0xFF << 16);
//       }
//     }
//   }
//   wl_surface_attach(cursor_surface, cursor_buffer, x, y);
//   wl_surface_damage(cursor_surface, x, y, bursh, bursh);
//   wl_surface_commit(cursor_surface);
// }

/**
 * @description: 分配指定大小的缓冲区，采用的是pool与buffer 1:1的大小
 * @param {int} width
 * @param {int} height
 * @return {wl_buffer*} 缓冲区句柄
 */
struct wl_buffer* create_buffer(int width, int height) {
  g_window.scale.width = width, g_window.scale.height = height;
  // 1. 计算需要申请的缓冲区大小
  int stride = width * 4;  // R G B Alpha，每个像素4B
  int size = stride * height;
  // 2. 分配一个匿名文件
  int fd = os_create_anonymous_file(size);  // 返回一个匿名文件的文件描述符
  ASSERT_MSG(fd >= 0, "creating a buffer file for %d B failed: %m", size);
  // 3.
  // 将创建的匿名文件映射到内存中并设置文件保护标志，使之成为共享内存（可读可写）
  g_window.shm.shm_data =
      mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (g_window.shm.shm_data == MAP_FAILED) {
    fprintf(stderr, "mmap failed: %m\n");
    close(fd);
    exit(EXIT_FAILURE);
  }
  // 4. 创建一个客户端使用的共享内存池对象，用来在共享内存上分配缓冲区内存
  struct wl_shm_pool* pool = wl_shm_create_pool(g_window.shm.shm, fd, size);
  // 5. 通过共享内存创建缓冲区
  struct wl_buffer* buff =
      wl_shm_pool_create_buffer(pool, 0, width, height, stride,g_window.shm.shm_format);
  wl_buffer_add_listener(buff, &buffer_listener, NULL);
  wl_shm_pool_destroy(
      pool);  // 由于我采用的是pool与buffer 1:1
              // 的大小对应关系，因此不会涉及pool大小的裁剪，因此pool只能用一次
  return buff;
}

/**
 * @description: 绘制一个surface
 * @param {int} x
 * @param {int} y
 * @param {int} width
 * @param {int} height
 */
void draw_surface(int x, int y, int width, int height) {
  g_window.scale.width = width, g_window.scale.height = height;
  // 1. 创建用于绘图的缓冲区
  if (g_window.shm.buffer == NULL || g_window.scale.width != width || g_window.scale.height != height)
    g_window.shm.buffer = create_buffer(width, height);
  // 2. 在缓冲区上绘图
  on_frame_redraw(NULL, NULL, 0);
}

int main() {
  // Wayland 根据XDG_RUNTIME_DIR来确定
  fprintf(stderr, "XDG_RUNTIME_DIR= %s\n", getenv("XDG_RUNTIME_DIR"));
  fprintf(stderr, "WAYLAND_DISPLAY= %s\n", getenv("WAYLAND_DISPLAY"));
  struct wl_display* dpy = wl_display_connect(NULL);
  ERROR_CHECK(dpy, "Can't open wayland compositor", "Connect OK");
  struct wl_registry* reg = wl_display_get_registry(dpy);
  ERROR_CHECK(reg, "Can't get registry", "Get registry OK");

  wl_registry_add_listener(reg, &reg_listener, NULL);

  wl_display_roundtrip(
      dpy);  // 等待服务器完成发送过去的全部请求的响应【因为我们申请了全局对象的代理】
  wl_display_dispatch(dpy);  // REVIEW - 不刷新颜色显示有问题，不懂

  xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

  // 用compositor来创建出surface
  g_window.surface.surface = wl_compositor_create_surface(g_compositor);
  ERROR_CHECK(g_window.surface.surface, "Can't create surface", "Created surface");
  wl_surface_add_listener(g_window.surface.surface, &surface_listener, NULL);

  // 用compositor来创建cursor_surface
  cursor_surface = wl_compositor_create_surface(g_compositor);
  ERROR_CHECK(cursor_surface, "Can't create cursor_surface",
              "Created cursor_surface");
  // // 将cursor_surface的角色设置为surface的子窗口
  // sub_surface = wl_subcompositor_get_subsurface(sub_compositor,
  // cursor_surface, g_window.surface.surface);
  // // 设置一些属性
  // wl_subsurface_set_position(sub_surface, 0, 0);
  // wl_subsurface_place_above(sub_surface, g_window.surface.surface);
  // wl_subsurface_set_sync(sub_surface);

  wl_display_roundtrip(dpy);  // 等待服务器完成发送过去的全部请求的响应

#ifdef HAS_XDG_
  // 用xdg_wm_base来用wl_surface创建出xdg_surface
  g_window.surface.shell_surface = xdg_wm_base_get_xdg_surface(g_window.surface.shell, g_window.surface.surface);
  ERROR_CHECK(g_window.surface.shell_surface, "Can't create shell surface",
              "Created shell surface");
  xdg_surface_add_listener(g_window.surface.shell_surface, &shell_surface_listener, NULL);
  // 获得顶层窗口
  g_window.surface.toplevel = xdg_surface_get_toplevel(g_window.surface.shell_surface);
  xdg_toplevel_set_title(g_window.surface.toplevel, "Wayland demo");
  xdg_toplevel_add_listener(g_window.surface.toplevel, &toplevel_listener, NULL);
#else
  // 用shell来创建出shell_surface
  g_window.surface.shell_surface = wl_shell_get_shell_surface(g_window.surface.shell, g_window.surface.surface);
  ERROR_CHECK(g_window.surface.shell_surface, "Can't get g_window.surface.shell_surface", "Got g_window.surface.shell_surface");
  wl_shell_surface_set_toplevel(g_window.surface.shell_surface);  // 指定surface角色为toplevel
  wl_shell_surface_add_listener(g_window.surface.shell_surface, &shell_surface_listener, NULL);
#endif

  g_window.surface.frame_callback = wl_surface_frame(g_window.surface.surface);
  wl_callback_add_listener(g_window.surface.frame_callback, &frame_callback_listener, NULL);

  wl_surface_commit(g_window.surface.surface);

  draw_surface(0, 0, 480, 360);

  // 在事件队列中读取事件并对其进行排队
  int num = 0;
  while ((num = wl_display_dispatch(dpy)) != -1) {
    // printf("Client receive %d event!\n", num);
    if (running == false) break;
  }

  xkb_context_unref(xkb_context);

#ifdef HAS_XDG_
  xdg_toplevel_destroy(g_window.surface.toplevel);
  xdg_surface_destroy(g_window.surface.shell_surface);
  xdg_wm_base_destroy(g_window.surface.shell);
#else
  // 被销毁时，wl_shell_surface 对象会自动销毁。在客户端，必须在销毁 wl_surface
  // 对象之前调用 wl_shell_surface_destroy() 这样就模拟了装饰器模式的析构
  wl_shell_surface_destroy(g_window.surface.shell_surface);
  wl_shell_destroy(g_window.surface.shell);
#endif
  // wl_subsurface_destroy(sub_surface);
  // wl_subcompositor_destroy(wl_sub_compositor);
  // wl_buffer_destroy(cursor_buffer);
  wl_surface_destroy(g_window.surface.surface);
  wl_compositor_destroy(g_compositor);
  wl_buffer_destroy(g_window.shm.buffer);
  wl_registry_destroy(reg);
  wl_display_disconnect(dpy);
  printf("disconnected from server\n");
  return 0;
}