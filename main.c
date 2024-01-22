#include <asm-generic/errno-base.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <unistd.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include "inc/utils.h"
#include "inc/xdg-shell-client-protocol.h"

static struct wl_compositor* compositor = NULL;
struct xdg_wm_base* xdg_shell = NULL;
struct wl_surface* surface = NULL;
struct wl_shell* shell = NULL;
struct wl_shell_surface* shell_surface = NULL;
struct wl_shm* shm = NULL;
struct wl_buffer* buffer = NULL;
void* shm_data;
uint32_t shm_format;
struct wl_callback* frame_callback;
struct wl_seat* seat = NULL;
struct wl_pointer* pointer = NULL;
struct wl_keyboard* keyboard = NULL;
struct xkb_context* xkb_context;
struct xkb_keymap* keymap = NULL;
struct xkb_state* xkb_state = NULL;
struct wl_touch* touch = NULL;

int WIDTH = 480;
int HEIGHT = 360;

struct wl_display* connect_to_server(const char* name) {
  struct wl_display* dpy = wl_display_connect(name);
  ERROR_CHECK(dpy, "Can't open wayland compositor", "Connect OK");
  return dpy;
}

static void on_shm_format(void* data, struct wl_shm* shm,
                          enum wl_shm_format fmt) {
  shm_format = fmt;
  char* s;
  switch (fmt) {
    case WL_SHM_FORMAT_ARGB8888:
      s = "ARGB8888";
      break;
    case WL_SHM_FORMAT_XRGB8888:
      s = "XRGB8888";
      break;
    case WL_SHM_FORMAT_RGB565:
      s = "RGB565";
      break;
    default:
      s = "other format";
      break;
  }
  printf("Avaiable pixel format: %s\n", s);
}

struct wl_shm_listener shm_listener = {.format = on_shm_format};

static void on_surface_enter_output(void* data, struct wl_surface* wl_surface,
                                    struct wl_output* output) {
  printf("Surface enter an output\n");
}

static void on_surface_leave_output(void* data, struct wl_surface* wl_surface,
                                    struct wl_output* output) {
  printf("Surface leave an output\n");
}

// 这2个事件类似于Expose？
struct wl_surface_listener surface_listener = {
    .enter = on_surface_enter_output, .leave = on_surface_leave_output};

static void on_surface_ping(void* data, struct wl_shell_surface* shell_surface,
                            uint32_t serial) {
  wl_shell_surface_pong(
      shell_surface,
      serial);  // 填写的回调是由客户端自己调用的，不是我们程序员手动调用的
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

static void on_keyboard_keymap(void* data, struct wl_keyboard* keyboard,
                               uint32_t format, int32_t fd, uint32_t size) {
  char* keymap_string = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  xkb_keymap_unref(keymap);
  keymap = xkb_keymap_new_from_string(xkb_context, keymap_string,
                                      XKB_KEYMAP_FORMAT_TEXT_V1,
                                      XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap(keymap_string, size);
  close(fd);
  xkb_state_unref(xkb_state);
  xkb_state = xkb_state_new(keymap);
}

static void on_keyboard_enter(void* data, struct wl_keyboard* keyboard,
                              uint32_t serial, struct wl_surface* surface,
                              struct wl_array* keys) {}

static void on_keyboard_leave(void* data, struct wl_keyboard* keyboard,
                              uint32_t serial, struct wl_surface* surface) {}

static void on_keyboard_key(void* data, struct wl_keyboard* keyboard,
                            uint32_t serial, uint32_t time, uint32_t key,
                            uint32_t state) {
  if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    xkb_keysym_t keysym = xkb_state_key_get_one_sym(xkb_state, key + 8);
    uint32_t utf32 = xkb_keysym_to_utf32(keysym);
    static bool running = true;
    if (utf32) {
      if (utf32 >= 0x21 && utf32 <= 0x7E) {
        printf("the key %c was pressed\n", (char)utf32);
        if (utf32 == 'q') running = false;
      } else {
        printf("the key U+%04X was pressed\n", utf32);
      }
    } else {
      char name[64];
      xkb_keysym_get_name(keysym, name, 64);
      printf("the key %s was pressed\n", name);
    }
  }
}

static void on_keyboard_modifiers(void* data, struct wl_keyboard* keyboard,
                                  uint32_t serial, uint32_t mods_depressed,
                                  uint32_t mods_latched, uint32_t mods_locked,
                                  uint32_t group) {
  xkb_state_update_mask(xkb_state, mods_depressed, mods_latched, mods_locked, 0,
                        0, group);
}

static struct wl_keyboard_listener keyboard_listener = {
    .keymap = on_keyboard_keymap,
    .enter = on_keyboard_enter,
    .leave = on_keyboard_leave,
    .key = on_keyboard_key,
    .modifiers = on_keyboard_modifiers};

// 输入设备改变其能力，即插入/拔出设备(鼠标、键盘、触摸屏)
static void on_seat_capabilities_changed(void* data, struct wl_seat* seat,
                                         enum wl_seat_capability caps) {
  if (caps == 0 && !pointer && !keyboard && !touch) {
    printf("No input deivice available on display\n");
    return;
  }
  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !pointer) {
    printf("Display got a Pointer\n");
    pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(pointer, &pointer_listener, NULL);
  } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && pointer) {
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
  printf("Current device is %s\n", name);
}

struct wl_seat_listener seat_listener = {
    .capabilities = on_seat_capabilities_changed, .name = on_identify_seat};

static void on_global_registry_added(void* data, struct wl_registry* registry,
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
  } else if (strcmp(interface, "wl_seat") == 0) {
    seat = wl_registry_bind(registry, id, &wl_seat_interface, version);
    ERROR_CHECK(seat, "Can't find wl_seat", "Found wl_seat");
    wl_seat_add_listener(seat, &seat_listener, NULL);
  }
}

static void on_global_registry_removed(void* data, struct wl_registry* registry,
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

int set_cloexec_or_close(int fd) {
  long flags;
  if (fd == -1) return -1;
  flags = fcntl(fd, F_GETFD);
  if (flags == -1) goto err;
  if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) goto err;
  return fd;
err:
  close(fd);
  return -1;
}

int create_tmpfile_cloexec(char* tmpname) {
  int fd;
#ifdef HAVE_MKOSTEMP
  fd = mkostemp(tmpname, O_CLOEXEC);
  if (fd >= 0) unlink(tmpname);
#else
  fd = mkstemp(tmpname);
  if (fd >= 0) {
    fd = set_cloexec_or_close(fd);
    unlink(tmpname);
  }
#endif
  return fd;
}

int os_create_anonymous_file(off_t size) {
  static const char template[] = "/weston-shared-XXXXXX";
  const char* path;
  char* name;
  int fd;

  path = getenv("XDG_RUNTIME_DIR");
  if (!path) {
    errno = ENOENT;
    return -1;
  }

  name = malloc(strlen(path) + sizeof(template));
  if (!name) return -1;

  strcpy(name, path);
  strcat(name, template);
  printf("File name: %s\n", name);

  fd = create_tmpfile_cloexec(name);

  free(name);

  if (fd < 0) return -1;

  if (ftruncate(fd, size) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

static void on_buffer_released(void* data, struct wl_buffer* buffer) {
  // printf("Buffer released\n");
}

struct wl_buffer_listener buffer_listener = {.release = on_buffer_released};

struct wl_buffer* create_buffer() {
  struct wl_shm_pool* pool;
  int stride = WIDTH * 4;  // R G B Alpha，每个像素4B
  int size = stride * HEIGHT;
  int fd;
  struct wl_buffer* buff;
  fd = os_create_anonymous_file(size);  // 返回一个匿名文件的文件描述符
  ASSERT_MSG(fd >= 0, "creating a buffer file for %d B failed: %m", size);
  shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm_data == MAP_FAILED) {
    fprintf(stderr, "mmap failed: %m\n");
    close(fd);
    exit(EXIT_FAILURE);
  }

  pool = wl_shm_create_pool(shm, fd, size);
  buff = wl_shm_pool_create_buffer(pool, 0, WIDTH, HEIGHT, stride,
                                   WL_SHM_FORMAT_XRGB8888);
  wl_buffer_add_listener(buff, &buffer_listener, NULL);
  wl_shm_pool_destroy(pool);  // 这个共享内存池用不到了，可以释放
  return buff;
}

void create_window() {
  buffer = create_buffer();
  // 1. 将缓冲区绑定到窗口上
  wl_surface_attach(surface, buffer, 0, 0);
  // 2. 标记窗口失效的区域
  // wl_surface_damage(surface,x, y, WIDTH, HEIGHT );
  // 3. 提交缓冲区
  wl_surface_commit(surface);
}

struct wl_callback_listener frame_callback_listener;

void paint_pixels() {
  static uint32_t pixel_rgb = BLACK;
  uint32_t* pixel = (uint32_t*)shm_data;
  for (int i = 0; i < WIDTH * HEIGHT; ++i) {
    *pixel++ = pixel_rgb;
  }
  // 每个RGB分量增加1
  pixel_rgb += 0x010101;
  if (pixel_rgb >= WHITE) {
    pixel_rgb = BLACK;
  }
}

void on_frame_redraw(void* data, struct wl_callback* callback,
                     uint32_t callback_data) {
  // printf("Frame redrawing\n");
  wl_callback_destroy(callback);  // 不需要这个callback了
  wl_surface_damage(surface, 0, 0, WIDTH, HEIGHT);
  paint_pixels();
  // 重新来一个callback
  frame_callback = wl_surface_frame(surface);
  wl_surface_attach(surface, buffer, 0, 0);
  wl_callback_add_listener(frame_callback, &frame_callback_listener, NULL);
  wl_surface_commit(surface);
}

struct wl_callback_listener frame_callback_listener = {.done = on_frame_redraw};

int main() {
  fprintf(stderr, "XDG_RUNTIME_DIR= %s\n", getenv("XDG_RUNTIME_DIR"));
  struct wl_display* dpy = connect_to_server(NULL);
  struct wl_registry* reg = init_registry(dpy);

  wl_display_dispatch(dpy);
  wl_display_roundtrip(dpy);

  surface = wl_compositor_create_surface(compositor);
  ERROR_CHECK(surface, "Can't create surface", "Created surface");
  wl_surface_add_listener(surface, &surface_listener, NULL);

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

  create_window();
  // printf("Nothing to display.\n");

  while (wl_display_dispatch(dpy) != -1) {
    ;
  }

  // xdg_surface_destroy(shell_surface);
  // xdg_wm_base_destroy(xdg_shell);

  wl_shell_surface_destroy(shell_surface);
  wl_shell_destroy(shell);
  wl_surface_destroy(surface);
  wl_compositor_destroy(compositor);
  wl_registry_destroy(reg);
  wl_display_disconnect(dpy);
  printf("disconnected from server\n");
  return 0;
}