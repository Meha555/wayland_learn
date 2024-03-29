#ifndef DEFS_H
#define DEFS_H

#include <sys/types.h>
#include <wayland-client-protocol.h>

#include "../utils.h"
#ifdef HAS_XDG_
#include "../extra/xdg-shell-client-protocol.h"
#endif

// 代理对象的本地映像，用来接一下wl_registry_bind的返回值wl_proxy

static int g_width = 480;
static int g_height = 360;

// 合成器和曲面
static struct wl_compositor* compositor = NULL;
// static struct wl_subcompositor* sub_compositor = NULL;
static struct wl_surface* surface = NULL;
// static struct wl_subsurface* sub_surface = NULL;
#ifdef HAS_XDG_
static struct xdg_wm_base* shell = NULL;
static struct xdg_surface* shell_surface = NULL;
struct xdg_toplevel* toplevel = NULL;
#else
static struct wl_shell* shell = NULL;
static struct wl_shell_surface* shell_surface = NULL;
#endif

// 共享内存和缓冲区
static struct wl_shm* shm = NULL;
static struct wl_shm_pool* pool = NULL;
static struct wl_buffer* buffer = NULL;
static int shm_fd = -1;
static void* surface_shm_data = NULL;
static void* sub_surface_shm_data = NULL;
static uint32_t shm_format;
static struct wl_callback* frame_callback;

// 输入设备
static struct wl_seat* seat = NULL;
static struct wl_pointer* pointer = NULL;
static struct wl_keyboard* keyboard = NULL;
static struct wl_touch* touch = NULL;

static int transpranet = 255;

// 鼠标光标
struct wl_cursor_theme* cursor_theme;
struct wl_cursor* default_cursor;
struct wl_surface* cursor_surface;
// struct wl_buffer* cursor_buffer;

// 键盘控制
static struct xkb_context* xkb_context = NULL;
static struct xkb_keymap* keymap = NULL;
static struct xkb_state* xkb_state = NULL;

void paint_pixels_into_shm_data(int width, int height, enum color_enum color1,
                                enum color_enum color2);

struct wl_buffer* create_buffer(int width, int height);
void draw_window(int x, int y, int width, int height);
// void draw_track(int x, int y);

#endif  // DEFS_H