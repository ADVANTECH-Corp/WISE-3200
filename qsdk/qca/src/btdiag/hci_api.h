/*
Copyright (c) 2016 Qualcomm Atheros, Inc.
All Rights Reserved.
Qualcomm Atheros Confidential and Proprietary.
*/

#ifndef __HCI_API_H__
#define __HCI_API_H__

/* HCI Packet types */
#define HCI_COMMAND_PKT		0x01
#define HCI_ACLDATA_PKT		0x02
#define HCI_SCODATA_PKT		0x03
#define HCI_EVENT_PKT		0x04
#define HCI_VENDOR_PKT		0xff

int hci_dev_open(char *devName);
int hci_dev_up(char *devName);
bool hci_dev_is_up(char *devName);
int hci_dev_send_cmd(int devFd, UINT16 ogf, UINT16 ocf, UINT8 len, void *param);

#endif /*__HCI_API_H__*/

