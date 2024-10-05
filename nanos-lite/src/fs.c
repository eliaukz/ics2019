#include "fs.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;    // 文件名
  size_t size;   // 文件大小
  size_t disk_offset;  // 文件在ramdisk中的偏移
  size_t open_offset;  // 文件被打开之后的读写指针
  ReadFn read;         // 读函数指针
  WriteFn write;  // 写函数指针
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, invalid_read, invalid_write},
  {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}


int fs_open(const char *pathname, int flags, int mode){
  for (int i = 3; i < NR_FILES; i++) {
    if (strcmp(pathname, file_table[i].name) == 0) return i;
  }
  printf("file open failed\n");
  assert(0);
  return 0;
}


size_t fs_read(int fd, void *buf, size_t len){
  if (file_table[fd].open_offset + len >= file_table[fd].size)
    len = file_table[fd].size - file_table[fd].open_offset;
  
  if (len < 0) len = 0;

  if (file_table[fd].read == NULL){
    ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset,
                 len);
  } else {
    len = file_table[fd].read(
        buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
  }
  file_table[fd].open_offset += len;
  return len;
}

size_t fs_write(int fd, const void *buf, size_t len){
  size_t length = 0;
  if (fd == 1 || fd == 2) {
    for (int i = 0; i < len; i++) {
      _putc(((char *)buf)[i]);
    }
    length = len;
  }
  if (file_table[fd].open_offset + len >= file_table[fd].size)
    len = file_table[fd].size - file_table[fd].open_offset;
  if (fd > 2) {
    if(file_table[fd].write==NULL)
      length = ramdisk_write(
          buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
    else
      length = file_table[fd].write(
          buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
    file_table[fd].open_offset += length;
  }
  return length;

}


size_t fs_lseek(int fd, size_t offset, int whence){
  switch (whence) {
    case SEEK_SET:
      file_table[fd].open_offset = offset;
      break;
    case SEEK_CUR:
      file_table[fd].open_offset += offset;
      break;
    case SEEK_END:
      file_table[fd].open_offset = file_table[fd].size + offset;
      break;
  }
  return file_table[fd].open_offset;
}

int fs_close(int fd){
  file_table[fd].open_offset = 0;
  return 0;
}