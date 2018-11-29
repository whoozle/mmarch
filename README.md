# Mapped Memory Friendly Archive Format - MMArch

## Goals

* Provide extensible platform-independent file archive format (what an invention!)
* Effective in-memory access, O(1) full-path file access, O(N) readdir (file lists are immediately available)
* Memory mapped - Rely on OS caching instead of caching in userspace.

## Structure

* c/ C-library
* python/ python archiver
* unpack/ simple unpacker, test tool for c library


## File Format

Some design considerations regarding format:
* Use only 32/64 bit types for effective aligned access
* No obscure arithmetics, all of offsets in header — absolute
* Group all headers at first 4G, saving on inner offsets
* Could be either LE or BE

```

1. File Header

u32     "MARC" magic, "CRAM"
u32     version, current == 1 (still experimental)
u32     page size, normally 4096, but you can use any arbitrary number, alignment for file data
u32     total header size. If you want to mmap header, use this number (aligned to page boundary)
u64     total file size. Aligned to page size. Reader shall not read anything past this point.

u32     object record table offset
u32     filename table offset
u32     readdir table offset

2. Object Record Table Header
u32     record fields counter, R (could be viewed as version, every record here has R*4 bytes size)
u32     record count, N. Total record counter. Size of this table is N * R + this header
u32     directory count, DN, all IDs below DN is directories

N *

3. Object Record, each of size R * u32
u64     file data offset (0 for directories)
u64     file size (0 for directories)
u32     full path offset (usually string pool below, must've been within header size)
u32     full path length (strings ARE NOT zero terminated, you shall read exact name size bytes from name offset)

[string pool]

4. Filename table
u32     hash function id. (0 - FNV1, 1 - FNV1A, 2 - PYTHON, 3 - R5A)
u32     bucket count, B

Filename table index:
(B + 1) * u32
u32     offset to bucket 0
u32     offset to bucket 1
…
u32     offset to the end of bucket B-1

Filename table entry:
u32     object id (see object record table, p.2)

5. Readdir table
(D + 1) * readdir table index follows
u32     offset to object list for directory 0
u32     offset to object list for directory 1
…
u32     offset to the end of for directory D - 1

Readdir table entry
u32     object id (see object record table, p.2)
u32     local name offset (no path)
u32     local name length

[gap to align to page size]
[file data referenced by offset/size in object table]
[gap to align to page size]
[file data referenced by offset/size in object table]
[gap to align to page size]
[file data referenced by offset/size in object table]


```
