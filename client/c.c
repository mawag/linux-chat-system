/*=============================================================================
#   >>   文件名: c.c
#   >> 	   描述: 客户端
#   >>     作者: wangbo
#   >>    Email: mawag@live.cn
#   >>	   主页: http://www.cnblogs.com/wangbo2008/
#   >>   Github: github.com/mawag
#   >> 程序版本: 0.0.1
#   >> 创建时间: 2014-08-01 09:59:36
#   >> 修改时间: 2014-08-15 10:35:44
#  Copyright (c) wangbo  All rights reserved.
=============================================================================*/
#include "in.h"

//本地聊天记录
int s_chat_log(char* buf, int buf_len)
{
	now_time nowtime;
	int fd;
	int flag = 0;
	char date[10];
	if((fd = open("chat.log",O_WRONLY|O_CREAT|O_APPEND,436)) == -1)
	{
		perror("open");
	}
	s_gettime(&nowtime);
	sprintf(date,"%02d:%02d:%02d: ",nowtime.hh,nowtime.mi,nowtime.ss);

	if(write(fd,date,sizeof(date)) < 0)
	{
		perror("write");
		return -1;
	}
	if(write(fd,buf,buf_len) <0)
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

//主函数
int main(int argc,char **argv)
{
	pthread_t thid_recv;//副线程接收系统数据
	pro_pack c_send_date;
	if(argc == 2)
	{
		c_conn_init(argc,argv[1]);
	}
	else
		c_conn_init(argc,NULL);//初始化
	c_main_do();//调用主窗体
	
	if(pthread_create(&thid_recv,NULL,(void *)c_conn_recv,NULL) != 0)
	{
		perror("线程创建失败！");
		exit(-1);
	}
	c_index_front();

	return 0;	
}

//在线
int c_online_list ( void )
{
	pro_pack c_temp;
	c_temp.opcode = C_ONLINE;				//操作码
	c_temp.verify = 1;					//校验位
	c_temp.flag = -1;					//返回位
	strncpy(c_temp.sendid,c_us.user,USER_LEN);		//发件id
	strncpy(c_temp.recid,"\0",USER_LEN);			//收件id
	strncpy(c_temp.data,"\0",MAX_DATA_LEN);			//数据
	
	c_conn_send(&c_temp);
	return 0;
}

//主页窗体
int c_main_front(void)
{
	printf("\t\t主页面前端模块\n\t\t");
	c_print_time(1);
	printf("\n\t\t1,登陆 2，注册 3，退出\n");
	return 0;
}
//主页窗体处理
int c_main_do(void)
{
	int i;
	while(1)
	{
		c_main_front();
		scanf("%d",&i);
		getchar();
		switch(i)
		{
			case 1:
				if(c_login_front() == 0)
				{
					char buf[50];
					sprintf(buf,">>>>>>  %s login success.",c_us.user);
					s_chat_log(buf,strlen(buf));
					return 0;
				}
				break;
			case 2:
				c_reg_front();
				break;
			case 3:
				c_conn_disconn();
				exit(0);
		}
	}
}

//注册前端
int c_reg_front(void)
{
	printf("\t\t注册前端\n");
	char user[USER_LEN];
	char fpasswd[PASSWD_LEN];
	char spasswd[PASSWD_LEN];
	int i;
	
	do
	{
		printf("\t\t用户名:");
		fflush(stdout);
		fgets(user,USER_LEN,stdin);
		user[strlen(user)-1]='\0';
		if(strcmp(user,"\0") != 0)
			break;
		printf("\t\t账户名不能为空!\n");
	}while(1);
	do
	{
		strncpy(fpasswd,getpass("\t\t请输入新密码:"),PASSWD_LEN);
		strncpy(spasswd,getpass("\t\t请在次输入新密码:"),PASSWD_LEN);
		if(strcmp(fpasswd,spasswd) == 0)
			break;
		printf("\t\t您两次输入的密码不一致，请重新输入!\n");
	}while(1);
	#ifdef DEBUG
	printf("user:%s--passwd1:%s--passwd2:%s\n",user,fpasswd,spasswd);
	#endif
	printf("\t\t正在申请账户");
	sleep(1);
	printf(".");
	fflush(stdout);
	sleep(1);
	printf(".");
	fflush(stdout);
	sleep(1);
	printf(".\n");
	if(c_reg_do(user,fpasswd) == 0)
	{
		printf("\t\t注册成功！\n");
		return 0;
	}
	printf("\t\t注册失败！\n");
	return 1;
}

//注册处理
int c_reg_do ( char* user, char* passwd )
{
	pro_pack c_login_data;
	int flag=1;
	
	//密码加密
	c_encode(passwd);
	
	//数据初始化
	c_login_data.opcode 	= C_REG;			//操作码
	c_login_data.verify 	= 0;				//校验位
	c_login_data.flag 	= 1;				//返回位
	strncpy(c_login_data.sendid,user,USER_LEN);		//发件id
	strncpy(c_login_data.recid,"\0",USER_LEN);		//收件id
	strncpy(c_login_data.data,passwd,PASSWD_LEN);		//数据
	
	c_conn_send(&c_login_data);
	c_conn_recv(&flag);
	
	return flag;
}

//登陆前端
int c_login_front(void)
{
	char user[USER_LEN];
	char passwd[PASSWD_LEN];
	char choose;
	int i;
	
	printf("\t\t登陆前端\n");
	for(i=0;i<3;i++)
	{
		printf("\t\t用户名:");
		fgets(user,USER_LEN,stdin);
		user[strlen(user)-1]='\0';
		strncpy(passwd,getpass("\t\t密码:"),PASSWD_LEN);
		#ifdef DEBUG
		printf("user:%s,passwd:%s\n",user,passwd);
		#endif
		if(c_login_do(user,passwd) == 0)
			return 0;
		else
		{
			printf("\t\t密码错误，请重试\n");
		}
		
	}
	printf("\t\t密码次数超过3次，很抱歉！\n");
	c_conn_disconn();
	exit(0);
}

//登陆处理
int c_login_do(char *user,char *passwd)
{
	pro_pack c_login_data;
	int flag;
	char buf[120];
	
	//密码加密
	c_encode(passwd);
	
	//数据初始化
	c_login_data.opcode 	= C_LOGIN;			//操作码
	c_login_data.verify 	= 0;				//校验位
	c_login_data.flag 	= 1;				//返回位
	strncpy(c_login_data.sendid,user,USER_LEN);		//发件id
	strncpy(c_login_data.recid,"\0",USER_LEN);		//收件id
	strncpy(c_login_data.data,passwd,PASSWD_LEN);		//数据
	
	c_conn_send(&c_login_data);
	c_conn_recv(&flag);
	if(flag == 0)
	{
		strncpy(c_us.user,c_login_data.sendid,USER_LEN);
		sprintf(buf,"~~~~~~~~~~~~logon~~~~~~~~~~~~\n%s login success!",c_us.user);
		s_chat_log(buf,strlen(buf));
		return 0;
	}
	return 1;
}

//系统交互主界面
int c_index_front(void)
{
	char inputbuf[512];

	printf("\t\t系统交互主界面\n");
	system("cat ../logo");
	printf("\t\t~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	c_print_time(1);
	printf("\t\t系统支持以下命令:\n");
	printf("\t\t私聊，格式为\"@name massage\"，公聊，直接发送消息即可\n");
	printf("\t\t查看在线用户/list，注销/logout，退出/exit,帮助/help\n");
	printf("\t\t~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	while(1)
	{
		//用户输入
		usleep(10000);
		printf(" %s:",c_us.user);
		fflush(stdout);
		fgets(inputbuf,512,stdin);
		inputbuf[strlen(inputbuf)-1]='\0';
		#ifdef DEBUG
		printf("msg == %s\n",inputbuf);
		#endif
		if(strcmp(inputbuf,"\0") != 0)
			c_do_comparse(inputbuf);
	}
}

//用户输入解析
int c_do_comparse(char *input)
{
	char buf[512];
	char comorfri[20];
	int i,j;
	#ifdef DEBUG
	printf("input = %s\n",input);
	#endif
	if(input[0] == '/')
	{
		//命令解析
		for(i=1,j=0;(input[i] != ' '&& input[i] != '\0');i++,j++)
			comorfri[j] = input[i];
		comorfri[j] = '\0';
		for(j=0;*(input+i) != '\0';i++,j++)
			buf[j] = input[i];
		#ifdef DEBUG
		printf("buf1=%s,buf2=%s\n",comorfri,buf);
		#endif
		if(strcmp(comorfri,"help") == 0)//帮助
			c_help();
		else if(strcmp(comorfri,"join") == 0)//加群
			;
		else if(strcmp(comorfri,"part") == 0)//退群
			;
		else if(strcmp(comorfri,"add") == 0)//加好友
			;
		else if(strcmp(comorfri,"del") == 0)//删好友
			;
		else if(strcmp(comorfri,"list") == 0)//查看在线列表
			c_online_list();
		else if(strcmp(comorfri,"logout") == 0)//注销
			c_logout();
		else if(strcmp(comorfri,"exit") == 0)//退出
			exit(0);
		else if(strcmp(comorfri,"quit") == 0)//退出
			exit(0);
		else if(strcmp(comorfri,"whois") == 0)//获取好友信息
			;
		else if(strcmp(comorfri,"send") == 0)//发文件
			;
		else
			printf("命令未找到!\n");
	}
	else if(input[0] == '@')
	{
		//特定聊天对象聊天
		for(i=1,j=0;(input[i] != ' '&& input[i] != '\0');i++,j++)
			comorfri[j] = input[i];
		comorfri[j] = '\0';
		for(j=0;input[i] != '\0';i++,j++)
			buf[j] = input[i];
		buf[j]='\0';
		#ifdef DEBUG
		printf("buf1=%s,buf2=%s\n",comorfri,buf);
		#endif
		c_send_mag(comorfri,buf,strlen(buf)+1);
	}
	else
	{
		//群聊
		c_send_allmag(input,strlen(input)+1);
	}
	
	return 0;
}

//发送公聊消息
void c_send_allmag(char *buf,int buf_len)
{
	pro_pack c_temp;
	c_temp.opcode = C_CHAT_GROUP;				//操作码
	c_temp.verify = 1;					//校验位
	c_temp.flag = 1;					//返回位
	strncpy(c_temp.sendid,c_us.user,USER_LEN);		//发件id
	strncpy(c_temp.recid,"group",USER_LEN);			//收件id
	strncpy(c_temp.data,buf,buf_len);			//数据
	
	c_conn_send(&c_temp);
}

//发送聊天消息
void c_send_mag(char *recid,char *buf,int buf_len)
{
	pro_pack c_temp;
	c_temp.opcode = C_CHAT_FRI;				//操作码
	c_temp.verify = 1;					//校验位
	c_temp.flag = 1;					//返回位
	strncpy(c_temp.sendid,c_us.user,USER_LEN);		//发件id
	strncpy(c_temp.recid,recid,USER_LEN);			//收件id
	strncpy(c_temp.data,buf,buf_len);			//数据
	
	c_conn_send(&c_temp);
}

//消息加密
int c_encode(char *buf)
{
	base64_encode(buf);
	return 0;
}

//消息解密
int c_decode(char *buf)
{
	//base64_decode(buf);
	return 0;
}

//获取时间函数
void s_gettime( now_time* gettime )
{
	time_t now;
	struct tm *timenow;
	now = time(NULL);//获取时间
	timenow=localtime(&now);//本地时间

	gettime->yy=timenow->tm_year+1900;
	gettime->mm=timenow->tm_mon+1;
	gettime->dd=timenow->tm_mday;
	gettime->hh=timenow->tm_hour;
	gettime->mi=timenow->tm_min;
	gettime->ss=timenow->tm_sec;
}

//打印时间
void c_print_time(int type)
{
	now_time nowtime;
	s_gettime(&nowtime);
	if(type == 0)
	printf("%02d:%02d:%02d ",nowtime.hh,nowtime.mi,nowtime.ss);
	else if(type == 1)
		printf("%04d年%02d月%02d日 %02d:%02d:%02d",nowtime.yy,nowtime.mm,nowtime.dd,nowtime.hh,nowtime.mi,nowtime.ss);
	
}

//获取服务器ip地址
int gethostip(char *hostip)
{
	struct hostent *h;
	if((h=gethostbyname(SERVER_WEB))==NULL)
	{
		printf("主动获取ip失败,");
		return -1;
	}
	printf("IP Address :%s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));
	strcpy(hostip,inet_ntoa(*((struct in_addr *)h->h_addr)));
	printf("%s\n",hostip);
	return 0;
}

//连接初始化
int c_conn_init(int flag,char *buf)
{
	struct sockaddr_in sock;
	char *hostip;
	//清空数据
	memset(&sock,0,sizeof(struct sockaddr_in));
	//设置ip类型为ipv4
	sock.sin_family = AF_INET;
	if(flag == 1)
	{
		if(gethostip(hostip) == -1)
		{
			printf("请输入服务器IP地址:");
			fflush(stdout);
			hostip = malloc(16);
			fgets(hostip,16,stdin);
			hostip[strlen(hostip)-1]='\0';
		}
		printf("%s\n",hostip);
	}
	else
	{
		hostip = buf;
	}
	//服务器ip
	sock.sin_addr.s_addr = inet_addr (hostip);
	//端口
	sock.sin_port = htons (SERVER_PORT);
	//填充0
	memset(sock.sin_zero,0,sizeof(sock.sin_zero));

	//创建套接字
	//IPV4,TCP连接
	c_us.sock_fd = socket(AF_INET,SOCK_STREAM,0);
	if(c_us.sock_fd == -1)
	{
		perror("socket");
		exit(-1);
	}

	//建立连接
	if(connect(c_us.sock_fd,(struct sockaddr *)&sock,sizeof(struct sockaddr_in)) == -1)
	{
		perror("connect");
		exit(-1);
	}

	#ifdef DEBUG
	printf("连接服务器成功！\n");
	#endif

	//成功返回0
	return 0;
}

//接收数据
pro_pack *c_conn_recv(void *tmp)
{
	int flag;
	pro_pack c_recv_data;

	while(1)
	{
		memset(&c_recv_data,0,sizeof(c_recv_data));
		flag = recv(c_us.sock_fd,(void *)&c_recv_data,sizeof(pro_pack),0);
		if(flag < 0)
		{
			perror("recv");
			exit(1);
		}
		else if(flag == 0) //网络中断
		{
			c_conn_disconn();
			return NULL;
		}
		else
		{
			#ifdef DEBUG
			printf("_________recv___________\n");
			printf("type:%d\n",c_recv_data.opcode);
			printf("from:%s\n",c_recv_data.sendid);
			printf("to:%s\n",c_recv_data.recid);
			printf("verify=%d, flag=%d\n",c_recv_data.verify,c_recv_data.flag);
			printf("date:%s\n",c_recv_data.data);
			printf("________________________\n");
			#endif
			//收到数据包，调用函数处理
			if(c_recv_data.opcode >= C_LOGIN && c_recv_data.opcode <= C_REG)
			{
				//登陆或者注册包
				if(c_recv_data.opcode == C_LOGOUT)
				{
					//服务器要求退出
					printf("您已经下线!\n");
					exit(0);
				}
				*(int *)tmp = c_recv_data.flag;
				return NULL;
			}
			else if(c_recv_data.opcode >= C_GROUP_GET)
			{
				//其他包
				c_recv_fun(&c_recv_data);
			}

		}
		
	}
	return NULL;
}

//接收消息处理
void c_recv_fun ( pro_pack *recv )
{
	char buf[120];
	if(recv->flag == 1)
		return ;
	else if(recv->opcode == C_CHAT_GROUP)
	{
		//公聊消息
		c_print_time(0);
		printf(" \033[0m%15s| %s\033[0m\n",recv->sendid,recv->data);
		sprintf(buf,"%s:%s",recv->sendid,recv->data);
		s_chat_log(buf,strlen(buf));
	}
	else if (recv->opcode == C_CHAT_FRI)
	{
		//私聊消息
		if(recv->flag == 2)
		{
			c_print_time(0);
			printf(" \033[32m%15s| 无法找到联系人！\033[0m\n",recv->sendid);
			return ;
		}
		c_print_time(0);
		printf(" \033[32m%15s| %s\033[0m\n",recv->sendid,recv->data);
		sprintf(buf,"%s>>%s",recv->sendid,recv->data);
		s_chat_log(buf,strlen(buf));
	}
	else if(recv->opcode == S_MASSWGE)
	{
		//系统消息
		c_print_time(0);
		printf(" \033[31m%15s| %s\033[0m\n","system",recv->data);
		sprintf(buf,"system:%s",recv->data);
		s_chat_log(buf,strlen(buf));
	}
	else if((recv->opcode == C_GETINFO_FRI) || (recv->opcode == C_GETINFO_ME))
	{
		//好友信息或者个人信息
		printf("\t\t\t\t___%s info___\n",recv->recid);
		c_print_time(0);
		printf(" \036[36m%15s| %s\033[0m\n","system",recv->data);
	}
	else if(recv->opcode == C_FRIEND_ADD)
	{
		//加好友返回信息
		c_print_time(0);
		c_print_time(0);
		if(recv->flag == 0)
		{
			printf(" \033[36m%15s| %s\033[0m\n","system","添加好友成功!\n");
		}
		else
		{
			printf(" \033[36m%15s| %s\033[0m\n","system","添加好友失败!\n");
		}
	}
	else if(recv->opcode == C_FRIEND_DEL)
	{
		//删好友返回信息
		c_print_time(0);
		if(recv->flag == 0)
		{
			printf(" \033[36m%15s| %s\033[0m\n","system","删除好友成功!\n");
		}
		else	
		{
			printf(" \033[36m%15s| %s\033[0m\n","system","删除好友失败!\n");
		}
	}
	else if(recv->opcode == C_ONLINE)
	{
		//在线列表
		c_print_time(0);
		printf(" \033[0m%15s| %s\n","system",recv->data);
	}
}

//发送数据
int c_conn_send(pro_pack *send_pack)
{
	int flag;
	//while(1)
	//{
	#ifdef DEBUG
	printf("_________send___________\n");
	printf("type:%d\n",send_pack->opcode);
	printf("from:%s\n",send_pack->sendid);
	printf("to:%s\n",send_pack->recid);
	printf("verify=%d, flag=%d\n",send_pack->verify,send_pack->flag);
	printf("date:%s\n",send_pack->data);
	printf("________________________\n");
	#endif
		if(( flag = send(c_us.sock_fd,send_pack,sizeof(pro_pack),0)) < 0)
		{
			perror("消息发送失败。");
			return -1;
		}
	//	else if(flag = 0)
	//		break;
	//}
	
	return 0;
}

//处理断开连接
void c_conn_disconn(void)
{
	printf("与服务器断开连接！\n");
	if(close(c_us.sock_fd) == -1)
	{
		perror("close");
	}
	exit(1);
}

//注销
void c_logout(void)
{
	int flag;
	pro_pack c_temp;
	c_temp.opcode = C_LOGOUT;				//操作码
	c_temp.verify = 1;					//校验位
	c_temp.flag = 1;					//返回位
	strncpy(c_temp.sendid,c_us.user,USER_LEN);		//发件id
	strncpy(c_temp.recid,"system",USER_LEN);		//收件id
	strncpy(c_temp.data,"\0",MAX_DATA_LEN);			//数据
	
	c_conn_send(&c_temp);
	c_conn_recv(&flag);
	if (flag == 0)
	{
		memset(&(c_us.user),0,sizeof(USER_LEN));
		strncpy(c_us.user,"\0",1);
		
		//登陆
		c_login_front();
	}

}

//帮助
void c_help ( void )
{
	printf("\t\t******************************>>>   help    <<<******************************\n");
	printf("\t\t Based on c / s architecture linux chat system\n\t\t基于c/s架构的linux聊天工具\n");
	printf("\t\t 版本: 0.1\t");
	printf("\t\t 已经实现的功能:\n");
	printf("\t\t 私聊，格式为\"@name massage\"，公聊，直接发送消息即可\n");
	printf("\t\t 查看在线好友/list，注销/logout，退出/exit,帮助/help\n");
	printf("\t\t******************************************************************************\n");
}


