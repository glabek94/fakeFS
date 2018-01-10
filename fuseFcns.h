//
// Created by dawid on 08.01.18.
//

#ifndef FAKEFS_FUSEFCNS_H
#define FAKEFS_FUSEFCNS_H

#define FUSE_USE_VERSION 26

#include "fuse.h"
#include "structs.h"

struct superBlock superBlock;

static int fakeFS_getattr(const char *path, struct stat *stbuf);

static int fakeFS_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

static int fakeFS_mknod(const char *path, mode_t mode, dev_t rdev);

static int fakeFS_unlink(const char *path);

static int fakeFS_open(const char *path, struct fuse_file_info *fi);

static int fakeFS_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

static int fakeFS_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int fakeFS_start(const char* mountPoint);


#endif //FAKEFS_FUSEFCNS_H
