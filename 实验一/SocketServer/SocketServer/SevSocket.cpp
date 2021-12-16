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
#define MAXLisNum 20  //�����׽��ֵ������Ŀ
#define BUFLEN 1024    
#define MAXBUFLEN 100000
char dir[100] = "D:\\Game\\test\\";//����12���ͻ���������ļ��ڷ������˵ĸ�·��
int dirlen;
char sendBuf[MAXBUFLEN], recvBuf[BUFLEN];//�趨���ͻ������ͽ��ջ���������
struct http_URL
{
	char filename[32];  //�����������url������
	char method;       //����0:GET;����1:POST 
};
class ContactSocket {//��ʾһ���Ựsocket�Ľṹ�壬
public:
	SOCKET s;
	char addr[20];//�ͻ��˵�ip��ַ
	int port;//�ͻ��˵Ķ˿�
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

char URL_PARSE(struct http_URL* HttpURL, char* StrURL)//strurlΪ����������ĵ��ַ���
{
	char chk = 0;
	char* Line_start = NULL;
	char* Line_end = NULL;
	char* Start_temp = NULL;
	char* End_temp = NULL;
	char* Line_temp = NULL;


	// ������������ 
	if ((HttpURL == NULL) || (StrURL == NULL))//�����Ĵ�����Ϊ��
	{
		return (chk=1);
	}

	// ��������Ľ�β��־ 
	Line_temp = (char*)strstr(StrURL, "\r\n\r\n");//�������Ĵ��в��ҽ�β�Ӵ���û���ҵ�����NULL
	if (Line_temp == NULL)
	{
		return (chk=1);
	}

	// �ҵ���һ�еĿ�ͷ�ͽ�β
	Line_start = StrURL;//ָ���Ĵ�ͷ��
	Line_end = (char*)strstr(Line_start, "\r\n");//���ҵ����Ĵ���һ�н�����λ�ã����û���ҵ�����NULL
	if (Line_end == NULL)
	{
		return (chk=1);
	}

	// Ѱ��URL���� 
	if (strncmp(Line_start, "GET ", 4) == 0)//˵��ΪGET���͵�������
	{
		HttpURL->method = 0;
		Start_temp = Line_start + 5;
	}
	else if (strncmp(Line_start, "POST ", 5) == 0)//˵��ΪPOST���͵�������
	{
		HttpURL->method = 1;
		Start_temp = Line_start + 6;
	}
	//��ʼ���ҵ�һ���е�URL���Ʋ����丳���ṹhttp_parse_URL�е�filename
	End_temp = (char*)strstr(Line_start, " HTTP");//�ҵ�������http�汾���ͱ�ʶ��ʼ��λ�ü�URL�����ĵط�
	strncpy_s(HttpURL->filename, Start_temp, End_temp - Start_temp);//��URL�����Ƹ���filename

	return chk;
}
int SendRequest(char* Request, int& status) {
	//�������������ݣ�������Ӧ����
	struct http_URL url1;
	int ret = URL_PARSE(&url1, Request);
	if (ret != 0)   //����ֵ��Ϊ��˵���������г���������
	{
		printf("����������ʧ��\r\n");
		return 0;
	}
	printf("�������ĳɹ�\r\n");
	if (url1.method == 0) {
		printf("������Ϊ:GET\r\n");
		printf("�ļ���Ϊ:%s\r\n", url1.filename);
	}
	if (url1.method == 1) {
		printf("������Ϊ:POST\r\n");
		printf("�ļ���Ϊ:%s\r\n", url1.filename);
	}
	memset(dir + dirlen, 0, 100 - dirlen);//��dir��������ĵ�ַ֮����ַ����滻��0
	strcat(dir, url1.filename);//�����ǽ�����URL���ּӵ����Ը�Ŀ¼�ַ�����
	printf("%s\n", dir);//��ӡ��������Ѱ�ҵ����ݵ�Ŀ¼λ��
	FILE* fp = fopen(dir, "rb");
	fpos_t lengthActual = 0;
	int length = 0;
	char* BufferTemp = NULL;
	memset(sendBuf, 0, MAXBUFLEN);
	if (fp == NULL) {
		printf("���ļ�ʧ�ܣ����Ҳ����ļ�\n");
		sprintf(sendBuf, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 47\r\n\r\n<h1>404 NOT FOUND</h1>");
		status = 0;//����������Ϊ0��ʾû�ж�λ��ָ���ļ�
		return strlen(sendBuf);
	}
	else {
		//����ļ���С
		fseek(fp, 0, SEEK_END);
		fgetpos(fp, &lengthActual);
		fseek(fp, 0, SEEK_SET);
		//������ļ��Ĵ�С�����ǽ��з����ڴ�
		BufferTemp = (char*)malloc(sizeof(char) * ((int)lengthActual));
		length = fread(BufferTemp, 1, (int)lengthActual, fp);
		printf("�ļ���С:%dbytes\n", lengthActual);
		fclose(fp);
		if (strstr(url1.filename, "html") != NULL) {
			printf("���ҵ��ļ�,Ҫ������ļ�����Ϊhtml\n");
			sprintf(sendBuf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", lengthActual);
		}
		if (strstr(url1.filename, "jpg") != NULL) {
			printf("���ҵ��ļ�,Ҫ������ļ�����Ϊjpg\n");
			sprintf(sendBuf, "HTTP/1.1 200 OK\r\nContent-Type: image/jpg\r\nContent-Length: %d\r\n\r\n", lengthActual);
		}
		if (strstr(url1.filename, "png") != NULL) {
			printf("��֧�ִ�����ļ�����,Ҫ������ļ�����Ϊpng\n");
			sprintf(sendBuf, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 47\r\n\r\n<h1>404 NOT SUPPORT</h1>");
			return strlen(sendBuf);
		}
		size_t pos = strlen(sendBuf);//���ͷ���ĳ���
		memcpy(sendBuf + pos, BufferTemp, lengthActual);//��ͷ�����濽���ڴ�BufferTemp������ΪlengthActual���ݵ�ͷ��֮��
		free(BufferTemp);
		status = 1;//״̬��Ϊ1��ʾ�ļ���λ�ɹ�����Ӧ���Ĺ���ɹ�
		return pos + lengthActual;//����Ҫ�������Ӧ���ĵĴ�С
	}
}
void main(int argc, char* argv[])
{
	dirlen = strlen(dir);
	int status = 0;//��ʾ�����ĵ�������
	struct timeval timeout = { 0,0 };//���÷�������select�������趨����
	WSADATA wsaData;
	int nRC;
	sockaddr_in srvAddr, clientAddr;
	SOCKET srvSock;
	int nAddrLen = sizeof(sockaddr);
	ListCONNECT conList; //����������Ч�ĻỰ SOCKET
	ListCONNECT::iterator itor; //�����������������ĵ�����
	ListContactErr conErrList; //��������ʧЧ�ĻỰ SOCKET
	ListContactErr::iterator itor1;
	FD_SET rfds, wfds; //�����ļ��������� 
	u_long uNonBlock;
	int cnt = 0;
	int port = GetPrivateProfileInt(_T("config"), _T("port"), 90,
		_T("C:\\Users\\������\\source\\repos\\SocketServer\\SocketServer\\config.ini"));
	CString cstr("000.000.000.000");
	GetPrivateProfileString(_T("config"), _T("ip"), _T("127.0.0.1"), cstr.GetBuffer(), 20,
		_T("C:\\Users\\������\\source\\repos\\SocketServer\\SocketServer\\config.ini"));
	USES_CONVERSION;
	const char* ipaddr = T2A(cstr);
	printf("�������ļ���ȡ�Ķ˿ں�Ϊport��%d\n", port);
	printf("�������ļ���ȡ��IP��ַΪip��%s\n", ipaddr);
	//��ʼ�� winsock
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
	//���� TCP socket
	srvSock = socket(AF_INET, SOCK_STREAM, 0);
	if (srvSock == INVALID_SOCKET)
	{
		printf("Server create socket error!\n");
		WSACleanup();
		return;
	}
	printf("Server TCP socket create OK!\n");
	//�� socket to Server's IP and port 
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(port);//���ö˿�
	//srvAddr.sin_addr.S_un.S_addr = INADDR_ANY; //���� IP��ַ
	srvAddr.sin_addr.S_un.S_addr = inet_addr(ipaddr);//�������ļ��е�IPת����Client Socket bunding��Ҫ���޷��ų�������
	nRC = bind(srvSock, (LPSOCKADDR)&srvAddr, sizeof(srvAddr));
	if (nRC == SOCKET_ERROR)
	{
		printf("Server socket bind error!\n");
		closesocket(srvSock);
		WSACleanup();
		return;
	}
	printf("Server socket bind OK!\n");
	//��ʼ�������̣��ȴ��ͻ�������
	nRC = listen(srvSock, MAXLisNum);
	if (nRC == SOCKET_ERROR)
	{
		printf("Server socket listen error!\n");
		closesocket(srvSock);
		WSACleanup();
		return;
	}
	//�� srvSock ��Ϊ������ģʽ�Լ����ͻ���������
	uNonBlock = 1; //���������÷�����ģʽ
	ioctlsocket(srvSock, FIONBIO, &uNonBlock);
	while (1)
	{
		//�� conList ��ɾ���Ѿ���������ĻỰ SOCKET
		for (itor1 = conErrList.begin(); itor1 != conErrList.end(); itor1++)
		{
			itor = find(conList.begin(), conList.end(), *itor1);
			if (itor != conList.end()) {
				if (closesocket(itor->s) != SOCKET_ERROR) {
					conList.erase(itor);
					printf("�����׽����ѳɹ����ر�,��ǰ����%d���Ự�׽���!\n", conList.size());
				}
				else printf("�ر��׽��ֳ���!!\n");
			}
		}
		
		FD_ZERO(&rfds);
		FD_SET(srvSock, &rfds);
		for (itor = conList.begin(); itor != conList.end(); itor++)
		{
			//�����лỰ SOCKET ��Ϊ������ģʽ
			uNonBlock = 1;
			ioctlsocket(itor->s, FIONBIO, &uNonBlock);
			//���õȴ��Ự SOKCET �ɽ������ݻ�ɷ�������
			FD_SET(itor->s, &rfds);
		}
		//��ʼ�ȴ���time=0�ͱ��һ������ķ����������������ļ��������Ƿ��б仯�������̷��ؼ���ִ�У��ļ��ޱ仯����0���б仯����һ����ֵ
		int nTotal = select(0, &rfds, NULL, NULL, &timeout);
		//��� srvSock �յ��������󣬽��ܿͻ���������
		if (FD_ISSET(srvSock, &rfds))  //����������ĳһ���׽����Ƿ��Ǽ����е�һ������Ϊ������select������ʹ�����û���������Ӹ��׽��־ͻ���select�����дӼ���ɾ��
		{
			printf("\n*************************************************************\n");
			printf("�������յ��������󣬳������Ӳ������Ự�׽���....\n");
			nTotal--;
			//�����Ự SOCKET
			SOCKET connSock = accept(srvSock,	(LPSOCKADDR)&clientAddr,	&nAddrLen);
			if (connSock == INVALID_SOCKET)
			{
				printf("�����Ự�׽���ʧ��\n");
				WSAGetLastError();
				closesocket(srvSock);
				WSACleanup();
				return;
			}
			ContactSocket sock;
			strcpy(sock.addr, inet_ntoa(clientAddr.sin_addr));//�����ַ�����ʽ��ip��ַ
			sock.s = connSock;
			sock.port = clientAddr.sin_port;
			//���������ӵ�socket�� conList ��
			conList.insert(conList.end(), sock);
			printf("����������Դ����Ϣ��\nIP��ַ��%s���˿ں�:%d\n", sock.addr, sock.port);
			printf("�µĻỰ�׽��ֳɹ�����,���ڹ���%d���Ự�׽���\n", conList.size());
			printf("\n*************************************************************\n");
		}
		if (nTotal > 0)
		{
			printf("��ǰ���׽�����Ҫ����!\n\n");
			for (itor = conList.begin(); itor != conList.end(); itor++)
			{
				//����Ự SOCKET �����ݵ���������ܿͻ�������
				if (FD_ISSET(itor->s, &rfds))
				{
					memset(recvBuf, 0, BUFLEN);//��ս��ջ������������ձ���
					nRC = recv(itor->s, recvBuf, BUFLEN, 0);//���ش���������Ĵ��ĳ���
					if (nRC == SOCKET_ERROR)
					{
						//�������ݴ���
						//��¼�²�������ĻỰ SOCKET
						conErrList.insert(conErrList.end(), *itor);
						printf("�������ݴ���\n");
					}if (nRC == 0) {//��ʾ�ͻ����Ѿ��رո��׽���
						printf("�ûỰ�׽��ֱ��رգ��������Ự\n");
						conErrList.insert(conErrList.end(), *itor);
					}
					else
					{
						recvBuf[nRC] = '\0';
						printf("\n*************************************************************\n");
						printf("����������Դ��IP��ַΪ:%s,�˿ں�Ϊ:%d\n", itor->addr, itor->port);
						printf("���յ�%d�ֽ�����\n���������Ϊ��\n%s", nRC, recvBuf);
						int len = SendRequest(recvBuf, status);//����Client�˵���Ӧ���ĵĳ���
						if (status == 1) {
							printf("�ɹ���λ��������ļ������ڷ���....\n");
						}
						else {
							printf("δ��λ����������ļ������ڷ���״̬404��Ӧ����....\n");
						}
						printf("����Client�˵���Ӧ���ĵĳ���Ϊ%d�ֽ�....\n", len);
						nRC = send(itor->s, sendBuf, len, 0);
						if (nRC == SOCKET_ERROR) {
							conErrList.insert(conErrList.end(), *itor);
							printf("����������ݴ���!\n");
						}
						else {
							printf("�ɹ�����%d�ֽ����ݵ�Client��!\n", nRC);
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
