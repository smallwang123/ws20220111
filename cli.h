#ifndef __SER_H__
#define __SER_H__

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

int  user_register(int sfd);
int  loin(int sfd);
int  quit(int sfd);
int  menu();			
int add_staff(int sfd);
int delete_staff(int sfd);
int updata_staff(int sfd);
int query_staff(int sfd);




#endif
