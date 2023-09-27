#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

extern size_t serial_write(const void *buf, size_t offset, size_t len);
extern size_t events_read(void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum
{
    FD_STDIN,
    FD_STDOUT,
    FD_STDERR,
    FD_FB,
    FD_KEYBORAD
};

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
    [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
    [FD_FB] = {},
    [FD_KEYBORAD] = {"/dev/events", 0, 0, events_read, invalid_write},
#include "files.h"
};

int fs_open(const char *path,int flag,word_t mode){
    for (int i = 0; i < LENGTH(file_table);i++){
        Log("Hello");
        if (strcmp(path, file_table[i].name)==0){
            file_table[i].open_offset = 0;
            return i;
        }
    }
    // return -1;
    Log("the path file %s is zero", path);
    assert(0);
}

int fs_read(int fd, void *buf, size_t count){
    if (((file_table[fd].size - file_table[fd].open_offset) < count) && (file_table[fd].size != 0))
        count = file_table[fd].size - file_table[fd].open_offset;
    if (file_table[fd].read != NULL){
        size_t read_num = file_table[fd].read(buf, file_table[fd].open_offset, count);
        file_table[fd].open_offset += count;
        return read_num;
    }
    size_t read_num=ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, count);
    file_table[fd].open_offset += count;
    return read_num;
}

int fs_write(int fd, const void *buf, size_t count){
    if (((file_table[fd].size - file_table[fd].open_offset) < count) && (file_table[fd].size!=0))
        count = file_table[fd].size - file_table[fd].open_offset;
    if (file_table[fd].write != NULL){
        size_t write_num = file_table[fd].write(buf, file_table[fd].open_offset, count);
        file_table[fd].open_offset += count;
        return write_num;
    }
    size_t write_num=ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, count);
    file_table[fd].open_offset += count;
    return write_num;
}

size_t fs_lseek(int fd, size_t offset, int whence){
    if(offset + file_table[fd].size==0){
        return 0;
    }
    switch (whence){
    case SEEK_SET:
        file_table[fd].open_offset = offset;
        break;
    case SEEK_CUR:
        file_table[fd].open_offset += offset;
        break;
    case SEEK_END:
        file_table[fd].open_offset += (offset + file_table[fd].size);
        break;
    default:
        panic("unknow value");
        break;
    }
    if (file_table[fd].open_offset <= file_table[fd].size){
        return file_table[fd].open_offset;
    }
    else{
        file_table[fd].open_offset = file_table[fd].size;
        return -1;
    }
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
}
