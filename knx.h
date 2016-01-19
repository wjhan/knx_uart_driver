#ifndef KNX_H
#define KNX_H

typedef unsigned char uint8_t;

#include <stdio.h>
#include <stdlib.h>


#define B10111100 0xBC
#define B11100001 0xE1
#define B10000000 0x80
#define B11000000 0xC0
#define B11111100 0xFB
#define B00111111 0x3F
#define B11110000 0xF0

#define B0000 0x00
#define B0010 0x02

#define FALSE 0
#define TRUE 1

#define MAX_FRAME_LENGTH 128
#define HANDER_FARME_LENGTH 6


#define TPUART_DATA_END 0x40
#define TPUART_DATA_SRAT_CONTINUE 0x80

enum KnxCommandType {
  KNX_COMMAND_WRITE = B0010,
  KNX_COMMAND_READ  = B0000
};

struct KnxFrameType {
  uint8_t  knxFrameBuffer[MAX_FRAME_LENGTH];
  int frameLength;
  int fd;
};


void initKnxFrame(int fd);

//int groupWriteBool(int mainGroup, int middleGroup, int subGroup, int value);

void createKNXMessageFrame(int payloadlength, enum KnxCommandType command, int mainGroup, int middleGroup, int subGroup, int firstDataByte);

void clean();

void setSourceAddress(int area, int line, int member);

void setTargetGroupAddress(int main, int middle, int sub);

void setFirsetDataByte(int data);

void setCommand(enum KnxCommandType command);

void setPayloadLength(int payloadlength);

void setChecksum();

int UART_Send(int fd,char *buffer, int length);

int groupWriteBool(int fd,int mainGroup, int middleGroup, int subGroup, int value);

int groupReadBoolReq(int fd, int mainGroup, int middleGroup, int subGroup, int value);

int uartSendFrame(int fd);

int groupWriteByte(int fd,int mainGroup, int middleGroup, int subGroup, int value);


#endif



