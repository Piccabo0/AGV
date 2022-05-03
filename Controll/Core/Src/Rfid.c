#include "Rfid.h"
#include "string.h"
#include "Usart.h"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

#define RFID_COMM               3

#define RFID_CMD_HEADER         0x7F
#define RFID_CMD_MIN_LENGTH     2
#define RFID_CMD_MAX_LENGTH     0x7E

u8 RfidMachineState=0;
u8 DATA_7F_Num=0;
u8 RFID_COMEIN_Flag=0;
u8 RFID_DATA_Buf[256];
u8 RFID_DATA_BufIndex=0;
u32 RFID_ReadBlockTimes=0;
u32 RFID_ReadBlockSuccessTimes=0;
u32 RFID_ReadBlockUnSuccessTimes=0;
u8 RFID_ReadBlockFailFlag=0;
u8 RFID_ReadBlockRetryNum=0;
u16 RFID_ReadBlockTimeout=0;
//卡片信息
u16 RFID_Type;
u8 RFID_CARD_ID[4] = {0, 0, 0, 0};
u8 RFID_BLOCK_Data[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const u8 READ_BLOCK_ONE[11]={0x7F ,0x09 ,0x14 ,0x01 ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0x1C};//一键读块发送指令——见AGV地标传感器手册
u16 RFID_ONLINE_Timeout=0;
u16 RFID_ONLINE_Flag=0;
u16 PlaceId=0;
u16 PlaceIdTime = 0;

extern int CARD;

void Analysis_Receive_From_RFID(u8 data)
{
	//RfidMachineState按帧字节逐位加一顺序判断
	//数据帧的格式都是自己定义的，要与发送的数据帧相对应
  switch(RfidMachineState)
  {
  case 0:
    {
      if(data==RFID_CMD_HEADER)
      {
        RfidMachineState+=1;
        RFID_DATA_BufIndex=0;//数组索引
        RFID_DATA_Buf[RFID_DATA_BufIndex++]=data;//RFID数据缓存数组
      }
    }
    break;
  case 1:
    {
      if((data>=RFID_CMD_MIN_LENGTH)&&(data<=RFID_CMD_MAX_LENGTH))
      {
        RFID_DATA_Buf[RFID_DATA_BufIndex++]=data;
        DATA_7F_Num=0;
        RfidMachineState+=1;
      }
      else
      {
        RfidMachineState=0;//返回帧头判断
      }
    }
    break;
  case 2:
    {
      if(data==0x7F)
      {
        DATA_7F_Num+=1;
        if(DATA_7F_Num==1) return;
        else DATA_7F_Num=0;
      }
      RFID_DATA_Buf[RFID_DATA_BufIndex++]=data;
      if(RFID_DATA_BufIndex>=(RFID_DATA_Buf[1]+2))
      {
        RfidMachineState=0;
        //解析命令
        HandlerRfidCmd(RFID_DATA_Buf ,RFID_DATA_BufIndex);
      }
    }
    break;
  }
}
/*
	与校验位对应的有关操作
*/
u8 RFID_checkSum(u8* data, u8 length)
{
  u8 temp = 0;
  u8 i;
  for (i = 0; i < length; i++)
  {
    temp ^= data[i];//异或
  }
  return temp; 
}

void HandlerRfidCmd(u8* data ,u8 length)
{
  if(length<4) return;
  if( RFID_checkSum(&data[1],data[1])
           ==data[length-1])//校验位或者密钥匹配
  {
    switch(data[2])
    {
    case 0x10://一键读卡
      {
        RFID_Type=(data[4]<<8)|data[5];
        memcpy(RFID_CARD_ID,&data[6],4);  
        RFID_COMEIN_Flag|=1;
        RFID_ReadBlockRetryNum=3;//最多重复读3次
        RFID_ReadBlockTimes+=1;

        //printf("RFID_CARD_ID: %02X %02X %02X %02X  \r\n", RFID_CARD_ID[0], RFID_CARD_ID[1], RFID_CARD_ID[2], RFID_CARD_ID[3]);
      }
      break;
    case 0x14://一键读块数据
      {
        if(data[1]==0x19)
        {
          memcpy(RFID_BLOCK_Data,&data[10],16);//把读取到的信息存到RFID_BLOCK_Data中
          RFID_COMEIN_Flag|=2;
          RFID_ONLINE_Timeout=1000;
          RFID_ONLINE_Flag=1;
        }
        else//其他的代表错误
        {
          RFID_ReadBlockFailFlag=1; 
        }
      }
      break;
    }
  }
}

//发送一键读卡指令
void ReadRfidBlock(void)
{
  FillUartTxBufN((u8*)READ_BLOCK_ONE,11,RFID_COMM);
}
/*
	RFID读卡器返回值错误时的操作
*/
void READ_RFID_BLOCK_Task(void)
{
  static u8 pro=0;
  switch(pro)
  {
  case 0:
    {
      if((RFID_ReadBlockRetryNum!=0)&&(RFID_ReadBlockTimeout==0))
      {
        RFID_ReadBlockRetryNum-=1;
        RFID_ReadBlockTimeout=10;
        RFID_COMEIN_Flag&=~(2+4);
        RFID_ReadBlockFailFlag=0;   
        pro+=1;
      }
    }
    break;
  case 1:
    {
      if(RFID_ReadBlockTimeout==0)
      {
        ReadRfidBlock();
        RFID_ReadBlockTimeout=200;//200ms读超时
        pro+=1;
      }
    }
    break;
  case 2:
    {
      if(RFID_ReadBlockTimeout!=0)
      {
        if(RFID_ReadBlockFailFlag!=0)
        {
          RFID_ReadBlockFailFlag=0;
          RFID_ReadBlockUnSuccessTimes+=1; 
          RFID_ReadBlockTimeout=50;
          pro=0;
        }
        if(RFID_COMEIN_Flag&2)
        {
          printf("RFID_BLOCK_DATA: %02X %02X %02X %02X  \r\n", RFID_BLOCK_Data[0], RFID_BLOCK_Data[1], RFID_BLOCK_Data[2], RFID_BLOCK_Data[3]); 
          
          RFID_ReadBlockSuccessTimes+=1;
          RFID_ReadBlockTimeout=2;
          RFID_ReadBlockRetryNum=0;
          pro=0;
					
					if(RFID_BLOCK_Data[0]==0x02) CARD = 1; //  动作一： 直行
					if(RFID_BLOCK_Data[0]==0x06) CARD = 2; //  动作二： 向左转动45度
					if(RFID_BLOCK_Data[0]==0x0A) CARD = 3; //  动作三： 向左转动45度
					if(RFID_BLOCK_Data[0]==0x0B) CARD = 4; //  动作四： 向右转动45度
					if(RFID_BLOCK_Data[0]==0xAA) CARD = 5; //  动作五： 向右转动45度					
					if(RFID_BLOCK_Data[0]==0x04) CARD = 6; //  动作六： 向右转动90度，延时5s，向左转动90度
					if(RFID_BLOCK_Data[0]==0x07) CARD = 7; //  动作七： 直行
					if(RFID_BLOCK_Data[0]==0xFF) CARD = 8; //  动作八： 停止
					
        }
      }
      else
      {
        RFID_ReadBlockUnSuccessTimes+=1; 
        pro=0;
      }
    }
    break;
  default:
    {
      pro=0;
    }
  }

  
  if((RFID_ONLINE_Timeout == 0) && (RFID_ONLINE_Flag != 0))
  {
    RFID_ONLINE_Flag = 0;
  }
}
