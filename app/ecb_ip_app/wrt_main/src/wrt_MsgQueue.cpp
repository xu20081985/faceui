#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "wrt_MsgQueue.h"
#include "wrt_common.h"


CWRTMsgQueue::CWRTMsgQueue(key_t key)
{
	m_msgKey = key;
	m_msgID = -1;
}

CWRTMsgQueue::~CWRTMsgQueue()
{
	if(m_msgID >= 0)
	{
		msgctl(m_msgID, IPC_RMID, NULL);
	}
}

bool CWRTMsgQueue::initMsgQueue(unsigned int qMaxBytes)
{
	//��������Ѵ��ڣ���ɾ������=======>>��ֹ�������н�������֮ǰmalloc���ڴ�
	int tmpID = msgget(m_msgKey, IPC_EXCL);
	if(tmpID >= 0)
	{
		msgctl(tmpID, IPC_RMID, NULL);
	}
	
	m_msgID = msgget(m_msgKey, IPC_CREAT|/*IPC_EXCL|*/0666);
	if(m_msgID < 0)
	{
		return false;
	}
	//DEBUG_DEBUG("m_msgID = %d\n", m_msgID);
	//==============test==============
	struct msqid_ds msg_info;
	int ret = msgctl(m_msgID, IPC_STAT, &msg_info);
	//DEBUG_DEBUG("=============msgctl=============\n");
	//DEBUG_DEBUG("[msgctl]---ret = %d\n", ret);
	//DEBUG_DEBUG("Maximum bytes = %lu\n", msg_info.msg_qbytes);
	//DEBUG_DEBUG("Sequence number = %lu\n", msg_info.msg_perm.__seq);
	//DEBUG_DEBUG("===============================\n");

	//������Ϣ����󻺴��ֽ���
	msg_info.msg_qbytes = qMaxBytes;
	ret = msgctl(m_msgID, IPC_SET, &msg_info);
	//DEBUG_DEBUG("IPC_SET ret = %d\n", ret);
	
	ret = msgctl(m_msgID, IPC_STAT, &msg_info);
	//DEBUG_DEBUG("=============msgctl=============2\n");
	//DEBUG_DEBUG("[msgctl]---ret = %d\n", ret);
	//DEBUG_DEBUG("Maximum bytes = %lu\n", msg_info.msg_qbytes);
	//DEBUG_DEBUG("Sequence number = %lu\n", msg_info.msg_perm.__seq);
	//DEBUG_DEBUG("===============================\n");






	return true;
}

void CWRTMsgQueue::MilSleep(unsigned int milliseconds)
{	
							
	struct timeval tv;
	tv.tv_sec  = milliseconds / 1000;
	tv.tv_usec = (milliseconds % 1000) * 1000;
	select(0, NULL, NULL, NULL, &tv); 				
}

int CWRTMsgQueue::msgSnd(WRT_MsgQueue_s *msg, unsigned int dataLen)
{
	int error=0;
	if( (m_msgID < 0) || !msg || (msg->myType <= 0) )
	{
		return -1;
	}	

//	msg->myType = 1;
//	if( -1 == msgsnd(m_msgID,(void *)msg, dataLen, 0))
	if( -1 == msgsnd(m_msgID,(void *)msg, dataLen, MSG_NOERROR|IPC_NOWAIT)) 		//��Ϊ������
	{
		error=errno;
		DEBUG_ERROR("send error = %d\n", errno);
		if(errno == EAGAIN)
		{//time out queue full
			return -2;
		}
		//if(errno==EACCES || errno==EIDRM || errno==EINTR ||errno==EINVAL)
		if(error)
		{
			error=errno;
		}
		return -3;
	}
	
	return 0;
}

//��ʱʱ�䵥λ0.1s��Ŀǰд�Ĳ��á�����
//�ɹ�����0
int CWRTMsgQueue::msgRcv(WRT_MsgQueue_s *msg, unsigned int seconds)
{
	if(MAX_TIMEOUT == seconds)
	{
		//������ֱ�������ݲŷ���
		if( (m_msgID < 0) ||!msg)
		{
			return -1;
		}	
		
		if(-1 == msgrcv(m_msgID, msg, sizeof(msg->myText), 0, 0))
		{
			return -1;
		}
		return 0;
	}
	else if(seconds > 0)
	{
		//����ָ��ʱ�䣬��ʱ����
		int iRound	 = seconds*10;
//		int iRound	 = time_100ms;
		int iErrFlag  = 0;

		if( (m_msgID < 0) ||!msg)
		{
			return -1;
		}	
		
		while(iRound-- > 0 )
		{
			if(-1 == msgrcv(m_msgID, msg, sizeof(msg->myText), 0, MSG_NOERROR|IPC_NOWAIT))
			{ 
//				printf("errno = %d		iRound = %d\n", errno, iRound);
				if(errno != ENOMSG)
				{//not time out error
					iErrFlag = 1;
					break;
				}
			}
			else
			{
				break;
			}
		//	usleep(100000);//0.1s
			MilSleep(100);//0.1s
		}
		if(iRound <= 0)
		{//time out
			return -2;
		}
		if(1 == iErrFlag)
		{
			return -1;
		}
		
		return 0;
	}
	else
	{
		//������
		if( (m_msgID < 0) ||!msg)
		{
			return -1;
		}
		if(-1 == msgrcv(m_msgID, msg, sizeof(msg->myText), 0, MSG_NOERROR|IPC_NOWAIT))
		{
			return -1;
		}
		return 0;
	}
}


