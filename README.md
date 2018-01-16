# fakeFS

Project on Operating Systems course at WUT.

### About
Implementation of fake FAT-like filesystem using FUSE library. It uses 512B blocks and only one root directory. Adding and removing files work. Copying to and from fakeFS must be done with dd with blocksize of 512B. For example:
```shell
dd if=foo of=fakeFS/bar bs=512
dd if=fakeFS/bar of=file bs=512
```
### Usage
Creating new disk file:
```shell
./fakeFS -c file sizeInKB
```
Mounting disk file:
```shell
./fakeFS -m diskFile mountingPoint
