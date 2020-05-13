#pragma once

typedef struct
{
	DWORD x;
	DWORD y;
	DWORD flag;
} TOUCHDATA;

typedef struct
{
	DWORD key;
	DWORD flag;
} KBDDATA;

typedef struct
{
	DWORD msg;
	DWORD wParam;
	DWORD lParam;
	DWORD zParam;
} SYS_MSG;

#include "appid.h"
#define	MSG_URGENT_TYPE			0
#define	MSG_USER_TYPE			1
#define	MSG_TIME_TYPE			2
#define	MSG_TOUCH_TYPE			3
#define	MSG_KEY_TYPE			4
#define	MSG_MAX_TYPE			5

#define	TIME_MESSAGE			10000		// ʱ����Ϣ

#define HARDKBD_MESSAGE  		10001		// ������ԭʼ��Ϣ
#define	KBD_MESSAGE				10002
#define	KBD_DOWN				0
#define	KBD_UP					1
#define KBD_CTRL				2			// ���̿ؼ���Ϣ

#define TOUCH_RAW_MESSAGE		10003		// ��������ԭʼ��Ϣ��������ĻУ�������
#define TOUCH_DOWN				0			// ĳ������������
#define TOUCH_VALID				1			// ĳ���������º��ƶ�������
#define TOUCH_UP				2			// �����µİ������ͷ�
#define	TOUCH_MOVEOUT			3			// �����µİ������Ƴ�

#define	TOUCH_MESSAGE			10004		// ĳ���ؼ�����������Ϣ
#define TOUCH_ACTIVE			12345
#define TOUCH_SLIDE				100041		// ������Ϣ
#define SLIDE_LEFT				0			// ��
#define SLIDE_RIGHT				1			// �һ�
#define SLIDE_UPSIDE			2			// �ϻ�
#define SLIDE_DOWN				3			// �»�

#define	MSG_SYSTEM				10005		// ϵͳ��Ϣ
#define	REBOOT_MACH				1			// �����ն�
#define	UPDATE_NETCFG			2			// ��������
#define	CODE_CHANGE				3			// ����仯
#define	RESET_MACH				4			// �ָ���������
#define WATCHDOG_CHANGE			5			// �Ƿ�ʼ���Ź�
#define TIME_CHANGE				6			// ʱ��仯
#define RESET_ZIGBEE			7			// ��ʼ��ZIGBEE

#define	MSG_BROADCAST			10006		// �㲥��Ϣ����ǰ���ڵ����д��ڶ������յ�
#define NETWORK_CHANGE			1			// ����״̬�仯
#define SMART_STATUS_ACK		2			// �����յ����������Ļظ�
#define SMART_STATUS_SCENE		3			// �龰���ƻظ�
#define SMART_STATUS_SYNC		4			// ���������ϱ�״̬
#define SMART_STATUS_GET		5			// ������ȡ����״̬
#define SMART_LIGHT_STUDY		6			// �ƹ�ͨ·ѧϰ


#define	MSG_PRIVATE				10007		// ˽����Ϣ�����͸�ָ���Ĵ���, wparamΪ���ڵ�id

#define	MSG_START_APP			10020		// �����µĴ���
#define	MSG_END_APP				10021		// ����ָ���Ĵ���
#define	MSG_START_FROM_ROOT		10022		// ����ָ���Ĵ��ڣ���������ǰ���еĴ���
#define	MSG_EXTSTART_APP		10023		// �����߳��������̷߳��͵Ĵ���������Ϣ
#define	MSG_START_FROM_OVER		10024		// ����ָ���Ĵ��ڣ����ӵ���ǰ������
#define	MSG_END_OVER_APP		10025		// ����ָ���Ĵ���, ���ָ�֮ǰ�Ĵ���

void InitSysMessage(void);
BOOL DPPostMessage(DWORD msgtype, DWORD wParam, DWORD lParam, DWORD zParam, DWORD type = 1);
DWORD DPGetMessage(SYS_MSG* msg);
void CleanUserInput(void);
