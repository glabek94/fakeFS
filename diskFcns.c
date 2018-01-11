//
// Created by dawid on 08.01.18.
//

#include <errno.h>
#include <memory.h>
#include "diskFcns.h"

void readSuperblock(struct superBlock* superBlock)
{
    fseek(diskFile, 0, SEEK_SET);
    fread(superBlock, sizeof(struct superBlock), 1, diskFile);
}

void writeSuperblock(struct superBlock* superBlock)
{
    writeBlock(0, (void*) superBlock, sizeof(struct superBlock), superBlock);
}

void writeEmptyBlock(size_t where, struct superBlock* superBlock)
{
    char buf[superBlock->blockSize];
    memset(buf, 0, sizeof(buf));
    writeBlock(where, buf, superBlock->blockSize, superBlock);
}

void readBlock(size_t block, char* data, size_t size, struct superBlock* superBlock)
{
    fseek(diskFile, block * superBlock->blockSize, SEEK_SET);
    fread(data, size, 1, diskFile);
}

void writeBlock(size_t block, const char* data, size_t size, struct superBlock* superBlock)
{
    fseek(diskFile, block * superBlock->blockSize, SEEK_SET);
    fwrite(data, size, 1, diskFile);
}

size_t findBlockOfFile(struct fileStruct* file, size_t offset, struct superBlock* superBlock)
{
    if (offset >= file->size)
    {
        return -ENOENT;
    }

    size_t currentBlock = file->firstBlock;
    size_t block = offset / superBlock->blockSize;

    for (int i = 0; i < block; i++)
    {
        currentBlock = findNextBlockInChain(currentBlock, superBlock);
    }

    return currentBlock;
}

size_t findEmptyBlock(struct superBlock* superBlock)
{
    fseek(diskFile, superBlock->blockSize * superBlock->blockFAT, SEEK_SET);
    size_t toReturn;
    fread(&toReturn, sizeof(size_t), 1, diskFile); //first value of FAT table should be always 0

    for (size_t i = 1; i < superBlock->nBlocks; i++)
    {
        size_t curRead;
        fread(&curRead, sizeof(size_t), 1, diskFile);

        if (curRead == 0)
        {
            return i;
        }
    }

    return toReturn;
}

bool findFile(const char* name, struct fileStruct* file, struct superBlock* superBlock)
{
    size_t currentBlock = superBlock->blockRootDir;
    size_t filesInBlock = superBlock->blockSize / sizeof(struct fileStruct);
    struct fileStruct currentFile;
    while (1)
    {
        fseek(diskFile, superBlock->blockSize * currentBlock, SEEK_SET);
        for (size_t i = 0; i < filesInBlock; i++)
        {
            fread(&currentFile, sizeof(struct fileStruct), 1, diskFile);

            if (currentFile.inUse && strcmp(name, currentFile.name) == 0)
            {
                *file = currentFile;
                return true;
            }
        }

        size_t newBlock = findNextBlockInChain(currentBlock, superBlock);
        if (newBlock == currentBlock)
        {
            return false;
        }
        else
        {
            currentBlock = newBlock;
        }
    }
}

size_t allocateNewBlock(size_t currentLastBlock, struct superBlock* superBlock)
{
    size_t newBlock = findEmptyBlock(superBlock);
    if (newBlock != 0)
    {
        size_t pos = superBlock->blockFAT * superBlock->blockSize;
        pos += currentLastBlock * sizeof(size_t);
        fseek(diskFile, pos, SEEK_SET);
        fwrite(&newBlock, sizeof(size_t), 1, diskFile);

        pos = superBlock->blockFAT * superBlock->blockSize;
        pos += newBlock * sizeof(size_t);
        fseek(diskFile, pos, SEEK_SET);
        fwrite(&newBlock, sizeof(size_t), 1, diskFile);

        return newBlock;
    }
    return 0;
}

void deallocateBlock(size_t block, struct superBlock* superBlock)
{
    size_t pos = superBlock->blockFAT * superBlock->blockSize;
    pos += block * sizeof(size_t);
    fseek(diskFile, pos, SEEK_SET);
    size_t toWrite = 0;
    fwrite(&toWrite, sizeof(size_t), 1, diskFile);

    writeEmptyBlock(block, superBlock);
}

bool updateFile(const char* name, struct fileStruct* file, struct superBlock* superBlock)
{
    size_t currentBlock = superBlock->blockRootDir;
    size_t filesInBlock = superBlock->blockSize / sizeof(struct fileStruct);
    struct fileStruct currentFile;
    while (1)
    {
        fseek(diskFile, superBlock->blockSize * currentBlock, SEEK_SET);
        for (size_t i = 0; i < filesInBlock; i++)
        {
            fread(&currentFile, sizeof(struct fileStruct), 1, diskFile);

            if (currentFile.inUse && strcmp(name, currentFile.name) == 0)
            {
                fseek(diskFile, -sizeof(struct fileStruct), SEEK_CUR);
                fwrite(file, sizeof(struct fileStruct), 1, diskFile);
                return true;
            }
        }

        size_t newBlock = findNextBlockInChain(currentBlock, superBlock);
        if (newBlock == currentBlock)
        {
            return false;
        }
        else
        {
            currentBlock = newBlock;
        }
    }
}

bool createFile(const char* name, struct superBlock* superBlock)
{
    size_t newOffset;
    if (findFreeFilePosInRoot(&newOffset, superBlock))
    {
        size_t newBlock;
        newBlock = findEmptyBlock(superBlock);
        if (newBlock == 0)
        {
            return false;
        }

        fseek(diskFile, superBlock->blockFAT * superBlock->blockSize + newBlock * sizeof(size_t), SEEK_SET);
        fwrite(&newBlock, sizeof(size_t), 1, diskFile);
        fflush(diskFile);
        struct fileStruct newFile;
        newFile.firstBlock = newBlock;
        strcpy(newFile.name, name);
        newFile.size = 0;
        newFile.inUse = true;

        fseek(diskFile, newOffset, SEEK_SET);
        fwrite(&newFile, sizeof(struct fileStruct), 1, diskFile);
        return true;
    }
    else
    {
        return false;
    }

}

size_t findNextBlockInChain(size_t block, struct superBlock* superBlock)
{
    size_t pos = superBlock->blockFAT * superBlock->blockSize;
    pos += block * sizeof(size_t);

    fseek(diskFile, pos, SEEK_SET);
    size_t toReturn;
    fread(&toReturn, sizeof(size_t), 1, diskFile);
    return toReturn;
}

bool findFreeFilePosInRoot(size_t* foundOffset, struct superBlock* superBlock)
{
    size_t currentBlock = superBlock->blockRootDir;
    size_t filesInBlock = superBlock->blockSize / sizeof(struct fileStruct);
    struct fileStruct currentFile;
    while (1)
    {
        fseek(diskFile, superBlock->blockSize * currentBlock, SEEK_SET);
        for (size_t i = 0; i < filesInBlock; i++)
        {
            fread(&currentFile, sizeof(struct fileStruct), 1, diskFile);

            if (!currentFile.inUse)
            {
                *foundOffset = superBlock->blockSize * currentBlock + i * sizeof(struct fileStruct);
                return true;
            }
        }

        size_t newBlock = findNextBlockInChain(currentBlock, superBlock);
        if (newBlock == currentBlock) //last pos in root dir
        {
            newBlock = allocateNewBlock(newBlock, superBlock);
            if (newBlock != 0)
            {
                *foundOffset = superBlock->blockSize * newBlock;
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            currentBlock = newBlock;
        }
    }
}