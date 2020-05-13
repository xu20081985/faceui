#include <roomlib.h>
#include <netinet/if_ether.h>
#include <linux/if.h>
#include <linux/route.h>

static int m_iNetState = 0;
static bool g_bConflict = false;

static void
IPAddrToStr(char* szStr, DWORD IPAddr)
{
	sprintf(szStr, "%d.%d.%d.%d", IPAddr & 0xFF,(IPAddr >> 8) & 0xFF,
		(IPAddr >> 16) & 0xFF,(IPAddr >> 24) & 0xFF);
}

bool SetIPAddress(const char *pIpAddr, const char *pMask, const char *pGateway, const char *pNetCardName)
{
	struct sockaddr_in  sin;
	struct ifreq        ifr;

	DBGMSG(DPINFO, "SetIPAddress %s %s %s\r\n", pIpAddr, pMask, pGateway);
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if( 0 > fd )
	{
		printf("%s(%d): socket error: %d\n", __FILE__, __LINE__, errno);
		return FALSE;
	}

	// 输入网卡名称，若网卡地址为NULL，则使用默认的ETH0
	strcpy(ifr.ifr_name, pNetCardName != NULL ? pNetCardName : "eth0");
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;

	// 设置 IP
	sin.sin_addr.s_addr = inet_addr(pIpAddr == NULL ? "0" : pIpAddr);
	memcpy(&ifr.ifr_addr, &sin, sizeof(sin));
	if(ioctl(fd, SIOCSIFADDR, &ifr) < 0)
	{
		printf("%s(%d): SIOCSIFADDR error: %d\n", __FILE__, __LINE__, errno);
		close(fd);
		return FALSE;
	}

	// 设置MASK
	sin.sin_addr.s_addr = inet_addr(pMask == NULL ? "255.255.255.0" : pMask);
	memcpy(&ifr.ifr_addr, &sin, sizeof(sin));
	if(ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
	{
		printf("%s(%d): SIOCSIFNETMASK errno: %d\n", __FILE__, __LINE__, errno);
		close(fd);
		return FALSE;
	}

	//// 设置网关
	struct rtentry rt;
	memset(&rt, 0, sizeof(struct rtentry));
	rt.rt_dst.sa_family = AF_INET;
	((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;
	rt.rt_genmask.sa_family = AF_INET;
	((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = 0;
	if(pGateway)
	{
		sin.sin_addr.s_addr = inet_addr(pGateway);
		memcpy(&rt.rt_gateway, &sin, sizeof(sin));
		((struct sockaddr_in *)&rt.rt_dst)->sin_family = AF_INET;
		((struct sockaddr_in *)&rt.rt_genmask)->sin_family = AF_INET;
		rt.rt_flags = RTF_UP | RTF_GATEWAY;
		if(ioctl(fd, SIOCADDRT, &rt) < 0)
		{
			//printf("%s(%d): Gateway SIOCADDRT errno:%d\n", __FILE__, __LINE__, errno);
		}
	}
	close(fd);

	char cmdline[256];
	sprintf(cmdline, "/sbin/route add -net 224.0.0.0 netmask 240.0.0 dev %s", pNetCardName == NULL ? "eth0" : pNetCardName);
	system(cmdline);
	return TRUE;
}

BOOL 
SetIPAddress(int ip, int mask, int gw)
{
	char IpAddr[20], MaskAddr[20], GwAddr[20];
	IPAddrToStr(IpAddr, ip);
	IPAddrToStr(MaskAddr, mask);
	IPAddrToStr(GwAddr, gw);
	BOOL ret = SetIPAddress(IpAddr, MaskAddr, GwAddr, NULL);

	g_bConflict = arp_check_ip(ip);
	return ret;
}

static void * CheckNetworkStatusMethod(void *wParam)
{
	int fd;
	struct ifreq ifr;

	// ����������
	strcpy(ifr.ifr_name, "eth0");
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	while(1)
	{
		if(fd != -1)
		{
			if(ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
			{
				printf("CheckNetworkStatusMethod ioctl fail\r\n");
			}

			if((ifr.ifr_flags & IFF_RUNNING) && m_iNetState == 0)
			{
				g_bConflict = arp_check_ip(GetIpAddress());

				// �������ӡ�
				m_iNetState = 3;
				DPPostMessage(MSG_BROADCAST, NETWORK_CHANGE, m_iNetState, 0);
			}
			else if(!(ifr.ifr_flags & IFF_RUNNING) && m_iNetState == 3)
			{
				// ���߶Ͽ���
				m_iNetState = 0;
				DPPostMessage(MSG_BROADCAST, NETWORK_CHANGE, m_iNetState, 0);
			}
		}
		sleep(2);
	}
	close(fd);
	return NULL;
}

BOOL StartNetMonitor(void)
{
	DBGMSG(SRV_MOD, "StartNetMonitor\r\n");
	pthread_t pid = 0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	return pthread_create(&pid, NULL, CheckNetworkStatusMethod, NULL) == 0;
}

int GetNetState()
{
	if(m_iNetState == 0)
		return 0;
	else
	{
		if(g_bConflict)
			return 1;		// Conflict
		else
			return 3;
	}
}

void SetNetState(BOOL bConflict)
{
	g_bConflict = bConflict;
}

BOOL GetNetworkcardInfo(unsigned char *pMac)
{
	struct ifreq ifreq;
	int sock;

	if((sock=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket");
		return FALSE;
	}
	strcpy(ifreq.ifr_name, "eth0");

	if(ioctl(sock,SIOCGIFHWADDR,&ifreq)<0)
	{
		perror("ioctl");
		return FALSE;
	}
	memcpy(pMac, ifreq.ifr_hwaddr.sa_data, 6);
	close(sock);
	return TRUE;
}

BOOL SetNetDhcp(void)
{	
	char result_buf[MAX_PATH];	
	int rc = 0; // ���ڽ��������ֵ	
	FILE *fp;	/*ִ��Ԥ���趨�����������������ı�׼���*/	
	DWORD starttick = DPGetTickCount();

	fp = popen("udhcpc -q -i eth0", "r");	
	if(NULL == fp)
	{		
		perror("popen fial\n");		
		return FALSE;	
	}	
	while(fgets(result_buf, sizeof(result_buf), fp) != NULL)
	{		
		/*Ϊ����������ÿ�Щ��������صĻ��з�ȥ��*/
		if('\n' == result_buf[strlen(result_buf)-1])
		{			
			result_buf[strlen(result_buf)-1] = '\0';		
		}		
		DBGMSG(DPINFO, "output [%s] %u %u\r\n", result_buf, DPGetTickCount(), starttick);	
		if((DPGetTickCount() - starttick) > 3000)
			break;
	}
	/*�ȴ�����ִ����ϲ��رչܵ����ļ�ָ��*/	
	rc = pclose(fp);	
	if(0 != rc)
	{		
		perror("close fail\n");		
		return FALSE;	
	}	
	else
	{		
		DBGMSG(DPINFO, "cmd:status[%d] returnvalue [%d]\r\n",rc, WEXITSTATUS(rc));	
		DPPostMessage(MSG_BROADCAST, NETWORK_CHANGE, m_iNetState, 0);
	}	
	return TRUE;
}

DWORD GetIpAddress(void)
{
	int inet_sock;
	struct ifreq ifr;
	struct sockaddr_in* paddr;

	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

	strcpy(ifr.ifr_name, "eth0");
	//SIOCGIFADDR��־�����ȡ�ӿڵ�ַ
	if (ioctl(inet_sock, SIOCGIFADDR, &ifr) <  0)
	{
		perror("ioctl");
		return 0;
	}
	paddr = (struct sockaddr_in*)&(ifr.ifr_addr);
	close(inet_sock);
	return (DWORD)paddr->sin_addr.s_addr;
}

DWORD GetIpAddressEx(char* szName)
{
	int inet_sock;
	struct ifreq ifr;
	struct sockaddr_in* paddr;

	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

	strcpy(ifr.ifr_name, szName);
	//SIOCGIFADDR��־�����ȡ�ӿڵ�ַ
	if (ioctl(inet_sock, SIOCGIFADDR, &ifr) <  0)
	{
		perror("ioctl");
		return 0;
	}
	paddr = (struct sockaddr_in*)&(ifr.ifr_addr);
	close(inet_sock);

	return (DWORD)paddr->sin_addr.s_addr;
}
