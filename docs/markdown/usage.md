[Main page](../../README.md)

# I/O Riot usage

## 1. Capture

The following steps are required to capture all I/O operation of the entire (Linux) system to the file ``io.capture``. For efficiency and security it is only capturing the meta data (amount of bytes written and read) and not the actual data itself. It is also capturing the system time in microseconds and the process IDs (PIDs) and thread IDs (TIDs) used as well as all relevant options and flags of the corresponding I/O syscalls. It will stop capturing automatically after 60 minutes:

 * 1) Stop all applications on the machine. Otherwise the kernel module won't recognize any already opened file handles. Stopping the applications before starting with the capture is essential for tracing the flags in how the files were opened. All I/O operations on unknown file handles will be ignored otherwise.
 * 2) Run:

```sh
sudo ioriot -c ~/io.capture
```

 * 3) Start all applications again.
 * 4) To stop capturing I/O type Ctrl+C. Alternatively one hour for the Kernel module to auto exit.

To capture only I/O caused by Java process run:

```sh
sudo ioriot -c ~/io.capture -m javaioriot.ko
```

To capture the I/O of a specific process run the following respectively:

```sh
sudo ioriot -c ~/io.capture -m targetedioriot.ko -p PID
```

The resulting capture log looks like this and can be multiple GB in size:

```sh
#|capture_version=3|
t=1545902286266599478;:,D=6253;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/c;:,s=0;:,
t=1545902286266607592;:,D=3207;:,i=24702:25810;:,o=fcntl;:,d=27;:,F=2;:,G=1;:,s=0;:,
t=1545902286266618764;:,D=6769;:,i=24702:25810;:,o=getdents;:,d=27;:,c=32768;:,b=80;:,
t=1545902286266629658;:,D=24054;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/8;:,s=0;:,
t=1545902286266642396;:,D=16506;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/0/riot/20181130;:,s=0;:,
t=1545902286266651561;:,D=3753;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/0/riot/20181130;:,s=0;:,
t=1545902286266658507;:,D=24732;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/e;:,s=0;:,
t=1545902286266664305;:,D=1390;:,i=24702:25810;:,o=getdents;:,d=27;:,c=32768;:,b=0;:,
t=1545902286266671200;:,D=2627;:,i=24702:25810;:,o=close;:,d=27;:,s=0;:,
t=1545902286266675423;:,D=837;:,i=24702:25810;:,o=close;:,d=119;:,s=0;:,
t=1545902286266685659;:,D=23300;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/1;:,s=0;:,
t=1545902286266704122;:,D=14658;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/f;:,s=0;:,
t=1545902286266709835;:,D=2022;:,i=24702:25799;:,o=close;:,d=63;:,s=0;:,
t=1545902286266721094;:,D=40107;:,i=24702:25810;:,o=open;:,d=27;:,p=/store/02/1/1A1/20181105/b;:,f=0;:,m=0;:,
t=1545902286266732025;:,D=3190;:,i=24702:25810;:,o=fstat;:,d=27;:,s=0;:,
t=1545902286266734491;:,D=21159;:,i=24702:25799;:,o=close;:,d=67;:,s=0;:,
t=1545902286266745975;:,D=3737;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/f/riot;:,s=-2;:,
t=1545902286266756861;:,D=6131;:,i=24702:25799;:,o=open;:,d=67;:,p=/store/13/1/1A1/20181022/b;:,f=0;:,m=0;:,
t=1545902286266765401;:,D=2275;:,i=24702:25799;:,o=fstat;:,d=67;:,s=0;:,
t=1545902286266767643;:,D=12190;:,i=24702:25810;:,o=fcntl;:,d=27;:,F=3;:,G=139845964703088;:,s=32768;:,
t=1545902286266772388;:,D=1100;:,i=24702:25810;:,o=fcntl;:,d=27;:,F=2;:,G=1;:,s=0;:,
t=1545902286266782571;:,D=3904;:,i=24702:25799;:,o=fcntl;:,d=67;:,F=3;:,G=139846284661488;:,s=32768;:,
t=1545902286266787978;:,D=1227;:,i=24702:25799;:,o=fcntl;:,d=67;:,F=2;:,G=1;:,s=0;:,
t=1545902286266803693;:,D=28014;:,i=24702:25810;:,o=getdents;:,d=27;:,c=32768;:,b=432;:,
t=1545902286266810495;:,D=18988;:,i=24702:25799;:,o=getdents;:,d=67;:,c=32768;:,b=432;:,
t=1545902286266819391;:,D=3919;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/6;:,s=0;:,
t=1545902286266822623;:,D=14447;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/c;:,s=0;:,
t=1545902286266830423;:,D=3205;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/9;:,s=0;:,
t=1545902286266837505;:,D=4034;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/a;:,s=0;:,
t=1545902286266844222;:,D=2968;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/0;:,s=0;:,
t=1545902286266848050;:,D=14112;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/e;:,s=0;:,
t=1545902286266854966;:,D=3056;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/1;:,s=0;:,
t=1545902286266861804;:,D=3917;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/9;:,s=0;:,
t=1545902286266868780;:,D=3009;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/d;:,s=0;:,
t=1545902286266872547;:,D=14063;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/d;:,s=0;:,
t=1545902286266879442;:,D=3113;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/f;:,s=0;:,
t=1545902286266886259;:,D=3935;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/5;:,s=0;:,
t=1545902286266892951;:,D=2982;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/c;:,s=0;:,
t=1545902286266897104;:,D=14134;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/7;:,s=0;:,
t=1545902286266904170;:,D=3137;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/3;:,s=0;:,
t=1545902286266910438;:,D=3961;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/8;:,s=0;:,
t=1545902286266917133;:,D=2989;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/e;:,s=0;:,
t=1545902286266921997;:,D=14212;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/5;:,s=0;:,
t=1545902286266928972;:,D=3112;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/4;:,s=0;:,
t=1545902286266934468;:,D=13903;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/1;:,s=0;:,
t=1545902286266941300;:,D=3086;:,i=24702:25799;:,o=stat;:,p=/store/13/1/1A1/20181022/b/f;:,s=0;:,
t=1545902286266946528;:,D=13997;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/2;:,s=0;:,
t=1545902286266953372;:,D=3065;:,i=24702:25810;:,o=stat;:,p=/store/02/1/1A1/20181105/b/6;:,s=0;:,
```

### Using a RAMdisk (optional)

It is beneficial to write ``io.capture`` to a RAMdisk so that we are not interfering so much with the system I/O:

```sh
sudo mkdir -p /mnt/ramdisk
sudo mount -t tmpfs -o size=32g tmpfs /mnt/ramdisk
```

Make sure that there is enough system memory available for such a RAMdisk and all the processes running on the machine. Eventually, RAM will be taken away from the Linux caches which potentially could decrease system I/O performance. Run the following command to capture to the RAMdisk respectively:

```sh
sudo ioriot -c /mnt/ramdisk/io.capture
```

## 2. Initialize

### 2.1 Pre-process the capture log / generate a replay log

After producing ``io.capture`` it must be pre-processed. The resulting replay log introduces the following changes (for improving replay performance and make the parsing easier):

* Time stamps begin from 0
* Use of internal opcodes rather than strings (e.g. ``30`` instead of ``open``) for faster parsing.
* All operations on unknown file handles are _removed_.
* All incomplete or corrupt lines from the capture file are ignored. There may be corrupt lines in the capture file because SystemTap may skips a very few probe points if it decides that capturing I/O is causing too much overhead.
* Rewrite of all file paths. ``ioriot`` adds ``/.ioriot/NAME`` to all file paths for each file system mount point.

To generate the the replay log ``io.replay`` from the capture log ``io.capture`` run:

```sh
ioriot -c io.capture -r io.replay -n NAME -u USER
```

In which NAME is a freely chosen name and USER must be a valid system user. It is the system user under which the replay test will run. This command also creates all required top level directories such as ``/.ioriot/NAME/``, ``/mnt/.ioriot/NAME/`` in all mounted file systems. These are the directories where the replay test will read/write files from/to. These directories will belong to user USER.

``ioriot`` will filter out many operations, especially all operations on pseudo file systems (e.g. sysfs, procfs), as it does not make a lot of sense to replay I/O on these file systems. Also, I/O operations on unknown file handles will be filtered out as well. This can happen when we start capturing the I/O *after* an application already opened a file. As a result we won't see how the application opened that file. The best practise is to stop all applications on the machine first, start capturing the I/O, and start all applications again. This may be improved in future releases of I/O Riot.

The resulting replay log will look like this: At the first line there is a meta header. It contains information about the test configuration. The meta header is followed by all the I/O operations. At the end of the file is the INIT section. It lists all files (also their sizes) and directories required to be present before replaying the I/O.


```sh
#|num_timelines=509591|num_mapped_pids=19189|num_mapped_fds=4292067|num_lines=55040114|replay_version=1|user=ioriotuser|name=test0|init_offset=2578735248|
23|1|1|0|0|30|11|/usr/local/mimecast/.ioriot/test0/someapp/somesubdir/vd11-9:1|438|0|open@31|
23|1|1|0|0|0|11|0|fstat@32|
23|1|1|0|0|10|11|12|read@33|
23|1|1|0|0|0|11|0|fstat@34|
23|1|1|0|0|72|11|0|1|12|lseek@35|
23|1|1|0|0|10|11|0|read@36|
23|1|1|0|0|50|11|0|close@37|
23|2|1|0|0|30|12|/usr/local/mimecast/.ioriot/test0/someapp/somesubdir/vd11-8:1|438|0|open@38|
23|2|1|0|0|0|12|0|fstat@39|
23|2|1|0|0|10|12|12|read@40|
23|2|1|0|0|0|12|0|fstat@41|
23|2|1|0|0|72|12|0|1|12|lseek@42|
23|2|1|0|0|10|12|0|read@43|
23|2|1|0|0|50|12|0|close@44|
23|3|1|0|0|30|13|/usr/local/mimecast/.ioriot/test0/someapp/somesubdir/vd11-9:0|438|0|open@45|
23|3|1|0|0|0|13|0|fstat@46|
23|3|1|0|0|10|13|12|read@47|
23|3|1|0|0|0|13|0|fstat@48|
23|3|1|0|0|72|13|0|1|12|lseek@49|
23|3|1|0|0|10|13|0|read@50|
23|3|1|0|0|50|13|0|close@51|
23|4|1|0|0|30|14|/usr/local/mimecast/.ioriot/test0/someapp/somesubdir/vd11-7:1|438|0|open@52|
23|4|1|0|0|0|14|0|fstat@53|
23|4|1|0|0|10|14|12|read@54|
23|4|1|0|0|0|14|0|fstat@55|
23|4|1|0|0|72|14|0|1|12|lseek@56|
23|4|1|0|0|10|14|0|read@57|
23|4|1|0|0|50|14|0|close@58|
23|5|1|0|0|30|15|/usr/local/mimecast/.ioriot/test0/someapp/somesubdir/vd11-8:0|438|0|open@59|
23|5|1|0|0|0|15|0|fstat@60|
23|5|1|0|0|10|15|12|read@61|
23|5|1|0|0|0|15|0|fstat@62|
23|5|1|0|0|72|15|0|1|12|lseek@63|
23|5|1|0|0|10|15|0|read@64|
23|5|1|0|0|50|15|0|close@65|
23|6|1|0|0|30|16|/usr/local/mimecast/.ioriot/test0/someapp/somesubdir/vd11-6:1|438|0|open@66|
23|6|1|0|0|0|16|0|fstat@67|
23|6|1|0|0|10|16|12|read@68|
23|6|1|0|0|0|16|0|fstat@69|
.
.
.
#INIT
0|1|688|/mnt/15/.ioriot/test0/bmnt/2/20171101/b/8/b_dv01_11_vd11-11_a|@55290437
0|1|2592|/mnt/15/.ioriot/test0/bmnt/2/20171101/b/3/b_dv01_11_vd11-11_b|@33907067
0|1|768|/mnt/14/.ioriot/test0/bmnt/2/20171101/b/d/b_dv01_11_vd11-11_c|@64247527
0|1|1440|/mnt/15/.ioriot/test0/bmnt/2/20171101/b/0/b_dv01_11_vd11-11_d|@2014896
0|1|960|/mnt/15/.ioriot/test0/bmnt/2/20171101/b/9/b_dv01_11_vd11-11_e|@17724079
0|1|928|/mnt/15/.ioriot/test0/bmnt/2/20171101/b/1/b_dv01_11_vd11-11_f|@4534389
0|1|1712|/mnt/14/.ioriot/test0/bmnt/2/20171101/b/5/b_dv01_11_vd11-11_g|@2738458
0|1|784|/mnt/14/.ioriot/test0/bmnt/2/20171101/b/b/b_dv01_11_vd11-11_h|@21136612
0|1|624|/mnt/14/.ioriot/test0/bmnt/2/20171101/b/6/b_dv01_11_vd11-11_i|@24683427
0|1|672|/mnt/14/.ioriot/test0/bmnt/2/20171101/b/9/b_dv01_11_vd11-11_j|@12584061
0|1|336|/mnt/15/.ioriot/test0/bmnt/2/20171101/b/5/b_dv01_11_vd11-11_k|@7737434
0|1|12|/mnt/06/.ioriot/test0/bmnt/tmp/b|@42498106
.
.
.

```

### 2.2 Initialize the replay test

It is very likely that the replay test wants to access already existing files. Therefore it has to be ensured that all of these exist already before starting the test. To create all files and directories required by the test run the following command:

```sh
sudo ioriot -i io.replay
```

For that ``ioriot`` makes use of the INIT section in ``io.replay``.

## 3. Replay

To replay the log run:

```sh
sudo ioriot -r io.replay
```

It is beneficial to read ``io.replay`` from RAMdisk so that we are not interfering so much with the system I/O. 

*Init and replay in one go*

It is posisble to initialise the test and run the test with one single command, just replace option `-r` with `-R`:

```sh
sudo ioriot -R io.replay
```

*Speed factor*

By default `ioriot` tries to replay all I/O operations as fast as it can. To replay the I/O at a different speed it is possible to configure the speed factor by using the `-s` command line option.

The following pseudo code demonstrates how the speed factor affects the replay speed. Here `current_time` represents the current time while replaying the I/O, `time_in_log` represents the time as logged in `io.replay` and `time_ahead` indicates whether the replay is too quick or not. 

```code
if (speed_factor != 0) {
  time_ahead = time_in_log / speed_factor - current_time
  if (time_ahead > 0) {
    sleep(time_ahead)
  }
}
```

A speed factor of `0` is interpreted as "replay as fast as possible". A speed factor of `1` can be used to replay everything in original speed (same speed as on the original host where the I/O was captured). A speed factor of `2` would double the speed and a speed factor of `0.5` would half the speed.
In order to replay the I/O in original speed the factor of `1` can be used as follows:

```sh
sudo ioriot -R io.replay -s 1
```

## 4. Analyse

Look at various operating system statistics during the test. Useful commands are for example ``iostat -x 1``, ``dstat --disk`` and ``sudo iotop -o``. Best would be to collect all I/O statistics of all drives to a time series database with graphing capabilities such as Collectd/Graphite/Whisper.

## 5. Repeat

It is important to understand the I/O statistics observed. It is possible to repeat the same test any time again. Each time with different settings applied. 

## Cleanup

To purge all temporally data files of all tests run

```sh
sudo ioriot -P
```

Note: It's not required to cleanup any test data manually when you intend to re-run a test or run a new test. During initialization (``-i`` or ``-R`` switch) ``ioriot`` will automatically move all old data to ``.ioriot/.trash/`` sub-folders. The data will be ignored there. However, once you intend to completely delete all test files and directories (e.g. you run out of disk space or want to deinstall ``ioriot`` you should purge them with ``-P`` as shown above.
