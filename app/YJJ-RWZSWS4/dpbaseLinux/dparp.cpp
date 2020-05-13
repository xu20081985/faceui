#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <pthread.h>

unsigned char src_mac[6];
unsigned char dst_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static int g_localIp;
static pthread_t g_pid;

typedef struct
{
	struct ether_header		eh;
	struct ether_arp		arp;
}ARP_PACKET_OBJ;

void arp_build_packet(int checkip, ARP_PACKET_OBJ* pPacket)
{
	//填充以太网头部
	memcpy(pPacket->eh.ether_dhost, dst_mac, 6);    //目的MAC地址
	memcpy(pPacket->eh.ether_shost, src_mac, 6);    //源MAC地址
	pPacket->eh.ether_type = htons(ETH_P_ARP);      //协议

	//填充ARP报文头部
	pPacket->arp.ea_hdr.ar_hrd = htons(ARPHRD_ETHER);		//硬件类型
	pPacket->arp.ea_hdr.ar_pro = htons(ETHERTYPE_IP);		//协议类型 ETHERTYPE_IP | ETH_P_IP
	pPacket->arp.ea_hdr.ar_hln = 6;						//硬件地址长度
	pPacket->arp.ea_hdr.ar_pln = 4;						//协议地址长度
	pPacket->arp.ea_hdr.ar_op = htons(ARPOP_REQUEST);		//ARP请求操作

	memcpy(pPacket->arp.arp_sha, src_mac, 6);		//源MAC地址
	memcpy(pPacket->arp.arp_spa, &checkip, 4);     //源IP地址
	memcpy(pPacket->arp.arp_tha, dst_mac, 6);		//目的MAC地址
	memcpy(pPacket->arp.arp_tpa, &checkip, 4);     //目的IP地址
}

int arp_udp_recv(int socketid, char* buf, int len, int itimeout)
{
	int retlen = 0;
	struct sockaddr_in user_addr;
	socklen_t usize;

	usize=sizeof(user_addr);

	if(itimeout > 0)
	{
		fd_set fdread;
		int rc;
		struct timeval timeout;

		timeout.tv_sec = itimeout/1000;
		timeout.tv_usec = (itimeout%1000) * 1000;
		FD_ZERO(&fdread);
		FD_SET(socketid,&fdread);

		rc = select(socketid + 1, &fdread, NULL, NULL, &timeout);
		if(rc < 0)
			return -1;
		if(rc == 0)
			return -1;

		retlen = recvfrom(socketid, buf, len, 0, (struct sockaddr *)&user_addr,&usize);
		if(retlen <= 0)
			return -1;
	}
	else
	{
		retlen = recvfrom(socketid, buf, len, 0, (struct sockaddr *)&user_addr, &usize);
	}

	return retlen;
}

bool arp_check_ip(int checkip)
{
	int sockfd;
	bool bConflict = false;

	do
	{
		sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
		if (sockfd < 0)
		{
			perror("arp_check_ip socket");
			break;
		}

		struct ifreq req;
		bzero(&req, sizeof(struct ifreq));
		strcpy(req.ifr_name, "eth0");
		if(ioctl(sockfd, SIOCGIFINDEX, &req) != 0)
		{
			perror("arp_check_ip ioctl");
			break;
		}

		struct sockaddr_ll peer_addr;
		memset(&peer_addr, 0, sizeof(peer_addr));
		peer_addr.sll_family = AF_PACKET;
		peer_addr.sll_ifindex = req.ifr_ifindex;
		peer_addr.sll_protocol = htons(ETH_P_ARP);

		int flags = fcntl(sockfd, F_GETFL, 0);
		fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

		ARP_PACKET_OBJ packet = {0};
		arp_build_packet(checkip, &packet);

		int ret = sendto(sockfd, &packet, sizeof(ARP_PACKET_OBJ), 0, (struct sockaddr*)&peer_addr, sizeof(struct sockaddr_ll));
		if (ret < 0)
		{
			perror("arp_check_ip sendto");
			break;
		}

		ret = arp_udp_recv(sockfd, (char*)&packet, sizeof(ARP_PACKET_OBJ), 500);
		if(ret != sizeof(ARP_PACKET_OBJ))
			break;

		if((htons(ARPOP_REPLY) == packet.arp.ea_hdr.ar_op)
			&& (memcmp(packet.arp.arp_spa, &checkip, 4) == 0))
		{
			bConflict = true;
		}
	}while(0);

	close(sockfd);
	return bConflict;
}

void* arp_recv_thread(void* param)
{
	printf("arp_recv_thread start\r\n");
	int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if(sockfd < 0)
	{
		printf("arp recv sockfd fail!\r\n");
		return 0;
	}

	int checkip = 0;
	ARP_PACKET_OBJ packet;
	while(1)
	{
		memset(&packet, 0, sizeof(ARP_PACKET_OBJ));
		int ret = recv(sockfd, &packet, sizeof(ARP_PACKET_OBJ), 0);
		if(ret < 0)
		{
			printf("arp_recv_thread recv %d, err:%d\r\n", ret, errno);
			break;
		}
		else if(ret == sizeof(ARP_PACKET_OBJ))
		{
			if((memcmp(dst_mac, packet.arp.arp_tha, 6) == 0)
				&& (memcmp(packet.arp.arp_spa, packet.arp.arp_tpa, 4) == 0))
			{
				if(memcmp(&g_localIp, packet.arp.arp_spa, 4) == 0)
				{
					// ifreq
					struct ifreq req;
					bzero(&req, sizeof(struct ifreq));
					strcpy(req.ifr_name, "eth0");
					if(ioctl(sockfd, SIOCGIFINDEX, &req) != 0)
						perror("ioctl ifindex");

					//sockaddr_ll
					struct sockaddr_ll peer_addr;
					memset(&peer_addr, 0, sizeof(peer_addr));
					peer_addr.sll_family = AF_PACKET;
					peer_addr.sll_ifindex = req.ifr_ifindex;
					peer_addr.sll_protocol = htons(ETH_P_ARP);

					// mac
					memset(&req, 0, sizeof(struct ifreq));
					strcpy(req.ifr_name, "eth0");
					if(ioctl(sockfd, SIOCGIFHWADDR, &req) != 0)
						perror("ioctl mac");

					// reply
					memcpy(packet.eh.ether_shost, req.ifr_hwaddr.sa_data, 6);		//源MAC地址
					memcpy(packet.arp.arp_sha, req.ifr_hwaddr.sa_data, 6);			//源MAC地址
					packet.arp.ea_hdr.ar_op = htons(ARPOP_REPLY); 

					// sendto
					ret = sendto(sockfd, &packet, sizeof(ARP_PACKET_OBJ), 0, (struct sockaddr*)&peer_addr, sizeof(struct sockaddr_ll));
					if(ret < 0)
					{
						printf("arp_recv_thread sendto %d, err:%d\r\n", ret, errno);
						break;
					}
				}
			}
		}
	}

	close(sockfd);
	printf("arp_recv_thread end\r\n");
	return 0;
}

static void GetNetworkCardInfo(unsigned char *pMac)
{
	struct ifreq ifreq;
	int sock;

	if((sock=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("GetNetworkCardInfo socket");
		return;
	}
	strcpy(ifreq.ifr_name, "eth0");

	if(ioctl(sock,SIOCGIFHWADDR,&ifreq)<0)
	{
		perror("GetNetworkCardInfo ioctl");
		return;
	}
	memcpy(pMac, ifreq.ifr_hwaddr.sa_data, 6);
	close(sock);
}

void arp_init(int localIp)
{
	if(g_pid != 0)
		return;

	g_localIp = localIp;
	GetNetworkCardInfo(src_mac);
	pthread_create(&g_pid, NULL, arp_recv_thread, NULL);
}

void arp_set_ip(int localIp)
{
	g_localIp = localIp;
}
