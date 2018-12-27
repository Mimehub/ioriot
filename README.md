# I/O Riot

## Overview

I/O Riot is an I/O benchmarking tool for Linux based operating systems which captures I/O operations on a (possibly production) server in order to replay the exact same I/O operations on a load test machine.

I/O Riot is operated in 5 steps:

1. Capture: Record all I/O operations over a given period of time to a capture log.
2. Initialize: Copy the log to a load test machine and initialize the load test environment.
3. Replay: Drop all OS caches and replay all I/O operations.
4. Analyze: Look at the OS and hardware stats (throughput, I/O ops, load average) from the run phase and draw conclusions. The aim is to identify possible I/O bottlenecks.
5. Repeat: Repeat steps 2-4 multiple times but adjust OS and hardware settings in order to improve I/O performance.

Examples of OS and hardware settings and adjustments:

* Change of system parameters (file system mount options, file system caching, file system type, file system creation flags).
* Replay the I/O at different speed(s).
* Replay the I/O with modified pattern(s) (e.g. remove reads from the replay journal).
* Replay the I/O on different types of hardware.

The file system fragmentation (depending on the file system type and utilisation) might affect I/O performance as well. Therefore, replaying the I/O will not give the exact same result as on a production system. But it provides a pretty good way to determine I/O bottlenecks. As a rule of thumb file system fragmentation will not be an issue, unless the file system begins to fill up. Modern file systems (such as Ext4) will slowly start to suffer from fragmentation and slow down then.

## Benefits

In contrast to traditional I/O benchmarking tools, I/O Riot reproduces real production I/O, and does not rely on a pre-defined set of I/O operations.

Also, I/O Riot only requires a server machine for capturing and another server machine for replaying. A traditional load test environment would usually be a distributed system which can consist of many components and machines. Such a distributed system can become quite complex which makes it difficult to isolate possible I/O bottlenecks. For example in order to trigger I/O events a client application would usually have to call a remote server application. The remote server application itself would query a database and the database would trigger the actual I/O operations in Linux. Furthermore, it is not easy to switch forth and back between hardware and OS settings. For example without a backup and restore procedure a database would most likely be corrupt after reformatting the data partitions with a different file system type.

The benefits of I/O Riot are:

* It is easy to determine whether a new hardware type is suitable for an already existing application.
* It is easy to change OS and hardware for performance tests and optimizations.
* Findings can be applied to production machines in order to optimize OS configuration and to save hardware costs.
* Benchmarks are based on production I/O patterns and not on artificial I/O patterns.
* Log files can be modified to see whether a change in the application behavior would improve I/O performance (without actually touching the application code)
* Log files could be generated synthetically in order to find out how a new application would perform (even if there isn't any code for the new application yet)
* It identifies possible flaws in the applications (e.g. Java programs which produce I/O operations on the server machines). Findings can be reported to the corresponding developers so that changes can be introduced to improve the applications I/O performance.
* It captures I/O in Linux Kernel space (very efficient, no system slowdowns even under heavy I/O load)
* It replays I/O via a tool developed in C with as little overhead as possible.

# Send in patches

Patches of any kind (bug fixes, new features...) are welcome! I/O Riot is new software and not everything might be perfect yet. Also, I/O Riot is used for a very specific use case at Mimecast. It may need tuning or extension for your use case. It will grow and mature over time.

This is also potentially a great tool just for analysing (not replaying) the I/O, therefore it would be a great opportunity to add more features related to that (e.g. more stats, filters, etc.).

Future work will also include file hole support and I/O support for memory mapped files.

# Installation

I/O Riot depends on SystemTap and a compatible version of the Linux Kernel. To get started have a read through the [installation guide](docs/markdown/installation.md).

# Operating I/O Riot

Check out the [I/O Riot usage guide](docs/markdown/usage.md) for a full usage workflow demonstration.

# Appendix

## Supported file systems

Currently I/O Riot supports replaying I/O on ``ext2``, ``ext3``, ``ext4`` and ``xfs``. However, it should be straightforward add additional file systems. 

## Supported syscalls

Currently, these file I/O related syscalls are supported (as of CentOS 7):

```code
open
openat
lseek
llseek
fcntl
creat
write
writev
unlink
unlinkat
rename
renameat
renameat2
read
readv
readahead - Initial support only
readdir
readlink
readlinkat
fdatasync
fsync
sync_file_range - Initial support only
sync
syncfs
close
getdents
mkdir
rmdir
mkdirat
stat
statfs - Initial support only
statfs64 - Initial support only
fstatfs - Initial support only
fstatfs64 - Initial support only
lstat
fstat
fstatat
chmod
fchmodat
fchmod
chown
chown16
lchown
lchown16
fchown
fchown16
fchownat
mmap2 - Initial support only
mremap - Initial support only
munmap - Initial support only
msync - Initial support only
exit_group - To detect process termination (closing all open file handles)
```

## Source code documentation

The documentation of the source code can be generated via the Doxygen Framework. To install doxygen run ``sudo yum install doxygen`` and to generate the documentation run ``make doxygen`` in the top level source directory.  Once done, the resulting documentation can be found in the ``docs/html`` subfolder of the project. It is worthwhile to start from ``ioriot/src/main.c`` and read your way through. Functions are generally documented in the header files. Exceptions are static functions which don't have any separate declarations.
