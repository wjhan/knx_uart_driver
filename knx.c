#include "knx.h"
#include <stdio.h>
#include <stdlib.h>
struct KnxFrameType* knxFrame;

//struct KnxFrameType* knxRecFrame;  //new:  Frame receive from knx bus

void clean()
{
  int i;


  for(i=0;i<MAX_FRAME_LENGTH;i++) {
    knxFrame->knxFrameBuffer[i] = 0;

  }
  //Control feild
  knxFrame->knxFrameBuffer[0]= B10111100;

  // Target group address, Router counter =6, length =1 (= 2 data bytes)
  knxFrame->knxFrameBuffer[5]= B11100001;

}

void initKnxFrame(int fd) {

        knxFrame = (struct KnxFrameType*)malloc(sizeof(struct KnxFrameType));


  //knxRecFrame = (struct KnxFrameType*)malloc(sizeof(struct KnxFrameType)); //new: init  KnxRecFrame


     knxFrame->fd=fd;
}

void setSourceAddress(int area, int line, int mumber) {
  knxFrame->knxFrameBuffer[1] =(uint8_t)((area << 4) | line);
  knxFrame->knxFrameBuffer[2] =(uint8_t) mumber;
}

void setTargetGroupAddress(int main, int middle, int sub) {
    knxFrame->knxFrameBuffer[3] = (uint8_t)((main << 3) | middle);
    knxFrame->knxFrameBuffer[4] = (uint8_t)sub;
    knxFrame->knxFrameBuffer[5]|= B10000000;
}

void setFirsetDataByte(int data) {
  knxFrame->knxFrameBuffer[7] &= B11000000;
  knxFrame->knxFrameBuffer[7] |= data;
}

void setSecondDataByte(uint8_t data)
{
	knxFrame->knxFrameBuffer[7] &= B11000000;
	knxFrame->knxFrameBuffer[8] = data;
	
}

void setCommand(enum KnxCommandType command) {
  knxFrame->knxFrameBuffer[6] &= B11111100;
  knxFrame->knxFrameBuffer[7] &= B00111111;

  knxFrame->knxFrameBuffer[6] |= (uint8_t)(command >>2);
  knxFrame->knxFrameBuffer[7] |= (uint8_t)(command <<6);
}

void setPayloadLength(int payloadlength) {
  knxFrame->knxFrameBuffer[5] &= B11110000;
  knxFrame->knxFrameBuffer[5] |= (payloadlength -1);

  knxFrame->frameLength = payloadlength + HANDER_FARME_LENGTH;
}

void setChecksum() {
  int bcc = 0x00;
  int i;

  for(i=0;i<knxFrame->frameLength;i++) {
    bcc ^= knxFrame->knxFrameBuffer[i];
  }

  knxFrame->frameLength++;

  knxFrame->knxFrameBuffer[i] = ~bcc;

}

void createKNXMessageFrame(int payloadlength, enum KnxCommandType command, int mainGroup, int middleGroup, int subGroup, int firstDataByte) {
  clean();

  setSourceAddress(4,7,13);
  setTargetGroupAddress(mainGroup,middleGroup,subGroup);
  setFirsetDataByte(firstDataByte);
  setCommand(command);
  setPayloadLength(payloadlength);
  setChecksum();
}

int uartSendFrame(int fd) {
  uint8_t sendbuf[2];
  uint8_t i=0;
  uint8_t frameLength = knxFrame->frameLength;

  for(i=0;i<frameLength;i++) {
    if (i == (frameLength - 1)) sendbuf[0] = TPUART_DATA_END;
    else sendbuf[0] = TPUART_DATA_SRAT_CONTINUE;

    sendbuf[0] |=i;
    sendbuf[1] = knxFrame->knxFrameBuffer[i];

   // printf("%02x\n%02x\n",sendbuf[0],sendbuf[1]);
    UART_Send(fd,sendbuf,2);
  }

  //confirmation;  unimplement

  return 1;
}

int groupWriteBool(int fd,int mainGroup, int middleGroup, int subGroup, int value) {
  uint8_t valueBool = 0;
  int i=0;
  if(value) {
    valueBool =0x01;
  }

  initKnxFrame(fd);

  createKNXMessageFrame(2,KNX_COMMAND_WRITE,mainGroup,middleGroup,subGroup,valueBool);
  /**
  for(i=0;i<knxFrame->frameLength;i++) {
    printf("%x\n", knxFrame->knxFrameBuffer[i]);
    }**/


  return uartSendFrame(fd);
}

int groupWriteByte(int fd,int mainGroup, int middleGroup, int subGroup, int value)
{
	int i=0;
	uint8_t valueByte = value&0xFF;

	initKnxFrame(fd);

	clean();

  	setSourceAddress(4,7,13);
  	setTargetGroupAddress(mainGroup,middleGroup,subGroup);
  	setSecondDataByte(value);
  	setCommand(KNX_COMMAND_WRITE);
  	setPayloadLength(3);
  	setChecksum();
	
	
	for(i=0;i<knxFrame->frameLength;i++) {
    printf("%0x\n", knxFrame->knxFrameBuffer[i]);
    }

	//createKNXMessageFrame(3,KNX_COMMAND_WRITE,mainGroup,middleGroup,subGroup,valueByte);
	

}

int groupReadBoolReq(int fd, int mainGroup, int middleGroup, int subGroup, int value)
{
	int valueBool=0;
	int i;

	initKnxFrame(fd);

	createKNXMessageFrame(2,KNX_COMMAND_READ,mainGroup,middleGroup,subGroup,valueBool);
	/***
	for(i=0;i<knxFrame->frameLength;i++) {
		printf("%02x\n", knxFrame->knxFrameBuffer[i]);
	
    }**/

	return uartSendFrame(fd);
}

int UART_Send(int fd,char *buffer, int length)
{
    write(fd,buffer,length);

    return (TRUE);


}
