//
// Created by dawid on 08.01.18.
//

#include <errno.h>
#include "fuseFcns.h"
#include "diskFcns.h"

static int fakeFS_getattr(const char *path, struct stat *stbuf)
{

}

static int fakeFS_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{

}

static int fakeFS_mknod(const char *path, mode_t mode, dev_t rdev)
{

}

static int fakeFS_unlink(const char *path)
{

}

static int fakeFS_open(const char *path, struct fuse_file_info *fi)
{
    struct fileStruct file;
    if(!findFile(path, &file, &superBlock))
    {
        return -ENOENT;
    }
     return 0;
}

static int fakeFS_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    struct fileStruct file;

    if (!findFile(path, &file, &superBlock))
    {
        return -ENOENT;
    }

    if (offset + size > file.size)
    {
        return 0;
    }

    size_t block = findBlockOfFile(&file, offset, &superBlock);
    readBlock(block, buf, size, &superBlock);
    return size;
}

static int fakeFS_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    struct fileStruct file;

    if (!findFile(path, &file, &superBlock))
    {
        return -ENOENT;
    }

    size_t newBlock;
    if (offset + size > file.size) //each write must be to new block
    {
        if (file.size == 0) //empty file has allocated first block
        {
            newBlock = findBlockOfFile(&file, 0, &superBlock);
        }
        else
        {
            newBlock = allocateNewBlock(&file, &superBlock);
        }
    }

    if (newBlock == 0) //no free blocks
    {
        return -ENOENT;
    }

    writeBlock(newBlock, buf, size, &superBlock);
    file.size += size;
    updateFile(path, &file, &superBlock);
    return size;
}