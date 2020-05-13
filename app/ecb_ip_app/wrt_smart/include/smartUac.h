#ifndef SMARTUAC_H_
#define SMARTUAC_H_



typedef enum{
	RegistrationProgress = 0, /**<Registration is in progress */
	RegistrationOk,	/**< Registration is successful */
	RegistrationCleared, /**< Unregistration succeeded */
	RegistrationFailed,	/**<Registration failed */
	MessageDeliveryInProgress, 
	MessageDeliverFailed,
	MessageDeliverOk,
	NewMessageDeliver,
	SubscribeActive,
	SubscribeTerminated,
	notifyArrived,
	PublishOK,
	PublishFailed

}globalState;

typedef enum {
	TransportUDP, /*UDP*/
	TransportTCP, /*TCP*/
	TransportTLS, /*TLS*/
}UACTransport;

typedef enum{
	NOREG=0,
	REGSUCCEEDED,
	UNREGSUCCEEDED,
	REGFAILED
}regStatus;

typedef enum{
	NOREGERROR=0,
	AUTHFAILED,
	NORESPONSE
}regError;

typedef struct _regState{
	int  state;
	int	 reason;
	char proxy[256];

}regState;




typedef struct _PubState{
	char eventname[64];
	char type[64];
	char subtype[64];
	int expires;
	int contentsize;
	char content[1];
}pubState;

typedef struct _notifyState{
	char eventname[64];
	int  status;
	int contentsize;
	char content[1];
}notifyState;


typedef struct _instantMessage{
	char remotename[256];
	char external_url[256];
	int usrdata;
	int msgstatus;
	int msglen;
	char msg[1];
}instantMessage;

typedef enum{
	UAC_DEBUG=1,
	UAC_MESSAGE=1<<1,
	UAC_WARNING=1<<2,
	UAC_ERROR=1<<3,
	UAC_FALTAL=1<<4,
	UAC_TRACE=1<<5,
	UAC_END=1<<6
}smartUacLogLevel;


typedef int (*smartUacFunc)(globalState state,void* buf,int size);
typedef void (*smartLogFunc)(smartUacLogLevel lev,const char* frm,va_list args);

#ifdef __cplusplus
extern "C"{
#endif

void smartUacInit(smartUacFunc func,smartLogFunc logfunc);
void smartUacUninit();

int  smartUacUserLogin(char* proxy,UACTransport protocol,char* usrname,char* pwd);
void smartUacUserLogout(char* proxy,char* usrname);

void smartUacSub(const char* to,const char* eventName); //to :sip:W_MSG@DOMAIN.
void smartUacPub(pubState* pub);


int  smartUacSendMessage(instantMessage* im);

char* smartUacGetUUID();
void smartUacSetUUID(const char* _u);

void smartUacSetLogEnabled(int enabled);
void smartUacSetUserAgent(const char* agent);
void smartUacSetCAPath(const char* path);

void test();

void adjustTime();
#ifdef __cplusplus
}
#endif

#endif