#pragma once

#include "dpsyncmsg.h"

#define	DPMSG_CHECK_ID				0x6666AAAA
#define	RSP_STATUS_OK				0
#define	RSP_STATUS_FAIL				1

#define	REQ_SET_DOOR_NUM			0x20001
#define	RSP_SET_DOOR_NUM			0x20002
#define	REQ_GET_LOCK_SETTING		0x20003
#define	RSP_GET_LOCK_SETTING		0x20004
#define	REQ_MODIFY_LOCK_SETTING		0x20005
#define	RSP_MODIFY_LOCK_SETTING		0x20006

#define	REQ_START_ADD_CALL			0x20101
#define	RSP_START_ADD_CALL			0x20102
#define	REQ_STOP_ADD_CALL			0x20103
#define	RSP_STOP_ADD_CALL			0x20104
#define	REQ_CLEAR_ALLCARD			0x20105
#define	RSP_CLEAR_ALLCARD			0x20106

#define	REQ_MODIFY_USER_PWD			0x20201
#define	REP_MODIFY_USER_PWD			0x20202
#define	REQ_MODIFY_HOSTAGE_PWD		0x20203
#define	RSP_MODIFY_HOSTAGE_PWD		0x20204

#define	REQ_CALL_ELEVATOR			0x20301		// ���ڻ�����
#define	RSP_CALL_ELEVATOR			0x20302		// �������ݸ����ڻ�
#define	REQ_CHECK_ELEVATOR			0x20303		// ���ڻ�����
#define	RSP_CHECK_ELEVATOR			0x20303		// �������ݸ����ڻ�

#define	REQ_ROOM_SYNC_MSG			0x20401
#define	RSP_ROOM_SYNC_MSG			0x20402
#define	REQ_GET_IPTABLE				0x20403
#define	RSP_GET_IPTABLE				0x20404

#define	REQ_LEAVE_HOME_MSG			0x20405		//���ڻ��������ģʽ���ſڻ�
#define	RSP_LEAVE_HOME_MSG			0x20406

// ���ſڻ������ſڼ�ͨѶ
#define	REQ_DOOR_SET_SYNC			0x10001
#define	RSP_DOOR_SET_SYNC			0x10002
#define	REQ_DOOR_GET_PUBSET			0x10003
#define	RSP_DOOR_GET_PUBSET			0x10004
#define	REQ_DOOR_GET_USRSET			0x10005
#define	RSP_DOOR_GET_USRSET			0x10006
#define	REQ_DOOR_GET_HOSTAGESET		0x10007
#define	RSP_DOOR_GET_HOSTAGESET		0x10008
#define	REQ_DOOR_GET_ICCARD			0x10009
#define	RSP_DOOR_GET_ICCARD			0x1000a

#define	REQ_CNC_ELEVATOR_SYNC		0x10100
#define	RSP_CNC_ELEVATOR_SYNC		0x10101

#define REQ_MODIFY_DOOR_PWD			0x20501
#define RSP_MODIFY_DOOR_PWD			0x20502

#define REQ_CALL_LEAVE_MSG			0x20601
#define RSP_CALL_LEAVE_MSG			0x20602

#define REQ_SETDEFENSE				0x20701
#define RSP_SETDEFENSE				0x20702
#pragma pack(1)

typedef struct 
{
	int		head;  //��Ϣ��ʼ����, 0x6666AAAA
	int		type;
	UINT64	id;
	int		length;
}MsgHeader_T;

typedef struct
{
	MsgHeader_T			msgHeader;
	DWORD 				statue;
} MsgRspStatue_T;

typedef struct
{
	MsgHeader_T			msgHeader;
	UINT64				roomid;		//���ڻ�����id
} MsgSubDoorCallEvt_T;

typedef struct
{
	MsgHeader_T		msgHeader;
	DWORD			settingver;
	DWORD			userpwdver;
	DWORD			hostagever;
	DWORD			iccardver;
} MsgDoorSyncResp_T;


/* Set Door Num command */
typedef struct
{
	MsgHeader_T			msgHeader;
	UINT64 				newid;
} MsgSetDoornumReq_T;

typedef struct
{
	MsgHeader_T			msgHeader;
	int		 			LockDelay;  	// ������ʱ
	int		 			LockLevel;  	// ������ƽ
	BOOL				MagicEnable;	// �Ƿ������Ŵż��
	int					MagicDelay;		// �Ŵ���ʱ
}MsgLockSetting_T;

typedef struct
{
	MsgHeader_T			msgHeader;
	DPCHAR 				Password[16];
}MsgModUserPwdReq_T;

typedef struct
{
	MsgHeader_T			msgHeader;
	UINT64				time;
}MsgRoomSyncResp_T;

typedef struct
{
	MsgHeader_T			msgHeader;
	DPCHAR 				trust[16];
}MsgLeaveHome_T;//�й�
//�ſڻ����͹��������������
typedef struct
{
	MsgHeader_T			msgHeader;
	DWORD				DoorType;
	DPCHAR 				PublicPwd[16];
	DPCHAR 				HostagePwd[16];
}MsgModDoorPwdReq_T;

typedef struct  
{
	MsgHeader_T			msgHeader;
	DWORD					mode;
}MsgDefenseSet_T;

typedef struct
{
	DWORD	checkID;	// 0x6666AAAA
	DWORD	msg;
	DWORD	length;
	char	data[];
}UdpSyncMsg;
#pragma pack()