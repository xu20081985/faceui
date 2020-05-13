#pragma once

#include "dpsyncmsg.h"

//���������ն�
#define MC_TYPE_SYNCHRO			1001	//ͬ��
#define MC_TYPE_SYNCHRO_ACK		1002
#define MC_TYPE_UPDATE			1003	//����
#define MC_TYPE_UPDATE_ACK		1004	
#define MC_TYPE_ALARM			1005	//����
#define MC_TYPE_CLEANALARM		1007	//ȡ������
#define MC_TYPE_LOCKLOG			1009	//������¼

#define	MC_TYPE_OPEN_LOCK		1101
#define MC_TYPE_OPEN_LOCKTWO	1301
#define	MC_TYPE_ADD_CARD		1201
#define	MC_TYPE_DEL_CARD		1203
#define	MC_TYPE_SYNC_CARD		1207

#define	MC_TYPE_PRIVATE_MSG		1205
#define	MC_TYPE_MAIL_BOX		1501	//�ű��� sedaר�ù���,��ʾס�����µ��ż�

#define XMLMSG_CHECKID			0xabcd1234
#pragma pack(1)
typedef struct 
{
	DWORD head;					//��Ϣ��ʼ����, 0xabcd1234
	DWORD length;				//��Ϣ����ĳ��ȣ�������Ϣͷ
	WORD content[];				//��Ϣ����,XML��ʽ	WCHAR
} MSGHEAD,*LPMSGHEAD;

typedef struct
{
	BYTE ictype;	// IC�����ͣ�1=ס������2=Ѳ����
	DWORD iccode; 	// IC������
	UINT64 roomid;	// ���ڻ�����
	DWORD ExpDate;	// �����գ���20130918 �����Ϊ0�������Ƶ��ڣ�ĿǰΪ�����ֶΣ�
} tagCard;
#pragma pack()

