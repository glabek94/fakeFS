//
// Created by dawid on 08.01.18.
//

#ifndef FAKEFS_DISKFCNS_H
#define FAKEFS_DISKFCNS_H

#include <stdio.h>
#include <stdbool.h>
#include "structs.h"

FILE *diskFile;

void readSuperblock(struct superBlock *superBlock);

void writeSuperblock(struct superBlock *superBlock);

bool findFile(const char *name, struct fileStruct *file, struct superBlock *superBlock);

bool updateFile(const char *name, struct fileStruct *file, struct superBlock *superBlock);

void readBlock(size_t block, char *data, size_t size, struct superBlock *superBlock);

void writeBlock(size_t block, const char *data, size_t size, struct superBlock *superBlock);

size_t findBlockOfFile(struct fileStruct *file, size_t offset, struct superBlock *superBlock);

size_t allocateNewBlock(struct fileStruct *file, struct superBlock *superBlock);

size_t findEmptyBlock(struct superBlock *superBlock);

#endif //FAKEFS_DISKFCNS_H
