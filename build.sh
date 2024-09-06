#!/bin/sh
DIR=`pwd`/qsdk/dl
FIRMWAREDIR=`pwd`/bin/ar71xx
IMAGEDIR=../images

    # check dl file is empty
    if [ $(ls -1 $DIR |wc -l) -lt 10 ]
    then
        tar zxvf dl.tgz -C qsdk/
    else
        echo "$DIR is not empty!"
    fi

	cd `pwd`/qsdk
    make package/symlinks
	cp qca/configs/advantech/wise-3200.config .config
    make defconfig

	if [ -e $FIRMWAREDIR ];then
        rm bin/ar71xx/*
    else
        echo "bin/ar71xx does not exist"
    fi

    make V=s 2>error

	if [ -e $IMAGEDIR ];then
    	echo "file exist"
    	rm ../images/*
    else
        mkdir ../images
    fi
    cp bin/ar71xx/openwrt-ar71xx-generic-cus531nand-kernel.bin ../images/vmlinux.lzma.uImage
    cp bin/ar71xx/openwrt-ar71xx-generic-cus531nand-rootfs-squashfs.ubi ../images/cus531-nand-jffs2
	cp bin/ar71xx/openwrt-ar71xx-generic-cus531nand-squashfs-sysupgrade.bin ../images/wise3200-sysupgrade.bin

	cd ../images
    echo "************************************************************************"
    md5sum * | tee $MODEL.MD5SUM;

