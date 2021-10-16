#ifndef __74HC245_H
#define __74HC245_H

#include "sys.h"

#define  LED_R1   PBout(8)
#define  LED_R2   PBout(9)
#define  LED_G1   PBout(12)
#define  LED_G2   PBout(13)
#define  LED_CK   PAout(1)
#define  LED_OE   PAout(11)
#define  LED_ST   PCout(13)
#define  LED_A    PAout(15)
#define  LED_B    PAout(14)
#define  LED_C    PAout(2)
#define  LED_D    PAout(3)

void SetRowLight(u8 signal);
void delay(void);
void SendDate(u8 DataR1);
void HC245_Init(void);
void RollLeftShow(u8 *table) ;
#endif

