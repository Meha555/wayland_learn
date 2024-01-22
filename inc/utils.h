#ifndef UTILS_H
#define UTILS_H

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

enum color_enum { BLACK = 0x0, WHITE = 0xffffff };

#endif  // UTILS_H