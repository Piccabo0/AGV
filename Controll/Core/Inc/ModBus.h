#ifndef __ModBus_H_
#define __ModBus_H_
#include "stm32f1xx.h"

#define DEFAULT_MODE_BUS_HALL_ADDR    1


#define return_OK                 0x00  // 一帧数据 OK 
#define illegal_function          0x01  // 功能码错误
#define illegal_register          0x02  // 寄存器地址错误
#define illegal_data              0x03  // 数据错误
#define crc_err                   0x04  // CRC 校验错误
#define switch_err                0x05  // 写数据时开关没开
#define switch_value_err          0x06  // 写开关值错误



#define CMD_ModBus_ReadEx         0x03
#define CMD_ModBus_Read           0x03
#define CMD_ModBus_Write          0x06
#define CMD_ModBus_WriteMore      0x16


#define LINE_SENSOR_NUM           16							//霍尔传感器灯的数量
#define WONDER_MID_SENSOR_INDEX   17     // (8.5*2)为了整数预算，乘以2
#define MAX_SENSOR_VALUE          32
#define MIN_SENSOR_VALUE          2
#define MAX_SEGMENT_NUM           8

typedef struct
{
  uint8_t head_index;
  uint8_t tail_index;
  uint16_t middle_index;
}SEGMENT_OPTION;

typedef struct
{
  uint8_t black_sensor_serial_flag;
  uint8_t Segment_Num;
  SEGMENT_OPTION seg_list[MAX_SEGMENT_NUM];
}SENSOR_STATUS_NEW;

typedef struct 
{
  uint8_t MachineState;
  uint8_t BufIndex;
  uint8_t read_receive_timer;
  uint8_t receive_CRC_H;
  uint8_t receive_CRC_L;
  uint8_t rev0;
  uint8_t ModBus_CMD;
  uint8_t err_state;
  uint32_t read_success_num;
  uint32_t write_success_num;
  uint16_t Read_Register_Num;
  uint8_t DataBuf[256];
}MODBUS_SAMPLE;


extern uint8_t HallStatusFresh;
extern uint8_t HallValue[LINE_SENSOR_NUM];
extern SENSOR_STATUS_NEW SENSOR_STATUS_New;
extern uint16_t HallSensor_Timeout;
extern struct  MODBUS  A8_Modbus;
extern MODBUS_SAMPLE MODBUS_HallSensor;
extern uint8_t ON_LINE_Flag;
extern uint8_t MODE_BUS_HALL_Addr;


extern void Analysis_Receive_From_HallSensor(uint8_t data, MODBUS_SAMPLE* pMODBUS);
extern uint16_t ModBus_CRC16_Calculate(unsigned char *aStr , unsigned char alen);
extern void MODBUS_READ_HALL_SERSOR_Task(void);
extern uint8_t CheckHallOnListNumNew(uint8_t* hall_list,uint8_t total_num,SENSOR_STATUS_NEW* St);






#endif
