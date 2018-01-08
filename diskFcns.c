//
// Created by dawid on 08.01.18.
//

#include <errno.h>
#include "diskFcns.h"

void readBlock(size_t block, char *data, size_t size, struct superBlock *superBlock)
{
    fseek(diskFile, block * superBlock->blockSize, SEEK_SET);
    fread(data, size, 1, diskFile);
}

void writeBlock(size_t block, const char *data, size_t size, struct superBlock *superBlock)
{
    fseek(diskFile, block * superBlock->blockSize, SEEK_SET);
    fwrite(data, size, 1, diskFile);
}

size_t findBlockOfFile(struct fileStruct *file, size_t offset, struct superBlock *superBlock)
{
    if (offset >= file->size)
    {
        return -ENOENT;
    }

    size_t currentBlock = file->firstBlock;
    size_t block = offset / superBlock->blockSize;

    for (int i = 0; i < block; i++)
    {
        fseek(diskFile, superBlock->blockFAT * superBlock->blockSize, SEEK_SET);
        fread(&currentBlock, sizeof(size_t), 1, diskFile);
    }

    return currentBlock;
}

size_t findEmptyBlock(struct superBlock *superBlock)
{

}