#ifndef __HEAD_H__
#define __HEAD_H__


#define LED1_ON   _IO('a',0)
#define LED1_OFF   _IO('a',1)

#define HMB_ON   _IO('b',0)
#define FUN_ON  _IO('c',0)
#define PRE_MOTOR_ON   _IO('d',0)
#define HMB_OFF   _IO('b',1)
#define FUN_OFF  _IO('c',1)
#define PRE_MOTOR_OFF   _IO('d',1)
#define GET_TMP  _IOR('a',0,int)
#define GET_HUMM  _IOR('a',1,int)
#define SEG_WHICH _IOW('k',0,int)
#define SEG_DAT  _IOW('k',1,int)
#define SEG_DAT1  _IOW('k',2,int)
#define DANGER_TMP 30

unsigned char code[] = {
	0x3f, //0
	0x06, //1
	0x5b, //2
	0x4f, //3
	0x66, //4
	0x6d, //5
	0x7d, //6
	0x07, //7
	0x7f, //8
	0x6f, //9
	0x77, //A
	0x7c, //b
	0x39, //c
	0x5e, //d
	0x79, //e
	0x71, //f
	0x39,//c
	0x76,//H
};
unsigned char  code1[] = {
	0xbf, //0
	0x86, //1
	0xdb, //2
	0xcf, //3
	0xe6, //4
	0xed, //5
	0xfd, //6
	0x87, //7
	0xff, //8
	0xef, //9
	0xf7, //A
	0xfc, //b
	0xb9, //c
	0xde, //d
	0xf9, //e
	0xf1, //f
};
unsigned char which[] = {
	0x1, //sg0
	0x2, //sg1
	0x4, //sg2
	0x8, //sg3
};
#endif

