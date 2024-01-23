#ifndef DEFS_H
#define DEFS_H

#include <sys/types.h>
#include <wayland-client-protocol.h>

#include "../utils.h"

// 代理对象的本地映像，用来接一下wl_registry_bind的返回值
static struct wl_compositor* compositor = NULL;
// static struct xdg_wm_base* xdg_shell = NULL;
static struct wl_surface* surface = NULL;
static struct wl_shell* shell = NULL;
static struct wl_shell_surface* shell_surface = NULL;
static struct wl_shm* shm = NULL;
static struct wl_buffer* buffer = NULL;
static void* shm_data;
static uint32_t shm_format;
static struct wl_callback* frame_callback;
static struct wl_seat* seat = NULL;
static struct wl_pointer* pointer = NULL;
static struct wl_keyboard* keyboard = NULL;
static struct xkb_context* xkb_context = NULL;
static struct xkb_keymap* keymap = NULL;
static struct xkb_state* xkb_state = NULL;
static struct wl_touch* touch = NULL;

void paint_pixels(int width, int height, enum color_enum color1,
                  enum color_enum color2);

#endif // DEFS_H