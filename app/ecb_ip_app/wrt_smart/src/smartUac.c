#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "port.h"
#include "sip_sal.h"
#include "ortp/b64.h"
#include "ortp/ortp.h"
#include "smartUac.h"
#include "private.h"
#include "auth.h"
#include "uuid.h"
#include "message.h"
#include "subpub.h"
#include "eXosip2/eXosip.h"
#include "eXosip2.h"

static bool_t gbRun=FALSE;
static ms_thread_t gmThread;
extern SalCallbacks linphone_sal_callbacks;



static smartUACCore* gpSmartUacCore = NULL;

static int apply_transports(int protol,int port)
{
	const char* anyaddr="0.0.0.0";

	if(gpSmartUacCore == NULL)
		return 0;

	sal_unlisten_ports(gpSmartUacCore->sal);
	switch(protol)
	{
	case 0:
		if(sal_listen_port(gpSmartUacCore->sal,anyaddr,port,SalTransportUDP,FALSE) != 0){
				ms_message("sal_listen_port %d failed \n",port);
				return -1;
		}
		break;
	case 1:
		if(sal_listen_port(gpSmartUacCore->sal,anyaddr,port,SalTransportTCP,FALSE) != 0){
				ms_message("sal_listen_port %d failed \n",port);
				return -1;
		}else
			ms_message("sal_listen_port %d ok \n",port);
		break;
	case 2:
		if(sal_listen_port(gpSmartUacCore->sal,anyaddr,port+1,SalTransportTLS,TRUE) != 0){
				ms_message("sal_listen_port %d failed \n",port+1);
				return -1;
		}else
			ms_message("sal_listen_port %d ok \n",port+1);
		break;
	default:
		return -1;

	}
	return 0;
}

static void set_network_reachable(smartUACCore* lc,bool_t isReachable, time_t curtime){
	// second get the list of available proxies

	if (lc->network_reachable==isReachable) return; // no change, ignore.
	ortp_message("Network state is now [%s]",isReachable?"UP":"DOWN");
	if (!isReachable) {
			ortp_message("network down \n");
	}else{
		if(lc->regOp)
			sal_register_refresh(lc->regOp,600);
	}
	
	lc->network_reachable=isReachable;
	if (!lc->network_reachable){
		sal_reset_transports(lc->sal);
	}
}


static int get_local_ip_for_with_connect(int type, const char *dest, char *result){
	int err,tmp;
	struct addrinfo hints;
	struct addrinfo *res=NULL;
	struct sockaddr_storage addr;
	struct sockaddr *p_addr=(struct sockaddr*)&addr;
	ortp_socket_t sock;
	socklen_t s;

	memset(&hints,0,sizeof(hints));
	hints.ai_family=type;
	hints.ai_socktype=SOCK_DGRAM;
	/*hints.ai_flags=AI_NUMERICHOST|AI_CANONNAME;*/
	err=getaddrinfo(dest,"28040",&hints,&res);
	if (err!=0){
		ms_error("getaddrinfo() error for %s : %s",dest, gai_strerror(err));
		return -1;
	}
	if (res==NULL){
		ms_error("bug: getaddrinfo returned nothing.");
		return -1;
	}
	sock=socket(res->ai_family,SOCK_DGRAM,0);
	tmp=1;
	err=setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(SOCKET_OPTION_VALUE)&tmp,sizeof(int));
	if (err<0){
		ms_warning("Error in setsockopt: %d",errno);
	}
	err=connect(sock,res->ai_addr,res->ai_addrlen);
	if (err<0) {
		/*the network isn't reachable*/
		if (errno!=ENETUNREACH) 
			 ms_error("Error in connect: %d",errno);
		freeaddrinfo(res);
		close_socket(sock);
		return -1;
	}
	freeaddrinfo(res);
	res=NULL;
	s=sizeof(addr);
	err=getsockname(sock,(struct sockaddr*)&addr,&s);
	if (err!=0) {
		ms_error("Error in getsockname: %d",errno);
		close_socket(sock);
		return -1;
	}
	if (p_addr->sa_family==AF_INET){
		struct sockaddr_in *p_sin=(struct sockaddr_in*)p_addr;
		if (p_sin->sin_addr.s_addr==0){
			close_socket(sock);
			return -1;
		}
	}
	err=getnameinfo((struct sockaddr *)&addr,s,result,64,NULL,0,NI_NUMERICHOST);
	if (err!=0){
		ms_error("getnameinfo error: %d",errno);
	}
	/*avoid ipv6 link-local addresses*/
	if (p_addr->sa_family==AF_INET6 && strchr(result,'%')!=NULL){
		strcpy(result,"::1");
		close_socket(sock);
		return -1;
	}
	close_socket(sock);
	return 0;
}

int linphone_core_get_local_ip_for(int type, const char *dest, char *result){
	int err;
	strcpy(result,type==AF_INET ? "127.0.0.1" : "::1");

	if (dest==NULL){
		if (type==AF_INET)
			dest="182.92.195.120"; /*a public IP address*/
	}
	err=get_local_ip_for_with_connect(type,dest,result);
	if (err==0) return 0;


	return 0;
}

static void monitor_network_state(smartUACCore *lc, time_t curtime){
	int new_status=lc->network_last_status;
	char newip[64];

	/* only do the network up checking every five seconds */
	if (lc->network_last_check==0 || (curtime-lc->network_last_check)>=5){
		memset(newip,0,64);
		linphone_core_get_local_ip_for(AF_INET,NULL,newip);
		if (strcmp(newip,"::1")!=0 && strcmp(newip,"127.0.0.1")!=0){
			new_status=TRUE;
		}else new_status=FALSE; /*no network*/

		if (new_status==lc->network_last_status && new_status==TRUE \
				&& strcmp(newip,lc->LocalIP)!=0){
			/*IP address change detected*/
			ortp_message("IP address change detected.");
			set_network_reachable(lc,FALSE,curtime);
			lc->network_last_status=FALSE;
		}
		strncpy(lc->LocalIP,newip,sizeof(lc->LocalIP));

		if (new_status!=lc->network_last_status) {
			if (new_status){
				ms_message("New local ip address is %s",lc->LocalIP);
			}
			set_network_reachable(lc,new_status, curtime);
			lc->network_last_status=new_status;
		}
		lc->network_last_check=curtime;
	}
}
static void* RunThread(void* ptr)
{
	time_t curtime;
	smartUACCore* core = (smartUACCore*)ptr;
	if(core == NULL)
		return NULL;

	while(gbRun)
	{
		curtime = time(NULL);
		monitor_network_state(core,curtime);
		sal_iterate(core->sal);
		
		
//		sleep(20);
		usleep(20 * 1000); 	//20ms
	}
	return NULL;
}

static void Start()
{
	
	ms_thread_create(&gmThread,NULL,RunThread,(void*)gpSmartUacCore);

}
static char * ortp_strdup1(const char *tmp){
	size_t sz;
	char *ret;
	if (tmp==NULL)
	  return NULL;
	sz=strlen(tmp)+1;
	ret=(char*)ortp_malloc(sz);
	strcpy(ret,tmp);
	ret[sz-1]='\0';
	return ret;
}
static char* getmyuuid()
{
	uuid_t u,u2;
	char *tmp,*u3,gruu[256],seed[16];
	char* ret = NULL;
	int i =0;
	eXosip_generate_random(seed,16);
	
    tmp = (char*)&u2;
	memcpy(tmp,seed,16);

	uuid_create_md5_from_name(&u,u2,"www.wrtsz.com",strlen("www.wrtsz.com"));
	
	u3 = get_uuid(u);
	memset(gruu,0,256);
	snprintf(gruu, 255,"\"<urn:uuid:%s>\"", u3);
	
	ret = ortp_strdup1(gruu);
	
	free(u3);
	u3 = NULL;

	return ret ;
}

void smartUacSetLogEnabled(int enabled)
{
	if(gpSmartUacCore == NULL)
		return;
	if(enabled == 1){
		ortp_set_log_level_mask(ORTP_WARNING|ORTP_ERROR|ORTP_FATAL|ORTP_DEBUG|ORTP_MESSAGE);
	}else {
		ortp_set_log_level_mask(ORTP_LOGLEV_END);
	}
}


void test()
{
	int i = 0; 
	
	printf("enter testTask!!\n");
	
	i = eXosip_init();  
	if (i != 0)  
		return -1;
	
	printf("sip version : %s\n",eXosip_get_version());
	
	//unsigned short port=7000;//本地sip端口 
	//i = eXosip_listen_addr (IPPROTO_TCP, NULL, 5020, AF_INET, 0); 
	i = eXosip_listen_addr(IPPROTO_TCP, NULL, 7000, AF_INET, 1);//17是udp，不想过多的引入头文件，故用数字表示一下  
	printf("eXosip_listen_addr!!!"); 
	if (i != 0)  
	{	
		printf("listen fail!\n");	
		eXosip_quit();  
		return -1;  
	}
	
	printf("while (1) !!!!\n");

	while (1);
}

void smartUacInit(smartUacFunc func,smartLogFunc logfunc)
{
	 smartUACCore* core= NULL;
	 //test();
	ortp_init();
	ortp_scheduler_init();
	ortp_set_log_level_mask(ORTP_WARNING|ORTP_ERROR|ORTP_FATAL|ORTP_DEBUG|ORTP_MESSAGE);
	core = ms_new0(smartUACCore,1);
	if(core == NULL)
		return;

	gbRun = TRUE;
	core->sal = (smartUACCore *)malloc(1024);
    core->sal = sal_init();
	if(core->sal == NULL)
	{
		ms_free(core);
		core = NULL;
		printf("sal_init()........error\n");
		return;
	}
	core->network_last_status = 0;
	core->gruu = getmyuuid();
	ms_message("core->gruu = %s \n",core->gruu);
	core->smartUacCallback = func;
	core->smartLogCallback = logfunc;
	sal_set_user_pointer(core->sal,(void*)core);

	gpSmartUacCore = core;
	sal_set_callbacks(core->sal,&linphone_sal_callbacks);
	apply_transports(2,5020);

	linphone_core_get_local_ip_for(AF_INET,NULL,core->LocalIP);

	//启动一个任务跑 20ms
	Start();

}
static void smartUacCoreDestroy(smartUACCore* core)
{
	if(core == NULL)
		return;
	if(core->gruu)
		ms_free(core->gruu);
	if(core->proxy)
		ms_free(core->proxy);
	if(core->from)
		ms_free(core->from);
	if(core->authList)
		authInfoClear(core);
	if(core->eventNameList)
		eventNameClear(core);
	if(core->last_recv_msg_ids){
		ms_list_for_each(core->last_recv_msg_ids,ms_free);
		core->last_recv_msg_ids=ms_list_free(core->last_recv_msg_ids);
	}
	if(core->msgList)
		instantMessageSaveClear(core);
	if(core->outPubList)
			outPubClear(core);
	if(core->outSubList)
			outSubClear(core);
	
	ms_free(core);
}
void smartUacUninit()
{
	if(gpSmartUacCore == NULL)
		return;

	gbRun =FALSE;
	ms_thread_join(&gmThread,NULL);
	//free SalOp
	if(gpSmartUacCore->regOp)
		sal_op_release(gpSmartUacCore->regOp);

	sal_uninit(gpSmartUacCore->sal);
	gpSmartUacCore->sal = NULL;
	smartUacCoreDestroy(gpSmartUacCore);
	gpSmartUacCore = NULL;
	ortp_exit();
}

static char* guess_my_contact(char* usrname)
{
	char localip[128];
	char contact[256];
	memset(localip,0,128);
	memset(contact,0,256);
	eXosip_guess_localip(AF_INET, localip, 128);
	sprintf(contact,"sip:%s@%s:5020",usrname,localip);
	return ms_strdup(contact);
	
}
int smartUacUserLogin(char* proxy,UACTransport protocol,char* usrname,char* pwd)
{
	SalAuthInfo * ai = NULL;
	SalAddress* proxyaddr = NULL;
	char* contact = NULL;
	if(gpSmartUacCore == NULL)
		return -1;
	if(gpSmartUacCore->regOp){
		if(gpSmartUacCore->proxy)
			ms_free(gpSmartUacCore->proxy);
		if(gpSmartUacCore->from);
			ms_free(gpSmartUacCore->from);
		sal_op_release(gpSmartUacCore->regOp);
		gpSmartUacCore->regOp = NULL;
		gpSmartUacCore->proxy = NULL;
		gpSmartUacCore->from = NULL;
		authInfoClear(gpSmartUacCore);
		gpSmartUacCore->authList = NULL;
	}
	gpSmartUacCore->regOp = NULL;
	gpSmartUacCore->regOp = sal_op_new(gpSmartUacCore->sal);
	if(gpSmartUacCore->regOp == NULL)
		return -1;
	contact = guess_my_contact(usrname);
	sal_op_set_contact(gpSmartUacCore->regOp,contact);
	ms_free(contact);
	 sal_op_set_user_pointer(gpSmartUacCore->regOp,(void*)gpSmartUacCore);

     ai = authInfoNew(usrname,pwd);
	 authInfoAdd(gpSmartUacCore,ai);
	 proxyaddr = sal_address_new(proxy);
	
	 if(proxyaddr){
		 char* strFrom =  NULL;
		 char* strProxy = NULL;
		 SalAddress* from  = NULL;
		 sal_address_set_transport(proxyaddr,(SalTransport)protocol);
		 strProxy=sal_address_as_string(proxyaddr);
		 from = sal_address_clone(proxyaddr);
		
		sal_address_set_username(from,usrname);
		strFrom = sal_address_as_string(from);

		sal_register(gpSmartUacCore->regOp,strProxy,strFrom,900);
		gpSmartUacCore->proxy = ms_strdup(strProxy);
		gpSmartUacCore->from = ms_strdup(strFrom);
		ms_free(strProxy);
		ms_free(strFrom);
		sal_address_destroy(from);
		sal_address_destroy(proxyaddr);
	 }
	 return 0;
}
void smartUacUserLogout(char* proxy,char* usrname)
{
	if(gpSmartUacCore){
		if(strcmp(proxy,gpSmartUacCore->proxy) == 0){
			sal_unregister(gpSmartUacCore->regOp);
		}
	}
}
void smartUacSub(const char* to,const char* eventName)
{
	SalOp* op = NULL;
	if(to == NULL ||gpSmartUacCore == NULL)
		return;
	op = sal_op_new(gpSmartUacCore->sal);
	if(op == NULL)
		return;


	sal_op_set_contact(op,sal_op_get_contact(gpSmartUacCore->regOp));
	sal_op_set_route(op,gpSmartUacCore->proxy);
	sal_op_set_user_pointer(op,gpSmartUacCore);
	sal_subscribe_event(op,gpSmartUacCore->from,to,eventName);
	eventNameAdd(gpSmartUacCore,eventName);
	gpSmartUacCore->outSubList = ms_list_append(gpSmartUacCore->outSubList,(void*)op);

}
void smartUacPub(pubState* pub)
{
	SalOp *op = NULL;
	int err = 0;
	if(pub == NULL || gpSmartUacCore == NULL)
		return;
	op=sal_op_new(gpSmartUacCore->sal);
	sal_op_set_route(op,gpSmartUacCore->proxy);
	sal_op_set_contact(op,sal_op_get_contact(gpSmartUacCore->regOp));
	sal_op_set_user_pointer(op,gpSmartUacCore);
	err=sal_publish_event(op,gpSmartUacCore->from,gpSmartUacCore->from,pub->eventname,pub->subtype,pub->content);
	gpSmartUacCore->outPubList = ms_list_append(gpSmartUacCore->outPubList,(void*)op);
}
int smartUacSendMessage(instantMessage* im){
	
	char* content_type;
	const char *route=NULL;
	SalOp *op=NULL;
	int b64enlen = 0;
	char *msgbuf = NULL;
	instantMessageSave* save;
	if(gpSmartUacCore == NULL)
		return -1;
	b64enlen = ((im->msglen + (NUM_PLAIN_DATA_BYTES-1))/NUM_PLAIN_DATA_BYTES) * NUM_ENCODED_DATA_BYTES;
	msgbuf = (char*)ms_malloc0(b64enlen+1);
	if(msgbuf == NULL){
			return -1;
	}
	route = gpSmartUacCore->proxy;
	op = sal_op_new(gpSmartUacCore->sal);
	sal_op_set_route(op,route);
	sal_op_set_contact(op,sal_op_get_contact(gpSmartUacCore->regOp));
	sal_op_set_user_pointer(op,(void*)gpSmartUacCore);
	if(strlen(im->external_url) > 0){
		content_type=ms_strdup_printf("message/external-body; access-type=URL; URL=\"%s\"",im->external_url);
		sal_message_send(op,gpSmartUacCore->from,im->remotename,content_type, NULL);
		ms_free(content_type);	
	}else{
		b64_encode(im->msg,im->msglen,msgbuf,b64enlen);
		sal_text_send(op,gpSmartUacCore->from,im->remotename,msgbuf);
	}
	ms_free(msgbuf);
	save =(instantMessageSave*)ms_malloc0(sizeof(instantMessageSave)+im->msglen+1);
	if(save == NULL)
		return -1;
	save->op = op;
	save->msglen = im->msglen;
	save->msgstatus = im->msgstatus;
	strcpy(save->external_url,im->external_url);
	strcpy(save->remotename,im->remotename);
	save->usrdata = im->usrdata;
	if(im->msglen > 0)
		strncpy(save->msg,im->msg,im->msglen);
	gpSmartUacCore->msgList=ms_list_append(gpSmartUacCore->msgList,save);
	return 0;
}

char* smartUacGetUUID()
{
	if(gpSmartUacCore == NULL)
		return NULL;
	return gpSmartUacCore->gruu;
}
void smartUacSetUUID(const char* _u)
{
	if(gpSmartUacCore == NULL)
		return;
	if(gpSmartUacCore->gruu)
		ms_free(gpSmartUacCore->gruu);
	gpSmartUacCore->gruu = NULL;
	gpSmartUacCore->gruu = ms_strdup(_u);
}

void smartUacSetUserAgent(const char* agent)
{
	if(gpSmartUacCore == NULL)
		return;
	sal_set_user_agent(gpSmartUacCore->sal,agent);
}

void smartUacSetCAPath(const char* path)
{
	if(gpSmartUacCore == NULL || path == NULL)
		return;
	sal_set_root_ca(gpSmartUacCore->sal,path);
	sal_verify_server_certificates(gpSmartUacCore->sal,TRUE);
	sal_verify_server_cn(gpSmartUacCore->sal,TRUE);
}
