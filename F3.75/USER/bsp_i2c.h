#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "sys.h"
#include "delay.h"
#include "usart.h"
//使用IIC1 挂载M24C02,OLED,LM75AD,HT1382    PB6,PB7
 
#define SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)8<<28;}
#define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)3<<28;}
 
//IO操作函数	 
#define IIC_SCL    PBout(6) //SCL
#define IIC_SDA    PBout(7) //SDA	 
#define READ_SDA   PBin(7)  //输入SDA 


//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号
 
void IIC_WriteByte(uint16_t addr,uint8_t data,uint8_t device_addr);
uint16_t IIC_ReadByte(uint16_t addr,uint8_t device_addr,uint8_t ByteNumToRead);//寄存器地址，器件地址，要读的字节数  


void  read_AHT10_once(void);
void  reset_AHT10(void);
void  init_AHT10(void);	
void  startMeasure_AHT10(void);
void  read_AHT10(void);
uint8_t  Receive_ACK(void);
void  Send_ACK(void);
void  SendNot_Ack(void);
void I2C_WriteByte(uint8_t  input);
uint8_t I2C_ReadByte(void);	
void  set_AHT10sendOutData(void);
void  I2C_Start(void);
void  I2C_Stop(void);
#endif
