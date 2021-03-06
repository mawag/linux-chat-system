﻿/*=============================================================================
#   >>   文件名: s.c
#   >> 	   描述: 服务端
#   >>     作者: wangbo
#   >>    Email: mawag@live.cn
#   >>	   主页: http://www.cnblogs.com/wangbo2008/
#   >>   Github: github.com/mawag
#   >> 程序版本: 0.0.1
#   >> 创建时间: 2014-08-01 10:27:13
#   >> 修改时间: 2014-08-15 10:36:27
#  Copyright (c) wangbo  All rights reserved.
=============================================================================*/

#include "in.h"

//主函数
int main(int argc,char **argv)
{
	struct sockaddr_in client_addr;
	int sock_fd;//套接字
	int client_len;
	int client_fd;
	pthread_t thid;
	
	system("python ./updateip.py");
	
	head = s_create_l();
	openlog(argv[0],LOG_CONS|LOG_PID,LOG_USER);
	//清空数据
	memset(&client_addr,0,sizeof(struct sockaddr_in));
	//设置ip类型为ipv4
	client_addr.sin_family = AF_INET;
	//接受所有ip
	client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//端口
	client_addr.sin_port = htons (SERVER_PORT);
	//填充0
	memset(client_addr.sin_zero,0,sizeof(client_addr.sin_zero));
	
	//创建套接字
	//IPV4,TCP连接
	sock_fd = socket(AF_INET,SOCK_STREAM,0);
	if(sock_fd == -1)
	{
		syslog(LOG_ERR,"create sock_fd failed. sock_fd:%d\nerror:%s\n",sock_fd,strerror(errno));
		perror("socket");
		exit(1);
	}
	syslog(LOG_INFO,"create sock_fd success.sock_fd:%d\n",sock_fd);
	
	int s_conn_opt = 1;
	socklen_t len=sizeof(s_conn_opt);
	if((setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&s_conn_opt,(socklen_t)sizeof(s_conn_opt)))==-1)
	{
		syslog(LOG_ERR,"set sock_fd failed.sock_fd:%d\nerror:%s\n",sock_fd,strerror(errno));
		perror("SO_REUSEADDR\n");
	}
	syslog(LOG_INFO,"set sock_fd success.sock_fd:%d\n",sock_fd);
	//绑定
	if(bind(sock_fd,(struct sockaddr *)&client_addr,sizeof(struct sockaddr_in)) == -1)
	{
		syslog(LOG_ERR,"bing port failed. sock_fd:%d\nerror:%s\n",sock_fd,strerror(errno));
		perror("bind");
		exit(1);
	}
	syslog(LOG_INFO,"bing port success.sock_fd:%d\n",sock_fd);
	//监听
	if(listen(sock_fd,MAX_ONLINE_LEN) == -1)
	{
		syslog(LOG_ERR,"listen port failed .sock_fd:%d\nerror:%s\n",sock_fd,strerror(errno));
		perror("listen");
		exit(1);
	}
	syslog(LOG_INFO,"listen port success.sock_fd:%d\n",sock_fd);
	
	printf("服务端启动成功，正在监听系统端口:%d\n",SERVER_PORT);
	
	//接受连接，阻塞方式
	while(1)
	{
		client_len = sizeof(struct sockaddr_in);
		client_fd = accept(sock_fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_len);
		if(client_fd < 0)
		{
			syslog(LOG_ERR,"accept error.from ip :%s, sock_fd is %d\nerror:%s\n",inet_ntoa(client_addr.sin_addr),client_fd,strerror(errno));
			perror("accept");
			exit(1);
		}
		syslog(LOG_INFO,"accept a client conn.from ip :%s, sock_fd is %d\n",inet_ntoa(client_addr.sin_addr),client_fd);
		printf("receive a new client,ip:%s\n",inet_ntoa(client_addr.sin_addr));
		if(pthread_create(&thid,NULL,(void *)fun,&client_fd) != 0)
		{
			syslog(LOG_ERR,"create thread failed .\nerror:%s\n",strerror(errno));
			perror("pthread_create");
			exit(1);
		}
		
		
	}
	if(close(sock_fd) == -1)
	{
		syslog(LOG_ERR,"close sock_fd failed .\nerror:%s\n",strerror(errno));
		perror("pthread_create");
		exit(1);
	}
	syslog( LOG_WARNING,"close sock_fd .\nerror:%s\n",strerror(errno));
	return 0;
}

//增加新用户
int s_add_user(char *user,char *passwd)
{
	int fd;
	int flag = 0;
	
	if((fd = open("user.dat",O_WRONLY|O_CREAT|O_APPEND,500)) == -1)
	{
		perror("open");
	}
	if(write(fd,user,USER_LEN) <0)
	{
		perror("write");
		return -1;
	}
	if(write(fd," ",1) < 0)
	{
		perror("write");
		return -1;
	}
	if(write(fd,passwd,PASSWD_LEN) < 0)
	{
		perror("write");
		return -1;
	}
	if(write(fd,"\n",1) < 0)
	{
		perror("write");
		return -1;
	}
	if(close(fd) < 0)
	{
		perror("close");
		return -1;
	}
	return 0;
}

//查询用户
int s_see_suer(char *user,char *passwd)    
{
	int fd;
	int flag =1;
	char ruser[USER_LEN];
	char rpasswd[PASSWD_LEN];
	
	if((fd = open ("user.dat",O_RDONLY)) < 0)
	{
		perror("open");
		return -1;
	}
	while(flag != 0)
	{	
		flag = read(fd,ruser,USER_LEN);
		if(flag < 0)
		{
			perror("read");
			return -1;
		}
		else if(flag == 0)
			break;
		#ifdef DEBUG
		printf("read: %s\n",ruser);
		#endif
		if(strcmp(ruser,user) == 0)
		{
			if(lseek(fd,1,SEEK_CUR) == -1)
			{
				perror("lseek");
				return -1;
			}
			if(read(fd,rpasswd,PASSWD_LEN) < 0)
			{
				perror("read");
				return -1;
			}
			if(strcmp(rpasswd,passwd) != 0)
				return -1;
			else
				return 0;
		}
		else
		if(lseek(fd,2 + PASSWD_LEN,SEEK_CUR) == -1)
		{
			perror("lseek");
			return -1;
		}
	}
	if(close(fd) < 0)
	{
		perror("close");
		return -1;
	}

	return -1;
}

//在线用户查询
void s_online(pro_pack *reavdata)
{
	c_user *p = head->next, *q = NULL;
	char buf[20];
	bzero(reavdata->data,MAX_DATA_LEN);
	strcat(reavdata->data, "online :");
	while(p != NULL)
	{
		{
			sprintf(buf,"%s ",p->user);
			strcat(reavdata->data, buf);
		}
		p = p->next;
	}
	sprintf(buf,"在线%d人\n",c_us_online);
	strcat(reavdata->data, buf);
	reavdata->opcode = C_ONLINE;//操作码
	reavdata->verify = 1;
	reavdata->flag = 0;//返回位
	strncpy(reavdata->recid,reavdata->sendid,USER_LEN);//收件id
	strncpy(reavdata->sendid,"system",USER_LEN);//发件id

	return ;
}

//建立链表
c_user *s_create_l(void)
{
	c_user *head;
	
	head = malloc(sizeof(c_user));
	head->next = NULL;
	
	return head;
}

//插入
void s_ins_l(char *user,int sock_fd)
{
	char buf[50];
	c_user *new;
	
	new = malloc(sizeof(c_user));
	strcpy(new->user,user);
	new->sock_fd = sock_fd;
	new->next = head->next;
	head->next = new;
	c_us_online++;
	sprintf(buf,"～～～%s上线了～～～当前在线人数:%d人\n",user,c_us_online);
	s_msg(buf);
	//return ;
}

//删除
void s_del_nm(char *user)
{
	char buf[50];
	c_user *p = head->next;
	c_user *q = head;
	while (p != NULL)
	{
		if (strcmp(p->user,user) == 0)
			break;
		p = p->next;
		q = q->next;
	}
	if(p == NULL)
		return ;
	q->next = p->next;
	//p->next = NULL;
	free(p);
	c_us_online--;
	sprintf(buf,"～～～%s下线了～～～当前在线人数:%d人\n",user,c_us_online);
	s_msg(buf);
	return ;
}

//删除 by sock_fd
void s_del_fd(int sock_fd)
{
	char buf[512];
	c_user *p = head->next;
	c_user *q = head;
	while (p != NULL)
	{
		if (p->sock_fd ==sock_fd)
			break;
		p = p->next;
		q = q->next;
	}
	if(p == NULL)
		return ;
	c_us_online--;
	sprintf(buf,"～～～%s下线了～～～当前在线人数:%d人\n",p->user,c_us_online);
	q->next = p->next;
	//p->next = NULL;
	free(p);
	
	s_msg(buf);
	return ;
}
//系统包处理
void s_stidy(pro_pack *reavdata)
{
	reavdata->opcode = 0;					//操作码
	reavdata->verify = 0;					//校验位
	reavdata->flag = 1;					//返回位
	strncpy(reavdata->recid,reavdata->sendid,USER_LEN);
	strncpy(reavdata->sendid,"system",USER_LEN);
	memset(reavdata->data,0,sizeof(reavdata->data));	//数据
}

//登陆
int s_login(pro_pack *reavdata)
{
	printf("recv new login,from:%s\n",reavdata->sendid);
	if(s_see_suer(reavdata->sendid,reavdata->data) == 0)
	{
		s_stidy(reavdata);
		reavdata->opcode = C_LOGIN;
		reavdata->verify = 1;
		reavdata->flag = 0;
		return 0;
	}
	s_stidy(reavdata);
	reavdata->opcode = C_LOGIN;
	reavdata->verify = 0;
	reavdata->flag = 1;
	return 1;
}

//注册
int s_reg(pro_pack *reavdata)
{
	printf("new reg,from:%s\n",reavdata->sendid);
	printf("user:%s\n",reavdata->sendid);
	printf("passwd:%s\n",reavdata->data);
	if(s_add_user(reavdata->sendid,reavdata->data) == 0)
	{
		s_stidy(reavdata);
		reavdata->opcode = C_REG;
		reavdata->verify = 0;
		reavdata->flag = 0;
		return 0;
	}
	else
	{
		s_stidy(reavdata);
		reavdata->opcode = C_REG;
		reavdata->verify = 0;
		reavdata->flag = 1;
		return 1;
	}
}

//注销
int s_logout(pro_pack *reavdata)
{	
	return 0;
}

//私聊
int s_chat_fri(pro_pack *reavdata)
{
	printf("chat: (%s_to_%s) msaaage:%s\n",reavdata->sendid,reavdata->recid,reavdata->data);
	c_user *p = head->next;
	int flag = 1;
	reavdata->flag = 0;

	while(p != NULL)
	{
		if (strcmp(p->user, reavdata->recid) == 0)
		{
			reavdata->flag = 0;
			flag = 0;
			#ifdef DEBUG
			printf("_________send___________\n");
			printf("type:%d\n",reavdata->opcode);
			printf("from:%s\n",reavdata->sendid);
			printf("to:%s\n",reavdata->recid);
			printf("verify=%d, flag=%d\n",reavdata->verify,reavdata->flag);
			printf("date:%s\n",reavdata->data);
			printf("________________________\n");
			#endif
			if(send(p->sock_fd,reavdata,sizeof(pro_pack), 0)<0)
				perror("send");
		}
		p = p->next;
	}
	printf("send_end!\n");
	if(flag == 1)
	{
		printf("无法找到联系人！\n");
		reavdata->flag = 2;
	}
	if(strcmp(reavdata->recid,reavdata->sendid) == 0)
	{
		reavdata->flag = 1;
	}
	return flag;
}

//公聊
int s_chat_group(pro_pack *reavdata)
{
	int flag = 1;
	printf("send_char_from %s,massage:%s\n",reavdata->sendid,reavdata->data);
	c_user *p = head->next;
	reavdata->flag = 0;
	
	while(p != NULL)
	{
		#ifdef DEBUG
		printf("_________send___________\n");
		printf("type:%d\n",reavdata->opcode);
		printf("from:%s\n",reavdata->sendid);
		printf("to:%s\n",reavdata->recid);
		printf("verify=%d, flag=%d\n",reavdata->verify,reavdata->flag);
		printf("date:%s\n",reavdata->data);
		printf("________________________\n");
		#endif
		if(send(p->sock_fd,reavdata,sizeof(pro_pack),0)==-1)
		{
			perror("send");
			printf("error_%s信息发送失败!(sock_fd = %d)\n",p->user,p->sock_fd);
		}
		else
			flag = 0;
		p = p->next;
	}
	printf("send_end!\n");
	reavdata->flag = 1;
	memset(&reavdata->data,0,sizeof(reavdata->data));
	return flag;
}

//系统通告
void s_msg(char *massage)
{
	printf("全网通告...start\n");
	printf("massage:%s\n",massage);
	c_user *p = head->next;
	pro_pack s_temp;
	s_temp.opcode = S_MASSWGE;				//操作码
	s_temp.verify = 1;					//校验位
	s_temp.flag = 0;					//返回位
	strncpy(s_temp.sendid,"system",USER_LEN);		//发件id
	strncpy(s_temp.data,massage,MAX_DATA_LEN);		//数据
	
	while(p != NULL)
	{
		strncpy(s_temp.recid,p->user,USER_LEN);	//发件id
		#ifdef DEBUG
		printf("_________send__________\n");
		printf("type:%d\n",s_temp.opcode);
		printf("from:%s\n",s_temp.sendid);
		printf("to:%s\n",s_temp.recid);
		printf("verify=%d, flag=%d\n",s_temp.verify,s_temp.flag);
		printf("date:%s\n",s_temp.data);
		printf("_______________________\n");
		#endif
		if (send(p->sock_fd,&s_temp,sizeof(pro_pack),0)==-1)
		{
			perror("send");
			printf("error_全网通告_%s信息发送失败!(sock_fd = %d)\n",p->user,p->sock_fd);
		}
		p = p->next;
	}
	printf("全网通告...end!\n");
	
	return ;
	
}

//处理接受到的包
int s_comparse(pro_pack *reavdata)
{
	if(reavdata->opcode == C_LOGIN)
	{
		//登陆包
		return s_login(reavdata);
	}
	else if(reavdata->opcode == C_REG)
	{
		//注册包
		return s_reg(reavdata);
	}
	else if(reavdata->opcode == C_LOGOUT)
	{
		//注销包
		return s_logout(reavdata);
	}
	else if(reavdata->opcode == C_GROUP_GET)
	{
		//获取已加入的群
		;
	}
	else if(reavdata->opcode == C_GROUP_JOIN)
	{
		//加群
		;
	}
	else if(reavdata->opcode == C_GROUP_QUIT)
	{
		//退出群
		;
	}
	else if(reavdata->opcode == C_GROUP_CREATE)
	{
		//创建群
		;
	}
	else if(reavdata->opcode == C_FRIEND_GET)
	{
		//获取好友
		;
	}
	else if(reavdata->opcode == C_FRIEND_ADD)
	{
		//加好友
		;
	}
	else if(reavdata->opcode == C_FRIEND_DEL)
	{
		//删除好友
		;
	}
	else if(reavdata->opcode == C_GETINFO_ME)
	{
		//设置我的资料
		;
	}
	else if(reavdata->opcode == C_GETINFO_FRI)
	{
		//查看好友资料
		;
	}
	else if(reavdata->opcode == C_GETINFO_GROUP)
	{
		//查看群资料
		;
	}
	else if(reavdata->opcode == C_ONLINE)
	{
		//查看在线好友
		s_online(reavdata);
	}
	else if(reavdata->opcode == C_CHAT_FRI)
	{
		//好友聊天信息
		return s_chat_fri(reavdata);
	}
	else if(reavdata->opcode == C_CHAT_GROUP)
	{
		//群聊天信息
		return s_chat_group(reavdata);
	}
	else
	{
		return -1;//返回-1，无效包丢弃
	}

	return 0;
}

//单线程处理用户数据
void fun(int* client_fd)
{
	pro_pack recv_data;
	int flag;
	int tmp_client_fd = *client_fd;
	int i;

	syslog(LOG_INFO,"create thread success.\n");
	int s_conn_opt = 1;
	socklen_t len=sizeof(s_conn_opt);
	if((setsockopt(tmp_client_fd,SOL_SOCKET,SO_KEEPALIVE,(char*)&s_conn_opt,(socklen_t)sizeof(s_conn_opt)))==-1)
	{
		syslog(LOG_ERR,"set (%s) sock_fd failed.client_fd:%d\nerror:%s\n","SO_KEEPALIVE",tmp_client_fd,strerror(errno));
		perror("SO_KEEPALIVE\n");
	}
	syslog(LOG_INFO,"set (%s) sock_fd success.client_fd:%d\n","SO_KEEPALIVE",tmp_client_fd);

	
	//处理请求
	while(1)
	{
		memset(&recv_data,0,sizeof(recv_data));
		//接收消息
		flag = recv(tmp_client_fd,&recv_data,sizeof(pro_pack),0);
		if(flag == -1)
		{
			syslog(LOG_ERR,"recv date failed.\n");
			perror("recv");
			exit(1);
		}
		else if(flag == 0)/*客户端断开连接*/
			break;
		syslog(LOG_INFO,"recv date.from :%s\n",recv_data.sendid);
		#ifdef DEBUG
		printf("_________recv___________\n");
		printf("type:%d\n",recv_data.opcode);
		printf("from:%s\n",recv_data.sendid);
		printf("to:%s\n",recv_data.recid);
		printf("verify=%d, flag=%d\n",recv_data.verify,recv_data.flag);
		printf("date:%s\n",recv_data.data);
		printf("________________________\n");
		#endif
		/*消息处理*/
		if(recv_data.flag != 0)
		{
			flag = s_comparse(&recv_data);
			if(flag == -1)
				continue;
			if(recv_data.flag == 0&&recv_data.opcode == C_LOGIN)
			{
				s_ins_l(recv_data.recid,tmp_client_fd);
			}
			
			#ifdef DEBUG
			printf("_________send__________\n");
 			printf("type:%d\n",recv_data.opcode);
			printf("from:%s\n",recv_data.sendid);
			printf("to:%s\n",recv_data.recid);
			printf("verify=%d, flag=%d\n",recv_data.verify,recv_data.flag);
			printf("date:%s\n",recv_data.data);
			printf("_______________________\n");
			#endif
			if (send(tmp_client_fd,&recv_data,sizeof(pro_pack),0)==-1)
			{
				syslog(LOG_ERR,"send date to %s failed.error:%s\n",recv_data.recid,strerror(errno));
				perror("发送通知信息失败!\n");
			}
		}
	}
	s_del_fd(tmp_client_fd);
	close(tmp_client_fd);
	printf("有客户端断开连接,现有%d个客户端在线。\n",c_us_online);
	syslog(LOG_WARNING,"Disconnect. from :%d\n",tmp_client_fd);
	return ;
}


