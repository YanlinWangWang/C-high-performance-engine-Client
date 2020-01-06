#define WIN32_LEAN_AND_MEAN
//编译指令 g++ client.cpp -pthread -o client
#ifdef _WIN32
//win操作系统
	#include<windows.h>
	#include<WinSock2.h>
#else
	#include<unistd.h>
	#include<arpa/inet.h>
	#include<string.h>

	#define SOCKET int //socket实际上只是一个整数
	#define INVALID_SOCKET (SOCKET)(-0)
	#define SOCKET_ERROR  (-1)
#endif

#include<stdio.h>
#include<thread>
#include <iostream>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader
{
	short dataLength;
	short cmd;
};

//DataPackage
struct Login : public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};

struct LoginResult : public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Logout : public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult : public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		scok = 0;
	}
	int scok;
};

int processor(SOCKET _cSock)
{

	//������
	char szRecv[4096] = {};
	// 5 ���տͻ�������
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("服务器断开链接%d任务结束\n", _cSock);
		return -1;
	}
	switch (header->cmd)
	{
		case CMD_LOGIN_RESULT:
		{
			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LoginResult* login = (LoginResult*)szRecv;
			printf("收到服务段信息： CMD_LOGIN_RESULT,数据长度%d\n", login->dataLength);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LogoutResult* logout = (LogoutResult*)szRecv;
			printf("收到服务段信息 ： CMD_LOGOUT_RESULT,数据长度%d\n", logout->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			NewUserJoin* userJoin = (NewUserJoin*)szRecv;
			printf("收到服务段信息：CMD_NEW_USER_JOIN,数据长度%d\n",  userJoin->dataLength);
		}
		break;
	}
}
bool g_bRun = true;
void cmdThread(SOCKET sock)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("收到exit命令，处理线程结束。\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "lyd");
			strcpy(login.PassWord, "lydmm");
			send(sock, (const char*)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "lyd");
			send(sock, (const char*)&logout, sizeof(Logout), 0);
		}
		else {
			printf("输入命令错误\n");
		}
	}
}

int main()
{
	#ifdef _WIN32
	  //启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	#endif

   //------------
	//-- 用Socket API建立简易TCP客户端
	// 1 建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		cout<<"错误建立socket失败"<<endl;
	}
	else {
		cout<<"建立socket成功"<<endl;
	}
	// 2 启动链接
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);


	#ifdef _WIN32//win
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	#else 
	_sin.sin_addr.s_addr = inet_addr("127.0.0.1");
	#endif

	int ret  = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("Socket链接建立失败......");
	}
	else {
		printf("建立socke链接t成功......");
	}
	//启动linux进程来进行操作
	std::thread t1(cmdThread,_sock);
	t1.detach();//进程分离 从这开始t1单独作为一个进程启动

	while (g_bRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = {1,0};//1s
		int ret = select(_sock, &fdReads, 0, 0, &t);//select监听fdRead组 如果出现操作那么就及时返回 最长阻塞时间为1S
		if (ret < 0)
		{
			printf("select操作错误\n");
			break;
		}
		if (FD_ISSET(_sock, &fdReads))//检测对应标志位
		{
			FD_CLR(_sock, &fdReads);//清空标志位

			if (-1 == processor(_sock))
			{
				printf("select处理结束\n");
				break;
			}
		}

	}


	#ifdef _WIN32
	// 7 closesocket
	closesocket(_sock);
	//清除Windows socket环境
	WSACleanup();
	#else
	close(_sock);//linux下关闭文件
	#endif

	printf("退出\n");
	getchar();
	return 0;
}