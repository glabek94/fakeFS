//
// Created by dawid on 08.01.18.
//

#include <errno.h>
#include <memory.h>
#include "fuseFcns.h"
#include "diskFcns.h"

static int fakeFS_getattr(const char* path, struct stat* stbuf)
{
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) //root dir
    {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    else
    {
        struct fileStruct file;
        if (findFile(path + 1, &file, &superBlock))
        {
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_nlink = 1;
            stbuf->st_size = file.size;
            return 0;
        }
        else
        {
            return -ENOENT;
        }
    }
}

static int fakeFS_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi)
{
    if (strcmp(path, "/") == 0) //only root dir
    {
        size_t numOfFiles = superBlock.blockSize / sizeof(struct fileStruct);
        struct fileStruct file[numOfFiles];
        size_t currentBlock = superBlock.blockRootDir;

        while (1)
        {
            readBlock(currentBlock, (char*) file, superBlock.blockSize, &superBlock);
            for (size_t i = 0; i < numOfFiles; i++)
            {
                if (file[i].inUse)
                {
                    filler(buf, file[i].name, NULL, 0);
                }
            }

            size_t newBlock = findNextBlockInChain(currentBlock, &superBlock);
            if (newBlock == currentBlock)
            {
                break;
            }
            else
            {
                currentBlock = newBlock;
            }
        }
        return 0;
    }

    return -ENOENT;
}

static int fakeFS_mknod(const char* path, mode_t mode, dev_t rdev)
{
    if (createFile(path + 1, &superBlock))
    {
        return 0;
    }
    else
    {
        return -ENOENT;
    }
}

static int fakeFS_unlink(const char* path)
{
    struct fileStruct fileToRemove;
    if (!findFile(path + 1, &fileToRemove, &superBlock))
    {
        return -ENOENT;
    }

    size_t block = findBlockOfFile(&fileToRemove, 0, &superBlock);

    while (true)
    {
        size_t nextBlock = findNextBlockInChain(block, &superBlock);
        deallocateBlock(block, &superBlock);

        if (nextBlock == block)
        {
            break;
        }
        block = nextBlock;
    }

    fileToRemove.inUse = false;
    fileToRemove.size = 0;
    fileToRemove.firstBlock = 0;
    updateFile(path + 1, &fileToRemove, &superBlock);
    return 0;
}

static int fakeFS_open(const char* path, struct fuse_file_info* fi)
{
    struct fileStruct file;
    if (!findFile(path + 1, &file, &superBlock))
    {
        return -ENOENT;
    }
    return 0;
}

static int fakeFS_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
    struct fileStruct file;

    if (!findFile(path + 1, &file, &superBlock))
    {
        return -ENOENT;
    }

    if (offset >= file.size)
    {
        return 0;
    }

    size_t blocksToRead = size / superBlock.blockSize;

    size_t block = findBlockOfFile(&file, offset, &superBlock);
    readBlock(block, buf, superBlock.blockSize, &superBlock);

    size_t readBlocks = 1;
    for (readBlocks = 1; readBlocks < blocksToRead; readBlocks++)
    {
        size_t newBlock = findNextBlockInChain(block, &superBlock);
        if (block == newBlock)
        {
            break;
        }
        block = newBlock;
        readBlock(block, buf + readBlocks * superBlock.blockSize, superBlock.blockSize, &superBlock);
    }

    size_t actuallyRead = readBlocks * superBlock.blockSize;
    if (offset + actuallyRead > file.size)
    {
        return file.size - offset;
    }
    return readBlocks * superBlock.blockSize;
}

static int fakeFS_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
    struct fileStruct file;

    if (!findFile(path + 1, &file, &superBlock))
    {
        return -ENOENT;
    }

    size_t newBlock = file.firstBlock;
    if (offset + size > file.size) //each write must be to new block
    {
        if (file.size != 0) //empty file has first block already allocated
        {
            size_t fileBlocks = offset / superBlock.blockSize;
            for (size_t i = 1; i < fileBlocks; i++)
            {
                newBlock = findNextBlockInChain(newBlock, &superBlock);
            }
            newBlock = allocateNewBlock(newBlock, &superBlock);
        }

        if (newBlock == 0) //no free blocks
        {
            return 0;
        }

        writeBlock(newBlock, buf, size, &superBlock);
        file.size += size;
        updateFile(path + 1, &file, &superBlock);
        return size;
    }

    return 0;
}

static struct fuse_operations fakeFS_oper = {
        .getattr    = fakeFS_getattr, //necessary for everything
        .readdir    = fakeFS_readdir, //to read root dir, hmm... is it necessary? assume that is is necessary
        .mknod      = fakeFS_mknod, //to create new file -> copy
        .unlink     = fakeFS_unlink, //for removing
        .open       = fakeFS_open, //to cp
        .read       = fakeFS_read, //to cp
        .write      = fakeFS_write, //to cp
};

int fakeFS_start(const char* mountPoint)
{
    char* newArgv[] = {"abc", "-d", (char*) mountPoint};
    return fuse_main(3, newArgv, &fakeFS_oper, NULL);
}

