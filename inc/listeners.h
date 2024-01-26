#ifndef LISTENERS_H
#define LISTENERS_H

#include <linux/input.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>

#include "core/defs.h"
#include "utils.h"
#include "xdg-shell-client-protocol.h"

extern bool running;

/* ---------------------------------- 像素格式 ---------------------------------- */

static void on_shm_format(void* data, struct wl_shm* shm,
                          enum wl_shm_format fmt) {
  g_window.shm.shm_format = fmt;
  char* s;
  switch (fmt) {
    case WL_SHM_FORMAT_ARGB8888: s = "ARGB8888"; break;
    case WL_SHM_FORMAT_XRGB8888: s = "XRGB8888"; break;
    case WL_SHM_FORMAT_RGB565: s = "RGB565"; break;
    default: s = "other format"; break;
  }
  printf("Avaiable pixel format: %s\n", s);
}

struct wl_shm_listener shm_listener = {.format = on_shm_format};

/* ----------------------------------- 缓冲区 ----------------------------------*/

// 合成器不再使用此缓冲区
void on_buffer_released(void* data, struct wl_buffer* buffer) {
  // printf("Buffer released\n");
}

struct wl_buffer_listener buffer_listener = {.release = on_buffer_released};

/* -------------------------------- surface相关 -------------------------------*/

static void on_surface_enter_output(void* data, struct wl_surface* wl_surface,
                                    struct wl_output* output) {
  printf("Surface enter an output\n");
}

static void on_surface_leave_output(void* data, struct wl_surface* wl_surface,
                                    struct wl_output* output) {
  printf("Surface leave an output\n");
}

// 这2个事件类似于Expose，enter表示surface进入显示区域，leave表示离开显示区域
struct wl_surface_listener surface_listener = {
    .enter = on_surface_enter_output, .leave = on_surface_leave_output};

#ifdef HAS_XDG_
static void on_xdg_surface_configure(void* data, 
                                     struct xdg_surface* xdg_surface, uint32_t serial){
  printf("Configure xdg_wm_base with sequence: %u\n", serial);
  xdg_surface_ack_configure(g_window.surface.shell_surface, serial); // 发送ack
}

struct xdg_surface_listener shell_surface_listener = {
  .configure = on_xdg_surface_configure
};

static void on_xdg_toplevel_surface_configure(void* data, struct xdg_toplevel* xdg_toplevel, int32_t width,
                  int32_t height, struct wl_array* states){
  printf("Configure xdg_toplevel size to: %dx%d\n", width, height);
  g_window.scale.width = width, g_window.scale.height = height;
}

static void on_xdg_toplevel_surface_close(void* data,
                                          struct xdg_toplevel* xdg_toplevel){
  printf("xdg_toplevel surface close.\n");
}

static void on_xdg_toplevel_surface_configure_bounds(
    void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height) {
  printf("Configure xdg_toplevel geometry bounds to %dx%d:\n", width, height);
  g_window.scale.width = width, g_window.scale.height = height;
}

static void on_xdg_toplevel_surface_wm_capabilities(
    void* data, struct xdg_toplevel* xdg_toplevel,
    struct wl_array* capabilities) {
  printf("xdg_toplevel surface wm capabilities changed.\n");
}

struct xdg_toplevel_listener toplevel_listener = {
    .close = on_xdg_toplevel_surface_close,
    .configure = on_xdg_toplevel_surface_configure,
    .configure_bounds = on_xdg_toplevel_surface_configure_bounds,
    .wm_capabilities = on_xdg_toplevel_surface_wm_capabilities};

#else
static void on_surface_ping(void* data, struct wl_shell_surface* g_window.surface.shell_surface,
                            uint32_t serial) {
  // 填写的回调是由客户端自己调用的，不是我们程序员手动调用的
  printf("Compositor ping to wl_shell_surface: %u\n", serial);
  wl_shell_surface_pong(g_window.surface.shell_surface, serial);  // 客户端pong一下
}

static void on_surface_configure(void* data,
                                 struct wl_shell_surface* g_window.surface.shell_surface,
                                 uint32_t edges, int32_t width,
                                 int32_t height) {
  printf("Resize to %dx%d\n", width, height);
}

static void on_surface_popup_done(void* data,
                                  struct wl_shell_surface* g_window.surface.shell_surface) {
  printf("Popup surface!\n");
}

struct wl_shell_surface_listener shell_surface_listener = {
    .ping = on_surface_ping,
    .configure = on_surface_configure,
    .popup_done = on_surface_popup_done};
#endif

/* ------------------------------- xdg_wm_base ------------------------------ */

#ifdef HAS_XDG_
void on_xdg_wm_base_ping(void* data, struct xdg_wm_base* xdg_wm_base, uint32_t serial) {
  printf("Compositor ping to xdg_wm_base: %u\n", serial);
  xdg_wm_base_pong(xdg_wm_base, serial);
}

struct xdg_wm_base_listener xdg_wm_base_listener = { .ping = on_xdg_wm_base_ping };
#endif

/* -------------------------------- 输入设备（鼠标） -------------------------------- */

static void on_pointer_enter(void* data, struct wl_pointer* pointer,
                             uint32_t serial, struct wl_surface* surface,
                             wl_fixed_t sx, wl_fixed_t sy) {
  printf("Pointer entered surface %p at %f %f\n", surface,
         wl_fixed_to_double(sx), wl_fixed_to_double(sy));
  struct wl_cursor_image* image = default_cursor->images[0];
  wl_pointer_set_cursor(pointer, serial, cursor_surface, image->hotspot_x,
                        image->hotspot_y);
  if (cursor_buffer == NULL) cursor_buffer = wl_cursor_image_get_buffer(image);
  wl_surface_attach(cursor_surface, cursor_buffer, 0, 0);
  wl_surface_damage_buffer(cursor_surface, 0, 0, image->width, image->height);
  wl_surface_commit(cursor_surface);
}

static void on_pointer_leave(void* data, struct wl_pointer* pointer,
                             uint32_t serial, struct wl_surface* surface) {
  printf("Pointer left surface %p\n", surface);
}

static void on_pointer_motion(void* data, struct wl_pointer* pointer,
                              uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {
  printf("Pointer moved at %f %f\n", wl_fixed_to_double(sx),
         wl_fixed_to_double(sy));
        //  draw_track(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
}

static void on_pointer_button(void* data, struct wl_pointer* wl_pointer,
                              uint32_t serial, uint32_t time, uint32_t button,
                              uint32_t state) {
  printf("Pointer button\n");
  // 当鼠标被按下，并且按的是左键时，就可以移动窗口了
  if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED)
#ifdef HAS_XDG_
    xdg_toplevel_move(g_window.surface.toplevel, seat, serial);
#else
    wl_shell_surface_move(g_window.surface.shell_surface, seat, serial);
#endif
}

static void on_pointer_axis(void* data, struct wl_pointer* wl_pointer,
                            uint32_t time, uint32_t axis, wl_fixed_t value) {
  printf("Pointer axis: %u %f\n", axis, wl_fixed_to_double(value));
  // 如果向上滚动鼠标滚轮，就减小透明度
  if(value > 0 && transpranet < 255) transpranet += 5;
  // 否则增加透明度
  else if(value < 0 && transpranet > 0) transpranet -= 5;
}

static void on_pointer_frame(void* data, struct wl_pointer *wl_pointer) {
  printf("Pointer frame\n");
}

static void on_pointer_axis_source(void* data, struct wl_pointer* wl_pointer,
                                   uint32_t axis_source) {
  printf("Pointer axis source: %u\n", axis_source);
}

static void on_pointer_axis_stop(void* data, struct wl_pointer* wl_pointer,
                         uint32_t time, uint32_t axis) {
  printf("Pointer stop at %u %u\n", time, axis);
}

static void on_pointer_axis_discrete(void* data, struct wl_pointer* wl_pointer,
                                     uint32_t axis, int32_t discrete) {
  printf("Pointer axis discrete at %u %d\n", axis, discrete);
}

static void on_pointer_warp(void* data, struct wl_pointer* wl_pointer,
                            wl_fixed_t surface_x, wl_fixed_t surface_y) {
  printf("Pointer warp at %f %f\n", wl_fixed_to_double(surface_x)
                                          , wl_fixed_to_double(surface_y));
}

struct wl_pointer_listener pointer_listener = {.enter = on_pointer_enter,
                                               .leave = on_pointer_leave,
                                               .motion = on_pointer_motion,
                                               .button = on_pointer_button,
                                               .axis = on_pointer_axis,
                                               .frame = on_pointer_frame,
                                               .axis_source = on_pointer_axis_source,
                                               .axis_stop = on_pointer_axis_stop,
                                               .axis_discrete = on_pointer_axis_discrete,
                                               .warp = on_pointer_warp};

/* -------------------------------- 输入设备（键盘） -------------------------------- */

// 通过共享内存来获得当前surface的键盘映射（键盘映射就是用于将物理按键分配到不同的功能，比如当前surface有自定义快捷键）
static void on_keyboard_keymap(void* data, struct wl_keyboard* keyboard,
                               uint32_t format, int32_t fd, uint32_t size) {
  char* keymap_string = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (keymap_string == MAP_FAILED) {
    fprintf(stderr, "mmap failed: %m\n");
    close(fd);
    exit(EXIT_FAILURE);
  }
  xkb_keymap_unref(keymap);  // 释放旧的keymap
  // 根据键盘上下文创建一个新的keymap
  keymap = xkb_keymap_new_from_string(xkb_context, keymap_string,
                                      XKB_KEYMAP_FORMAT_TEXT_V1,
                                      XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (munmap(keymap_string, size) == -1) {
    fprintf(stderr, "munmap failed: %m\n");
    close(fd);
    exit(EXIT_FAILURE);
  }
  close(fd);
  xkb_state_unref(xkb_state); // 释放旧的键盘状态
  xkb_state = xkb_state_new(keymap); // 更新键盘状态
}
// 当前surface获得键盘焦点
static void on_keyboard_enter(void* data, struct wl_keyboard* keyboard,
                              uint32_t serial, struct wl_surface* surface,
                              struct wl_array* keys) {
  printf("Surface %p got keyboard focus\n", surface);
}
// 当前surface失去键盘焦点
static void on_keyboard_leave(void* data, struct wl_keyboard* keyboard,
                              uint32_t serial, struct wl_surface* surface) {
  printf("Surface %p lost keyboard focus\n", surface);
}

static void on_keyboard_key(void* data, struct wl_keyboard* keyboard,
                            uint32_t serial, uint32_t time, uint32_t key,
                            uint32_t state) {
  if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {  // 当键被按下
    xkb_keysym_t keysym = xkb_state_key_get_one_sym(xkb_state, key + 8);
    uint32_t utf32 = xkb_keysym_to_utf32(keysym);
    if (utf32) {
      // 如果按下的键是字母或数字，则打印对应的字符
      if (utf32 >= 0x21 && utf32 <= 0x7E) {
        printf("the key %c was pressed\n", (char)utf32);
        if (utf32 == 'q') running = false;  // 如果按下的是 q 键，则终止窗口运行
      } else {  // 如果按下的键是其他键，则打印对应的Unicode编码
        printf("the key U+%04X was pressed\n", utf32);
      }
    } else {
      static char name[64];
      memset(name, 0, sizeof(name));
      xkb_keysym_get_name(keysym, name, 64);
      printf("the key %s was pressed\n", name);
    }
  }
}

static void on_keyboard_modifiers(void* data, struct wl_keyboard* keyboard,
                                  uint32_t serial, uint32_t mods_depressed,
                                  uint32_t mods_latched, uint32_t mods_locked,
                                  uint32_t group) {
  // 对于键盘修饰键（CTRL、ALT这种），在按下、释放、锁定时，需要更改键盘状态
  xkb_state_update_mask(xkb_state, mods_depressed, mods_latched, mods_locked, 0,
                        0, group);
}

static void on_keyboard_repeat_info(void* data, struct wl_keyboard* keyboard, int32_t rate, int32_t delay) {
  printf("Keyboard repeat info: rate= %d, delay= %d\n", rate, delay);
}

struct wl_keyboard_listener keyboard_listener = {
    .keymap = on_keyboard_keymap,
    .enter = on_keyboard_enter,
    .leave = on_keyboard_leave,
    .key = on_keyboard_key,
    .modifiers = on_keyboard_modifiers,
    .repeat_info = on_keyboard_repeat_info};

/* --------------------------------- 输入设备管理器 -------------------------------- */

// 输入设备改变其能力，即插入/拔出设备(鼠标、键盘、触摸屏)
static void on_seat_capabilities_changed(void* data, struct wl_seat* seat,
                                         enum wl_seat_capability caps) {
  if (caps == 0 && !pointer && !keyboard && !touch) {
    printf("No input deivice available on display\n");
    return;
  }
  // 如果输入设备是鼠标且当前窗口鼠标指针不为空，说明鼠标存在且在当前窗口
  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !pointer) {
    printf("Display got a Pointer\n");
    pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(pointer, &pointer_listener, NULL);
  }
  // 如果输入设备为鼠标但是当前窗口鼠标指针为空，说明鼠标存在但是已经离开当前窗口
  else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && pointer) {
    printf("Display lost a Pointer\n");
    wl_pointer_release(pointer);
    pointer = NULL;
  }
  if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !keyboard) {
    printf("Display got a Keyboard\n");
    keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
  } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && keyboard) {
    printf("Display lost a Keyboard\n");
    wl_keyboard_release(keyboard);
    keyboard = NULL;
  }
  if ((caps & WL_SEAT_CAPABILITY_TOUCH) && !touch) {
    printf("Display got a Touch screen\n");
    touch = wl_seat_get_touch(seat);
  } else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && touch) {
    printf("Display lost a Touch screen\n");
    wl_touch_release(touch);
    touch = NULL;
  }
}

// wl_seat管理多个输入设备时，用这个name参数来识别是哪个设备
static void on_identify_seat(void* data, struct wl_seat* seat,
                             const char* name) {
  printf("Current device_seat is %s\n", name);
}

struct wl_seat_listener seat_listener = {
    .capabilities = on_seat_capabilities_changed, .name = on_identify_seat};

/* ----------------------------------- 帧回调 ---------------------------------- */

void on_frame_redraw(void* data, struct wl_callback* callback,
                     uint32_t callback_data);

struct wl_callback_listener frame_callback_listener = {.done = on_frame_redraw};

// 绘制一帧画面
void on_frame_redraw(void* data, struct wl_callback* callback,
                     uint32_t callback_data) {
  // printf("Frame redrawing\n");
  if(callback) // 这里做这个判断是因为我将这个函数拿去自己用了，就是说不光是由合成器自动调用，因此自己调用的时候callback并没有生成
    wl_callback_destroy(callback);  // 不需要这个callback了，可以销毁
  paint_pixels_into_shm_data(g_window.scale.width, g_window.scale.height, GREEN, WHITE);
  // 重新创建一帧，然后填充surface的新内容
  g_window.surface.frame_callback = wl_surface_frame(g_window.surface.surface);
  wl_callback_add_listener(g_window.surface.frame_callback, &frame_callback_listener, NULL);
  // 将缓冲区对应到窗口的特定位置
  wl_surface_attach(g_window.surface.surface, g_window.shm.buffer, 0, 0);
  // 标记窗口失效需要重绘的区域
  wl_surface_damage_buffer(g_window.surface.surface, 0, 0, g_window.scale.width, g_window.scale.height);
  // 提交挂起的缓冲区内容到合成器
  wl_surface_commit(g_window.surface.surface);
}

/* ----------------------------------- 注册表 ---------------------------------- */

void on_global_registry_added(void* data, struct wl_registry* registry,
                              uint32_t id, const char* interface,
                              uint32_t version) {
  printf("Global add: %s , version: %u, name: %u\n", interface, version, id);

  if (strcmp(interface, "wl_compositor") == 0) {
    g_compositor =
        wl_registry_bind(registry, id, &wl_compositor_interface, version);
    ERROR_CHECK(g_compositor, "Can't find wl_compositor", "Found wl_compositor");
  }
#ifdef HAS_XDG_
  else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    g_window.surface.shell = wl_registry_bind(registry, id, &xdg_wm_base_interface, version);
    ERROR_CHECK(g_window.surface.shell, "Can't find xdg_shell", "Found xdg_shell");
    xdg_wm_base_add_listener(g_window.surface.shell, &xdg_wm_base_listener, NULL);
  }
#else
  else if (strcmp(interface, "wl_shell") == 0) {
    g_window.surface.shell = wl_registry_bind(registry, id, &wl_shell_interface, version);
    ERROR_CHECK(g_window.surface.shell, "Can't find wl_shell", "Found wl_shell");
  }
#endif
  else if (strcmp(interface, "wl_shm") == 0) {
    g_window.shm.shm = wl_registry_bind(registry, id, &wl_shm_interface, version);
    ERROR_CHECK(g_window.shm.shm, "Can't find wl_shm", "Found wl_shm");
    wl_shm_add_listener(g_window.shm.shm, &shm_listener, NULL);
    cursor_theme = wl_cursor_theme_load(NULL, 32, g_window.shm.shm);
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

struct wl_registry_listener reg_listener = {
    .global = on_global_registry_added,
    .global_remove = on_global_registry_removed};

#endif  // LISTENERS_H