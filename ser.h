#ifndef __CLI_H__
#define __CLI_H__

#define N 20
char account[N] = "";
char pwd[N] = "";

typedef struct user{
    char type;
    char account[10];
    char pwd[10];
	char name[10];
    char sex;
    int age;
    char iphone[12];
    int salar;
	char buf[128];

}USER;
typedef struct info{
    char name[10];
    char sex;
    int age;
    char iphone[12];
    int salar;
}INFO;
int ser_register(USER u2,sqlite3* db,int newfd,struct sockaddr_in cin);
int ser_loin(USER u2,sqlite3* db,int newfd);
int ser_quit(USER u2,sqlite3* db,int newfd);
int ser_add(USER u2,sqlite3 *db,int newfd);
int ser_delete(USER u2,sqlite3 *db,int newfd);
int ser_updata(USER u2,sqlite3 *db,int newfd);
int ser_query(USER u2,sqlite3 *db,int newfd);















#endif
