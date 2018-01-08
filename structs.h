//
// Created by dawid on 08.01.18.
//

#ifndef FAKEFS_STRUCTS_H
#define FAKEFS_STRUCTS_H

#include <stdint.h>
#include <stddef.h>

struct superBlock
{
    char fsName[12];
    uint8_t blockSize;
    size_t nBlocks;
    size_t blockFAT;
    size_t nBlocksFAT;
    size_t blockRootDir;
};

struct fileStruct
{
    bool inUse;
    char name[12];
    size_t firstBlock;
    size_t size;
};

#endif //FAKEFS_STRUCTS_H
