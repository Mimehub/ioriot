[Main page](../../README.md)

# I/O Riot installation

I/O Riot consists of a set of SystemTap kernel modules (capturing I/O) and the tool ``ioriot`` (replaying I/O). Usually you want to capture I/O from a production machine and want to replay it on a separate load testing machine.
 
## System requirements

I/O Riot has been tested on

* CentOS 7.6 64Bit (latest version, all packages up to date, booted into the installed Kernel)
* SystemTap (from the default CentOS repository)
* GCC C-Compiler (from the default CentOS repository)

Before proceeding please ensure that the latest CentOS 7 kernel is installed and running on all machines involved. It should also be ensured that the capture machine and the load test machine have the same mount points mounted. This is to ensure that I/O is being replayed on the corresponding data drives on the load test machine.

## Compiling and installing ioriot 

I/O Riot has to be installed on all machines involved.  To install I/O Riot perform the following steps:

```sh
sudo yum install gcc systemtap yum-utils kernel-devel-$(uname -r)
sudo debuginfo-install kernel-$(uname -r)
make && sudo make install
export PATH=$PATH:/opt/ioriot/bin
```

This will install the ``ioriot`` utility to ``/opt/ioriot/bin/`` and the SystemTap kernel modules to ``/opt/ioriot/systemtap/``. Run ``ioriot -h`` to print out a brief help.

However, best practise is not to install any compilers on a production machine. You can either compile I/O Riot from scratch on all machines involved like shown above or only compile it on a build machine and distribute the ``/opt/ioriot`` directory to the remaining machines. In the latter case you will also need to install the ``systemtap-runtime`` package as an additional dependency.

In case you decided to deinstall I/O Riot you can do so by running

```sh
sudo ioriot -P # purges all test files created by ioriot
sudo make uninstall
```

# Compiling I/O Riot using Docker

In case there is no suitable build host for I/O Riot (e.g. you want to compile everything, including the SystemTap Kernel modules, on a Mac. Or you want to integrate the build process with Jenkins) it is possible to use Docker for that. First of all you have to install Docker Engine. Depending on the system in use the steps may differ. Once Docker is installed you can run for example the following commands to build everything for CentOS 7 and Linux Kernel 3.10.0-957.1.3.el7.x86_64:

```sh
export KERNEL=3.10.0-957.1.3.el7.x86_64
make dockerbuild
```

That will run for quite a while (depending on the download speed it can take more than 15 minutes!). Afterwards distribute directory ``./docker/opt/ioriot`` to ``/opt/ioriot`` to all boxes with that particular running kernel version.

By repeating the steps above it is possible to compile the SystemTap Kernel modules for multiple versions. All the modules will be found seamlessly in ``./docker/opt/ioriot/systemtap``.
