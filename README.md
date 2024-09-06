# Usage

You can generate the image for WISE-3200 by the following steps.


## Get BSP

```
$ cd /home/{user}/share
$ git clone https://github.com/ADVANTECH-Corp/WISE-3200.git
```

## Docker Environment & Setting

Install Docker Engine on your host

Please refer to [**Docker Installation Guide**](https://docs.docker.com/engine/installation/) for details

Pull docker image from Docker Hub
```
$ docker pull advrisc/u14.04-4531obv1
```
Create Docker container  or  Create Docker container with user share folder
```
$ docker run --name wise3200 -it advrisc/u14.04-4531obv1 /bin/bash
```

Docker command with share folder in user /home/{user}/share
```
$ docker run --name wise3200 -v  /home/{user}/share:/home/share -it advrisc/u14.04-4531obv1 /bin/bash
$ cd /home/share/WISE-3200
```

## Compile

```
$ ./build.sh
```

## Images

You can get the images in `WISE-3200/images/` folder.

These images are for WISE-3200.
* Kernel:
 - *`vmlinux.lzma.uImage`*
* OpenWRT rootfs:
 - *`cus531-nand-jffs2`*
* System upgrade image:
 - *`wise-3200-sysupgrade.bin`*

# Firmware Update

There are two ways to flash the image: one is through the LuCI web interface, and the other is via TFTP.

## Method 1

Use wise-3240-sysupgrade.bin file and upgrade firmware reference link as follow

[**LuCI web flash image**](http://ess-wiki.advantech.com.tw/view/IoTGateway/LuCI#Firmware_Update)

## Method 2
**[Kernel & Rootfs]** It can be flashed via TFTP.

1. Put the images into your TFTP
 - Kernel : *`vmlinux.lzma.uImage`*
 - Rootfs : *`cus531-nand-jffs2`*
2. Power on WISE-3200 and stop autoboot
 ```
 sc = 0x87ff7170 page = 0x800 block = 0x20000
 Setting 0x181162c0 to 0x1429a100
 Hit any key to stop autoboot:  0
 ath>
 ```

3. Set IP address for TFTP & device. For example, my TFTP server is 192.168.0.1, and I set the device as 192.168.0.2.
 ```
 ath> setenv ipaddr 192.168.0.2
 ath> setenv serverip 192.168.0.1
 ath> saveenv
 Saving Environment to Flash...
 Protect off 9F040000 ... 9F04FFFF
 Un-Protecting sectors 4..4 in bank 1
 Un-Protected 1 sectors
 Erasing Flash...Erasing flash...
 First 0x4 last 0x4 sector size 0x10000                                                            4
 Erased 1 sectors
 Writing to Flash... write addr: 9f040000
 done
 Protecting sectors 4..4 in bank 1
 Protected 1 sectors
 ```

4. Flash the device & boot
 ```
 ath> run lk lf && boot
 ```

