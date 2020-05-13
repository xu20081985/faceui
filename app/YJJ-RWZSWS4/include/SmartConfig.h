#pragma once

#include <roomlib.h>
#include "list.h"

#pragma pack(1)
typedef struct
{
	WORD head;				// ֡ͷ  0xA55A
	BYTE length;			// ֡�ܳ��� - sizeof(head) - sizeof(length)
	WORD src;				// Դ��ַ
	WORD dst;				// Ŀ���ַ
	BYTE type;				// ��������
	BYTE cmd;				// ��������
	DWORD pwd;				// ϵͳ����
	BYTE data[];			// ֡����
}ECB_DATA;

typedef struct
{
	DEVICE device;			// �豸��ʶ
	BYTE cmd;				// ���Ʒ�ʽ
	WORD param;				// ���Ʋ���
}ECB_CTRL_ACK;


typedef struct
{
	DEVICE device;			// �豸��ʶ
	BYTE cmd;				// ���Ʒ�ʽ
	BYTE param1;		    // ���Ʋ���1
	BYTE param2;			// ���Ʋ���2
	BYTE param3;		    // ���Ʋ���3
	BYTE param4;		    // ���Ʋ���4
#if 0	
	BYTE param5;		    // ���Ʋ���5
	BYTE param6;			// ���Ʋ���6
	BYTE param7;		    // ���Ʋ���7
	BYTE param8;		    // ���Ʋ���8
	BYTE param9;		    // ���Ʋ���9
	BYTE param10;			// ���Ʋ���10
	BYTE param11;		    // ���Ʋ���11
	BYTE param12;		    // ���Ʋ���12
#endif	
}ECB_CTRL_ACK_AIR;

typedef struct
{
	unsigned char temp:5;	// bit0 - bit4 �¶�
	unsigned char speed:3;	// bit5 - bit7 ����
	unsigned char func:2;	// bit8 - bit9 ����(ɨ��)
	unsigned char mode:4;	// bit10 - bit13 ģʽ
	unsigned char onoff:2;	// bit14 - bit15 ״̬
}AC_DATA;

typedef struct
{
	unsigned char voice:5;  // bit0 - bit4 ����
	unsigned char source:3; // bit5 - bit7 ��Դ
	unsigned char feel:2;   // bit8 - bit9 ��Ч
	unsigned char func:4;   // bit10 - bit13 ����
	unsigned char status:2; // bit14 - bit15 ״̬
}MUSIC_DATA;


typedef struct
{
	WORD type;				// �豸����
	WORD phy_addr;			// �豸�����ַ
	DEVICE device;   		// �豸Ψһ��ʶ
	BYTE cmd;				// ���Ʒ�ʽ
	WORD status;			// �豸״̬
	WORD param1;			// ���Ʋ���
	char name[32];			// �豸����
	BOOL exist;				// �豸�Ƿ����
	BYTE scene;
}SmartDev;

typedef struct {
	LISTOBJ lpObj;
	BYTE hour;
	BYTE minute;
	BYTE week;
	DEVICE device;
	BYTE way;
	WORD param;
	BOOL onoff;
	char timeStr[64];
	char weekStr[64];
	char devStr[64];
	char wayStr[64];
} SmartTimer,  *PSmartTimer;

typedef struct
{
	DEVICE device; 			// �豸��ʶ
	BYTE num;				// �洢���
 	BYTE scene;				// �������
 	BYTE cmd;				// ���Ʒ�ʽ
 	WORD param;				// ���Ʋ���
}SceneCtrl;

typedef struct
{
	BYTE type;				// ����ѧϰ����
	BYTE singleNum;			// ����ѧϰ����
	BYTE doubleNum;			// ˫��ѧϰ����
	SENSOR senosr[16];		// ����ѧϰ��Ϣ
}LightStudy;

#pragma pack()


// ֡ͷ����
const WORD ECB_HEAD				=  0xA55A;	// head = 0x5AA5
const BYTE ECB_DATA_HEAD 		=  11;		// sizeof(ECB_DATA) - sizeof(head) - sizeof(length) + 1
const BYTE ECB_HEAD_LENGTH 		=  2;		// sizeof(head)
const BYTE ECB_DATA_MIN_LEN		=  14;		// sizeof(ECB_DATA) + sizeof(check_num)
const BYTE ECB_HEAD_LEN_LENGTH 	=  3;		// sizeof(head) + sizeof(length)

// �豸����
const WORD ECB_DEV_TYPE	        =  0xF12D;  // type 0xF12D
const WORD WIND_DEV_TYPE	    =  0xB1F3;  // type 0xB1F3

// ÿ֡�����38���ֽ�
#define MAX_ECB_DATA_LEN			38

// ����
#define ECB_TYPE_ATTR				0x01
// ����
#define ECB_TYPE_CTRL				0x02
// ͬ��
#define ECB_TYPE_SYNC				0x03
// ����
#define ECB_TYPE_SET				0x05
// ȫ��
#define ECB_TYPE_ALL				0x0A

// ����������ʱ���ƹ���
#define ECB_SET_TIMER				0xBA
#define ECB_SET_TIMER_ACK			0xBB

// ������������/����/ʱ��
#define ECB_SET_TIME				0x61
#define ECB_SET_TIME_ACK			0x62

// ���ó���ģʽ����ͬ����Ϣ
#define ECB_SET_SCENE				0x25
#define ECB_SET_SCENE_ACK			0x26

// ��������֮������Ϣ
#define ECB_SET_DEV_CFG				0xBE
#define ECB_SET_DEV_CFG_ACK			0xBF

// ��������
#define ECB_SEARCH_DEV				0x01
#define ECB_SEARCH_DEV_ACK			0x02

// ���������ϱ�
#define ECB_REPORT_INFO				0x03
#define ECB_REPORT_INFO_ACK			0x04

// Ӧ�ÿ���
#define ECB_CTRL_DEV				0x05
#define ECB_CTRL_DEV_ACK			0x06

// ����ͬ������/����/ʱ��
#define ECB_SYNC_TIME				0x30
#define ECB_SYNC_TIME_ACK			0x31

// �������ϱ�״̬(�๦�ܴ�����)
#define ECB_SENSOR_REPORT			0x50
#define ECB_SENSOR_REPORT_ACK		0x51

// ��ѯ��������״̬
#define ECB_GET_STATUS				0x84
#define ECB_GET_STATUS_ACK			0x85

// ���������ϱ�״̬
#define ECB_SYNC_STATUS				0x94
#define ECB_SYNC_STATUS_ACK			0x95

// ���ÿ���
#define ECB_CFG_CTRL				0xA0
#define ECB_CFG_CTRL_ACK			0xA1

// ������ʼ��
#define ECB_INIT_ALL				0xAE
#define ECB_INIT_ALL_ACK			0xAF
// ������λ
#define ECB_RESET_ALL				0xAC
#define ECB_RESET_ALL_ACK			0xAD

// ���������ַ��ϵͳ����
#define ECB_SET_ADDR_PWD			0x53
#define ECB_SET_ADDR_PWD_ACK		0x54

// �뿪�����
#define ECB_CTRL_ALL				0x0A
#define ECB_CTRL_ALL_ACK			0x0B
/////////////////////////////////////////////
/////////////////////////////////////////////

// ҳ��
#define MAX_PAGE_NUM				16
// ÿһҳ��ͼ������
#define MAX_ICON_NUM				5
// ���֧���豸����
#define MAX_DEV_NUM					MAX_PAGE_NUM * MAX_ICON_NUM

// �ƹ�ͨ·
#define MAX_CHAN_NUM				3
// ״̬����
#define MSG_SMART_UPDATE			1

// ��ʼ�豸����
#define INIT_DEV_PWD				0x99999999
// ��ʼ�������ַ
#define INIT_PHY_ADDR				0x9999

// ��Ч��ͨ· ID TYPE
#define INVALID_DEV_CHAN			0xFF
#define INVALID_DEV_ID				0xFFFFFFFF
#define INVALID_DEV_TYPE			0xFFFF
// ��Ч�����ַ
#define INVALID_PHY_ADDR			0xFFFF

enum SMART_TYPE
{
	// �ƹ�
	ST_LIGHT_A = 1,
	ST_LIGHT_B = 2,
	ST_LIGHT_C = 3,
	ST_LIGHT_D = 4,
	// ����
	ST_DIMMER_A = 11,
	ST_DIMMER_B,
	ST_DIMMER_C,
	ST_DIMMER_D,
	// ���� ���������ϡ���������Ҷ
	ST_CURTAIN_A = 21,		
	ST_CURTAIN_B,
	ST_CURTAIN_C,
	ST_CURTAIN_D,
	// ���� �������������������촰
	ST_WINDOW_A = 31,
	ST_WINDOW_B,
	ST_WINDOW_C,
	ST_WINDOW_D,
	// ����
	ST_OUTLET_A = 41,
	ST_OUTLET_B,
	ST_OUTLET_C,
	ST_OUTLET_D,
	// ����
	ST_FAN_A = 51,
	ST_FAN_B,
	ST_FAN_C,
	ST_FAN_D,
	// �յ�
	ST_AC_A = 61,
	ST_AC_B,
	ST_AC_C,
	ST_AC_D,
	//����յ�
	ST_IRAIR_A = 66,
	ST_IRAIR_B,
	ST_IRAIR_C,
	ST_IRAIR_D,	
	// ��ů
	ST_HEAT_A = 71,
	ST_HEAT_B,
	ST_HEAT_C,
	ST_HEAT_D,
	// �·�
	ST_WIND_A = 81,
	ST_WIND_B,
	ST_WIND_C,
	ST_WIND_D,
	// ����
	ST_TV_A = 91,
	ST_TV_B,
	ST_TV_C,
	ST_TV_D,
	// ��������
	ST_MUSIC_A = 101,
	ST_MUSIC_B,
	ST_MUSIC_C,
	ST_MUSIC_D,
	// ����
	ST_LOCK_A = 111,
	ST_LOCK_B,
	ST_LOCK_C,
	ST_LOCK_D,	
	// �龰
	ST_SCENE_A = 201,
	ST_SCENE_B,
	ST_SCENE_C,
	ST_SCENE_D,
	ST_SCENE_E,
	ST_SCENE_F,
	ST_SCENE_G,
	ST_SCENE_H,
	ST_SCENE_I,
	ST_SCENE_J,
	ST_SCENE_K,
	ST_SCENE_L,
	ST_SCENE_M,
	ST_SCENE_N,
	ST_SCENE_O,
	ST_SCENE_P,
	ST_SCENE_Q,
	ST_SCENE_R,
	ST_SCENE_S,
	ST_SCENE_T,
	ST_SCENE_U,
	ST_SCENE_V,
	ST_SCENE_W,
	ST_SCENE_X,
	ST_SCENE_Y,
	ST_SCENE_Z,  
};

enum
{
	SCMD_OPEN = 0x01,					// ��
	SCMD_DIMMER_OPEN,					// ��(1-100%)
	SCMD_CLOSE,							// ��
	SCMD_DELAY_OPEN,					// ��ʱ��(1��65535��)	
	SCMD_DELAY_CLOSE,					// ��ʱ��(1��65535��)
	SCMD_DELAY_OPEN_CLOSE,				// ��֮����ʱ��(1��65535��)
	SCMD_GRADUALLY_OPEN,				// ����(1-100%)
	SCMD_GRADUALLY_CLOSE,				// ����
	SCMD_CURTAIN_OPEN,					// ������(1-100%)	
	SCMD_CURTAIN_CLOSE = 0x10,			// ������
	SCMD_CURTAIN_STOP,					// ����ͣ
	SCMD_OPEN_CLOSE,					// ��/��(ͬһ�������ƿ��أ� 0ͬ�� 1��ͬ��)
	SCMD_BRIGHT_ADD,					// ����+(1-100%)	
	SCMD_BRIGHT_SUB,					// ����-(1-100%)	
	SCMD_TEMP_ADD,						// ����+(1��127����1����127�棩128��255��0��127�棩)
	SCMD_TEMP_SUB,						// ����-(1��127����1����127�棩128��255��0��127�棩)
	SCMD_SPEED_ADD,						// ����+(1-100%)
	SCMD_SPEED_SUB,						// ����-(1-100%)
	SCMD_SCENE,							// ��������
	SCDM_INFRARED = 0x20,				// ����(1~255)
	SCMD_AC = 0x32,						// �յ�
	SCMD_MUSIC = 0x33,					// ����
	SCMD_SAFE = 0xA0,					// ����
	//========================================================================================
	// �յ�����˵��
    //Bit15��Bit14��״̬����1������ 2��ֹͣ 3������ 0�����ֲ���
    //Bit13��Bit10��ģʽ����1������ 2������ 3��ͨ�� 4: ˯�� 5����ʪ 6������ 7����˪ 8������ 9������ 10������ 11����ǿ 12���Զ����� 13���ֶ�14����ʱ 0��ȱʡ 0�����ֲ���
    //Bit9��Bit8�����ܣ���1��ɨ�� 2��ҶƬ����ˮƽ�� 3��ҶƬ���򣨴�ֱ��0�����ֲ���
    //Bit7��Bit5�����٣���1������ 2���� 3���� 4���� 5������ 6���Զ��������٣�7������ֹͣ 0�����ֲ���
    //Bit4��Bit0���¶ȣ���1��31��10�桫40�棩0�����ֲ���
	//�·��豸������չ��
	//����3��4��Bit15��Bit0 
	//�趨VOC��1��65535ppm��1��65535 ��ʾ0.1��6553.5ppm��0�����ֲ���
	//����5��6��Bit15��Bit0
	//�趨PM2.5��1��65535ug/m3 0�����ֲ���
	//��ʪ���豸������չ��
	//����3��Bit7��Bit0 �趨�¶ȣ�1��127����1����127�棩128��255��0��127�棩0�����ֲ���
	//����4��Bit7��Bit0 �趨ʪ�ȣ�1��100�� 0�����ֲ���
	//========================================================================================
	// ���ֲ���˵��
	//Bit15��Bit14��״̬����1:���� 2:ֹͣ 3:����0�����ֲ���
	//Bit13��Bit10�����ܣ���1:���� 2:��ͣ 3:����/��ͣ 4:��һ�� 5:��һ�� 6:����+ 7:����-0�����ֲ���
	//Bit9��Bit8����Ч����1:��ͨ 2:���� 3:ҡ��0�����ֲ���
	//Bit7��Bit5����Դ����1:CD 2:FM1 3:FM2 4:MP3 5:AUX 6:���� 7: iPhone/iPod0�����ֲ���
	//Bit4��Bit0���������� 1��31��1��100����0�����ֲ���
};


enum 
{
	CURTAIN_ALL_CLOSE = 1,		// ȫ��
	CURTAIN_HALF_OPEN = 2,		// �뿪
	CURTAIN_ALL_OPEN  = 3		// ȫ��	
};

enum 
{
	WIND_HIGH 		= 1,		// �߷�
	WIND_MIDDLE 	= 2,		// �з�
	WIND_LOW  		= 3			// �ͷ�	
};

/******* �յ�״̬ *****************************************************************************************************/
#define AC_MODE_MAX				14
#define AC_FUNC_MAX				4
#define AC_SPEED_MAX			7
#define AC_TEMPRATRUE_MAX		22
#define AC_TEMPRATURE_INDEX		14

#define AC_STATUS_ON			1
#define AC_STATUS_OFF			2
/**********************************************************************************************************************/

/******* �·�״̬ *****************************************************************************************************/
#define WIND_STATUS_ON			1
#define WIND_STATUS_OFF			2
/**********************************************************************************************************************/


/******* ��ů״̬ *****************************************************************************************************/
#define HEAT_MODE_MAX           1
#define HEAT_TEMPRATURE_INDEX   14
/**********************************************************************************************************************/

/*******��������״̬***************************************************************************************************/
#define MUSIC_STATUS_ON         1
#define MUSIC_STATUS_OFF        2

#define MUSIC_STATUS_PAUSE      3
#define MUSIC_STATUS_LAST       4
#define MUSIC_STATUS_NEXT       5
#define MUSIC_STATUS_ADD        6
#define MUSIC_STATUS_SUB        7


#define MUSIC_VOICE_MAX         31
/**********************************************************************************************************************/


void InitSmartDev();
void InitSmartTimer();

void UpdatSmartTimerSet();

void ResetSmartDev();
void ResetSmartTimer();

void DeleteSmartTimer();

void SetSmartInit(DWORD flag);

DWORD GetSmartAddr();
void SetSmartAddr(DWORD addr);

DWORD GetSmartPwd();
void SetSmartPwd(DWORD pwd);

DWORD GetSmartRoute();
void SetSmartRoute(DWORD route);

DWORD GetSmartEnvId();
void SetSmartEnvId(DWORD id);

void SetStudyChan(DWORD i);
void SetStudyNum(DWORD i);


LightStudy* GetLightStudy(int i);
void SetLightStudy(LightStudy lightstudy, int i);

SmartDev* GetSmartDev(DWORD* dwCount);
PSmartTimer GetTimerHead();
PSmartTimer GetTimerNext(LPLISTOBJ pobject);
DWORD GetTimerCount();

void GetTimerShow(PSmartTimer pItem);

void AddSmartDev(ECB_DATA* pECB);
void AddSceneCtrlList(ECB_DATA* pECB);
void AddSmartTimer(ECB_DATA* pECB);    

void SmartTimeSync();

void SetLightStudySync(ECB_DATA* pECB);
void AddLightStudy(ECB_DATA* pECB);

void SetStatusByScene(BYTE scene);
void SetStatusBySync(DEVICE *device, BYTE cmd, WORD param);
DEVICE *GetSmartDevByDev(DEVICE *device);

void SetStatusByAll(BYTE cmd);


char* GetSmartPngCurtain(DWORD type, DWORD status);
char *GetSmartPngWind(DWORD type, DWORD status);
char* GetSmartPng(DWORD type, DWORD status);
char* GetSmartPngMusic(DWORD status);
char* GetSmartPngPoint(DWORD status);
char* GetSmartPngOnOff(DWORD status);
char* GetSmartAutoOnOff(DWORD status);
char* GetSmartPngSelect(DWORD status);

void* SmartTimerThread(void* pParam);

BOOL SmartComSend(char* buf, int len);
void SmartSendCmd(char* buf, int len);
void* SmartSendThread(void* pParam);
void* SmartComThread(void* pParam);
void SmartComRecv(ECB_DATA* pECB);
void DPCreateSmartSend();
void DPCreateSmartCom();
