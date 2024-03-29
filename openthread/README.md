Package OTBR for OpenWRT
====================

The OpenWrt Project is a Linux operating system targeting embedded devices. Instead of trying to create a single, static firmware, OpenWrt provides a fully writable filesystem with package management.

OpenWRT use opkg to manage its packages, and this Makefile is to help package our project into .ipk and then can install on OpenWRT.

Build
-----

#### Prepare OpenWRT environment

```sh
git clone https://github.com/openwrt/openwrt.git
# enter the OpenWRT directory
cd openwrt
# update and install feeds
./scripts/feeds update -a
./scripts/feeds install -a
# select your preferred configuration for the toolchain, target system & firmware packages.
make menuconfig
# make the OpenWRT project
make
```
#### Package and Install otbr package

```sh
# move feeds.conf to OpenWRT directory and then run
./scripts/feeds update openthread
./scripts/feeds install -a -p openthread
# in "Network" column, select "openthread" option
make menuconfig
# package the project
make package/openthread/compile
# the .ipk file will be in ./bin/packages/mips_24kc/openthread/openthread_xxx.ipk
# copy the .ipk file into OpenWRT device, and install
opkg install openthread_xxx.ipk
# then can use openthread on OpenWRT
otbr-agent -d7 -v /dev/ttyUSB0 115200
```
