

#define NUM_PLAIN_DATA_BYTES 3
#define NUM_ENCODED_DATA_BYTES 	4

typedef struct _smartUACCore
{
	Sal* sal;
	SalOp* regOp;
	char* gruu;
	char* proxy;
	char* from;
	regStatus  rstatus;
	regError   rError;
	MSList *authList;/*MSList of SalAuthInfo*/
	MSList *last_recv_msg_ids;
	MSList *outSubList;    /*MSList of outsub op*/
	MSList *eventNameList; /* event name char* list */
	MSList *msgList; /* MSList of instantMessageSave*/
	MSList *outPubList; /* MSList of outpub op*/
	smartUacFunc smartUacCallback;
	smartLogFunc smartLogCallback;
	char LocalIP[64];
	bool_t network_reachable;
	time_t network_last_check;
	int network_last_status;
}smartUACCore;

typedef struct _instantMessageSave{
	 SalOp* op;
	char remotename[256];
	char external_url[256];
	int usrdata;
	int msgstatus;
	int msglen;
	char msg[1];
	
}instantMessageSave;