#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include "head.h"
#include<linux/kernel.h>


int main(int argc, const char *argv[])
{
	int fd,choose,ret;
	int size,tmp,hum;
	int rtmp,rhum;
	char tmp_value[4]={0};
	char hum_value[4]={0};
	int flag=4,time=0;
	int which=0;
	int data=0;
	if((fd = open("/dev/mycdev0",O_RDWR)) < 0){
		perror("open /dev/mycdev0  error");
		return -1;
	}

	while(1){
		switch(flag){

		case 1:
			ioctl(fd,SEG_WHICH,0);
			ioctl(fd,SEG_DAT,tmp_value[0]);
			ioctl(fd,SEG_WHICH,1);
			ioctl(fd,SEG_DAT1,tmp_value[1]);
			ioctl(fd,SEG_WHICH,2);
			ioctl(fd,SEG_DAT,tmp_value[2]);
			ioctl(fd,SEG_WHICH,3);
			ioctl(fd,SEG_DAT,tmp_value[3]);
			if(time++>10000)
			{
			time=0;
			flag=3;
			}
			break;
		case 2:		
			ioctl(fd,SEG_WHICH,0);
			ioctl(fd,SEG_DAT,hum_value[0]);
			ioctl(fd,SEG_WHICH,1);
			ioctl(fd,SEG_DAT1,hum_value[1]);
			ioctl(fd,SEG_WHICH,2);
			ioctl(fd,SEG_DAT,hum_value[2]);
			ioctl(fd,SEG_WHICH,3);
			ioctl(fd,SEG_DAT,hum_value[3]);
			if(time++>10000)
			{
			time=0;
			flag=4;
			}
			break;
			case 3:
				ioctl(fd,GET_HUMM,&hum);	
			rhum = (125.0*hum/65536-6)*10;
			hum_value[0]= rhum/100;
			hum_value[1]=rhum%100/10;
			hum_value[2]=rhum%10;
			hum_value[3]=17;
			flag =2;
			printf("hum = %d\n",rhum);
			printf("hum = %d--%d--%d--%d\n",hum_value[0],hum_value[1],hum_value[2]);
			break;
		case 4:
			ioctl(fd,GET_TMP,&tmp);

			rtmp = (175.72*tmp/65536 - 46.85)*10;
			tmp_value[0]= rtmp/100;
			tmp_value[1]=rtmp%100/10;
			tmp_value[2]=rtmp%10;
			tmp_value[3]=16;
			if((rtmp/10)>DANGER_TMP)
			{
			   ioctl(fd,HMB_ON);
			   ioctl(fd,FUN_ON);
			   ioctl(fd,PRE_MOTOR_ON);

			}
			flag=1;
			printf("tmp = %d\n",rtmp);
			printf("tem = %d--%d--%d--%d\n",tmp_value[0],tmp_value[1],tmp_value[2]);
			break;

			/*
			   printf("*******rtmp = %.2f***\n",rtmp);
			   printf("*******rhum = %.2f***\n",rhum);
			   printf("*******please enter number*\n");
			   printf("*******1--LED_ON**********\n");
			   printf("*******2--LED_OFF**********\n");
			   printf("*******3--HMB_ON**********\n");
			   printf("*******4--HMB_OFF**********\n");
			   printf("*******5--FUN_ON**********\n");
			   printf("*******6--FUN_OFF**********\n");
			   printf("*******7--MOTOR_ON**********\n");
			   printf("*******8--MOTOR_OFF**********\n");

			   scanf("%d",&choose);
			   while(getchar()!='\n');
			   switch(choose){
			   case 1:

			   ioctl(fd,LED1_ON);
			   break;
			   case 3:
			   ioctl(fd,HMB_ON);
			   break;
			   case 5:
			   ioctl(fd,FUN_ON);
			   break;
			   case 7:
			   ioctl(fd,PRE_MOTOR_ON);
			   break;
			   case 2:
			   ioctl(fd,LED1_OFF);
			   break;
			   case 4:
			   ioctl(fd,HMB_OFF);
			   break;
			   case 6:
			   ioctl(fd,FUN_OFF);
			   break;
			   case 8:
			   ioctl(fd,PRE_MOTOR_OFF);
			   break;
			   }
			   printf("按键按下次数 =%d\n",choose);*/

		}
	}

	close(fd);
	return 0;
}

