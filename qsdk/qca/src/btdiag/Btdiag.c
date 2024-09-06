/*
Copyright (c) 2013 Qualcomm Atheros, Inc.
All Rights Reserved.
Qualcomm Atheros Confidential and Proprietary.
*/

/*
 *  This app makes a DIAG bridge for standalone BT SoC devices to communicate with QMSL/QRCT
 *  QPST is a requirement to make connection with QMSL/QRCT
 *  Requires at least four command line arguments:
 *  1. UDT or not, forced to be 'yes'
 *  2. Server port number
 *  3. Connection type, SERIAL or USB
 *  4. Device name
 *  5. Baudrate, optional and used only in SERIAL mode, default is 115200
 */
#include <stdio.h>      //printf
#include <errno.h>
#include <stdlib.h>      //printf
#include <string.h>     //strlen
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <libevent/event.h>

#include <stdbool.h>
#include <limits.h>     //PATH_MAX
#include <stdlib.h>     //atoi
#include <pthread.h>    //thread
#include <sys/time.h>
#include <time.h>

#include "global.h"
#include "hci_api.h"

//#include <bluetooth/bluetooth.h>
//#include <bluetooth/hci.h>
//#include <bluetooth/hci_lib.h>

/*
#define IPADDRESS         "IPADDRESS"
#define PORT              "PORT"
#define IOTYPE            "IOType"
#define CONNECTIONDETAILS "ConnectionDetails"
#define BAUDRATE          "BAUDRATE"
#define HANDSHAKE         "HANDSHAKE"     //NONE: 0; XOnXoff: 1; RequestToSend: 2; RequestToSendXOnXOff: 3
#define PATCH             "PATCH"
#define NVM               "NVM"
*/

// DK_DEBUG
#define UCHAR unsigned char
#define UINT8 unsigned char
#define UINT16 unsigned short int

#define MAX_EVENT_SIZE	260
#define MAX_TAGS              50
#define PS_HDR_LEN            4
#define HCI_VENDOR_CMD_OGF    0x3F
#define HCI_PS_CMD_OCF        0x0B
#define PS_EVENT_LEN 100
#define HCI_EV_SUCCESS        0x00
#define HCI_COMMAND_HEADER_SIZE 3
#define HCI_OPCODE_PACK(ogf, ocf) (UINT16)((ocf & 0x03ff)|(ogf << 10))
//#define htobs(d) (d)

//CSR8811
//============================================================
// PSKEY
//============================================================
//UCHAR PSKEY_BDADDR[]					={0xc2, 0x02, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x03, 0x70, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x78, 0x00, 0x12, 0x90, 0x56, 0x00, 0x34, 0x12};
UCHAR PSKEY_BDADDR[]					={0xc2, 0x02, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x03, 0x70, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xee, 0xff, 0x88, 0x00, 0x00, 0x00};
UCHAR PSKEY_ANA_FREQ[]					={0xc2, 0x02, 0x00, 0x09, 0x00, 0x02, 0x00, 0x03, 0x70, 0x00, 0x00, 0xfe, 0x01, 0x01, 0x00, 0x00, 0x00, 0x90, 0x65};
UCHAR PSKEY_ANA_FTRIM[]					={0xc2, 0x02, 0x00, 0x09, 0x00, 0x03, 0x00, 0x03, 0x70, 0x00, 0x00, 0xf6, 0x01, 0x01, 0x00, 0x00, 0x00, 0x1d, 0x00};
UCHAR PSKEY_ANA_FTRIM_READWRITE[]		={0xc2, 0x02, 0x00, 0x09, 0x00, 0x04, 0x00, 0x03, 0x70, 0x00, 0x00, 0x3f, 0x68, 0x01, 0x00, 0x00, 0x00, 0x1d, 0x00};
UCHAR PSKEY_BAUD_RATE[]					={0xc2, 0x02, 0x00, 0x0a, 0x00, 0x05, 0x00, 0x03, 0x70, 0x00, 0x00, 0xea, 0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xc2};
UCHAR PSKEY_INTERFACE_H4[]				={0xc2, 0x02, 0x00, 0x09, 0x00, 0x06, 0x00, 0x03, 0x70, 0x00, 0x00, 0xf9, 0x01, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00};	
UCHAR PSKEY_COEX_SCHEME[]				={0xc2, 0x02, 0x00, 0x0a, 0x00, 0x07, 0x00, 0x03, 0x70, 0x00, 0x00, 0x80, 0x24, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00};
UCHAR PSKEY_COEX_PIO_UNITY_3_BT_ACTIVE[]={0xc2, 0x02, 0x00, 0x0a, 0x00, 0x08, 0x00, 0x03, 0x70, 0x00, 0x00, 0x83, 0x24, 0x02, 0x00, 0x00, 0x00, 0x05, 0x00, 0x01, 0x00};
UCHAR PSKEY_COEX_PIO_UNITY_3_BT_STATUS[]={0xc2, 0x02, 0x00, 0x0a, 0x00, 0x09, 0x00, 0x03, 0x70, 0x00, 0x00, 0x84, 0x24, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00};
UCHAR PSKEY_COEX_PIO_UNITY_3_WLAN_DENY[]={0xc2, 0x02, 0x00, 0x0a, 0x00, 0x0a, 0x00, 0x03, 0x70, 0x00, 0x00, 0x85, 0x24, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
//============================================================
// BCCMD
//============================================================
UCHAR BCCMD_WARM_RESET[]		={0xc2, 0x02, 0x00, 0x09, 0x00, 0x20, 0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	
UCHAR BCCMD_WARM_HALT[]			={0xc2, 0x02, 0x00, 0x09, 0x00, 0x21, 0x00, 0x04, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	
UCHAR BCCMD_ENABLE_TX[]			={0xc2, 0x02, 0x00, 0x09, 0x00, 0x22, 0x00, 0x07, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	
UCHAR BCCMD_DISABLE_TX[]		={0xc2, 0x02, 0x00, 0x09, 0x00, 0x23, 0x00, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	
UCHAR BCCMD_COEX_ENABLE[]		={0xc2, 0x02, 0x00, 0x09, 0x00, 0x24, 0x00, 0x78, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	
UCHAR BCCMD_PAUSE[]				={0xc2, 0x02, 0x00, 0x09, 0x00, 0x25, 0x00, 0x04, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	
UCHAR BCCMD_CONFIG_PACKAGE_DH1[]={0xc2, 0x02, 0x00, 0x09, 0x00, 0x26, 0x00, 0x04, 0x50, 0x00, 0x00, 0x17, 0x00, 0x0f, 0x00, 0x53, 0x01, 0x00, 0x00};	
//BCCMD_CONFIG_XTAL_FTRIM(default = 27, 0 ~ 0x3f)
UCHAR BCCMD_CONFIG_XTAL_FTRIM[]	={0xc2, 0x02, 0x00, 0x09, 0x00, 0x27, 0x00, 0x04, 0x50, 0x00, 0x00, 0x1d, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00};
UCHAR BCCMD_TXDATA1_2402[]		={0xc2, 0x02, 0x00, 0x09, 0x00, 0x28, 0x00, 0x04, 0x50, 0x00, 0x00, 0x04, 0x00, 0x62, 0x09, 0x32, 0xff, 0x00, 0x00};	
//0x5004 = VarID = Radiotest (BCCMD)
//0x0013 = TestID=19 = BER1 (BCCMD)
//0x0962 = 2402
//0x0001 = highside
//0x0000 = attn
UCHAR BCCMD_BER1_2402[]			={0xc2, 0x02, 0x00, 0x09, 0x00, 0x29, 0x00, 0x04, 0x50, 0x00, 0x00, 0x13, 0x00, 0x62, 0x09, 0x01, 0x00, 0x00, 0x00};	
UCHAR BCCMD_RXDATA1_2402[]		={0xc2, 0x02, 0x00, 0x09, 0x00, 0x2a, 0x00, 0x04, 0x50, 0x00, 0x00, 0x08, 0x00, 0x62, 0x09, 0x01, 0x00, 0x00, 0x00};	

#define PORT_NUM 2390

struct event_base* base = NULL;

int sock = -1;
int client_sock = -1;
bool legacyMode = 0;
ConnectionType connectionTypeOptions = INVALID;

int btDevFd = -1;
char devName[PATH_MAX] = {0};
int srvPortNum = -1;
int uartBaudRate = -1;

BYTE warmResetRespMsg_raw[] = {
                     0x04, 0x0F,
                     0x04,
                     0x00, 0x01, 0x00, 0x00
                    };
int delay = 800;

void help(void);
int init_bt(char *devName);

void print_time(void)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    printf("\ntime %u.%u\n",(BYTE)tv.tv_sec,tv.tv_usec/1000);
    return;
}

void dump_data(char *msg, uint8_t *data, int len)
{
    int i;

    if (NULL != msg){
        printf("%s\n", msg);
    }
    printf("\tData Len=%d\n\t", len);
    for (i = 0; i < len; i++){
        if (i % 16 == 0 && i != 0){
            printf("\n\t");
        }
        printf("%02x ", data[i]);
    }
    printf("\n");

    return ;
}

int add_event(int fd, void (*fn)(int iCliFd, short sEvent, void *arg))
{
    struct event *pEvent = NULL;

    pEvent = (struct event*)malloc(sizeof(struct event));
    if (NULL == pEvent){
        printf("Failed to malloc event\n", __FUNCTION__);
        return -1;
    }

    event_set(pEvent, fd, EV_READ|EV_PERSIST, fn, pEvent);
    event_base_set(base, pEvent);
    event_add(pEvent, NULL);

    return 0;
}

int socket_create(int portNum)
{
    int sockFd;
    struct sockaddr_in server;
    int opt;

    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0){
        printf("Create socket failed.\n");
        return -1;
    }
    printf("Create socket OK\n");

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(portNum);

    opt = 1;
    setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(sockFd,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("Bind socket failed.\n");
        close(sockFd);
        return -1;
    }
    printf("Bind socket to port %d OK\n", portNum);

    //Listen
    printf("Listening on socket(%d)...\n",sockFd);
    listen(sockFd, 3);

    return sockFd;
}

void on_read(int iCliFd, short sEvent, void *arg)
{
    int len;
    static BYTE buf[TCP_BUF_LENGTH];

    len = recv(iCliFd, buf, TCP_BUF_LENGTH, 0);
    if (len <= 0){
        struct event *pEvRead = (struct event*)arg;
        printf("Recv failed from client(%d).\n", iCliFd);
        event_del(pEvRead);
        free(pEvRead);
        close(iCliFd);
        client_sock = -1;
        return;
    }

    printf("\n");
    print_time();
    printf(" -> Recv data from client(%d):\n", iCliFd);
    dump_data(NULL, buf, len);

    processDiagPacket(buf);
    return;
}

void on_accept(int iCliFd, short sEvent, void *arg)
{
    struct sockaddr_in cliAddr;
    int sinSize;

    printf("Enter %s\n", __FUNCTION__);

    sinSize = sizeof(struct sockaddr_in);
    client_sock = accept(iCliFd, (struct sockaddr*)&cliAddr, &sinSize);
    printf("Accept client(%d)\n", client_sock);
    add_event(client_sock, on_read);
    return;
}

int init_server(int portNum)
{
    int sockFd;

    sockFd = socket_create(portNum);
    return add_event(sockFd, on_accept);
}

/*Create fd for USB bt device and set configuration for it.
*/
int open_bluetooth_device(char *devName)
{
    if (NULL == devName){
        printf("Invalid device name.\n");
        return -1;
    }

    if (CONN_TYPE_IS_SERIAL()){
        btDevFd = SerialConnection(devName, uartBaudRate);
        if (btDevFd < 0){
            printf("Failed to open serial device: %s\n", devName);
            return -1;
        }
    }
    else if (CONN_TYPE_IS_USB()){
        wait_for_bt_ready(devName);

        btDevFd = hci_dev_open(devName);
        if (btDevFd < 0){
            printf("Failed to open USB device: %s\n", devName);
            return -1;
        }
        printf("%s(%d)is open\n", devName, btDevFd);
    }
    else{
        printf("Invalid connection type\n");
        return -1;
    }

    return 0;
}

int modify_and_send_bt_data(BYTE *buf, int len)
{
    BYTE Hdr1366[] = {0x10, 0x00, 0x12, 0x00, 0x12,
                        0x00, 0x66, 0x13, 0xE5, 0x62,
                        0x2A, 0xC5, 0xFF, 0x01, 0x00};
    #define HDR1366_SIZE 15

    int txLen;
    int fakeTxLen;

    if (len + HDR1366_SIZE > HCI_MAX_EVENT_SIZE){
        printf("Length is out of range\n");
        return -1;
    }

    memmove(buf + HDR1366_SIZE, buf, len);
    memcpy(buf, Hdr1366, HDR1366_SIZE);
    buf[HDR1366_SIZE] = HCI_EVENT_PKT;

    /*Modify tx len*/
    txLen = HDR1366_SIZE + len;

    /*Modify length field*/
    fakeTxLen = txLen - 4;
    buf[2] = (BYTE)fakeTxLen;
    buf[4] = buf[2];

    /*Send*/
    DiagPrintToStream(buf, txLen, true);

    return 0;
}


int wait_for_bt_ready(char *devName)
{
    int devId;
    int count = 0;


    while(true){
        //printf("Sleep %d msec\n", count);
        usleep(delay * 1000);
        count += delay;
        hci_dev_up(devName);
        if (hci_dev_is_up(devName)){
            printf("%s is up!\n", devName);
            break;
        }
        else{
            printf("%s is down!\n", devName);
        }
    }
    printf("After %d msec, %s is ready\n", count, devName);

    return 0;
}

void on_read_bt(int iFd, short sEvent, void *arg)
{
    int len;
    static BYTE buf[HCI_MAX_EVENT_SIZE];

    if (CONN_TYPE_IS_SERIAL()){
        len = read_hci_event(iFd, buf, HCI_MAX_EVENT_SIZE);
    }
    else if (CONN_TYPE_IS_USB()){
        len = read(iFd, buf, HCI_MAX_EVENT_SIZE);
        if (len <= 0){
            struct event *pEvRead = (struct event*)arg;
            print_time();
            printf("BT device(%d) recv failed.\n", btDevFd);
            event_del(pEvRead);
            free(pEvRead);
            close(iFd);

            btDevFd = -1;

            //wait_for_bt_ready(devName);
            init_bt(devName);

            /*Make a fake packet to QDART*/
            len = sizeof(warmResetRespMsg_raw);
            memcpy(buf, warmResetRespMsg_raw, len);
        }
    }
    else{
        printf("Invalid connection type\n");
        return;
    }

    print_time();
    dump_data("Recv data from BT device:", buf, len);

    /*Modify pkt and send it to QDART*/
    modify_and_send_bt_data(buf, len);

    return;
}

int csr8811_init(int devFd)
{
    //Set frequency 26MHz
    hci_dev_send_cmd(devFd, HCI_VENDOR_CMD_OGF, 0x00, 0x13, PSKEY_ANA_FREQ);

    if (CONN_TYPE_IS_SERIAL()){
        //Baud rate, 115200
        hci_dev_send_cmd(devFd, HCI_VENDOR_CMD_OGF, 0x00, 0x15, PSKEY_BAUD_RATE);
        //Interface, H4
        hci_dev_send_cmd(devFd, HCI_VENDOR_CMD_OGF, 0x00, 0x13, PSKEY_INTERFACE_H4);
    }
    //Warm reset
    hci_dev_send_cmd(devFd, HCI_VENDOR_CMD_OGF, 0x00, 0x13, BCCMD_WARM_RESET);
    return 0;
}

int init_bt(char *devName)
{
    static bool csr8811_is_inited = false;

    open_bluetooth_device(devName);
    if (!csr8811_is_inited){
        csr8811_init(btDevFd);
        csr8811_is_inited = true;
    }
    return add_event(btDevFd, on_read_bt);
}

int parse_param(int argc , char *argv[])
{
    int ret;
    char iotType[20] = {0};

    if (argc < 5){
        help();
        return -1;
    }

    /*Check UDT=yes*/
    if (0 != strcmp(argv[1], "UDT=yes")){
        printf("UDT value error\n");
        return -1;
    }

    /*Parse PORT param*/
    ret = sscanf(argv[2], "PORT=%d", &srvPortNum);
    if (1 != ret){
        printf("Failed to parse PORT.\n");
        return -1;
    }

    /*Parse IOType param*/
    ret = sscanf(argv[3], "IOType=%s", iotType);
    if (1 != ret){
        printf("Failed to parse IOType.\n");
        return -1;
    }

    if (0 == strcmp(iotType, "SERIAL")){
        connectionTypeOptions = SERIAL;
    }
    else if (0 == strcmp(iotType, "USB")){
        connectionTypeOptions = USB;
    }
    else{
        printf("IOType is invalid.\n");
        return -1;
    }

    /*Parse DEVICE*/
    ret = sscanf(argv[4], "DEVICE=%s", devName);
    if (1 != ret){
        printf("Failed to parse DEVICE.\n");
        return -1;
    }

    /*Parse BAUDRATE*/
    if (SERIAL == connectionTypeOptions){
        if (argc != 6){
            help();
            return -1;
        }

        /*Parse BAUDRATE*/
        ret = sscanf(argv[5], "BAUDRATE=%d", &uartBaudRate);
        if (1 != ret){
            printf("Failed to parse DEVICE.\n");
            return -1;
        }
    }


    printf("%-10s: %d\n", "PORT", srvPortNum);
    printf("%-10s: %s\n", "IOType", (USB == connectionTypeOptions)? "USB" : "SERIAL");
    printf("%-10s: %s\n", "DEVICE", devName);
    if (CONN_TYPE_IS_SERIAL()){
        printf("%-10s: %d\n", "BAUDRATE", uartBaudRate);
    }

    if (6 == argc){
        ret = sscanf(argv[5], "delay=%d", &delay);
        printf("%-10s: %d\n", "Delay", delay);
     }

    printf("\n");

    return 0;
}


void help(void)
{
    printf("Require at least 5 arguments.\n\nExample of client mode, Btdiag usage:\n");
    printf("- BT FTM mode for serial:\n./Btdiag UDT=yes PORT=2390 IOType=SERIAL DEVICE=/dev/ttyUSB0 BAUDRATE=115200\n\n");
    printf("- BT FTM mode for USB:\n./Btdiag UDT=yes PORT=2390 IOType=USB DEVICE=hci0\n\n");
}

int main(int argc , char *argv[])
{
    int ret;

    ret = parse_param(argc, argv);
    if (0 != ret){
        printf("Parse param failed.\n");
        return -1;
    }

    base = event_base_new();

    ret = init_bt(devName);
    if (0 != ret){
        printf("Init bluetooth failed.\n");
        return -1;
    }

    ret = init_server(srvPortNum);
    if (0 != ret){
        printf("Init server failed.\n");
        return -1;
    }

    event_base_dispatch(base);

    return 0;
}
