#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-server.h>

struct wl_display *display = NULL;
struct wl_compositor *compositor = NULL;
struct wl_surface *surface;
struct wl_egl_window *egl_window;
struct wl_region *region;
struct wl_shell *shell;
struct wl_shell_surface *shell_surface;

EGLDisplay egl_display;
EGLConfig egl_conf;
EGLSurface egl_surface;
EGLContext egl_context;

static void global_registry_handler(void *data, struct wl_registry *registry,
                                    uint32_t id, const char *interface,
                                    uint32_t version) {
  printf("Got a registry event for %s id %d\n", interface, id);
  if (strcmp(interface, "wl_compositor") == 0) {
    compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
  } else if (strcmp(interface, "wl_shell") == 0) {
    shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
  }
}

static void global_registry_remover(void *data, struct wl_registry *registry,
                                    uint32_t id) {
  printf("Got a registry losing event for %d\n", id);
}

static const struct wl_registry_listener registry_listener = {
    global_registry_handler, global_registry_remover};

static void create_opaque_region() {
  region = wl_compositor_create_region(compositor);
  wl_region_add(region, 0, 0, 480, 360);
  wl_surface_set_opaque_region(surface, region);
}

static void create_window() {
  egl_window = wl_egl_window_create(surface, 480, 360);
  if (egl_window == EGL_NO_SURFACE) {
    fprintf(stderr, "Can't create egl window\n");
    exit(1);
  } else {
    fprintf(stderr, "Created egl window\n");
  }

  egl_surface = eglCreateWindowSurface(egl_display, egl_conf, (Window)egl_window, NULL);

  if (eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) {
    fprintf(stderr, "Made current\n");
  } else {
    fprintf(stderr, "Made current failed\n");
  }

  /*
  glClearColor(1.0, 1.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  glFlush();
  */

  if (eglSwapBuffers(egl_display, egl_surface)) {
    fprintf(stderr, "Swapped buffers\n");
  } else {
    fprintf(stderr, "Swapped buffers failed\n");
  }
}

static void init_egl() {
  EGLint major, minor, count, n, size;
  EGLConfig *configs;
  int i;
  EGLint config_attribs[] = {EGL_SURFACE_TYPE,
                             EGL_WINDOW_BIT,
                             EGL_RED_SIZE,
                             8,
                             EGL_GREEN_SIZE,
                             8,
                             EGL_BLUE_SIZE,
                             8,
                             EGL_RENDERABLE_TYPE,
                             EGL_OPENGL_ES2_BIT,
                             EGL_NONE};

  static const EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2,
                                           EGL_NONE};

  egl_display = eglGetDisplay((EGLNativeDisplayType)display);
  if (egl_display == EGL_NO_DISPLAY) {
    fprintf(stderr, "Can't create egl display\n");
    exit(1);
  } else {
    fprintf(stderr, "Created egl display\n");
  }

  if (eglInitialize(egl_display, &major, &minor) != EGL_TRUE) {
    fprintf(stderr, "Can't initialise egl display\n");
    exit(1);
  }
  printf("EGL major: %d, minor %d\n", major, minor);

  eglGetConfigs(egl_display, NULL, 0, &count);
  printf("EGL has %d configs\n", count);

  configs = calloc(count, sizeof *configs);

  eglChooseConfig(egl_display, config_attribs, configs, count, &n);

  for (i = 0; i < n; i++) {
    eglGetConfigAttrib(egl_display, configs[i], EGL_BUFFER_SIZE, &size);
    printf("Buffer size for config %d is %d\n", i, size);
    eglGetConfigAttrib(egl_display, configs[i], EGL_RED_SIZE, &size);
    printf("Red size for config %d is %d\n", i, size);

    // just choose the first one
    egl_conf = configs[i];
    break;
  }

  egl_context =
      eglCreateContext(egl_display, egl_conf, EGL_NO_CONTEXT, context_attribs);
}

static void get_server_references(void) {
  display = wl_display_connect(NULL);
  if (display == NULL) {
    fprintf(stderr, "Can't connect to display\n");
    exit(1);
  }
  printf("connected to display\n");

  struct wl_registry *registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_listener, NULL);

  wl_display_dispatch(display);
  wl_display_roundtrip(display);

  if (compositor == NULL || shell == NULL) {
    fprintf(stderr, "Can't find compositor or shell\n");
    exit(1);
  } else {
    fprintf(stderr, "Found compositor and shell\n");
  }
}

int main(int argc, char **argv) {
  get_server_references();

  surface = wl_compositor_create_surface(compositor);
  if (surface == NULL) {
    fprintf(stderr, "Can't create surface\n");
    exit(1);
  } else {
    fprintf(stderr, "Created surface\n");
  }

  shell_surface = wl_shell_get_shell_surface(shell, surface);
  wl_shell_surface_set_toplevel(shell_surface);

  create_opaque_region();
  init_egl();
  create_window();

  while (wl_display_dispatch(display) != -1) {
    ;
  }

  wl_display_disconnect(display);
  printf("disconnected from display\n");

  exit(0);
}