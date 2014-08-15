/*=============================================================================
#   >>   文件名: in.h
#   >> 	   描述: 服务端in.h
#   >>     作者: wangbo
#   >>    Email: mawag@live.cn
#   >>	   主页: http://www.cnblogs.com/wangbo2008/
#   >>   Github: github.com/mawag
#   >> 程序版本: 0.0.1
#   >> 创建时间: 2014-08-15 10:36:41
#   >> 修改时间: 2014-08-15 10:36:41
#  Copyright (c) wangbo  All rights reserved.
=============================================================================*/
#ifndef __S_IN_H_____
#define __S_IN_H_____
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
#include <syslog.h>
#include "base64.h"
#pragma pack(1)

#define C_LOGIN		100
#define C_LOGOUT		101
#define C_REG			200
#define C_GROUP_GET		300
#define C_GROUP_JOIN		301
#define C_GROUP_QUIT		302
#define C_GROUP_CREATE	303
#define C_FRIEND_GET		400
#define C_FRIEND_ADD		401
#define C_FRIEND_DEL		402
#define C_GETINFO_ME		500
#define C_GETINFO_FRI		501
#define C_GETINFO_GROUP 	502
#define C_ONLINE		503
#define S_MASSWGE		600
#define C_CHAT_FRI 		601
#define C_CHAT_GROUP 	602

//设置服务器ip和端口
#define SERVER_IP 		"127.0.0.1"
#define SERVER_WEB 		"localhost"
#define SERVER_PORT 		8888


#define MAX_ONLINE_LEN 	50	//在线人数限制
#define MAX_DATA_LEN 	512	//data长度限制
#define USER_LEN 	15	//用户名长度限制
#define PASSWD_LEN 	15	//密码长度限制

//网络传输协议部分
typedef struct protocol_packet
{
	int	opcode;		//操作码
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

//在线账户处理信息
typedef struct c_loginuser
{
	char	user[USER_LEN];	//已登录用户id
	int	sock_fd;		//建立好连接的套接字
	struct c_loginuser *next;	//单链表
}c_user;

//在线用户全局变量
int 	c_us_online = 0;
c_user *head;

c_user 	*s_create_l(void);			//建立链表
void 	s_ins_l(char* user, int sock_fd);	//插入
void 	s_del_nm(char* user );			//删除
void 	s_del_fd(int sock_fd);			//删除根据sock_fd
int 	s_login(pro_pack* reavdata);		//登陆
int 	s_reg(pro_pack *reavdata);		//注册
int	s_logout(pro_pack* reavdata);		//注销
int 	s_chat_fri(pro_pack *reavdata);		//私聊
int 	s_chat_group(pro_pack *reavdata);	//公聊
void 	s_online(pro_pack* reavdata);		//在线
void 	s_msg(char* massage);			//系统通告
void 	s_stidy(pro_pack* reavdata);		//系统包处理
int 	s_comparse(pro_pack *reavdata);		//解析包内容
void 	fun(int *client_fd);			//收包发包
int 	s_add_user(char *user,char *passwd);	//增加新用户
int 	s_see_user(char *user,char *passwd);	//查询用户

//调试
//#define DEBUG


#endif
