#include "74hc245.h"
#include "usart.h"
#include "delay.h"

void HC245_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE) ;
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3  | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_12 | GPIO_Pin_13 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

}

void SendDate(u8 DataR1)
{
    u8 i;

    for(i = 0; i < 8; i++)
    {
        if(DataR1 & 0x80)
        {
            LED_R1 = 0;
        }
        else
        {
            LED_R1 = 1;
        }

        LED_CK = 0;
        LED_CK = 1;
        delay_us(1);
        LED_CK = 0;
        DataR1 <<= 1;
    }
}
void SetRowLight(u8 signal)
{

    LED_ST = 1;
    LED_ST = 0;
    LED_OE = 1;
    LED_A = 0x01 & signal;
    LED_B = 0x01 & (signal >> 1);
}
void RollLeftShow(u8 *table)
{
    u16 i, j, l, k, add, shan_1, shun;
    u32 buf_0, buf_1, buf_2, buf_3;
    u8 *buf = table;
    u32 roll[16] = {0};

    for(shun = 0; shun < 17; shun++)                                     //显示字体（数组）个数-3
    {
        for(add = 0; add < 16; add++)                                    //只知道这里是显示地址，但不怎么改都不行
        {
            for(shan_1 = 0; shan_1 < 200; shan_1++)    //控制滚动时长
            {
                for(k = 0; k < 16; k++)           //显示16行
                {
                    roll[k] = table[shun * 32 + 2 * k];                                          //第1个字的数组第1位
                    roll[k] = (roll[k] << 8) + table[shun * 32 + 2 * k + 1];    //第1个字的数组第2位
                    roll[k] = (roll[k] << 8) + table[(shun + 1) * 32 + 2 * k]; //第2个字的数组第1位
                    roll[k] = (roll[k] << 8) + table[(shun + 1) * 32 + 2 * k + 1]; //第2个字的数组第2位
                    buf_0 = roll[k];
                    buf[0 * 32 + 2 * k] = (buf_0 >> (24 - add)) & 0xff;
                    buf_1 = (buf_0 << 8) + table[(shun + 2) * 32 + 2 * k];
                    buf[0 * 32 + 2 * k + 1] = (buf_1 >> (24 - add)) & 0xff;
                    buf_2 = (buf_1 << 8) + table[(shun + 2) * 32 + 2 * k + 1];
                    buf[1 * 32 + 2 * k] = (buf_2 >> (24 - add)) & 0xff;
                    buf_3 = (buf_2 << 8) + table[(shun + 3) * 32 + 2 * k];
                    buf[1 * 32 + 2 * k + 1] = (buf_3 >> (24 - add)) & 0xff;
                }

                for(l = 0; l < 4; l++)        //显示的四行
                {
                    for(j = 0; j < 2; j++)    //显示3、4列
                    {
                        for(i = 0; i < 2; i++) //显示1、2列
                        {
                            SendDate(buf[j * 32 + 3 * 8 + 2 * l + i]);
                            SendDate(buf[j * 32 + 2 * 8 + 2 * l + i]);
                            SendDate(buf[j * 32 + 1 * 8 + 2 * l + i]);
                            SendDate(buf[j * 32 + 0 * 8 + 2 * l + i]);
                        }
                    }

                    SetRowLight(l);
                }
            }
        }
    }
}
