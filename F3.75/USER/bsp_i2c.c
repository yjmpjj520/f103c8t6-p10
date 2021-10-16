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
u8 TempMeasure_Flag;//�¶Ȳ��Դ�����־
float AveId[4];//���ݲ������������20
float AdcData_Temp[20];//�н��¶ȵ��м�����
float AdcData_Humi[20];//�н�ʪ�ȵ��м�����




/*
����:ZuoYi
����:20������ѭ������1
*/
void ZuoYi(float *arr){
	int i;
	for(i=0;i<19;i++){
		arr[i]=arr[i+1];
	}
}

//������arr2��20�����ݸ�����arr1;
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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
 
	IIC_SCL=1;
	IIC_SDA=1;
 
}
//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT();     //sda�����
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  
//����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT();//sda�����
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;//����I2C���߽����ź�
	delay_us(4);							   	
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA����Ϊ����  
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
	IIC_SCL=0;//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
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
//������ACKӦ��		    
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
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
		SDA_OUT(); 	    
    IIC_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(2);   //��TEA5767��������ʱ���Ǳ����
		IIC_SCL=1;
		delay_us(2); 
		IIC_SCL=0;	
		delay_us(2);
    }	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA����Ϊ����
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
			IIC_NAck();//����nACK
	else
			IIC_Ack(); //����ACK   
	return receive;
}
 
void IIC_WriteByte(uint16_t addr,uint8_t data,uint8_t device_addr)
{
	IIC_Start();  
	
	if(device_addr==0xA0) //eeprom��ַ����1�ֽ�
		IIC_Send_Byte(0xA0 + ((addr/256)<<1));//���͸ߵ�ַ
	else
		IIC_Send_Byte(device_addr);	    //��������ַ
	IIC_Wait_Ack(); 
	IIC_Send_Byte(addr&0xFF);   //���͵͵�ַ
	IIC_Wait_Ack(); 
	IIC_Send_Byte(data);     //�����ֽ�							   
	IIC_Wait_Ack();  		    	   
  IIC_Stop();//����һ��ֹͣ���� 
	if(device_addr==0xA0) //
		delay_ms(10);
	else
		delay_us(2);
}
 
uint16_t IIC_ReadByte(uint16_t addr,uint8_t device_addr,uint8_t ByteNumToRead)  //���Ĵ����������
{	
		uint16_t data;
		IIC_Start();  
		if(device_addr==0xA0)
			IIC_Send_Byte(0xA0 + ((addr/256)<<1));
		else
			IIC_Send_Byte(device_addr);	
		IIC_Wait_Ack();
		IIC_Send_Byte(addr&0xFF);   //���͵͵�ַ
		IIC_Wait_Ack(); 
 
		IIC_Start();  	
		IIC_Send_Byte(device_addr+1);	    //��������ַ
		IIC_Wait_Ack();
		if(ByteNumToRead == 1)//LM75�¶�����Ϊ11bit
		{
			data=IIC_Read_Byte(0);
		}
		else
			{
				data=IIC_Read_Byte(1);
				data=(data<<8)+IIC_Read_Byte(0);
			}
		IIC_Stop();//����һ��ֹͣ����	    
		return data;
}


/**********
*���沿��ΪIO��ģ��I2C����
*
*�������¿�ʼΪAHT10������I2C
*��������IIC��I2C��������ע�⣡��������
*
*2020/2/23����޸�����
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
		printf("�ɹ���");
	}
	else
	{
		AHT10_OutData[0] = 0xFF;
		AHT10_OutData[1] = 0xFF;

		AHT10_OutData[2] = 0xFF;
		AHT10_OutData[3] = 0xFF;
		printf("ʧ����");

	}
//	printf("\r\n");
//	printf("�¶�:%d%d.%d",T1/100,(T1/10)%10,T1%10);
//	printf("ʪ��:%d%d.%d",H1/100,(H1/10)%10,H1%10);
//	printf("\r\n");
		
//				if(AveId[0]<20&&TempMeasure_Flag<20)   //���Ȳɼ�20������
//			{
//				dataid=AveId[0];
//				AdcData[0][dataid]=T1;
//				AdcData[1][dataid]=H1;
//				AveId[0]++;
//				TempMeasure_Flag++;
//				if(TempMeasure_Flag==20){  //��һ�μ�¼20�����ݡ�
//				JiaoHuan(AdcData_Temp,AdcData[0])	;//����ά�������¶�δ����������¼������		
//				JiaoHuan(AdcData_Humi,AdcData[1])	;//����ά����ʪ�ȵ�һ���ռ���20�����ݼ�¼���м�����
//				}
//			}
//			else if(AveId[0]==20&&TempMeasure_Flag>=20)  //��21�ο�ʼ������
//			{
//				ZuoYi(AdcData_Temp);//�������������¶ȵ�20����������1����ʱ��18���ڵ�19����ѭ�����ơ�
//				ZuoYi(AdcData_Humi);//�������������¶ȵ�20����������1����ʱ��18���ڵ�19����ѭ�����ơ�
//				AdcData_Temp[19]= T1;//�ڽ��������н���ȡ���µ����ݸ���19��
//				AdcData_Humi[19]= H1;//�ڽ��������н���ȡ���µ����ݸ���19��
//				JiaoHuan(AdcData[0],AdcData_Temp)	;//��δ�����20�����ݸ�Ҫ��������顣
//				JiaoHuan(AdcData[1],AdcData_Humi)	;//��δ�����20�����ݸ�Ҫ��������顣
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
