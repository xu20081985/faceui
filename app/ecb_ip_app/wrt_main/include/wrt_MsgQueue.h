#ifndef _WRT_MSG_QUEUE_H
#define _WRT_MSG_QUEUE_H

#include <sys/types.h>
#include <sys/ipc.h>


#define WRT_MSG_QUEUE_MAX 	300 		//每条消息的最大长度
#define MAX_TIMEOUT 	(~((unsigned int)0))

typedef struct
{
	char test_str[4];
} test_str_s;

//消息的结构
typedef struct{
	long myType;            //消息类型，必须大于0
	char myText[WRT_MSG_QUEUE_MAX];   //消息数据
}WRT_MsgQueue_s;




/**************************************************************************
*                            类声明                                       *
**************************************************************************/
//using namespace CORE;

class CWRTMsgQueue
{
public:
    CWRTMsgQueue(key_t key);
    ~CWRTMsgQueue();
	bool initMsgQueue(unsigned int qMaxBytes);
	void MilSleep(unsigned int milliseconds);
	
	int msgSnd(WRT_MsgQueue_s *msg, unsigned int dataLen);
	int msgRcv(WRT_MsgQueue_s *msg, unsigned int seconds);
private:
	key_t 	m_msgKey;
	int 	m_msgID;	
};

#endif  //_WRT_MSG_QUEUE_H

