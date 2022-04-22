#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "cli.h"
#define ERR_MSG(msg) do{\
	fprintf(stderr, "__%d__ ", __LINE__);\
	perror(msg);\
}while(0)
#define PORT 8888
#define IPADDR "10.102.132.26"
USER u1;

int main(int argc, const char *argv[])
{
	//if(argc < 3)
	//	{
	//		fprintf(stderr, "请输入 IP  端口\n");
	//		return -1;
	//	}

	//创建流式套接字
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0)
	{
		ERR_MSG("socket");
		return -1;
	}

	//绑定客户端的IP和端口---->非必须

	//填充要连接的服务器的地址信息结构体
	struct sockaddr_in sin;
	sin.sin_family 		= AF_INET;
	sin.sin_port 		= htons(PORT); 	//服务器的端口
	sin.sin_addr.s_addr = inet_addr(IPADDR); 	//服务器绑定的IP

	//连接服务器
	if(connect(sfd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		ERR_MSG("connect");
		return -1;
	}
	printf("connect success\n");

	ssize_t res = 0;
	char buf[128] = "";
	int choose = 0;
	while(1)
	{
		bzero(buf, sizeof(buf));
		//从终端获取数据
		printf("*************************\n");
		printf("*******1-注册************\n");
		printf("*******2-登陆*** ********\n");
		printf("*******3-退出************\n");
		printf("*************************\n");
		printf("请输入选择>>>");

		scanf("%d",&choose);
		getchar();
		switch(choose)
		{
		case 1:
			user_register(sfd);

			break;
		case 2:
			loin(sfd);
			break;
		case 3:
			quit(sfd);
			break;
		}

		printf(":%s\n", buf);
	}

	//关闭套接字
	close(sfd);
	return 0;
}
int user_register(int sfd)
{
	char buf[1024]={0};
	ssize_t res =0;
	bzero(u1.account, sizeof(u1.account));
	bzero(&u1.pwd, sizeof(u1.pwd));
	u1.type= 'R';
	printf("请输入account>>>");
	fgets(u1.account, sizeof(u1.account), stdin);
	u1.account[strlen(u1.account)-1] = 0;

	printf("请输入passwd>>>");

	fgets(u1.pwd, sizeof(u1.pwd), stdin);
	u1.pwd[strlen(u1.pwd)-1] = 0;
	//fgets(u1->pwd, sizeof(u1->pwd), stdin);
	//buf[strlen(u1->pwd)-1] = 0;
	res = send(sfd, &u1, sizeof(u1), 0);
	if (res < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	res = recv(sfd, buf, sizeof(buf), 0);
	if (res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	printf(":>%s\n", buf);
	return 0;

}
int loin(int sfd)
{
	char buf[1024] = "";
	ssize_t res = 0;
	int choose=0;

	bzero(u1.account, sizeof(u1.account));
	bzero(u1.pwd, sizeof(u1.pwd));
	u1.type= 'L';

	printf("请输入用户名:>");
	scanf("%s", u1.account);
	while(getchar() != '\n');

	printf("请输入密码:>");
	scanf("%s", u1.pwd);
	while(getchar() != '\n');

	res = send(sfd, &u1, sizeof(u1), 0);
	if (res < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	res = recv(sfd, buf, sizeof(buf), 0);
	if (res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	printf(":>%s\n", buf);
	if(((strcmp(buf,"该账号以登陆,请勿重复登陆!"))==0)||((strcmp(buf,"账号或密码错误,请重新登陆!"))==0))
	{
		return 0;
	}
	if((strcmp(u1.account,"1")==0)&&(strcmp(u1.pwd,"1")==0))
	{
		while(1)
		{
			choose = menu();
			switch(choose){
			case 1:
				add_staff(sfd);
				break;
			case 2:
				delete_staff(sfd);
				break;
			case 3:
				updata_staff(sfd);
				break;
			case 4:
				query_staff(sfd);
				break;


			}
			if(6==choose)
			{	break;}
		}
	}else{
		while(1)
		{
			printf("*******1-查询员工************\n");
			printf("*******2-返回****************\n");
			scanf("%d",&choose);
			switch(choose){
			case 1:
				query_staff(sfd);
				break;
			case 2:
				break;
			default:
				printf("输入错误************\n");
			}
			if(2==choose)
			{break;}

		}
	}
	return 0;
}
//退出
int quit(int sfd)
{	
	char buf[1024] = "";
	ssize_t res = 0;
	u1.type = 'S';
	printf("%c\n",u1.type);
	res = send(sfd, &u1, sizeof(u1), 0);
	if (res < 0)
	{
		ERR_MSG("send");
		return -1;
	}
	res = recv(sfd, buf, sizeof(buf), 0);
	if (res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	printf(":>%s\n", buf);

	exit(0);
	return 0;
}
int menu()
{
	int temp;
	printf("*****************************\n");
	printf("*******1-添加员工************\n");
	printf("*******2-删除员工*** ********\n");
	printf("*******3-修改信息************\n");
	printf("*******4-查询员工************\n");
	printf("*****************************\n");
	printf("*******6-返回****************\n");
	printf("请输入选择>>>");
	scanf("%d",&temp);
	while(getchar()!='\n');
	return temp;
}
int add_staff(int sfd)
{
	char buf[1024] = "";
	ssize_t res = 0;

	bzero(u1.account, sizeof(u1.account));
	bzero(u1.pwd, sizeof(u1.pwd));
	u1.type= 'A';

	printf("请输入员工姓名:>");
	scanf("%s", u1.name);
	while(getchar() != '\n');

	printf("请输入员工性别:>");
	scanf("%c", &u1.sex);
	while(getchar() != '\n');


	printf("请输入员工年龄:>");
	scanf("%d", &u1.age);
	while(getchar() != '\n');

	printf("请输入员工电话号码:>");
	scanf("%s", u1.iphone);
	while(getchar() != '\n');

	printf("请输入员工薪资:>");
	scanf("%d", &u1.salar);
	while(getchar() != '\n');
	res = send(sfd, &u1, sizeof(u1), 0);
	if (res < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	res = recv(sfd, buf, sizeof(buf), 0);
	if (res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	printf(":>%s\n", buf);

}
int delete_staff(int sfd)
{
	char buf[1024] = "";
	ssize_t res = 0;
	u1.type = 'D';
	printf("请输入员工电话号码:>");

	scanf("%s",u1.iphone);
	res = send(sfd, &u1, sizeof(u1), 0);
	if (res < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	res = recv(sfd, buf, sizeof(buf), 0);
	if (res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	printf(":>%s\n", buf);


}
int updata_staff(int sfd)
{
	char buf[1024] = "";
	ssize_t res = 0;

	u1.type= 'U';

	printf("请输入要修改员工电话号码:>");
	scanf("%s", u1.iphone);
	while(getchar() != '\n');
	printf("请输入修改后的员工姓名:>");
	scanf("%s", u1.name);
	while(getchar() != '\n');

	printf("请输入修改后的员工性别:>");
	scanf("%c", &u1.sex);
	while(getchar() != '\n');


	printf("请输入修改后的员工年龄:>");
	scanf("%d", &u1.age);
	while(getchar() != '\n');


	printf("请输入修改后的员工薪资:>");
	scanf("%d", &u1.salar);
	while(getchar() != '\n');
	res = send(sfd, &u1, sizeof(u1), 0);
	if (res < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	res = recv(sfd, buf, sizeof(buf), 0);
	if (res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	printf(":>%s\n", buf);

}
int query_staff(int sfd)
{
	char buf[1024] = "";
	ssize_t res = 0;

	u1.type= 'Q';

	printf("请输入要查询员工的电话号码:>");
	scanf("%s", u1.iphone);
	res = send(sfd, &u1, sizeof(u1), 0);
	if (res < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	res = recv(sfd, &u1, sizeof(u1), 0);
	if (res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	if(u1.type=='F')
	{
		printf(":>姓名：%s 性别：%c 年龄：%d薪资：%d\n", u1.name,u1.sex,u1.age,u1.salar);
	}else{
		printf(":>%s\n", u1.buf);

	}
}
