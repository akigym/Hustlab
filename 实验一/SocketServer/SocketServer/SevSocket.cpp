#include <stdio.h>
#include <winsock2.h>
#include <list>
#include <algorithm>
#include <string.h>
#include<iostream>
#include<Windows.h>
#include <tchar.h>
#include <atlstr.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
#define MAXLisNum 20  //监听套接字的最大数目
#define BUFLEN 1024    
#define MAXBUFLEN 100000
char dir[100] = "D:\\Game\\test\\";//长度12，客户端请求的文件在服务器端的根路径
int dirlen;
char sendBuf[MAXBUFLEN], recvBuf[BUFLEN];//设定发送缓冲区和接收缓冲区长度
struct http_URL
{
	char filename[32];  //存放请求报文中url的名称
	char method;       //等于0:GET;等于1:POST 
};
class ContactSocket {//表示一个会话socket的结构体，
public:
	SOCKET s;
	char addr[20];//客户端的ip地址
	int port;//客户端的端口
	ContactSocket& operator=(const ContactSocket& obj) {
		s = obj.s;
		strcpy(addr, obj.addr);
		port = obj.port;
		return *this;
	}
	bool operator==(const ContactSocket& obj) {
		return s == obj.s && (!strcmp(addr, obj.addr)) && port == obj.port;
	}
};

typedef list<ContactSocket> ListCONNECT;
typedef list<ContactSocket> ListContactErr;

char URL_PARSE(struct http_URL* HttpURL, char* StrURL)//strurl为传入的请求报文的字符串
{
	char chk = 0;
	char* Line_start = NULL;
	char* Line_end = NULL;
	char* Start_temp = NULL;
	char* End_temp = NULL;
	char* Line_temp = NULL;


	// 请求报文输入检查 
	if ((HttpURL == NULL) || (StrURL == NULL))//请求报文串内容为空
	{
		return (chk=1);
	}

	// 检查请求报文结尾标志 
	Line_temp = (char*)strstr(StrURL, "\r\n\r\n");//在请求报文串中查找结尾子串，没有找到返回NULL
	if (Line_temp == NULL)
	{
		return (chk=1);
	}

	// 找到第一行的开头和结尾
	Line_start = StrURL;//指向报文串头部
	Line_end = (char*)strstr(Line_start, "\r\n");//查找到报文串第一行结束的位置，如果没有找到返回NULL
	if (Line_end == NULL)
	{
		return (chk=1);
	}

	// 寻找URL名称 
	if (strncmp(Line_start, "GET ", 4) == 0)//说明为GET类型的请求报文
	{
		HttpURL->method = 0;
		Start_temp = Line_start + 5;
	}
	else if (strncmp(Line_start, "POST ", 5) == 0)//说明为POST类型的请求报文
	{
		HttpURL->method = 1;
		Start_temp = Line_start + 6;
	}
	//开始查找第一行中的URL名称并将其赋给结构http_parse_URL中的filename
	End_temp = (char*)strstr(Line_start, " HTTP");//找到首行中http版本类型标识开始的位置即URL结束的地方
	strncpy_s(HttpURL->filename, Start_temp, End_temp - Start_temp);//将URL的名称赋给filename

	return chk;
}
int SendRequest(char* Request, int& status) {
	//根据请求报文内容，构造响应报文
	struct http_URL url1;
	int ret = URL_PARSE(&url1, Request);
	if (ret != 0)   //返回值不为零说明请求报文中出现了问题
	{
		printf("解析请求报文失败\r\n");
		return 0;
	}
	printf("解析报文成功\r\n");
	if (url1.method == 0) {
		printf("方法名为:GET\r\n");
		printf("文件名为:%s\r\n", url1.filename);
	}
	if (url1.method == 1) {
		printf("方法名为:POST\r\n");
		printf("文件名为:%s\r\n", url1.filename);
	}
	memset(dir + dirlen, 0, 100 - dirlen);//将dir除了输入的地址之后的字符串替换成0
	strcat(dir, url1.filename);//将我们解析的URL名字加到测试根目录字符串后
	printf("%s\n", dir);//打印出请求报文寻找的内容的目录位置
	FILE* fp = fopen(dir, "rb");
	fpos_t lengthActual = 0;
	int length = 0;
	char* BufferTemp = NULL;
	memset(sendBuf, 0, MAXBUFLEN);
	if (fp == NULL) {
		printf("打开文件失败，即找不到文件\n");
		sprintf(sendBuf, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 47\r\n\r\n<h1>404 NOT FOUND</h1>");
		status = 0;//将处理结果置为0表示没有定位到指定文件
		return strlen(sendBuf);
	}
	else {
		//获得文件大小
		fseek(fp, 0, SEEK_END);
		fgetpos(fp, &lengthActual);
		fseek(fp, 0, SEEK_SET);
		//计算出文件的大小后我们进行分配内存
		BufferTemp = (char*)malloc(sizeof(char) * ((int)lengthActual));
		length = fread(BufferTemp, 1, (int)lengthActual, fp);
		printf("文件大小:%dbytes\n", lengthActual);
		fclose(fp);
		if (strstr(url1.filename, "html") != NULL) {
			printf("已找到文件,要传输的文件类型为html\n");
			sprintf(sendBuf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", lengthActual);
		}
		if (strstr(url1.filename, "jpg") != NULL) {
			printf("已找到文件,要传输的文件类型为jpg\n");
			sprintf(sendBuf, "HTTP/1.1 200 OK\r\nContent-Type: image/jpg\r\nContent-Length: %d\r\n\r\n", lengthActual);
		}
		if (strstr(url1.filename, "png") != NULL) {
			printf("不支持传输此文件类型,要传输的文件类型为png\n");
			sprintf(sendBuf, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 47\r\n\r\n<h1>404 NOT SUPPORT</h1>");
			return strlen(sendBuf);
		}
		size_t pos = strlen(sendBuf);//获得头部的长度
		memcpy(sendBuf + pos, BufferTemp, lengthActual);//在头部后面拷贝内存BufferTemp处长度为lengthActual内容到头部之后
		free(BufferTemp);
		status = 1;//状态置为1表示文件定位成功，响应报文构造成功
		return pos + lengthActual;//返回要传输的响应报文的大小
	}
}
void main(int argc, char* argv[])
{
	dirlen = strlen(dir);
	int status = 0;//表示请求报文的请求结果
	struct timeval timeout = { 0,0 };//设置非阻塞的select函数的设定变量
	WSADATA wsaData;
	int nRC;
	sockaddr_in srvAddr, clientAddr;
	SOCKET srvSock;
	int nAddrLen = sizeof(sockaddr);
	ListCONNECT conList; //保存所有有效的会话 SOCKET
	ListCONNECT::iterator itor; //迭代器，用来遍历的迭代器
	ListContactErr conErrList; //保存所有失效的会话 SOCKET
	ListContactErr::iterator itor1;
	FD_SET rfds, wfds; //两个文件的描述符 
	u_long uNonBlock;
	int cnt = 0;
	int port = GetPrivateProfileInt(_T("config"), _T("port"), 90,
		_T("C:\\Users\\龚宇蒙\\source\\repos\\SocketServer\\SocketServer\\config.ini"));
	CString cstr("000.000.000.000");
	GetPrivateProfileString(_T("config"), _T("ip"), _T("127.0.0.1"), cstr.GetBuffer(), 20,
		_T("C:\\Users\\龚宇蒙\\source\\repos\\SocketServer\\SocketServer\\config.ini"));
	USES_CONVERSION;
	const char* ipaddr = T2A(cstr);
	printf("从配置文件读取的端口号为port：%d\n", port);
	printf("从配置文件读取的IP地址为ip：%s\n", ipaddr);
	//初始化 winsock
	nRC = WSAStartup(0x0101, &wsaData);
	if (nRC)
	{
		printf("Server initialize winsock error!\n");
		return;
	}
	if (wsaData.wVersion != 0x0101)
	{
		printf("Server's winsock version error!\n");
		WSACleanup();
		return;
	}
	printf("Server's winsock initialized !\n");
	//创建 TCP socket
	srvSock = socket(AF_INET, SOCK_STREAM, 0);
	if (srvSock == INVALID_SOCKET)
	{
		printf("Server create socket error!\n");
		WSACleanup();
		return;
	}
	printf("Server TCP socket create OK!\n");
	//绑定 socket to Server's IP and port 
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(port);//设置端口
	//srvAddr.sin_addr.S_un.S_addr = INADDR_ANY; //设置 IP地址
	srvAddr.sin_addr.S_un.S_addr = inet_addr(ipaddr);//将配置文件中的IP转换成Client Socket bunding需要的无符号长整型数
	nRC = bind(srvSock, (LPSOCKADDR)&srvAddr, sizeof(srvAddr));
	if (nRC == SOCKET_ERROR)
	{
		printf("Server socket bind error!\n");
		closesocket(srvSock);
		WSACleanup();
		return;
	}
	printf("Server socket bind OK!\n");
	//开始监听过程，等待客户的连接
	nRC = listen(srvSock, MAXLisNum);
	if (nRC == SOCKET_ERROR)
	{
		printf("Server socket listen error!\n");
		closesocket(srvSock);
		WSACleanup();
		return;
	}
	//将 srvSock 设为非阻塞模式以监听客户连接请求
	uNonBlock = 1; //这句就是设置非阻塞模式
	ioctlsocket(srvSock, FIONBIO, &uNonBlock);
	while (1)
	{
		//从 conList 中删除已经产生错误的会话 SOCKET
		for (itor1 = conErrList.begin(); itor1 != conErrList.end(); itor1++)
		{
			itor = find(conList.begin(), conList.end(), *itor1);
			if (itor != conList.end()) {
				if (closesocket(itor->s) != SOCKET_ERROR) {
					conList.erase(itor);
					printf("出错套接字已成功被关闭,当前仍有%d个会话套接字!\n", conList.size());
				}
				else printf("关闭套接字出错!!\n");
			}
		}
		
		FD_ZERO(&rfds);
		FD_SET(srvSock, &rfds);
		for (itor = conList.begin(); itor != conList.end(); itor++)
		{
			//把所有会话 SOCKET 设为非阻塞模式
			uNonBlock = 1;
			ioctlsocket(itor->s, FIONBIO, &uNonBlock);
			//设置等待会话 SOKCET 可接受数据或可发送数据
			FD_SET(itor->s, &rfds);
		}
		//开始等待，time=0就变成一个纯粹的非阻塞函数，不管文件描述符是否有变化，都立刻返回继续执行，文件无变化返回0，有变化返回一个正值
		int nTotal = select(0, &rfds, NULL, NULL, &timeout);
		//如果 srvSock 收到连接请求，接受客户连接请求
		if (FD_ISSET(srvSock, &rfds))  //这个函数检查某一个套接字是否是集合中的一个，因为调用了select函数，使得如果没有请求链接该套接字就会在select函数中从集合删除
		{
			printf("\n*************************************************************\n");
			printf("服务器收到连接请求，尝试连接并创建会话套接字....\n");
			nTotal--;
			//产生会话 SOCKET
			SOCKET connSock = accept(srvSock,	(LPSOCKADDR)&clientAddr,	&nAddrLen);
			if (connSock == INVALID_SOCKET)
			{
				printf("创建会话套接字失败\n");
				WSAGetLastError();
				closesocket(srvSock);
				WSACleanup();
				return;
			}
			ContactSocket sock;
			strcpy(sock.addr, inet_ntoa(clientAddr.sin_addr));//储存字符串形式的ip地址
			sock.s = connSock;
			sock.port = clientAddr.sin_port;
			//将建立连接的socket在 conList 中
			conList.insert(conList.end(), sock);
			printf("连接请求来源的信息：\nIP地址：%s，端口号:%d\n", sock.addr, sock.port);
			printf("新的会话套接字成功产生,现在共有%d个会话套接字\n", conList.size());
			printf("\n*************************************************************\n");
		}
		if (nTotal > 0)
		{
			printf("当前有套接字需要处理!\n\n");
			for (itor = conList.begin(); itor != conList.end(); itor++)
			{
				//如果会话 SOCKET 有数据到来，则接受客户的数据
				if (FD_ISSET(itor->s, &rfds))
				{
					memset(recvBuf, 0, BUFLEN);//清空接收缓存区用来接收报文
					nRC = recv(itor->s, recvBuf, BUFLEN, 0);//返回传输的请求报文串的长度
					if (nRC == SOCKET_ERROR)
					{
						//接受数据错误，
						//记录下产生错误的会话 SOCKET
						conErrList.insert(conErrList.end(), *itor);
						printf("接收数据错误\n");
					}if (nRC == 0) {//表示客户端已经关闭该套接字
						printf("该会话套接字被关闭，加入错误会话\n");
						conErrList.insert(conErrList.end(), *itor);
					}
					else
					{
						recvBuf[nRC] = '\0';
						printf("\n*************************************************************\n");
						printf("本次请求来源的IP地址为:%s,端口号为:%d\n", itor->addr, itor->port);
						printf("接收到%d字节内容\n请求的内容为：\n%s", nRC, recvBuf);
						int len = SendRequest(recvBuf, status);//传回Client端的响应报文的长度
						if (status == 1) {
							printf("成功定位到请求的文件，正在发送....\n");
						}
						else {
							printf("未定位到所请求的文件，正在发送状态404响应报文....\n");
						}
						printf("传回Client端的响应报文的长度为%d字节....\n", len);
						nRC = send(itor->s, sendBuf, len, 0);
						if (nRC == SOCKET_ERROR) {
							conErrList.insert(conErrList.end(), *itor);
							printf("传输出现数据错误!\n");
						}
						else {
							printf("成功传出%d字节内容到Client端!\n", nRC);
							printf("\n*************************************************************\n");
						}
					}
				}
			}
		}
	}
	closesocket(srvSock);
	WSACleanup();
}
