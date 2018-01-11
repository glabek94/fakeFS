#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "structs.h"
#include "diskFcns.h"
#include "fuseFcns.h"

void createNewDisk(size_t size, char* path); //size in bytes
void readDisk(char* path);

int main(int argc, char* argv[])
{
    char usage[] = "usage: -c [path] [size in kB]\n       -m [disk path] [mount point]\n";

    if (argc < 3)
    {
        printf("%s", usage);
        return 0;
    }

    if (strcmp(argv[1], "-c") == 0)
    {
        char* path = argv[2];
        size_t size = atol(argv[3]);
        createNewDisk(size * 1024, path);
        printf("Fake hardisk created. Name: %s, size: %ld.\n", path, size);
        return 0;
    }
    else if (strcmp(argv[1], "-m") == 0)
    {
        diskFile = fopen(argv[2], "r+b");
        readSuperblock(&superBlock);
        printf("Fake harddisk mounted. Blocksize: %ld, n blocks: %ld.\n", superBlock.blockSize, superBlock.nBlocks);
        return fakeFS_start(argv[3]);
    }
    else
    {
        printf("%s", usage);
        return 0;
    }
}

void createNewDisk(size_t size, char* path)
{
    diskFile = fopen(path, "wb");

    struct superBlock newSuper;
    char fsName[] = "fakeFS";
    strcpy(newSuper.fsName, fsName);

    newSuper.blockSize = 512;
    newSuper.nBlocks = size / newSuper.blockSize;
    newSuper.blockFAT = 1;
    newSuper.nBlocksFAT = newSuper.nBlocks * sizeof(newSuper.nBlocks) / newSuper.blockSize + 1;
    newSuper.blockRootDir = newSuper.nBlocksFAT + 1;

    writeSuperblock(&newSuper);

    for (size_t i = 1; i < newSuper.nBlocks; i++)
    {
        writeEmptyBlock(i, &newSuper);
    }

    //write FAT table
    fseek(diskFile, newSuper.blockSize, SEEK_SET);
    size_t nextBlock = 0;
    fwrite(&nextBlock, sizeof(size_t), 1, diskFile); //first block is for superblock
    for (int i = 1; i < newSuper.nBlocksFAT; i++) //next blocks are for FAT table
    {
        nextBlock = (size_t) i + 1; //current block of FAT table links to next block
        fwrite(&nextBlock, sizeof(size_t), 1, diskFile);
    }

    fwrite(&(newSuper.nBlocksFAT), sizeof(size_t), 1, diskFile); //and last block of FAT link to itself - end of chain
    fwrite(&(newSuper.blockRootDir), sizeof(size_t), 1,
           diskFile); //and next block is for first root dir block - end of chain

    fclose(diskFile);
}