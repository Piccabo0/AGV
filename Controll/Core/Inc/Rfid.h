#ifndef _RFID_H_
#define _RFID_H_
#include "stm32f1xx.h"


extern uint8_t RfidMachineState;
extern uint8_t RFID_COMEIN_Flag;
extern uint8_t RFID_DATA_Buf[256];
extern uint8_t RFID_BLOCK_Data[16];
extern uint16_t RFID_ReadBlockTimeout;
extern uint32_t RFID_ReadBlockTimes;
extern uint32_t RFID_ReadBlockSuccessTimes;
extern uint16_t RFID_ONLINE_Timeout;
extern uint16_t RFID_ONLINE_Flag;
extern uint16_t PlaceId;
extern uint8_t RFID_CARD_ID[4];

uint8_t RFID_checkSum(uint8_t* data, uint8_t length);
void Analysis_Receive_From_RFID(uint8_t data);
void HandlerRfidCmd(uint8_t* data ,uint8_t length);
void ReadRfidBlock(void);
void READ_RFID_BLOCK_Task(void);

#define RFID_READ_INFOR_SUCCESS_MASK  0x04
#define RFID_UPLOAD_ONLY_ID       0
#define RFID_UPLOAD_ID_AND_INFOR  1




#endif
