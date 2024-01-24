#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 以下内容来自于wayland/cursor，或者weston/shared

// 设置文件描述符的CLOEXEC标志
int set_cloexec_or_close(int fd) {
  long flags;
  if (fd == -1) return -1;
  flags = fcntl(fd, F_GETFD);  // 获取文件描述符属性
  if (flags == -1) goto err;
  if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)  // 设置close-on-exec属性
    goto err;
  return fd;
err:
  close(fd);
  return -1;
}

// 创建一个具有CLOEXEC标志的临时文件
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

// 创建一个匿名文件，大小为指定的size字节
int os_create_anonymous_file(off_t size) {
  static const char template[] = "/wayland-shared-XXXXXX";
  const char* path;
  char* name;
  int fd;

  path = getenv("XDG_RUNTIME_DIR");  // 这里将文件创建在 $XDG_RUNTIME_DIR 下
  if (!path) {
    errno = ENOENT;
    return -1;
  }

  name = malloc(strlen(path) + sizeof(template));  // 完整的文件名
  if (!name) return -1;

  // 拼接出完整的文件名
  strcpy(name, path);
  strcat(name, template);
  printf("File name: %s\n", name);

  fd = create_tmpfile_cloexec(name);

  free(name);

  if (fd < 0) return -1;

  // 对文件大小裁剪为size指定的字节数
  if (ftruncate(fd, size) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}