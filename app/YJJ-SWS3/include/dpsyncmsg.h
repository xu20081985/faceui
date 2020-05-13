#pragma once

/* Open Type Define */
#define	IDCARD_OPEN				0x01	// �û�����������¼����
#define	USERPWD_OPEN			0x02	// ס�����뿪�ţ���¼����
#define	PUBPWD_OPEN				0x03	// �������뿪��������¼������Ϣ
#define	ROOM_OPEN				0x04	// ���ڻ���������¼ס������
#define	MCENTER_OPEN			0x05	// �������Ŀ����� ��¼����Ա����
#define	XG_OPEN					0x06	// Ѳ��������  ��¼����
#define	INVALID_CARD			0x07	// �Ƿ���

#define	SYNC_SEND_OPENRECORD	1		// ���Ϳ�����¼

#define	SYNC_SEND_ALARM			2		// ���ͱ�����Ϣ
#define	MAGNETIC_ALARM			1		// �Ŵű���
#define	HOSTAGE_ALARM			2		// Ю�ֱ���
#define	TAMPER_ALARM			3		// ���𱨾�

#define	SYNC_SEND_ELEVATOR		3		// �����ݿ���Ϣ�����ſڻ�

typedef struct 
{
	char CompInfo[512];
	char CompInfoEn[512];
	char HardInfo[512];
} HardInfo_T;

typedef struct
{
	DWORD msg;
	DWORD wParam;
	DWORD lParam;
	UINT64 roomid;
} SyncMSG;

