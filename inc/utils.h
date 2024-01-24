#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

// #define HAS_XDG_

#define ERROR_CHECK(handle, errormsg, successmsg) \
  do {                                            \
    if (!handle) {                                \
      fprintf(stderr, errormsg "\n");             \
      exit(EXIT_FAILURE);                         \
    } else                                        \
      fprintf(stderr, successmsg "\n");           \
  } while (0)

#define BIND_WL_REG(registry, ptr, id, intf, n)                   \
  do {                                                            \
    (ptr) = (typeof(ptr))wl_registry_bind(registry, id, intf, n); \
  } while (0)

#define ASSERT_MSG(cond, fmt, ...)              \
  do {                                          \
    if (!(cond)) {                              \
      fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
      exit(EXIT_FAILURE);                       \
    }                                           \
  } while (0)

enum color_enum {
  BLACK = 0x0,
  WHITE = 0xffffff,
  RED = 0xFF0000,
  ORRANGE = 0xFF7F00,
  YELLOW = 0xFFFF00,
  GREEN = 0x00FF00,
  CYAN = 0x00FFFF,
  BLUE = 0x0000FF,
  PURPLE = 0x8B00FF,
};

int set_cloexec_or_close(int fd);
int create_tmpfile_cloexec(char* tmpname);
int os_create_anonymous_file(off_t size);

#endif  // UTILS_H