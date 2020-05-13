#ifndef __WRT_NETWORK_H__
#define __WRT_NETWORK_H__

#ifdef __cplusplus
extern "C" {
#endif	
#define WRT_LEN 64

struct _direct_call_info
{
	char 	username[WRT_LEN];
	char 	ip[WRT_LEN]; //domain
	int 	port;
};

//注意，如果是采用非服务器的方式则需要将username 置NULL,同时设置_direct_call_info
struct _transfer_call_info{
	int 	callid;
	char 	username[WRT_LEN];
	struct _direct_call_info 	callinfo;
};

struct _auth_info{
	char 	username[WRT_LEN];
	char 	userid[WRT_LEN];
	char 	password[WRT_LEN];
};

struct _display_name_info{
	char 	displayname[WRT_LEN];
	char 	username[WRT_LEN];
};

struct _proxy_info{
	int     type;  //0:reg,1:alarm等。
	char 	ip[WRT_LEN];
	int  	port;
	char 	outboundproxy_ip[WRT_LEN];
	int	outboundproxy_port;
};

struct _agent_info{
	char 	agent_name[WRT_LEN];
	char 	version[WRT_LEN];
};

#ifdef __cplusplus
}
#endif

void get_version_info(char *tmp, int size);
int checkMD5(const void *data, int data_len, bool check_flag);
static void setPacketStatusEmpty( const int index );
int RC4DecPacket_KEY_1(const void *buf, char **out_buf, const int buf_len);
static int AppGetVariableKey(struct sockaddr_in from, void *recv_buf, int recv_len);
static int resolutionPacket(const int packet_index, const void *recv_buf, const int recv_len);


#endif

