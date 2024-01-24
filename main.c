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

bool running = true;

static uint32_t pixel_rgb;

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
      (uint32_t*)surface_shm_data;  // 这里强转一下，因为后面用到了指针运算
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
//   uint32_t* pixel = (uint32_t*)surface_shm_data + (x + y * WIDTH);
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

struct wl_buffer* create_buffer(int width, int height) {
  int stride = width * 4;  // R G B Alpha，每个像素4B
  int size = stride * height;
  int fd = os_create_anonymous_file(size);  // 返回一个匿名文件的文件描述符
  ASSERT_MSG(fd >= 0, "creating a buffer file for %d B failed: %m", size);
  // 将创建的匿名文件映射到内存中并设置文件保护标志，使之成为共享内存（可读可写）
  surface_shm_data =
      mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (surface_shm_data == MAP_FAILED) {
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

void draw_window(int x, int y, int width, int height) {
  // 1. 创建用于绘图的缓冲区
  if (buffer == NULL) buffer = create_buffer(width, height);
  on_frame_redraw(NULL, NULL, 0);  // 这里用这个函数画一下图
  // 2. 将缓冲区对应到窗口的特定位置
  wl_surface_attach(surface, buffer, x, y);
  // 3. 标记窗口失效需要重绘的区域
  // wl_surface_damage(surface, x, y, width, height);
  // 4. 提交挂起的缓冲区内容到合成器
  wl_surface_commit(surface);
}

int main() {
  // Wayland 根据XDG_RUNTIME_DIR来确定
  fprintf(stderr, "XDG_RUNTIME_DIR= %s\n", getenv("XDG_RUNTIME_DIR"));
  fprintf(stderr, "WAYLAND_DISPLAY= %s\n", getenv("WAYLAND_DISPLAY"));
  struct wl_display* dpy = wl_display_connect(NULL);
  ERROR_CHECK(dpy, "Can't open wayland compositor", "Connect OK");
  struct wl_registry* reg = wl_display_get_registry(dpy);
  ERROR_CHECK(reg, "Can't get registry", "Get registry OK");

  xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  
  wl_registry_add_listener(reg, &reg_listener, NULL);

  wl_display_roundtrip(dpy);  // 等待服务器完成发送过去的全部请求的响应
  wl_display_dispatch(dpy);  // REVIEW - 不刷新颜色显示有问题，不懂

  // 用compositor来创建出surface
  surface = wl_compositor_create_surface(compositor);
  ERROR_CHECK(surface, "Can't create surface", "Created surface");
  wl_surface_add_listener(surface, &surface_listener, NULL);

  // 用compositor来创建cursor_surface
  cursor_surface = wl_compositor_create_surface(compositor);
  ERROR_CHECK(cursor_surface, "Can't create cursor_surface",
              "Created cursor_surface");
  // // 将cursor_surface的角色设置为surface的子窗口
  // sub_surface = wl_subcompositor_get_subsurface(sub_compositor, cursor_surface, surface);
  // // 设置一些属性
  // wl_subsurface_set_position(sub_surface, 0, 0);
  // wl_subsurface_place_above(sub_surface, surface);
  // wl_subsurface_set_sync(sub_surface);

  wl_display_roundtrip(dpy);  // 等待服务器完成发送过去的全部请求的响应

#ifdef HAS_XDG_
  // 用xdg_wm_base来用wl_surface创建出xdg_surface
  shell_surface = xdg_wm_base_get_xdg_surface(shell, surface);
  ERROR_CHECK(shell_surface, "Can't create shell surface",
              "Created shell surface");
  // 获得顶层窗口
  toplevel = xdg_surface_get_toplevel(shell_surface);
  xdg_toplevel_set_title(toplevel, "Wayland demo");
  xdg_surface_add_listener(shell_surface, &shell_surface_listener, NULL);
#else
  // 用shell来创建出shell_surface
  shell_surface = wl_shell_get_shell_surface(shell, surface);
  ERROR_CHECK(shell_surface, "Can't get shell_surface", "Got shell_surface");
  wl_shell_surface_set_toplevel(shell_surface);  // 指定surface角色为toplevel
  wl_shell_surface_add_listener(shell_surface, &shell_surface_listener, NULL);
#endif

  frame_callback = wl_surface_frame(surface);
  wl_callback_add_listener(frame_callback, &frame_callback_listener, NULL);

  wl_surface_commit(surface);


  // draw_window(0, 0, 480, 360);

  // 在事件队列中读取事件并对其进行排队
  int num = 0;
  while ((num = wl_display_dispatch(dpy)) != -1) {
    // printf("Client receive %d event!\n", num);
    if (running == false) break;
  }

  xkb_context_unref(xkb_context);
  // 被销毁时，wl_shell_surface 对象会自动销毁。
  // 在客户端，必须在销毁 wl_surface 对象之前调用 wl_shell_surface_destroy()
#ifdef HAS_XDG_
  xdg_toplevel_destroy(toplevel);
  xdg_surface_destroy(shell_surface);
  xdg_wm_base_destroy(shell);
#else
  wl_shell_surface_destroy(shell_surface);
  wl_shell_destroy(shell);
#endif
  // wl_subsurface_destroy(sub_surface);
  // wl_subcompositor_destroy(wl_sub_compositor);
  // wl_buffer_destroy(cursor_buffer);
  wl_surface_destroy(surface);
  wl_compositor_destroy(compositor);
  wl_buffer_destroy(buffer);
  wl_registry_destroy(reg);
  wl_display_disconnect(dpy);
  printf("disconnected from server\n");
  return 0;
}