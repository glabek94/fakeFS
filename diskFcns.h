//
// Created by dawid on 08.01.18.
//

#ifndef FAKEFS_DISKFCNS_H
#define FAKEFS_DISKFCNS_H

#include <stdio.h>
#include <stdbool.h>
#include "structs.h"

FILE *diskFile;

void readSuperblock(struct superBlock *superBlock); //OK

void writeSuperblock(struct superBlock *superBlock); //OK

void writeEmptyBlock(size_t where, struct superBlock *superBlock); //OK

bool findFile(const char *name, struct fileStruct *file, struct superBlock *superBlock);

bool updateFile(const char *name, struct fileStruct *file, struct superBlock *superBlock);

bool createFile(const char* name, struct superBlock *superBlock);

void readBlock(size_t block, char *data, size_t size, struct superBlock *superBlock);

void writeBlock(size_t block, const char *data, size_t size, struct superBlock *superBlock);

size_t findBlockOfFile(struct fileStruct *file, size_t offset, struct superBlock *superBlock);

size_t allocateNewBlock(size_t currentLastBlock, struct superBlock *superBlock);

size_t findEmptyBlock(struct superBlock *superBlock); //OK

size_t findNextBlockInChain(size_t block, struct superBlock *superBlock);

bool findFreeFilePosInRoot(size_t *foundPos, struct superBlock *superBlock);

#endif //FAKEFS_DISKFCNS_H
