#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "74hc245.h"
#include "zimo.h"


int main(void)
{
    u8 i = 0;
    int a, j;
    delay_init();
    uart_init(115200);	 //串口初始化为115200
    HC245_Init();
    LED_Init();
    LED = 0;

    while(1)
    {
      /*  for(i = 0; i < 4; i++)          //显示的四行
        {
            for(j = 0; j < 2; j++)   //低位显示第几个字
            {
                for(a = 0; a < 2; a++) //显示1、2列
                {
                    SendDate(table[j * 32 + 3 * 8 + 2 * i + a]);
                    SendDate(table[j * 32 + 2 * 8 + 2 * i + a]);
                    SendDate(table[j * 32 + 1 * 8 + 2 * i + a]);
                    SendDate(table[j * 32 + 0 * 8 + 2 * i + a]);
                }
            }

            SetRowLight(i);
        }*/

       	RollLeftShow(table);

    }
}


