# Mapped Memory Friendly Archive Format - MMArch

## Goals
* Provide extensible platform-independent file archive format (what an invention!)
* Effective in-memory access, O(1) full-path file access, O(N) readdir (file lists are immediately available)
* Memory mapped - Rely on OS caching instead of caching in userspace.
