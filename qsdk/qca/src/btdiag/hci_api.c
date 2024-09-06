/*
Copyright (c) 2016 Qualcomm Atheros, Inc.
All Rights Reserved.
Qualcomm Atheros Confidential and Proprietary.
*/
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <endian.h>
#include <byteswap.h>

#include <sys/param.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "global.h"
#include "hci_api.h"

#define BTPROTO_HCI	1

#define SOL_HCI		0
#define HCI_FILTER	2

#define HCIDEVUP	_IOW('H', 201, int)
#define HCIDEVDOWN	_IOW('H', 202, int)
#define HCIGETDEVINFO	_IOR('H', 211, int)


#define cmd_opcode_pack(ogf, ocf) (unsigned short)((ocf & 0x03ff)|(ogf << 10))

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htobs(d)  (d)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define htobs(d)  bswap_16(d)
#else
#error "Unknown byte order"
#endif

/* HCI device flags */
enum {
	HCI_UP,
	HCI_INIT,
	HCI_RUNNING,

	HCI_PSCAN,
	HCI_ISCAN,
	HCI_AUTH,
	HCI_ENCRYPT,
	HCI_INQUIRY,

	HCI_RAW,

	HCI_RESET,
};

typedef struct {
	UINT8 b[6];
} __attribute__((packed)) bdaddr_t;

typedef struct {
    unsigned short    opcode;
    unsigned char     plen;
} __attribute__ ((packed))  hci_command_hdr;

/* Ioctl requests structures */
struct hci_dev_stats {
    UINT32 err_rx;
    UINT32 err_tx;
    UINT32 cmd_tx;
    UINT32 evt_rx;
    UINT32 acl_tx;
    UINT32 acl_rx;
    UINT32 sco_tx;
    UINT32 sco_rx;
    UINT32 byte_rx;
    UINT32 byte_tx;
};


struct hci_dev_info {
    UINT16 dev_id;
    char  name[8];

    bdaddr_t bdaddr;

    UINT32 flags;
    UINT8  type;

    UINT8  features[8];

    UINT32 pkt_type;
    UINT32 link_policy;
    UINT32 link_mode;

    UINT16 acl_mtu;
    UINT16 acl_pkts;
    UINT16 sco_mtu;
    UINT16 sco_pkts;

    struct hci_dev_stats stat;
};


struct sockaddr_hci {
	sa_family_t	hci_family;
	unsigned short	hci_dev;
	unsigned short  hci_channel;
};

typedef struct  {
    unsigned long type_mask;
    unsigned long event_mask[2];
    UINT16 opcode;
}hci_filter;


static int get_id_from_devname(char *devName)
{
    int devId;


    if (NULL == devName
        || strlen(devName) < 4
        || 0 != strncmp(devName, "hci", 3)){
        return -1;
    }
    devId = atoi(devName + 3);
    return devId;
}

static int hci_test_bit(int nr, void *addr)
{
	return *((UINT32 *) addr + (nr >> 5)) & (1 << (nr & 31));
}


int hci_dev_open(char *devName)
{
    int devId;
    int devFd;

    struct sockaddr_hci hciSAddr;
    int ret;

    devId = get_id_from_devname(devName);
    if (devId < 0){
        printf("Invalid hci device name(%s)\n", devName);
        return FD_INVALID;
    }

    devFd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
    if (devFd < 0){
        printf("Failed to open hci dev(%s)\n", devName);
        return FD_INVALID;
    }

    memset(&hciSAddr, 0, sizeof(hciSAddr));
    hciSAddr.hci_family = AF_BLUETOOTH;
    hciSAddr.hci_dev = devId;
    ret = bind(devFd, (struct sockaddr*) &hciSAddr, sizeof(hciSAddr));
    if (ret < 0){
        printf("Failed to bind address for hci device\n");
        close(devFd);
        return FD_INVALID;
    }

    return devFd;
}

int hci_dev_up(char *devName)
{
    int devId;
    int devFd;
    int ret;

    devId = get_id_from_devname(devName);
    if (devId < 0){
        printf("Invalid hci device name(%s)\n", devName);
        return FD_INVALID;
    }

    devFd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
    if (devFd < 0){
        printf("Failed to open hci dev(%s)\n", devName);
        return -1;
    }

    ret = ioctl(devFd, HCIDEVUP, devId);
    if (ret < 0){
		if (errno == EALREADY){
            close(devFd);
			return 0;
		}
        else{
            printf("Failed to start hci device(%s)\n ", devName);
            close(devFd);
            return -1;
        }
    }
    close(devFd);
    return 0;
}

bool hci_dev_is_up(char *devName)
{
    int devId;
    int devFd;
    static struct hci_dev_info devInfo;
    int ret;


    devId = get_id_from_devname(devName);
    if (devId < 0){
        printf("Invalid hci device name(%s)\n", devName);
        return false;
    }

    devFd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
    if (devFd < 0){
        printf("Failed to open hci dev(%s)\n", devName);
        return false;
    }

    devInfo.dev_id = devId;
    ret = ioctl(devFd, HCIGETDEVINFO, (void *)&devInfo);
    if (ret < 0){
        close(devFd);
        return false;
    }

    close(devFd);
    return hci_test_bit(HCI_UP,&devInfo.flags);

}


int hci_dev_send_cmd(int devFd, UINT16 ogf, UINT16 ocf, UINT8 len, void *param)
{
	UINT8 cmdType = HCI_COMMAND_PKT;
	hci_command_hdr cmd_hdr;
    hci_filter filter;
    UINT16 opCode;

    struct iovec ioVec[3];
    int vecNum = 0;


    opCode = htobs(cmd_opcode_pack(ogf, ocf));

    /*Set filter*/
    memset(&filter, 0, sizeof(filter));
    filter.type_mask |= (1 << HCI_EVENT_PKT);
    memset(&filter.event_mask, 0xff, sizeof(filter.event_mask));
    filter.opcode = opCode;
    if (setsockopt(devFd, SOL_HCI, HCI_FILTER, &filter, sizeof(filter)) < 0) {
        printf("Set hci filter failed\n");
        return -1;
    }

    /*Send cmd*/
	cmd_hdr.opcode = opCode;
	cmd_hdr.plen= len;

	ioVec[0].iov_base = &cmdType;
	ioVec[0].iov_len  = sizeof(cmdType);
	ioVec[1].iov_base = &cmd_hdr;
	ioVec[1].iov_len  = sizeof(cmd_hdr);
	vecNum = 2;

	if (len) {
		ioVec[2].iov_base = param;
		ioVec[2].iov_len  = len;
		 vecNum = 3;
	}

    printf("Write iovec to fd(%d)\n", devFd);
	while (writev(devFd, ioVec, vecNum) < 0) {
		if (errno == EAGAIN || errno == EINTR)
			continue;
        else
		    return -1;
	}
	return 0;
}

