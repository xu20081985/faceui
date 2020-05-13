#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <linux/input.h>
#include <errno.h> 
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "wpa_cli.h"

#include "dpwifi.h"
#include "roomlib.h"

#if 0
#define MSG_PRIVATE 1
#define MSG_BROADCAST 1
#define CLOUD_WIFI_APPID 1
#define INADDR_ANY 1
#define WIFI_DISCONNECT 1
#define NETWORK_CHANGE 1
#define FALSE 0
#define TRUE 1
typedef unsigned int DWORD;
typedef unsigned int BOOL;

void DPPostMessage(int a, int b, int c, int d)
{
	
}
int GetIpAddressEx(char* name)
{
	return 0;
}
DWORD DPGetTickCount(void)
{
	struct timespec t_start;
	clock_gettime(CLOCK_MONOTONIC, &t_start);
	return (t_start.tv_sec * 1000 + t_start.tv_nsec / 1000000);
}
int main()
{
	StartWifiServer();
	while(1)
	{
		char option[10];
		fgets(option, 10, stdin);
		if(option[0] == 'c')
			ReqWifi(WIFI_CONNECT, "CE", 1, "12345678");
		else if(option[0] == 'd')
			ReqWifi(WIFI_DISCONNECT, NULL, 0, NULL);
		else if(option[0] == 's')
			ReqWifi(WIFI_SCAN, NULL, 0, NULL);
	}
}
#endif


#define WIFI_PORT				18051	
// 连接wifi超时时间(ms)
#define CONNECT_TIMEOUT			16000	

typedef struct
{
	int	 cmd;
	char ssid[32];
	int	 wpa_key;
	char key[32];
}wifi_user_data;

enum
{
	STATE_IDLE,
	STATE_INACTIVE,
	STATE_CONNECTING,
	STATE_CONNECTED,
	STAGE_DISCONNECTED
};

static int			g_sockfd = -1;
static char			g_ssid[32];
static char			g_key[32];
static int			g_wpaKey;
static wifi_info	g_info;
static BOOL			g_bWifiConnect;
static BOOL			g_bConnecting;			// 正在连接
static DWORD		g_dwConnectTick;		// 开始连接wifi的tick
static BOOL			g_bAutoConnect;			// 是否进行自动连接

/************************************************************************************/
/**********	回调函数，用来通知上层应用处理  *****************************************/
/************************************************************************************/
// 连接断开
static void cbDisconnected()
{
	DPPostMessage(MSG_BROADCAST, NETWORK_CHANGE, INADDR_ANY, TRUE);
}

// 连接成功
static void cbConnected()
{
	// 记住密码
	SetWifiInfo(g_ssid, g_wpaKey, g_key);

	DPPostMessage(MSG_BROADCAST, NETWORK_CHANGE, GetIpAddressEx("wlan0"), TRUE);
}

// 密码错误
static void cbPasswdError()
{
	DPPostMessage(MSG_PRIVATE, CLOUD_WIFI_APPID, WIFI_DISCONNECT, 0);
}

// wifi列表更新
static void cbScanResult(wifi_info* pInfo)
{
	DPPostMessage(MSG_PRIVATE, CLOUD_WIFI_APPID, WIFI_SCAN, (DWORD)pInfo);
}

// 开机自动连接wifi
static void cbAutoConnect(wifi_info* pInfo)
{
	if(g_bConnecting || g_bWifiConnect)
		return;

	if(!g_bAutoConnect)
		return;

	char ssid[32];
	char key[32];
	int wpa_key = GetWifiInfo(ssid, key);
	if(ssid[0] != 0)
	{
		for(int i = 0; i < pInfo->count; i++)
		{
			// 找到对应的ssid 才进行连接
			if(strcmp(pInfo->pitem[i].ssid, ssid) == 0)
			{
				ReqWifi(WIFI_CONNECT, ssid, wpa_key, key);
				break;
			}
		}
	}
	else
	{
		g_bAutoConnect = FALSE;
	}
}
/************************************************************************************/		

static int wifi_socket_create()
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		printf("wifi socket fail:%d\r\n", errno);
		return -1;
	}

	int b = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&b, sizeof(b));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(WIFI_PORT);
	addr.sin_addr.s_addr = 0;
	if(-1 == bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)))
	{
		printf("wifi bind:%d\r\n", errno);
		close(sockfd);
		return -1;
	}

	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags|O_NONBLOCK);

	return sockfd;
}

static void wifi_send_msg(int cmd, char* ssid, int wpa_key, char* key)
{
	if(g_sockfd != -1)
	{
		wifi_user_data pMsg;
		pMsg.cmd = cmd;
		pMsg.wpa_key = wpa_key;
		if(ssid)
		{
			strncpy(pMsg.ssid, ssid, sizeof(pMsg.ssid) - 1);
		}
		if(key)
		{
			strncpy(pMsg.key, key, sizeof(pMsg.key) - 1);
		}

		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(WIFI_PORT);
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		sendto(g_sockfd, &pMsg, sizeof(pMsg), 0, (struct sockaddr*)&addr, sizeof(addr));
	}
}

static bool wifi_cmd_function(char* buf, char* argv)
{
#if 0
	printf("=================================================================================== 1, %s\r\n", argv);
#endif
	char szContent[1][32];
	char* pCmd[1] = {szContent[0]};

	strcpy(pCmd[0], argv);
	int ret = wpa_cli_cmd_function(1, pCmd, buf);
	if(ret || (strncmp(buf, "FAIL", 4) == 0))
	{
		printf("wap_cli_function %s fail\r\n", argv);
		return false;
	}

	return true;
}

static bool wifi_cmd_function(char* buf, int argc, char* argv1, char* argv2, char* argv3, char* argv4)
{
#if 0
	printf("=================================================================================== %d", argc);
	if(argv1)
		printf(" %s", argv1);
	if(argv2)
		printf(" %s", argv2);
	if(argv3)
		printf(" %s", argv3);
	if(argv4)
		printf(" %s", argv4);
	printf("\r\n");
#endif

	char szContent[4][32];
	char* pCmd[4];
	for(int i = 0; i < 4; i++)
		pCmd[i] = szContent[i];

	if(argv1)
		strcpy(pCmd[0], argv1);
	if(argv2)
		strcpy(pCmd[1], argv2);
	if(argv3)
		strcpy(pCmd[2], argv3);
	if(argv4)
		strcpy(pCmd[3], argv4);

	int ret = wpa_cli_cmd_function(argc, pCmd, buf);
	if(ret || (strncmp(buf, "FAIL", 4) == 0))
	{
		printf("wap_cli_function %s fail\r\n", argv1);
		return false;
	}

	return true;
}

static int wifi_get_ssid_count(char* buf)
{
	char szContent[4][32];
	char* pCmd[4];
	for(int i = 0; i < 4; i++)
		pCmd[i] = szContent[i];

	if(!wifi_cmd_function(buf, "scan_result"))
		return -1;

	char* ptr = strstr(buf, "bssid / frequency / signal level / flags / ssid\n");
	if(!ptr)
	{
		printf("wifi_get_ssid_count fail\r\n");
		return -1;
	}

	ptr += strlen("bssid / frequency / signal level / flags / ssid\n");
	if(*ptr == 0)
		return 0;

	int count = 0;
	while(1)
	{
		ptr = strchr(ptr, '\n');
		if(!ptr)
			break;

		ptr++;
		count++;
	}

	return count;
}

static int wifi_get_ssid_info(char* buf)
{
	// 先置为0
	g_info.count = 0;

	int count = wifi_get_ssid_count(buf);
	if(count < 0)
		return -1;
	else if(count == 0)
		return 0;
	else if(count > MAX_SSID_NUMBER)
		count = MAX_SSID_NUMBER;

	wifi_scan_item* pInfo = g_info.pitem;
	char* ptr = strstr(buf, "bssid / frequency / signal level / flags / ssid\n");
	if(!ptr)
	{
		printf("wifi_get_ssid_info fail\r\n");
		free(pInfo);
		return -1;
	}

	ptr += strlen("bssid / frequency / signal level / flags / ssid\n");
	if(*ptr == 0)
	{
		printf("wifi_get_ssid_info ssid 0\r\n");	
		free(pInfo);
		return -1;
	}

	int index = 0;
	char* ptr_n = NULL;
	char* ptr_t = NULL;
	while(1)
	{
		ptr_n = strchr(ptr, '\n');
		if(!ptr_n)
			break;

		for(int i = 0; i < 4; i++)
		{
			ptr_t = strchr(ptr, '\t');
			if(!ptr_t)
				break;
			*ptr_t = 0;

			switch(i)
			{
			case 0:
				strcpy(pInfo[index].bssid, ptr);
				break;
			case 1:
				strcpy(pInfo[index].freq, ptr);
				break;
			case 2:
				strcpy(pInfo[index].level, ptr);
				break;
			case 3:
				strcpy(pInfo[index].flags, ptr);
				break;
			}

			ptr = ptr_t + 1;
		}

		*ptr_n = 0;
		strcpy(pInfo[index].ssid, ptr);

		ptr = ptr_n + 1;
		index++;
	}

	g_info.count = count;
	return 1;
}

static void wifi_disconnect(char* buf, char* netid)
{
	if(!wifi_cmd_function(buf, "disconnect"))
		return;

	if(!wifi_cmd_function(buf, 2, "disable_network", netid, NULL, NULL))
		return;

	wifi_cmd_function(buf, 2, "remove_network", netid, NULL, NULL);
}

static bool wifi_connect(char* buf, char* ssid, int wpa_key, char* key, char* net_id)
{
	printf("wifi_connect ssid:%s, wap_key:%d, key:%s\r\n", ssid, wpa_key, key);

	// get netid
	char netid[4];
	if(!wifi_cmd_function(buf, "add_network"))
		return false;
	strcpy(netid, buf);

	do
	{
		char temp[64];
		sprintf(temp, "\"%s\"", ssid);
		if(!wifi_cmd_function(buf, 4, "set_network", netid, "ssid", temp))
			break;

		if(wpa_key == 0)
		{
			// 不加密
			if(!wifi_cmd_function(buf, 4, "set_network", netid, "key_mgmt", "NONE"))
				break;
		}
		else if(wpa_key == 1)
		{
			// wpa_psk加密
			sprintf(temp, "\"%s\"", key);
			if(!wifi_cmd_function(buf, 4, "set_network", netid, "psk", temp))
				break;
		}
		else if(wpa_key == 2)
		{
			// web 加密
			int keylen = strlen(key);
			if(keylen == 5 || keylen == 13)	// ASCII
				sprintf(temp, "\"%s\"", key);
			else if(keylen == 10 || keylen == 26)	// HEX
				sprintf(temp, "%s", key);
			else
			{
				printf("wifi_connect key wrong length\r\n");
				break;
			}

			if(!wifi_cmd_function(buf, 4, "set_network", netid, "key_mgmt", "NONE"))
				break;

			if(!wifi_cmd_function(buf, 4, "set_network", netid, "wep_key0", temp))
				break;

			if(!wifi_cmd_function(buf, 4, "set_network", netid, "auth_alg", "SHARED"))
				break;
		}

		if(!wifi_cmd_function(buf, 2, "select_network", netid, NULL, NULL))
			break;

		if(!wifi_cmd_function(buf, 2, "enable_network", netid, NULL, NULL))
			break;

		strcpy(net_id, netid);
		return true;

	}while(0);

	wifi_cmd_function(buf, 2, "remove_network", netid, NULL, NULL);
	return false;
}

static void* wifi_server(void* param)
{
	printf("wifi_server start\r\n");

	int len ;
	char buf[2048];
	wifi_user_data* pMsg = (wifi_user_data *)buf;
	char netid[4];

	while(1)
	{
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(g_sockfd, &fd);

		struct timeval tv_out;
		tv_out.tv_sec = 1000;
		tv_out.tv_usec = 0;

		int ret = select(g_sockfd + 1, &fd, NULL, NULL, &tv_out);
		if(ret == -1)
		{
			printf("wifi_server select fail:%d\r\n", errno);
			break;
		}
		else if(ret > 0)
		{
			struct sockaddr_in addr;
			socklen_t addrlen = sizeof(addr);
			ret = recvfrom(g_sockfd, buf, 1024, 0, (struct sockaddr*)&addr, &addrlen);
			if(ret <= 0)
			{
				printf("wifi_server recvfrom ret:%d, errno:%d\r\n", ret, errno);
				break;
			}
			else if(ret != sizeof(wifi_user_data))
			{
				printf("wifi_server recvfrom fail len:%d\r\n", ret);
				continue;
			}
			else
			{
				switch(pMsg->cmd)
				{
				case WIFI_SCAN:
					wifi_cmd_function(buf, "scan");
					break;
				case WIFI_CONNECT:
					if(g_bConnecting)
					{
						// 正在连接中，判断是否是同样的 ssid
						if(strcmp(g_ssid, pMsg->ssid) == 0)
							break;

						// 中断前一次的连接
						wifi_disconnect(buf, netid);
					}
					if(!wifi_connect(buf, pMsg->ssid, pMsg->wpa_key, pMsg->key, netid))
					{
						cbPasswdError();
					}
					else
					{
						g_bConnecting = TRUE;
						g_bAutoConnect = TRUE;
						g_dwConnectTick = DPGetTickCount();
						strcpy(g_ssid, pMsg->ssid);
						strcpy(g_key, pMsg->key);
						g_wpaKey = pMsg->wpa_key;
					}
					break;
				case WIFI_DISCONNECT:
					g_bWifiConnect = FALSE;
					wifi_disconnect(buf, netid);
					break;
				default:
					printf("wifi_server recvfrom fail cmd:%d\r\n", pMsg->cmd);
					break;
				}
			}
		}
	}

	printf("wifi_server stop\r\n");
	return 0;
}

void* wifi_recv(void* param)
{
	printf("wifi_recv start\r\n");

	char buf[2048];
	int ret;

	while(1)
	{
		int ret = wpa_cli_pending();
		if(ret > 0)
		{
			memset(buf, 0, 2048);
			if(0 == wpa_cli_recv(buf))
			{
				if(strstr(buf, "CTRL-EVENT-SCAN-RESULTS"))
				{
					wifi_get_ssid_info(buf);
					cbScanResult(&g_info);

					// 自动连接
					cbAutoConnect(&g_info);
				}
				else if(strstr(buf, "CTRL-EVENT-CONNECTED"))
				{
					system("/sbin/udhcpc -q -i wlan0");
					g_bWifiConnect = TRUE;
					g_bConnecting = FALSE;
					cbConnected();
				}
				else if(strstr(buf, "CTRL-EVENT-DISCONNECTED"))
				{
					if(strstr(buf, "bssid=00:00:00:00:00:00"))
					{
						// 主动断开
						g_bConnecting = FALSE;
						g_bAutoConnect = FALSE;
					}
					else
					{
						// 路由器断开
						wifi_send_msg(WIFI_DISCONNECT, NULL, 0, NULL);
						g_bWifiConnect = FALSE;
					}
					cbDisconnected();
				}
				else if(strstr(buf, "Handshake failed"))
				{
					// 密码错误
					g_bConnecting = FALSE;
					g_bWifiConnect = FALSE;
					g_bAutoConnect = FALSE;
					wifi_send_msg(WIFI_DISCONNECT, NULL, 0, NULL);
					cbPasswdError();
				}

				if(g_bConnecting 
					&& (DPGetTickCount() - g_dwConnectTick > CONNECT_TIMEOUT))
				{
					// 连接超时,断开连接
					g_bConnecting = FALSE;
					g_bWifiConnect = FALSE;
					wifi_send_msg(WIFI_DISCONNECT, NULL, 0, NULL);
				}
			}
		}
		else
		{
			sleep(1);
		}
	}

	printf("wifi_recv start\r\n");
	return 0;
}

void StartWifiServer()
{
	g_sockfd = wifi_socket_create();
	if(g_sockfd == -1)
		return;

	int ret = wpa_cli_start();
	if(ret)
	{
		printf("wpa_cli_start fail\r\n");
		return;
	}

	// 开机即进入自动连接状态
	g_bAutoConnect = TRUE;

	pthread_t pid1;
	pthread_create(&pid1, NULL, wifi_server, NULL);

	pthread_t pid2;
	pthread_create(&pid2, NULL, wifi_recv, NULL);

	g_info.count = 0;
	g_info.pitem = (wifi_scan_item *)malloc(sizeof(wifi_scan_item) * MAX_SSID_NUMBER);
	wifi_send_msg(WIFI_SCAN, NULL, 0, NULL);
}

void StopWifiServer()
{

}

void ReqWifi(int cmd, char* ssid, int wpa_key, char* key)
{
	wifi_send_msg(cmd, ssid, wpa_key, key);
}

bool GetWifiStatus(char* ssid)
{
	if(ssid)
		strcpy(ssid, g_ssid);
	return g_bWifiConnect;
}