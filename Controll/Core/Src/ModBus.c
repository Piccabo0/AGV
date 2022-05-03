#include "ModBus.h"
#include "string.h"
#include "stdlib.h"
#include "Rfid.h"
#include "Usart.h"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

#define HALL_COMM_ENUM              4  // 4�� 5
#define HALL_SENSOR_READ_ONCE_MS    100
#define HALL_RS485_TX_ACTIVE        RS485_1_TX_Active
#define HALL_RS485_RX_ACTIVE        RS485_1_RX_Active
#define HALL_BD_IN_K                115  // 19

uint16_t left,right;
extern int ROTATE;
//�������������ն����ȫ�ֱ���
MODBUS_SAMPLE MODBUS_HallSensor = {
  .MachineState = 0,
  .read_success_num = 0,
  .write_success_num = 0,
};

u8 HallStatusFresh=0;
u8 HallValue[LINE_SENSOR_NUM];
//SENSOR_STATUS SENSOR_Status={0,0,0};
SENSOR_STATUS_NEW SENSOR_STATUS_New={
  .black_sensor_serial_flag=0,
  .Segment_Num=0,
};
//�������������Ͷ����ȫ�ֱ���
const u8 MODBUS_READ_SENSOR_DATA1[8]=
{0x01 ,0x03 ,0x00 ,0x00 ,0x00 ,0x08 ,0x44 ,0x0C};


u16 HallSensor_Timeout = 3500;

u8 ON_LINE_Flag=0;
static u8 ON_LINE_Counter=0;
u8 MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR;

/*******************************************************************
��������:void Analysis_Receive_From_ModeBusSlaveDev(u8 data)
��������:���ջ��������� ����������� ��״̬����

pMODBUS��һ���ṹ�壨����
*******************************************************************/
void Analysis_Receive_From_HallSensor(u8 data, MODBUS_SAMPLE* pMODBUS)
{
	//����PMODBUS�е�MachineState��־λ�������¶�ȡ����֡
    switch(pMODBUS->MachineState)//��ʼ�� Ĭ�� Ϊ 00;
    {
    case 0x00:
      {
        if(data >= MODE_BUS_HALL_Addr)//�ӻ���ַ01
        {
          pMODBUS->MachineState = 0x01;
          pMODBUS->BufIndex = 0;
          pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        }
        else
        {
          pMODBUS->MachineState = 0x0B;//����������������Ҫ�����м�����Ϊ01������Ϊ��Ҫ�ӻ���ַ��
          pMODBUS->BufIndex = 0;//��������λ
        }  
      }
      break;
    case 0x01:
      {
        pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        if(data == CMD_ModBus_Read) //03��ȡָ��
        {
          pMODBUS->MachineState = 0x02; 
          pMODBUS->ModBus_CMD = data;
          pMODBUS->read_receive_timer = 0;
        }
        else
        { 
          pMODBUS->MachineState = 0x0B;
          pMODBUS->BufIndex = 0;
        }
      }
      break;
      case 0x02: //read part 00
      {    
        //���յ������ܵ��ֽ���
        pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        pMODBUS->read_receive_timer++;
        if(pMODBUS->read_receive_timer == 1)
        {
          pMODBUS->Read_Register_Num = pMODBUS->DataBuf[pMODBUS->BufIndex-1];
          if(pMODBUS->Read_Register_Num == 16) //��֧��һ�ֶ�ȡ���ݵķ�ʽ
          {
            pMODBUS->MachineState = 0x03;
          }
          else//��λ
          {
            pMODBUS->MachineState = 0x00;
          }
          pMODBUS->read_receive_timer = 0;
        } 
      }
      break;
      case 0x03: //read part 01
      {   
        pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        pMODBUS->read_receive_timer++;
        if(pMODBUS->read_receive_timer >= (pMODBUS->Read_Register_Num + 2))
        {
          u16 cal_crc;
          cal_crc=ModBus_CRC16_Calculate(pMODBUS->DataBuf,pMODBUS->Read_Register_Num+3);
              
          pMODBUS->receive_CRC_L = pMODBUS->DataBuf[pMODBUS->BufIndex-2];
          pMODBUS->receive_CRC_H = pMODBUS->DataBuf[pMODBUS->BufIndex-1];
          if(((cal_crc>>8) == pMODBUS->receive_CRC_H) && ((cal_crc&0xFF) == pMODBUS->receive_CRC_L))
          {
            pMODBUS->err_state = 0x00;//CRCУ����ȷ 
            pMODBUS->read_success_num += 1;
            
            //��ȷ��������
            memcpy(HallValue,pMODBUS->DataBuf + 3,16);
						int q1=65;
						int q2=40;
						int q3=30;
						int q4=25;
						int q5=25;
						int q6=25;
						int q7=25;
						int q8=20;	
						int _const =130;
						uint8_t speed1[16] = {0x01,0x16,0x00,0x38,0x00,0x04 ,0x00,0x00,0x00,0x64, 0x00,0x00,0x00,0x64, 0x0B,0x15};
						if(HallValue[15]+HallValue[14]+HallValue[13]+HallValue[12]+HallValue[11]+HallValue[10]+HallValue[9]+HallValue[8]+HallValue[7]+HallValue[6]+HallValue[5]+HallValue[4]+HallValue[3]+HallValue[2]+HallValue[1]+HallValue[0] != 0)//���ź�
							{
								ROTATE=0;
								right =   HallValue[15]*q1+HallValue[14]*q2+HallValue[13]*q3+HallValue[12]*q4+HallValue[11]*q5+HallValue[10]*q6+HallValue[9]*q7+HallValue[8]*q8+_const;
								left  =   HallValue[0]*q1+HallValue[1]*q2+HallValue[2]*q3+HallValue[3]*q4+HallValue[4]*q5+HallValue[5]*q6+HallValue[6]*q7+HallValue[7]*q8+_const;
								speed1[7] = 0x00; //���ַ�����ת
								speed1[11] = 0x00;//���ַ�������
								speed1[9] = left&0xFF;
								speed1[8] = left>>8;
								speed1[13] = right&0xFF ;
								speed1[12] = right>>8;
								cal_crc=ModBus_CRC16_Calculate(speed1 , sizeof(speed1)-2);
								speed1[14]=cal_crc&0xFF;
								speed1[15]=cal_crc>>8; 
								FillUartTxBufN((uint8_t*)speed1, 16, 1);
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);//������
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);//�����
								}
						else
							//�������
						{
								uint8_t stop[16] = {0x01,0x16,0x00,0x38,0x00,0x04 ,0x00,0x00,0x00,0x00, 0x00,0x00 ,0x00,0x00, 0x7B,0x36};
								FillUartTxBufN((uint8_t*)stop, 16, 1);
								ROTATE=1;								
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);//������
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);//�����
						}
            HallStatusFresh=1;
          }    
          else	  
          {
             pMODBUS->err_state = 0x04;
          }   
          pMODBUS->BufIndex = 0;  
          pMODBUS->read_receive_timer = 0;  
          pMODBUS->MachineState = 0x00;                
        }  
      }
      break;
      case 0xb:
      {
      }
      break;      
      default://�������������Զ��ָ�
      {
        pMODBUS->MachineState=0;
      }
    }
}

/********************************************************************************
��������:u16 ModBus_CRC16_Calculate(unsigned char *aStr , unsigned char alen)
��������:���㷢������ CRC У�鹦��
********************************************************************************/
u16 ModBus_CRC16_Calculate(u8 *aStr , u8 alen)
{
  u16 xda,xdapoly;
  u8 i,j,xdabit;
  xda = 0xFFFF;
  xdapoly = 0xA001;	// (X**16 + X**15 + X**2 + 1)
  for(i=0;i<alen;i++) 
  {
    xda ^= aStr[i];
    for(j=0;j<8;j++)
    {
      xdabit = (u8)(xda & 0x01);
      xda >>= 1;
      if( xdabit ) xda ^= xdapoly;
    }
  }    
  return xda;
}



void MODBUS_READ_HALL_SERSOR_Task(void)
{
  static u8 modebus_hall_tx_pro=0;
  static u16 ms_waste;
  switch(modebus_hall_tx_pro)
  {
  case 0:
    {
      if(HallSensor_Timeout == 0)
      {
        HALL_RS485_TX_ACTIVE();//RS485_1_TX_����ʹ��
        HallSensor_Timeout = 2;
        ms_waste = 2;
        modebus_hall_tx_pro++;
      }
    }
    break;
  case 1:
    {
      //���Ͷ�ȡsensor��ָ��
      if(HallSensor_Timeout == 0)
      {
        u8 temp_buf[16];
        u16 cal_crc;
        u16 bits = sizeof(MODBUS_READ_SENSOR_DATA1) * 10;
				/*void *memcpy(void *destin, void *source, unsigned n)��
				�����Ĺ����Ǵ�Դ�ڴ��ַ����ʼλ�ÿ�ʼ�������ɸ��ֽڵ�Ŀ���ڴ��ַ�У�����Դsource�п���n���ֽڵ�Ŀ��destin��
				*/
				memcpy(temp_buf,MODBUS_READ_SENSOR_DATA1,sizeof(MODBUS_READ_SENSOR_DATA1));
        temp_buf[0]=MODE_BUS_HALL_Addr;
        cal_crc=ModBus_CRC16_Calculate(temp_buf,sizeof(MODBUS_READ_SENSOR_DATA1)-2);
        temp_buf[6]=cal_crc&0xFF;
        temp_buf[7]=cal_crc>>8;   
        
        FillUartTxBufN((u8*)temp_buf, sizeof(MODBUS_READ_SENSOR_DATA1), HALL_COMM_ENUM);
        HallSensor_Timeout = (bits / HALL_BD_IN_K) + 3; // 2
        ms_waste += (bits / HALL_BD_IN_K) + 3;
        modebus_hall_tx_pro++;
      }
    }
    break;
  case 2:
    if(HallSensor_Timeout == 0)
    {
      HALL_RS485_RX_ACTIVE();//����ʹ��
      HallSensor_Timeout = HALL_SENSOR_READ_ONCE_MS - ms_waste;
      modebus_hall_tx_pro = 0;
    }
    break;
  default:  
    {
      modebus_hall_tx_pro=0; 
    }
  }
  
  //���������
  if(HallStatusFresh)
  {
    HallStatusFresh=0;
    CheckHallOnListNumNew(HallValue,LINE_SENSOR_NUM,&SENSOR_STATUS_New);
    
    if(SENSOR_STATUS_New.black_sensor_serial_flag==0)
    {
      //SetBeep(1,200,100);//��������
    }
    if(ON_LINE_Flag)
    {
      if(SENSOR_STATUS_New.Segment_Num==0)//���segment_Num=0���������
      {
        ON_LINE_Counter+=1;
        if(ON_LINE_Counter>=5) // 3 -> 5 
        {
          ON_LINE_Flag=0;
          ON_LINE_Counter=0;

          printf("HALL: Off LINE!  \r\n");
        }
      }
      else
      {
        ON_LINE_Counter=0;
      }
    }
    else
    {
      if(SENSOR_STATUS_New.Segment_Num!=0)
      {
        ON_LINE_Counter+=1;
        if(ON_LINE_Counter>=10)
        {
          ON_LINE_Flag=1;
          ON_LINE_Counter=0;
          
          printf("HALL: On LINE!  \r\n");
        }
      }
      else
      {
        ON_LINE_Counter=0;
      }
    }
  }
}


u8 CheckHallOnListNumNew(u8* hall_list,u8 total_num,SENSOR_STATUS_NEW* St)
{
  u8 i, num = 0;
  u8 start_flag=0;
  u8 seg_index=0;
  for(i=0;i<total_num;i++)
  {
    if(hall_list[i]!=0)
    {
      num++;
      if(start_flag==0) 
      {
        start_flag=1;
        St->seg_list[seg_index].head_index=(i+1);
      }
    }
    else
    {
      if(start_flag!=0)
      {
        start_flag=0;
        St->seg_list[seg_index].tail_index=i;
        //�߶γ����˲�
        if((St->seg_list[seg_index].tail_index-St->seg_list[seg_index].head_index+1)>=2)
        {
          St->seg_list[seg_index].middle_index=
            St->seg_list[seg_index].tail_index+St->seg_list[seg_index].head_index;
          seg_index+=1;
          
          if(seg_index>=MAX_SEGMENT_NUM) break;
        }
      }
    }
  }
  //���һ���о�
  {
      if(start_flag!=0)
      {
        start_flag=0;
        St->seg_list[seg_index].tail_index=i;
        //�߶γ����˲�
        if((St->seg_list[seg_index].tail_index-St->seg_list[seg_index].head_index+1)>=2)
        {
          St->seg_list[seg_index].middle_index=
            St->seg_list[seg_index].tail_index+St->seg_list[seg_index].head_index;
          seg_index+=1;
          
          //if(seg_index>=MAX_SEGMENT_NUM) break;
        }
      }  
  }
  
  if(seg_index==1) St->black_sensor_serial_flag=1;
  else St->black_sensor_serial_flag=0;
  St->Segment_Num=seg_index;
  return num;
}

