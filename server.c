#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "ser.h"

#define ERR_MSG(msg) do{\
	fprintf(stderr, "__%d__ ", __LINE__);\
	perror(msg);\
}while(0)

#define PORT 8888
typedef void (*sighandler_t)(int);

USER u2;

int rcv_cli_info(int , struct sockaddr_in );
int do_insert(sqlite3*);
int do_select(sqlite3 *db);
int flag =0;


//用信号的方式回收僵尸进程
void handler(int sig)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}


int main(int argc, const char *argv[])
{
	//捕获17号信号 SIGCHLD
	sighandler_t s = signal(SIGCHLD, handler);
	if(SIG_ERR == s)
	{
		ERR_MSG("signal");
		return -1;
	}


	//创建流式套接字
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0)
	{
		ERR_MSG("socket");
		return -1;
	}

	//允许端口快速重用
	int reuse = 1;
	if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		ERR_MSG("setsockopt");
		return -1;
	}


	//填充地址信息结构体
	struct sockaddr_in sin; 			//man 7 ip
	sin.sin_family 		= AF_INET;
	sin.sin_port 		= htons(PORT); 	//端口号的网络字节序，1024~49151
	sin.sin_addr.s_addr = inet_addr("10.102.132.26"); 	//本机IP地址 终端输入ifconfig查找

	//绑定服务器的IP地址和端口号 bind
	if(bind(sfd, (struct sockaddr*)&sin, sizeof(sin))< 0)
	{
		ERR_MSG("bind");
		return -1;
	}
	printf("bind success\n");

	//将套接字设置为被动监听状态
	if(listen(sfd, 10) < 0)
	{
		ERR_MSG("listen");
		return -1;
	}
	printf("listen success\n");

	struct sockaddr_in cin;
	socklen_t addrlen = sizeof(cin);

	int newfd = 0;
	pid_t pid;
	//打开数据库
	sqlite3* db = NULL;
	if(sqlite3_open("./my.db", &db) != SQLITE_OK)
	{
		printf("sqlite3_open failed\n");
		printf("%d\n", sqlite3_errcode(db));
		fprintf(stderr, "__%d__ sqlite3_open: %s\n", __LINE__,sqlite3_errmsg(db));
		return -1;
	}

	printf("sqlite3_open success\n");

	//创建一张表
	char sql[128] = "create table if not exists info(name char ,sex char ,age int,iphone char primary key,salar int)";
	char* errmsg = NULL;
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return -1;
	}
	printf("table stu 初始化完成\n");
	//创建2张表
	char sql2[128] = "create table if not exists user(account char primary key,pwd char,stage)";
	char* errmsg2 = NULL;
	if(sqlite3_exec(db, sql2, NULL, NULL, &errmsg) != 0)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return -1;
	}
	printf("table user 初始化完成\n");

	int choose = 1;

	while(1)
	{
		//父进程运行
		//获取新的文件描述符
		newfd = accept(sfd, (struct sockaddr*)&cin, &addrlen);
		if(newfd < 0)
		{
			ERR_MSG("accept");
			return -1;
		}
		printf("[%s | %d]newfd = %d 连接成功\n", \
				inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);

		//能运行到当前步骤，则代表有客户端连接成功
		//则需要创建一个子进程，与连接成功的客户端进行交互。

		pid = fork();
		if(0 == pid)
		{
			close(sfd);
			//子进程运行

			ssize_t res = 0;
			while(1)
			{

				//接收数据
				res = recv(newfd, &u2, sizeof(u2), 0);
				if(res < 0)
				{
					ERR_MSG("recv");
					return -1;
				}
				else if(0 == res)
				{
					printf("[%s | %d]newfd = %d 客户端退出\n", \
							inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
					break;
				}
				printf("%s,%s\n",u2.account,u2.pwd);
				switch(u2.type)
				{
				case 'R':
					ser_register(u2,db,newfd,cin);
					break;
				case 'L':
					ser_loin(u2,db,newfd);
					break;
				case 'S':
					ser_quit(u2,db,newfd);
					break;
				case 'A':
					ser_add(u2,db,newfd);
					break;
				case 'D':
					ser_delete(u2,db,newfd);
					break;
				case 'U':
					ser_updata(u2,db,newfd);
					break;
				case 'Q':
					ser_query(u2,db,newfd);
					break;
				}
			}
			close(newfd);
			//由于子进程只负责交互，当交互函数结束后，必须要将子进程退出.
			exit(0);
		}

		close(newfd);
	}


	close(sfd);
	sqlite3_close(db);
	return 0;
}


//注册
int ser_register(USER u2,sqlite3* db,int newfd,struct sockaddr_in cin)
{
	char buf[128]="";
	char* errmsg = NULL;
	char sql[128] = "";
	int row, column, k, stage = 0, flag = 0;
	char** result 	 = NULL;

	strcpy(account, u2.account);
	strcpy(pwd, u2.pwd);


	//如果flag 为0 则说明数据库中没有相同的姓名 则注册一个账号
	bzero(sql, sizeof(sql));
	sprintf(sql, "INSERT INTO user values(\"%s\", \"%s\", %d)", account, pwd, stage);
	if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
	{
		printf("ERROR: %s\n __%d__", errmsg, __LINE__);
		bzero(buf, sizeof(buf));
		sprintf(buf, "该账号以注册!");
		send(newfd, buf, sizeof(buf), 0);


		return -1;
	}

	bzero(buf, sizeof(buf));
	sprintf(buf, "注册成功!");
	send(newfd, buf, sizeof(buf), 0);

	//释放空间
	return 0;
}
//功能：登陆
int ser_loin(USER u2,sqlite3* db,int newfd)
{
	//初始化变量
	char buf[1024] 	 = "";
	char sql[1024] 	 = "select * from user";
	//char account[N] = "";
	//char pwd[N] = "";
	char** result 	 = NULL;
	char* errmsg 	 = NULL;
	char c = '0';
	int row, column, k, stage, flag = 0;

	if(sqlite3_get_table(db, sql, &result, &row, &column, &errmsg)  != 0)
	{
		printf("ERROR:%s __%d__\n", errmsg, __LINE__);
		return -1;
	}

	strcpy(account, u2.account);
	strcpy(pwd, u2.pwd);

	for (k = 3; k <= row*column; k+=3)
	{
		if ((strcmp(result[k], account) == 0) && (strcmp(result[k+1], pwd) == 0))
		{
			flag = 1;
			if (strncmp(result[k+2], &c, 1) == 0)
			{		
				stage = 1;
				printf("%c***\n",c);
				bzero(sql, sizeof(sql));
				sprintf(sql, "update user SET stage=%d where account=\"%s\";", stage, account);
				if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
				{
					printf("ERROR: %s\n __%d__", errmsg, __LINE__);
					return -1;
				}

				printf("%c***\n",c);
				bzero(buf, sizeof(buf));
				sprintf(buf, "登陆成功!");
				send(newfd, buf, sizeof(buf), 0);
				break;
			}
			else
			{
				bzero(buf, sizeof(buf));
				sprintf(buf, "该账号以登陆,请勿重复登陆!");
				send(newfd, buf, sizeof(buf), 0);
				break;
			}
		}
	}
	if (flag == 0)
	{
		bzero(buf, sizeof(buf));
		sprintf(buf, "账号或密码错误,请重新登陆!");
		send(newfd, buf, sizeof(buf), 0);
	}

	//释放空间
	sqlite3_free_table(result);

}

int ser_quit(USER u2,sqlite3* db,int newfd)
{
	//初始化变量
	char buf[1024] 	 = "";
	char sql[1024] 	 = "select * from user;";
	char** result 	 = NULL;
	char* errmsg 	 = NULL;
	char c = '1';
	int row, column, k, stage, flag = 0;

	if(sqlite3_get_table(db, sql, &result, &row, &column, &errmsg)  != 0)
	{
		printf("ERROR:%s __%d__\n", errmsg, __LINE__);
		return -1;
	}
	for (k = 3; k <= row*column; k+=3)
	{

		if ((strcmp(result[k], account) == 0) && (strcmp(result[k+1],pwd) == 0))
		{

			printf("result[k+2]=%s\n",result[k+2]);
			if (strncmp(result[k+2], &c, 1) == 0)
			{		
				stage = 0;
				bzero(sql, sizeof(sql));
				sprintf(sql, "update user set stage=%d where account=\"%s\";", stage, account);
				if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
				{
					printf("ERROR: %s\n __%d__\n", errmsg, __LINE__);
					return -1;
				}
				bzero(buf, sizeof(buf));
				sprintf(buf, "退出成功*");
				if((send(newfd, buf, sizeof(buf), 0))<0)
				{
					ERR_MSG("send");
					return -1;

				}
				sqlite3_free_table(result);

				return 0;

			}

		}
	}
	sprintf(buf, "退出成功**");
		if((send(newfd, buf, sizeof(buf), 0))<0)
		{
			ERR_MSG("send");
			return -1;
		}
	sqlite3_free_table(result);
	return 0;

}
int ser_add(USER s1,sqlite3 *db,int newfd)
{
	//初始化变量
	char buf[1024] 	 = "";
	char sql[1024] 	 = "select * from info;";
	char iphone[N] 	 = "";
	char** result 	 = NULL;
	char* errmsg 	 = NULL;

	int row, column, k, stage, flag = 0;

	if(sqlite3_get_table(db, sql, &result, &row, &column, &errmsg)  != 0)
	{
		printf("ERROR:%s __%d__\n", errmsg, __LINE__);
		return -1;
	}



	strcpy(iphone,u2.iphone);

	printf("%d  %d\n",column,row);


	bzero(sql, sizeof(sql));
	sprintf(sql, "insert into info values(\"%s\",'%c',%d,\"%s\",%d);", u2.name,u2.sex,u2.age,u2.iphone,u2.salar);
	if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
	{
		printf("ERROR: %s\n __%d__\n", errmsg, __LINE__);
		bzero(buf, sizeof(buf));
		sprintf(buf, "添加失败");
		if((send(newfd, buf, sizeof(buf), 0))<0)
		{
			ERR_MSG("send");
			return -1;

		}
		sqlite3_free_table(result);
		return -1;
	}
	bzero(buf, sizeof(buf));
	sprintf(buf, "添加成功");
	if((send(newfd, buf, sizeof(buf), 0))<0)
	{
		ERR_MSG("send");
		return -1;

	}



	sqlite3_free_table(result);
	return 0;

}
int ser_delete(USER u1,sqlite3 *db,int newfd)
{
	//初始化变量
	char buf[1024] 	 = "";
	char sql[1024] 	 = "select * from info;";
	char iphone[N] 	 = "";
	char** result 	 = NULL;
	char* errmsg 	 = NULL;

	int row, column, k, stage, flag = 0;

	if(sqlite3_get_table(db, sql, &result, &row, &column, &errmsg)  != 0)
	{
		printf("ERROR:%s __%d__\n", errmsg, __LINE__);
		return -1;
	}



	strcpy(iphone,u2.iphone);

	printf("%d  %d\n",column,row);
	for (k = 5; k <= row*column; k+=5)
	{
		if ((strcmp(result[k+3], iphone) == 0))
		{
			bzero(sql, sizeof(sql));
			sprintf(sql, "delete from info where iphone=\"%s\";",u2.iphone);
			if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
			{
				printf("ERROR: %s\n __%d__", errmsg, __LINE__);

			}
			bzero(buf, sizeof(buf));
			sprintf(buf, "删除成功");
			if((send(newfd, buf, sizeof(buf), 0))<0)
			{
				ERR_MSG("send");
				return -1;
			}	
			sqlite3_free_table(result);
			return 0;

		}

	}
	bzero(buf, sizeof(buf));
	sprintf(buf, "没有该员工");
	if((send(newfd, buf, sizeof(buf), 0))<0)
	{
		ERR_MSG("send");
		return -1;

	}
	sqlite3_free_table(result);

	return 0;

}
int ser_updata(USER u1,sqlite3 *db,int newfd)
{
	//初始化变量
	char buf[1024] 	 = "";
	char sql[1024] 	 = "select * from info;";
	char iphone[N] 	 = "";
	char** result 	 = NULL;
	char* errmsg 	 = NULL;

	int row, column, k, stage, flag = 0;

	if(sqlite3_get_table(db, sql, &result, &row, &column, &errmsg)  != 0)
	{
		printf("ERROR:%s __%d__\n", errmsg, __LINE__);
		return -1;
	}



	strcpy(iphone,u2.iphone);

	printf("%d  %d\n",column,row);
	for (k = 5; k <= row*column; k+=5)
	{
		if ((strcmp(result[k+3], iphone) == 0))
		{
			bzero(sql, sizeof(sql));
			sprintf(sql, " update info set name=\"%s\",sex='%c',age=%d,salar=%d where iphone=\"%s\";",u2.name,u2.sex,u2.age,u2.salar,u2.iphone);
			if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
			{
				printf("ERROR: %s\n __%d__\n", errmsg, __LINE__);

			}
			bzero(buf, sizeof(buf));
			sprintf(buf, "修改成功");
			if((send(newfd, buf, sizeof(buf), 0))<0)
			{
				ERR_MSG("send");
				return -1;
			}
			sqlite3_free_table(result);
			return 0;
		}

	}
	bzero(buf, sizeof(buf));
	sprintf(buf, "没有该员工");
	if((send(newfd, buf, sizeof(buf), 0))<0)
	{
		ERR_MSG("send");
		return -1;

	}
	sqlite3_free_table(result);

	return 0;

}
int ser_query(USER u1,sqlite3 *db,int newfd)
{
	//初始化变量
	char buf[1024] 	 = "";
	char sql[1024] 	 = "select * from info;";
	char iphone[N] 	 = "";
	char** result 	 = NULL;
	char* errmsg 	 = NULL;

	int row, column, k, stage, flag = 0;

	if(sqlite3_get_table(db, sql, &result, &row, &column, &errmsg)  != 0)
	{
		printf("ERROR:%s __%d__\n", errmsg, __LINE__);
		return -1;
	}



	strcpy(iphone,u2.iphone);

	printf("%d  %d\n",column,row);
	for (k = 5; k <= row*column; k+=5)
	{	printf("result[k]=%s\n",result[k]);
		if ((strcmp(result[k+3], iphone) == 0))
		{
			strcpy(u2.name,result[k]);

			u2.sex=*result[k+1];
			u2.age=atoi(result[k+2]);
			u2.salar=atoi(result[k+4]);
			u2.type = 'F';
			if((send(newfd, &u2, sizeof(u2), 0))<0)
			{
				ERR_MSG("send");
				return -1;
			}
			sqlite3_free_table(result);

			return 0;
		}

	}	
	bzero(u2.buf, sizeof(u2.buf));
	sprintf(u2.buf, "没有该员工");
	if((send(newfd, &u2, sizeof(u2), 0))<0)
	{
		ERR_MSG("send");
		return -1;

	}


	sqlite3_free_table(result);

	return 0;
}


