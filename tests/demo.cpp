/***
 * @Author: Meha555
 * @Date: 2024-01-22 10:13:02
 * @LastEditors: Meha555
 * @LastEditTime: 2024-04-18 16:45:40
 * @FilePath: /wayland_learn/examples/demo.cpp
 * @Description:本文件用于测试bind一个已被销毁的全局对象的后果
 * @Copyright (c) 2024 by Meha555, All Rights Reserved.
 */

extern "C" {
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include "../inc/extra/xdg-shell-client-protocol.h"
}

template <typename Obj>
struct wl_object_wrapper {
    Obj* obj;
    uint32_t name;
};

wl_object_wrapper<wl_seat> seat;
wl_object_wrapper<wl_keyboard> kb;
wl_object_wrapper<wl_pointer> ptr;
wl_object_wrapper<wl_output> out;

struct wl_compositor* comp;
struct xdg_wm_base* sh;
struct xdg_toplevel* top;
struct wl_surface* srfc;
struct wl_buffer* bfr;
struct wl_shm* shm;
// struct wl_seat* seat;
// struct wl_keyboard* kb;
// struct wl_pointer* ptr;
// struct wl_output* out;
uint8_t* pixl;
uint16_t w = 200;
uint16_t h = 100;
uint8_t c;
uint8_t cls;

// 为我们的共享内存生成一个随机的文件名，类似/12345这样的【linux下一切皆文件，因此共享内存实际上就是一个文件，这个文件可能有很多页，要看这个文件多大，这就是由OS去分页了】
int32_t alc_shm(uint64_t sz)
{
    char name[8];
    name[0] = '/';
    name[7] = 0;
    for (uint8_t i = 1; i < 6; i++) {
        name[i] = (rand() & 23) + 97;
    }

    // 分配这个共享内存，得到文件描述符
    int32_t fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL,
        S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH);

    shm_unlink(name);
    ftruncate(fd, sz);

    return fd;
}

// 将在共享内存上分配指定大小w*h的缓冲区
void resz()
{
    int32_t fd = alc_shm(w * h * 4); // 4是因为每个像素占4个字节（R G B Alpha）

    pixl = (uint8_t*)mmap(0, w * h * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
        0); // 将共享内存映射到进程地址空间，获得其fd

    struct wl_shm_pool* pool = wl_shm_create_pool(shm, fd, w * h * 4);
    bfr = wl_shm_pool_create_buffer(pool, 0, w, h, w * 4, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);
}

void draw()
{
    memset(pixl, c, w * h * 4); // 在共享内存上填充值为c的像素

    wl_surface_attach(srfc, bfr, 0, 0);
    wl_surface_damage_buffer(srfc, 0, 0, w, h);
    wl_surface_commit(srfc);
}

void frame_new(void* data, struct wl_callback* cb, uint32_t a);
struct wl_callback_listener cb_list = { .done = frame_new };

// 帧回调：绘制下一帧
void frame_new(void* data, struct wl_callback* cb, uint32_t a)
{
    wl_callback_destroy(cb);
    cb = wl_surface_frame(srfc);
    wl_callback_add_listener(cb, &cb_list, &cb);

    c++;
    draw();
}

void xrfc_conf(void* data, struct xdg_surface* xrfc, uint32_t ser)
{
    xdg_surface_ack_configure(xrfc, ser);
    if (!pixl) {
        resz();
    }

    draw();
}

struct xdg_surface_listener xrfc_list = { .configure = xrfc_conf };

void top_conf(void* data, struct xdg_toplevel* top, int32_t nw, int32_t nh,
    struct wl_array* stat)
{
    if (!nw && !nh) {
        return;
    }

    if (w != nw || h != nh) {
        munmap(pixl, w * h * 4);
        w = nw;
        h = nh;
        resz();
    }
}

void top_cls(void* data, struct xdg_toplevel* top) { cls = 1; }

void top_conf_bounds(void* data, struct xdg_toplevel* top, int32_t w,
    int32_t h)
{
    fprintf(stderr, "Recommend bounds for %p: %dx%d\n", top, w, h);
}
void top_wm_cap(void* data, struct xdg_toplevel* top, struct wl_array* caps)
{
    fprintf(stderr, "Compositor supports:\n");
    uint32_t* cap = nullptr;
    // wl_array_for_each(cap, caps)
    for (cap = (uint32_t*)caps->data;
         (const char*)cap < ((const char*)caps->data + caps->size); cap++) {
        fprintf(stderr, "\t%p\n", cap);
    }
}

struct xdg_toplevel_listener top_list = { .configure = top_conf,
    .close = top_cls,
    .configure_bounds = top_conf_bounds,
    .wm_capabilities = top_wm_cap };

void sh_ping(void* data, struct xdg_wm_base* sh, uint32_t ser)
{
    xdg_wm_base_pong(sh, ser);
}

struct xdg_wm_base_listener sh_list = { .ping = sh_ping };

static void on_pointer_enter(void* data, struct wl_pointer* pointer,
    uint32_t serial, struct wl_surface* surface,
    wl_fixed_t sx, wl_fixed_t sy)
{
    printf("Pointer entered surface %p at %f %f\n", surface,
        wl_fixed_to_double(sx), wl_fixed_to_double(sy));
}

static void on_pointer_leave(void* data, struct wl_pointer* pointer,
    uint32_t serial, struct wl_surface* surface)
{
    printf("Pointer left surface %p\n", surface);
}

static void on_pointer_motion(void* data, struct wl_pointer* pointer,
    uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
    printf("Pointer moved at %f %f\n", wl_fixed_to_double(sx),
        wl_fixed_to_double(sy));
}

static void on_pointer_button(void* data, struct wl_pointer* wl_pointer,
    uint32_t serial, uint32_t time, uint32_t button,
    uint32_t state)
{
    printf("Pointer button\n");
}

static void on_pointer_axis(void* data, struct wl_pointer* wl_pointer,
    uint32_t time, uint32_t axis, wl_fixed_t value)
{
    printf("Pointer axis: %u %f\n", axis, wl_fixed_to_double(value));
}

static void on_pointer_frame(void* data, struct wl_pointer* wl_pointer)
{
    printf("Pointer frame\n");
}

static void on_pointer_axis_source(void* data, struct wl_pointer* wl_pointer,
    uint32_t axis_source)
{
    printf("Pointer axis source: %u\n", axis_source);
}

static void on_pointer_axis_stop(void* data, struct wl_pointer* wl_pointer,
    uint32_t time, uint32_t axis)
{
    printf("Pointer stop at %u %u\n", time, axis);
}

static void on_pointer_axis_discrete(void* data, struct wl_pointer* wl_pointer,
    uint32_t axis, int32_t discrete)
{
    printf("Pointer axis discrete at %u %d\n", axis, discrete);
}

static void on_pointer_warp(void* data, struct wl_pointer* wl_pointer,
    wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    printf("Pointer warp at %f %f\n", wl_fixed_to_double(surface_x),
        wl_fixed_to_double(surface_y));
}

struct wl_pointer_listener pointer_listener = {
    .enter = on_pointer_enter,
    .leave = on_pointer_leave,
    .motion = on_pointer_motion,
    .button = on_pointer_button,
    .axis = on_pointer_axis,
    .frame = on_pointer_frame,
    .axis_source = on_pointer_axis_source,
    .axis_stop = on_pointer_axis_stop,
    .axis_discrete = on_pointer_axis_discrete,
    .warp = on_pointer_warp
};

void kb_map(void* data, struct wl_keyboard* kb, uint32_t frmt, int32_t fd,
    uint32_t sz) { }

void kb_enter(void* data, struct wl_keyboard* kb, uint32_t ser,
    struct wl_surface* srfc, struct wl_array* keys) { }

void kb_leave(void* data, struct wl_keyboard* kb, uint32_t ser,
    struct wl_surface* srfc) { }

void kb_key(void* data, struct wl_keyboard* kb, uint32_t ser, uint32_t t,
    uint32_t key, uint32_t stat)
{
    if (key == 1) {
        cls = 1;
    } else if (key == 30) {
        printf("a\n");
    } else if (key == 32) {
        printf("d\n");
    }
}

void kb_mod(void* data, struct wl_keyboard* kb, uint32_t ser, uint32_t dep,
    uint32_t lat, uint32_t lock, uint32_t grp) { }

void kb_rep(void* data, struct wl_keyboard* kb, int32_t rate, int32_t del) { }

struct wl_keyboard_listener kb_list = { .keymap = kb_map,
    .enter = kb_enter,
    .leave = kb_leave,
    .key = kb_key,
    .modifiers = kb_mod,
    .repeat_info = kb_rep };

void seat_cap(void* data, struct wl_seat* seat, uint32_t cap)
{
    fprintf(stderr, "Apply for as many devices as possible\n");
    if (cap & WL_SEAT_CAPABILITY_POINTER && ptr.obj == nullptr) {
        ptr.obj = wl_seat_get_pointer(seat);
        ptr.name = WL_SEAT_CAPABILITY_POINTER;
        fprintf(stderr, "pointer = %p\n", ptr.obj);
        wl_pointer_add_listener(ptr.obj, &pointer_listener, &ptr.obj);
    }
    if (cap & WL_SEAT_CAPABILITY_KEYBOARD && kb.obj == nullptr) {
        kb.obj = wl_seat_get_keyboard(seat);
        kb.name = WL_SEAT_CAPABILITY_KEYBOARD;
        fprintf(stderr, "keyboard = %p\n", kb.obj);
        wl_keyboard_add_listener(kb.obj, &kb_list, &kb.obj);
    }
}

void seat_name(void* data, struct wl_seat* seat, const char* name) { }

struct wl_seat_listener seat_list = { .capabilities = seat_cap,
    .name = seat_name };

static void out_geo(void* data, struct wl_output* wl_output, int32_t x,
    int32_t y, int32_t physical_width, int32_t physical_height,
    int32_t subpixel, const char* make, const char* model,
    int32_t transform)
{
    fprintf(stderr, "wl_output %p Output geo: (%d, %d) (%dx%d) %d %s %s %d\n",
        wl_output, x, y, physical_width, physical_height, subpixel, make,
        model, transform);
}

static void out_mode(void* data, struct wl_output* wl_output, uint32_t flags,
    int32_t width, int32_t height, int32_t refresh)
{
    fprintf(stderr, "wl_output %p Available mode: %u %dx%d %dmHz\n", wl_output,
        flags, width, height, refresh);
}

static void out_done(void* data, struct wl_output* wl_output)
{
    fprintf(stderr, "wl_output %p 's attributes has been changed!\n", wl_output);
}

static void out_scale(void* data, struct wl_output* wl_output, int32_t factor)
{
    fprintf(stderr, "wl_output %p 's Scale ratio: %d\n", wl_output, factor);
}

static void out_name(void* data, struct wl_output* wl_output,
    const char* name)
{
    fprintf(stderr, "wl_output %p 's Name: %s\n", wl_output, name);
}

static void out_description(void* data, struct wl_output* wl_output,
    const char* description)
{
    fprintf(stderr, "wl_output %p 's Description: %s\n", wl_output, description);
}

struct wl_output_listener out_list = { .geometry = out_geo,
    .mode = out_mode,
    .done = out_done,
    .scale = out_scale,
    .name = out_name,
    .description = out_description };

void reg_glob(void* data, struct wl_registry* reg, uint32_t name,
    const char* intf, uint32_t v)
{
    fprintf(stderr, "Global add: %s , version: %u, name: %u\n", intf, v, name);
    // 为了测试，这里申请尽可能多的全局对象
    if (!strcmp(intf, wl_compositor_interface.name)) {
        comp = static_cast<wl_compositor*>(
            wl_registry_bind(reg, name, &wl_compositor_interface, v));
        fprintf(stderr, "compositor = %p\n", comp);
    } else if (!strcmp(intf, wl_shm_interface.name)) {
        shm = static_cast<wl_shm*>(wl_registry_bind(reg, name, &wl_shm_interface, v));
        fprintf(stderr, "shm = %p\n", shm);
    } else if (!strcmp(intf, xdg_wm_base_interface.name)) {
        sh = static_cast<xdg_wm_base*>(
            wl_registry_bind(reg, name, &xdg_wm_base_interface, v));
        fprintf(stderr, "shell = %p\n", sh);
        xdg_wm_base_add_listener(sh, &sh_list, &sh);
    } else if (!strcmp(intf, wl_seat_interface.name)) {
        seat = { .obj = static_cast<wl_seat*>(wl_registry_bind(reg, name, &wl_seat_interface, v)),
            .name = name };
        fprintf(stderr, "seat = %p\n", seat.obj);
        wl_seat_add_listener(seat.obj, &seat_list, &seat.obj);
    } else if (!strcmp(intf, wl_output_interface.name)) {
        out = { .obj = static_cast<wl_output*>(wl_registry_bind(reg, name, &wl_output_interface, v)),
            .name = name };
        fprintf(stderr, "output = %p\n", out.obj);
        wl_output_add_listener(out.obj, &out_list, &out.obj);
    }
}

void reg_glob_rem(void* data, struct wl_registry* reg, uint32_t name)
{
    fprintf(stderr, "Notify the client of removed global objects.\n");
    /*
    This event notifies the client that the global identified by name is no
    longer available. If the client bound to the global using the bind request,
    the client should now destroy that object.
    这个事件通知客户端，由name标识的全局变量不再可用。如果客户端之前使用绑定请求绑定了这个全局变量，那么客户端现在应该销毁这个对象。
    The object remains valid and requests to the object will be ignored until the
    client destroys it, to avoid races between the global going away and a client
    sending a request to it.
    对象直到客户端销毁它之前都还一直有效但有关这个对象的所有请求都会被忽略，这样可以避免在全局对象消失之前客户端发送请求所导致的竞态问题。
    */
    if (name == out.name) {
        // 手动发起release请求，不release的话协议没有说会发生什么，但是应该会导致资源泄露
        wl_output_release(out.obj);
        fprintf(stderr, "wl_output %p@%d removed!\n", out.obj, out.name);
        fprintf(stderr, "========================111111=========================\n");
        sleep(10); // 需要加延迟，否则全局对象还没有那么快被销毁【QtWayland中设置的是5s，见deferred_destroy_global_func这个函数】
        out.obj = static_cast<wl_output*>(wl_registry_bind(reg, name, &wl_output_interface, 4));
        out.name = name;
        fprintf(stderr, "========================222222=========================\n");
    } else if (name == kb.name) {
        wl_keyboard_release(kb.obj);
        fprintf(stderr, "wl_keyboard %p@%d removed!\n", kb.obj, kb.name);
    } else if (name == ptr.name) {
        wl_pointer_release(ptr.obj);
        fprintf(stderr, "wl_pointer %p@%d removed!\n", kb.obj, kb.name);
    } else if (name == seat.name) {
        wl_seat_release(seat.obj);
        fprintf(stderr, "wl_seat %p@%d removed!\n", seat.obj, seat.name);
    } else {
        fprintf(stderr, "Unkown obj %p@%d removed!\n", data, name);
    }
}

struct wl_registry_listener reg_list = { .global = reg_glob,
    .global_remove = reg_glob_rem };

int main()
{
    struct wl_display* disp = wl_display_connect(0);
    struct wl_registry* reg = wl_display_get_registry(disp);
    wl_registry_add_listener(reg, &reg_list, &reg);
    wl_display_roundtrip(disp);

    // srfc = wl_compositor_create_surface(comp);
    // struct wl_callback* cb = wl_surface_frame(srfc);
    // wl_callback_add_listener(cb, &cb_list, &cb);

    // struct xdg_surface* xrfc = xdg_wm_base_get_xdg_surface(sh, srfc);
    // xdg_surface_add_listener(xrfc, &xrfc_list, &xrfc);
    // top = xdg_surface_get_toplevel(xrfc);
    // xdg_toplevel_add_listener(top, &top_list, &top);
    // xdg_toplevel_set_title(top, "wayland client");
    // wl_surface_commit(srfc);

    while (wl_display_dispatch(disp)) {
        if (cls)
            break;
    }

    if (kb.obj) {
        wl_keyboard_destroy(kb.obj);
    }
    if (ptr.obj) {
        wl_pointer_destroy(ptr.obj);
    }
    wl_seat_release(seat.obj);
    if (bfr) {
        wl_buffer_destroy(bfr);
    }
    // xdg_toplevel_destroy(top);
    // xdg_surface_destroy(xrfc);
    // wl_surface_destroy(srfc);
    wl_shm_destroy(shm);
    wl_registry_destroy(reg);
    wl_display_disconnect(disp);
    return 0;
}