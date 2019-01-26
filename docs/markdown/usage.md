[Main page](../../README.md)

# I/O Riot usage

I/O Riot is operated in 5 steps:

1. Capture: Record all I/O operations over a given period of time to a capture log.
2. Initialize: Copy the log to a load test machine and initialize the load test environment.
3. Replay: Drop all OS caches and replay all I/O operations.
4. Analyze: Look at the OS and hardware stats (throughput, I/O ops, load average) from the run phase and draw conclusions. The aim is to identify possible I/O bottlenecks.
5. Repeat: Repeat steps 2-4 multiple times but adjust OS and hardware settings in order to improve I/O performance.

## 1. Capture

Run the following command to capture all I/O operation of the entire (Linux) system to the file ``io.capture``. For efficiency and security it is only capturing the meta data (amount of bytes written and read) and not the actual data. It is also capturing the system time in microseconds and the process IDs (PIDs) and thread IDs (TIDs) used as well as all relevant options and flags of the corresponding I/O syscalls.

```sh
sudo ioriot -c ~/io.capture
```

To stop capturing I/O type Ctrl+C. Alternatively one hour for the Kernel module to auto exit.

To capture only I/O caused by Java process run:

```sh
sudo ioriot -c ~/io.capture -m javaioriot.ko
```

To capture the I/O of a specific process run the following respectively:

```sh
sudo ioriot -c ~/io.capture -m targetedioriot.ko -p PID
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

### 2.1 Generate a replay log

In order to replay the I/O operation we must generate a replay log first. To generate the the replay log ``io.replay`` from the capture log ``io.capture`` run:

```sh
ioriot -c io.capture -r io.replay -n NAME -u USER
```

In which NAME is a freely chosen name and USER must be a valid system user. It is the system user under which the replay test will run. This command also creates all required top level directories such as ``/.ioriot/NAME/``, ``/mnt/.ioriot/NAME/``... etc in all mounted file systems. These are the directories where the replay test will read/write files from/to. These directories will belong to user USER. Notice that I/O Riot only operates in these  dedicated sub-directories. This is to prevent to overwrite actual system files.

``ioriot`` will filter out many I/O operations, especially all operations on pseudo file systems (e.g. sysfs, procfs), as it does not make a lot of sense to replay I/O on these file systems. Also, I/O operations on unknown file handles will be filtered out as well. This can happen when we start capturing the I/O *after* an application already opened a file. As a result we won't see how the application opened that file. The best practise is to stop all applications on the machine first, start capturing the I/O, and start all applications again. This may be improved in future releases of I/O Riot.

### 2.2 Initialize the replay test

It is very likely that the replay test wants to access already existing files. Therefore it has to be ensured that all of these exist already before starting the test. To create all files and directories required by the test run the following command:

```sh
sudo ioriot -i io.replay
```

## 3. Replay

To replay the log run:

```sh
sudo ioriot -r io.replay
```

Optional: It is beneficial to read ``io.replay`` from RAMdisk so that we are not interfering so much with the system I/O. 

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

Examples of OS and hardware settings and adjustments include:

* Change of system parameters (file system mount options, file system caching, file system type, file system creation flags).
* Replay the I/O at different speed(s).
* Replay the I/O with modified pattern(s) (e.g. remove reads from the replay journal).
* Replay the I/O on different types of hardware.

## Cleanup

To purge all temporally data files of all tests run

```sh
sudo ioriot -P
```

Note: It's not required to cleanup any test data manually when you intend to re-run a test or run a new test. During initialization (``-i`` or ``-R`` switch) ``ioriot`` will automatically move all old data to ``.ioriot/.trash/`` sub-directories. The data will be ignored there. However, once you intend to completely purge all test files and directories (e.g. you run out of disk space or want to deinstall ``ioriot`` you should purge them with ``-P`` as shown above.
