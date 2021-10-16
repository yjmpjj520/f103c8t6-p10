#include "bsp_i2c.h"
#include "delay.h"

uint8_t   ack_status=0;
uint8_t   readByte[6];
uint8_t   aht10_status=0;

uint32_t  H1=0;  //Humility
uint32_t  T1=0;  //Temperature

uint8_t  AHT10_OutData[4];
uint8_t  AHT10sendOutData[10] = {0xFA, 0x06, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};

float AdcData[4][20];
u8 TempMeasure_Flag;//温度测试次数标志
float AveId[4];//数据测量次数，最多20
float AdcData_Temp[20];//承接温度的中间数据
float AdcData_Humi[20];//承接湿度的中间数据




/*
函数:ZuoYi
功能:20个数据循环左移1
*/
void ZuoYi(float *arr){
	int i;
	for(i=0;i<19;i++){
		arr[i]=arr[i+1];
	}
}

//将数组arr2的20个数据给数组arr1;
void JiaoHuan(float *arr1,float *arr2) {
	int i;
	for(i=0;i<20;i++){
	arr1[i]= arr2[i];
	}
}



void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
 
	IIC_SCL=1;
	IIC_SDA=1;
 
}
//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;//发送I2C总线结束信号
	delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	IIC_SDA=1;delay_us(1);	   
	IIC_SCL=1;delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
		SDA_OUT(); 	    
    IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		IIC_SCL=1;
		delay_us(2); 
		IIC_SCL=0;	
		delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
  for(i=0;i<8;i++ )
	{
    IIC_SCL=0; 
    delay_us(2);
		IIC_SCL=1;
    receive<<=1;
    if(READ_SDA)receive++;   
		delay_us(1); 
  }					 
	if (!ack)
			IIC_NAck();//发送nACK
	else
			IIC_Ack(); //发送ACK   
	return receive;
}
 
void IIC_WriteByte(uint16_t addr,uint8_t data,uint8_t device_addr)
{
	IIC_Start();  
	
	if(device_addr==0xA0) //eeprom地址大于1字节
		IIC_Send_Byte(0xA0 + ((addr/256)<<1));//发送高地址
	else
		IIC_Send_Byte(device_addr);	    //发器件地址
	IIC_Wait_Ack(); 
	IIC_Send_Byte(addr&0xFF);   //发送低地址
	IIC_Wait_Ack(); 
	IIC_Send_Byte(data);     //发送字节							   
	IIC_Wait_Ack();  		    	   
  IIC_Stop();//产生一个停止条件 
	if(device_addr==0xA0) //
		delay_ms(10);
	else
		delay_us(2);
}
 
uint16_t IIC_ReadByte(uint16_t addr,uint8_t device_addr,uint8_t ByteNumToRead)  //读寄存器或读数据
{	
		uint16_t data;
		IIC_Start();  
		if(device_addr==0xA0)
			IIC_Send_Byte(0xA0 + ((addr/256)<<1));
		else
			IIC_Send_Byte(device_addr);	
		IIC_Wait_Ack();
		IIC_Send_Byte(addr&0xFF);   //发送低地址
		IIC_Wait_Ack(); 
 
		IIC_Start();  	
		IIC_Send_Byte(device_addr+1);	    //发器件地址
		IIC_Wait_Ack();
		if(ByteNumToRead == 1)//LM75温度数据为11bit
		{
			data=IIC_Read_Byte(0);
		}
		else
			{
				data=IIC_Read_Byte(1);
				data=(data<<8)+IIC_Read_Byte(0);
			}
		IIC_Stop();//产生一个停止条件	    
		return data;
}


/**********
*上面部分为IO口模块I2C配置
*
*从这以下开始为AHT10的配置I2C
*函数名有IIC和I2C的区别，请注意！！！！！
*
*2020/2/23最后修改日期
*
***********/
void  read_AHT10_once(void)
{
	delay_ms(10);

	reset_AHT10();
	delay_ms(10);

	init_AHT10();
	delay_ms(10);

	startMeasure_AHT10();
	delay_ms(80);

	read_AHT10();
	delay_ms(5);
}


void  reset_AHT10(void)
{

	I2C_Start();

	I2C_WriteByte(0x70);
	ack_status = Receive_ACK();
	if(ack_status) printf("1");
	else printf("1-n-");
	I2C_WriteByte(0xBA);
	ack_status = Receive_ACK();
		if(ack_status) printf("2");
	else printf("2-n-");
	I2C_Stop();

	/*
	AHT10_OutData[0] = 0;
	AHT10_OutData[1] = 0;
	AHT10_OutData[2] = 0;
	AHT10_OutData[3] = 0;
	*/
}



void  init_AHT10(void)
{
	I2C_Start();

	I2C_WriteByte(0x70);
	ack_status = Receive_ACK();
	if(ack_status) printf("3");
	else printf("3-n-");	
	I2C_WriteByte(0xE1);
	ack_status = Receive_ACK();
	if(ack_status) printf("4");
	else printf("4-n-");
	I2C_WriteByte(0x08);
	ack_status = Receive_ACK();
	if(ack_status) printf("5");
	else printf("5-n-");
	I2C_WriteByte(0x00);
	ack_status = Receive_ACK();
	if(ack_status) printf("6");
	else printf("6-n-");
	I2C_Stop();
}



void  startMeasure_AHT10(void)
{
	//------------
	I2C_Start();

	I2C_WriteByte(0x70);
	ack_status = Receive_ACK();
	if(ack_status) printf("7");
	else printf("7-n-");
	I2C_WriteByte(0xAC);
	ack_status = Receive_ACK();
	if(ack_status) printf("8");
	else printf("8-n-");
	I2C_WriteByte(0x33);
	ack_status = Receive_ACK();
	if(ack_status) printf("9");
	else printf("9-n-");
	I2C_WriteByte(0x00);
	ack_status = Receive_ACK();
	if(ack_status) printf("10");
	else printf("10-n-");
	I2C_Stop();
}



void read_AHT10(void)
{
	uint8_t   i;
u8 dataid;
	for(i=0; i<6; i++)
	{
		readByte[i]=0;
	}

	//-------------
	I2C_Start();

	I2C_WriteByte(0x71);
	ack_status = Receive_ACK();
	if(ack_status) printf("11");
	else printf("11-n-");
	readByte[0]= I2C_ReadByte();
	printf("test0:%d",readByte[0]);
	Send_ACK();

	readByte[1]= I2C_ReadByte();
	printf("test1:%d",readByte[1]);
	Send_ACK();

	readByte[2]= I2C_ReadByte();
	printf("test2:%d",readByte[2]);
	Send_ACK();

	readByte[3]= I2C_ReadByte();
	printf("test3:%d",readByte[3]);
	Send_ACK();

	readByte[4]= I2C_ReadByte();
	printf("test4:%d",readByte[4]);
	Send_ACK();

	readByte[5]= I2C_ReadByte();
	printf("test5:%d",readByte[5]);
	SendNot_Ack();
	//Send_ACK();

	I2C_Stop();

	//--------------
	if( (readByte[0] & 0x68) == 0x08 )
	{
		H1 = readByte[1];
		H1 = (H1<<8) | readByte[2];
		H1 = (H1<<8) | readByte[3];
		H1 = H1>>4;

		H1 = (H1*1000)/1024/1024;

		T1 = readByte[3];
		T1 = T1 & 0x0000000F;
		T1 = (T1<<8) | readByte[4];
		T1 = (T1<<8) | readByte[5];

		T1 = (T1*2000)/1024/1024 - 500;

		AHT10_OutData[0] = (H1>>8) & 0x000000FF;
		AHT10_OutData[1] = H1 & 0x000000FF;

		AHT10_OutData[2] = (T1>>8) & 0x000000FF;
		AHT10_OutData[3] = T1 & 0x000000FF;
		printf("成功了");
	}
	else
	{
		AHT10_OutData[0] = 0xFF;
		AHT10_OutData[1] = 0xFF;

		AHT10_OutData[2] = 0xFF;
		AHT10_OutData[3] = 0xFF;
		printf("失败了");

	}
//	printf("\r\n");
//	printf("温度:%d%d.%d",T1/100,(T1/10)%10,T1%10);
//	printf("湿度:%d%d.%d",H1/100,(H1/10)%10,H1%10);
//	printf("\r\n");
		
//				if(AveId[0]<20&&TempMeasure_Flag<20)   //首先采集20次数据
//			{
//				dataid=AveId[0];
//				AdcData[0][dataid]=T1;
//				AdcData[1][dataid]=H1;
//				AveId[0]++;
//				TempMeasure_Flag++;
//				if(TempMeasure_Flag==20){  //第一次记录20次数据。
//				JiaoHuan(AdcData_Temp,AdcData[0])	;//将二维数组中温度未排序的数组记录下来。		
//				JiaoHuan(AdcData_Humi,AdcData[1])	;//将二维数组湿度第一次收集的20个数据记录给中间数组
//				}
//			}
//			else if(AveId[0]==20&&TempMeasure_Flag>=20)  //从21次开始走这里
//			{
//				ZuoYi(AdcData_Temp);//将交换数组中温度的20个数组左移1，此时第18等于第19，非循环左移。
//				ZuoYi(AdcData_Humi);//将交换数组中温度的20个数组左移1，此时第18等于第19，非循环左移。
//				AdcData_Temp[19]= T1;//在交换数组中将获取的新的数据覆盖19。
//				AdcData_Humi[19]= H1;//在交换数组中将获取的新的数据覆盖19。
//				JiaoHuan(AdcData[0],AdcData_Temp)	;//将未排序的20个数据给要运算的数组。
//				JiaoHuan(AdcData[1],AdcData_Humi)	;//将未排序的20个数据给要运算的数组。
//			}
}




uint8_t  Receive_ACK(void)
{
	uint8_t result=0;
	uint8_t cnt=0;

	IIC_SCL = 0;
	SDA_IN(); 
	delay_us(4);

	IIC_SCL = 1;
	delay_us(4);

	while(READ_SDA && (cnt<100))
	{
		cnt++;
	}

	IIC_SCL = 0;
	delay_us(4);

	if(cnt<100)
	{
		result=1;
	}
	return result;
}



void  Send_ACK(void)
{
	SDA_OUT();
	IIC_SCL = 0;
	delay_us(4);

	IIC_SDA = 0;
	delay_us(4);

	IIC_SCL = 1;
	delay_us(4);
	IIC_SCL = 0;
	delay_us(4);

	SDA_IN();
}



void  SendNot_Ack(void)
{
	SDA_OUT();
	IIC_SCL = 0;
	delay_us(4);

	IIC_SDA = 1;
	delay_us(4);

	IIC_SCL = 1;
	delay_us(4);

	IIC_SCL = 0;
	delay_us(4);

	IIC_SDA = 0;
	delay_us(4);
}


void I2C_WriteByte(uint8_t  input)
{
	uint8_t  i;
	SDA_OUT();
	for(i=0; i<8; i++)
	{
		IIC_SCL = 0;
		delay_ms(5);

		if(input & 0x80)
		{
			IIC_SDA = 1;
			//delaymm(10);
		}
		else
		{
			IIC_SDA = 0;
			//delaymm(10);
		}

		IIC_SCL = 1;
		delay_ms(5);

		input = (input<<1);
	}

	IIC_SCL = 0;
	delay_us(4);

	SDA_IN();
	delay_us(4);
}	


uint8_t I2C_ReadByte(void)
{
	uint8_t  resultByte=0;
	uint8_t  i=0, a=0;

	IIC_SCL = 0;
	SDA_IN();
	delay_ms(4);

	for(i=0; i<8; i++)
	{
		IIC_SCL = 1;
		delay_ms(3);

		a=0;
		if(READ_SDA)
		{
			a=1;
		}
		else
		{
			a=0;
		}

		//resultByte = resultByte | a;
		resultByte = (resultByte << 1) | a;

		IIC_SCL = 0;
		delay_ms(3);
	}

	SDA_IN();
	delay_ms(10);

	return   resultByte;
}


void  set_AHT10sendOutData(void)
{
	/* --------------------------
	 * 0xFA 0x06 0x0A temperature(2 Bytes) humility(2Bytes) short Address(2 Bytes)
	 * And Check (1 byte)
	 * -------------------------*/
	AHT10sendOutData[3] = AHT10_OutData[0];
	AHT10sendOutData[4] = AHT10_OutData[1];
	AHT10sendOutData[5] = AHT10_OutData[2];
	AHT10sendOutData[6] = AHT10_OutData[3];

//	AHT10sendOutData[7] = (drf1609.shortAddress >> 8) & 0x00FF;
//	AHT10sendOutData[8] = drf1609.shortAddress  & 0x00FF;

//	AHT10sendOutData[9] = getXY(AHT10sendOutData,10);
}


void  I2C_Start(void)
{
	SDA_OUT();
	IIC_SCL = 1;
	delay_ms(4);

	IIC_SDA = 1;
	delay_ms(4);
	IIC_SDA = 0;
	delay_ms(4);

	IIC_SCL = 0;
	delay_ms(4);
}



void  I2C_Stop(void)
{
	SDA_OUT();
	IIC_SDA = 0;
	delay_ms(4);

	IIC_SCL = 1;
	delay_ms(4);

	IIC_SDA = 1;
	delay_ms(4);
}
