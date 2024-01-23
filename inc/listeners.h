#ifndef LISTENERS_H
#define LISTENERS_H

#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

#include "core/defs.h"
#include "utils.h"

int WIDTH = 480;
int HEIGHT = 360;

/* ---------------------------------- 像素格式 ---------------------------------- */

static void on_shm_format(void* data, struct wl_shm* shm,
                          enum wl_shm_format fmt) {
  shm_format = fmt;
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

/* ----------------------------------- 缓冲区 ---------------------------------- */

// 合成器不再使用此缓冲区
void on_buffer_released(void* data, struct wl_buffer* buffer) {
  // printf("Buffer released\n");
}

struct wl_buffer_listener buffer_listener = {.release = on_buffer_released};

/* -------------------------------- surface相关 ------------------------------- */

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

static void on_surface_ping(void* data, struct wl_shell_surface* shell_surface,
                            uint32_t serial) {
  // 填写的回调是由客户端自己调用的，不是我们程序员手动调用的
  wl_shell_surface_pong(shell_surface, serial); // 客户端pong一下
  printf("Compositor ping : %u\n", serial);
}

static void on_surface_configure(void* data,
                                 struct wl_shell_surface* shell_surface,
                                 uint32_t edges, int32_t width,
                                 int32_t height) {
  printf("Resize to %dx%d\n", width, height);
}

static void on_surface_popup_done(void* data,
                                  struct wl_shell_surface* shell_surface) {
  printf("Popup surface!\n");
}

struct wl_shell_surface_listener shell_surface_listener = {
    .ping = on_surface_ping,
    .configure = on_surface_configure,
    .popup_done = on_surface_popup_done};

/* -------------------------------- 输入设备（鼠标） -------------------------------- */

static void on_pointer_enter(void* data, struct wl_pointer* pointer,
                             uint32_t serial, struct wl_surface* surface,
                             wl_fixed_t sx, wl_fixed_t sy) {
  printf("Pointer entered surface %p at %f %f\n", surface,
         wl_fixed_to_double(sx), wl_fixed_to_double(sy));
}

static void on_pointer_leave(void* data, struct wl_pointer* pointer,
                             uint32_t serial, struct wl_surface* surface) {
  printf("Pointer left surface %p\n", surface);
}

static void on_pointer_motion(void* data, struct wl_pointer* pointer,
                              uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {
  printf("Pointer moved at %f %f\n", wl_fixed_to_double(sx),
         wl_fixed_to_double(sy));
}

static void on_pointer_button(void* data, struct wl_pointer* wl_pointer,
                              uint32_t serial, uint32_t time, uint32_t button,
                              uint32_t state) {
  printf("Pointer button\n");
  if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED)
    wl_shell_surface_move(shell_surface, seat, serial);
}

static void on_pointer_axis(void* data, struct wl_pointer* wl_pointer,
                            uint32_t time, uint32_t axis, wl_fixed_t value) {
  printf("Pointer axis\n");
}

struct wl_pointer_listener pointer_listener = {.enter = on_pointer_enter,
                                               .leave = on_pointer_leave,
                                               .motion = on_pointer_motion,
                                               .button = on_pointer_button,
                                               .axis = on_pointer_axis};

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
  xkb_keymap_unref(keymap); // 释放旧的keymap
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
                              struct wl_array* keys) {}
// 当前surface失去键盘焦点
static void on_keyboard_leave(void* data, struct wl_keyboard* keyboard,
                              uint32_t serial, struct wl_surface* surface) {}

static void on_keyboard_key(void* data, struct wl_keyboard* keyboard,
                            uint32_t serial, uint32_t time, uint32_t key,
                            uint32_t state) {
  if (state == WL_KEYBOARD_KEY_STATE_PRESSED) { // 当键被按下
    xkb_keysym_t keysym = xkb_state_key_get_one_sym(xkb_state, key + 8);
    uint32_t utf32 = xkb_keysym_to_utf32(keysym);
    static bool running = true;
    if (utf32) {
      if (utf32 >= 0x21 && utf32 <= 0x7E) { // 如果按下的键是字母或数字，则打印对应的字符
        printf("the key %c was pressed\n", (char)utf32);
        if (utf32 == 'q') running = false; // 如果按下的是 q 键，则终止窗口运行
      } else { // 如果按下的键是其他键，则打印对应的Unicode编码
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

static struct wl_keyboard_listener keyboard_listener = {
    .keymap = on_keyboard_keymap,
    .enter = on_keyboard_enter,
    .leave = on_keyboard_leave,
    .key = on_keyboard_key,
    .modifiers = on_keyboard_modifiers};

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
    // wl_pointer_add_listener(pointer, &pointer_listener, NULL);
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
    // wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
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
  printf("Current device is %s\n", name);
}

struct wl_seat_listener seat_listener = {
    .capabilities = on_seat_capabilities_changed, .name = on_identify_seat};

/* ----------------------------------- 事件回调 ----------------------------------- */

void on_frame_redraw(void* data, struct wl_callback* callback,
                     uint32_t callback_data);

struct wl_callback_listener frame_callback_listener = {.done = on_frame_redraw};

// 重新绘制一帧画面
void on_frame_redraw(void* data, struct wl_callback* callback,
                     uint32_t callback_data) {
  // printf("Frame redrawing\n");
  wl_callback_destroy(callback);  // 不需要这个callback了
  wl_surface_damage(surface, 0, 0, WIDTH, HEIGHT);
  paint_pixels(WIDTH, HEIGHT, GREEN, WHITE);
  // 重新来一个callback
  frame_callback = wl_surface_frame(surface);
  wl_surface_attach(surface, buffer, 0, 0);
  wl_callback_add_listener(frame_callback, &frame_callback_listener, NULL);
  wl_surface_commit(surface);
}

#endif  // LISTENERS_H