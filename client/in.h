/*=============================================================================
#   >>   文件名: in.h
#   >> 	   描述: 客户端in.h
#   >>     作者: wangbo
#   >>    Email: mawag@live.cn
#   >>	   主页: http://www.cnblogs.com/wangbo2008/
#   >>   Github: github.com/mawag
#   >> 程序版本: 0.0.1
#   >> 创建时间: 2014-08-15 10:35:54
#   >> 修改时间: 2014-08-15 10:35:54
#  Copyright (c) wangbo  All rights reserved.
=============================================================================*/
#ifndef __C_IN_H_____
#define __C_IN_H_____

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <linux/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
//#include <curses.h>
#include "base64.h"

#define C_LOGIN		100
#define C_LOGOUT	101
#define C_REG		200
#define C_GROUP_GET	300
#define C_GROUP_JOIN	301
#define C_GROUP_QUIT	302
#define C_GROUP_CREATE	303
#define C_FRIEND_GET	400
#define C_FRIEND_ADD	401
#define C_FRIEND_DEL	402
#define C_GETINFO_ME	500
#define C_GETINFO_FRI	501
#define C_GETINFO_GROUP 502
#define C_ONLINE	503
#define S_MASSWGE	600
#define C_CHAT_FRI 	601
#define C_CHAT_GROUP 	602
#pragma pack(1)

//设置服务器ip和端口
#define SERVER_WEB "mawag.oicp.net"
#define SERVER_PORT 8888

#define MAX_DATA_LEN 	512	//data长度限制
#define USER_LEN 	15	//用户名长度限制
#define PASSWD_LEN 	15	//密码长度限制

//网络传输协议部分
typedef struct protocol_packet
{
	int	opcode;			//操作码
	int	verify;			//校验位
	int	flag;			//返回位
	char	sendid[USER_LEN];	//发件id
	char	recid[USER_LEN];	//收件id
	char	data[MAX_DATA_LEN];	//数据
}pro_pack;

//时间日期
typedef struct s_timedate
{
	int yy;//年
	int mm;//月
	int dd;//日
	int hh;//时
	int mi;//分
	int ss;//秒
}now_time;

//客户端账户处理信息
typedef struct c_loginuser
{
	char	user[USER_LEN];		//已登录用户id
	int	sock_fd;		//建立好连接的套接字
}c_user;

//全局变量
c_user c_us;


int c_encode(char *buf);			//消息加密
int c_decode(char *buf);			//消息解密
void s_gettime(now_time *gettime);		//获取时间函数
void c_print_time( int type );			//打印时间
void c_conn_disconn(void);			//处理异常断开
pro_pack *c_conn_recv(void *tmp);		//接收数据
int c_conn_send(pro_pack *send_pack);		//发送数据

int c_conn_init(int flag,char *buf);		//连接初始化
int c_main_front(void);				//主页窗体
int c_main_do( void );				//主页窗体处理
int c_reg_front(void);				//注册前端
int c_reg_do (char* user, char* passwd);	//注册处理
int c_login_front(void);			//登陆前端
int c_login_do(char *user,char *passwd);	//登陆处理
int c_index_front(void);			//系统交互主界面

int c_online_list(void);			//在线用户
int s_chat_log(char *buf,int buf_len);		//聊天记录
void c_send_allmag(char *buf,int buf_len);	//发送群聊消息
int c_do_comparse(char *input);			//用户输入解析
void c_send_mag(char *recid,char *buf,int buf_len);//发送聊天消息
void c_logout(void);				//注销
void c_recv_fun(pro_pack *recv);		//接收包处理
void c_help(void);				//帮助


//调试
//#define DEBUG

# endif
