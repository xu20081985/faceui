#pragma once

#pragma pack(1)
typedef struct
{
	WORD head;				// ֡ͷ  0xA55A
	BYTE length;			// ֡�ܳ��� - sizeof(head) - sizeof(length)
	WORD src;				// Դ��ַ
	WORD dst;				// Ŀ���ַ
	BYTE type;				// ��������
	BYTE cmd;				// ��������
	BYTE data[];			// ����
//  BYTE check;				// У���
}ECB_DATA;

typedef struct
{
	WORD addr;				// ���ַ(2 Byte)	����ǳ������ƣ�Ϊ0xFFFF, ���Բ����жϴ˱��������� src ���ж�Ϊ�ĸ�����
	BYTE cmd;				// ���Ʒ�ʽ
	WORD param;				// ���Ʋ���
}ECB_CTRL_ACK;


// 2018.2.26���
typedef struct
{
	WORD addr;				// ���ַ(2 Byte)	����ǳ������ƣ�Ϊ0xFFFF, ���Բ����жϴ˱��������� src ���ж�Ϊ�ĸ�����
	BYTE param1;		    // ���Ʋ���1
	BYTE param2;			// ���Ʋ���2
	BYTE param3;		    // ���Ʋ���3
	BYTE param4;		    // ���Ʋ���4
	BYTE param5;		    // ���Ʋ���5
	BYTE param6;			// ���Ʋ���6
	BYTE param7;		    // ���Ʋ���7
	BYTE param8;		    // ���Ʋ���8
	BYTE param9;		    // ���Ʋ���9
	BYTE param10;		    // ���Ʋ���10
	BYTE param11;		    // ���Ʋ���11
	BYTE param12;			// ���Ʋ���12
	BYTE param13;		    // ���Ʋ���13
	BYTE param14;		    // ���Ʋ���14
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
	WORD type;				// ����
	WORD addr;				// ���ַ
	WORD phy_addr;			// ���������ַ
	WORD param0;			// ����
	WORD param1;			// ����1
	char name[32];			// ����
	WORD status;			// ״̬
	BOOL exist;				// �Ƿ����
	WORD scene_status;
}SmartDev;

typedef struct
{
	BYTE scene;				// �������
	BYTE num;				// �豸���
	BYTE cmd;				// ��������
	WORD param;				// ���Ʋ���
}SceneCtrl;
#pragma pack()

// ֡ͷ����
const WORD ECB_HEAD	=				0xA55A;
const BYTE ECB_DATA_HEAD =			7;		// sizeof(ECB_DATA) - sizeof(head) - sizeof(length) + 1
const BYTE ECB_HEAD_LENGTH = 		2;		// sizeof(head)
const BYTE ECB_DATA_MIN_LEN	=		10;		// sizeof(ECB_DATA) + sizeof(check_num)
const BYTE ECB_HEAD_LEN_LENGTH = 	3;		// sizeof(head) + sizeof(length)

// ��������
const WORD ECB_DEV_TYPE	=			0x0228;

// ÿ֡�����38���ֽڣ�ECB�������磩
#define MAX_ECB_DATA_LEN			38

// ����
#define ECB_TYPE_ATTR				0x01
// ����
#define ECB_TYPE_CTRL				0x02
// ͬ��
#define ECB_TYPE_SYNC				0x03
// ȫ��
#define ECB_TYPE_ALL				0x0A


// ���������������������
#define ECB_SEARCH_DEV				0x01
#define ECB_SEARCH_DEV_ACK			0x02
// ���������Ӧ��
#define ECB_SEARCH_DEV_ACK2			0xA0
// �������������������������
#define ECB_SEARCH_DEV_END			0xA1
// �����������ϱ�
#define ECB_REPORT_INFO				0x03
#define ECB_REPORT_INFO_ACK			0x04
// ����������������������ַ
#define ECB_SET_ADDR				0x05
#define ECB_SET_ADDR_ACK			0x06
// �����������ѯ���������ַ
#define ECB_GET_ADDR				0x09
#define ECB_GET_ADDR_ACK			0x0A
// �����������������֮ͨ·���ַ
#define ECB_SET_GROUP_ADDR			0x13
#define ECB_SET_GROUP_ADDR_ACK		0x14
// �����������ѯ����֮ͨ·���ַ
#define ECB_GET_GROUP_ADDR			0x15
#define ECB_GET_GROUP_ADDR_ACK		0x16
// ������������ó���ģʽ����
#define ECB_SET_SCENE				0x25
#define ECB_SET_SCENE_ACK			0x26
// �����������ѯ����ģʽ����
#define ECB_GET_SCENE				0x27
#define ECB_GET_SCENE_ACK			0x28
// �����������������ʱ��
#define ECB_SET_TIME				0x61
#define ECB_SET_TIME_ACK			0x62
// �����������ѯ����ʱ��
#define ECB_GET_TIME				0x63
#define ECB_GET_TIME_ACK			0x64
// ������������ö�ʱ����
#define ECB_SET_TIMER				0xBA
#define ECB_SET_TIMER_ACK			0xBB
// �����������ѯ��ʱ����
#define ECB_GET_TIMER				0xBC
#define ECB_GET_TIMER_ACK			0xBD
// �����������������֮������Ϣ
#define ECB_SET_DEV_CFG				0xBE
#define ECB_SET_DEV_CFG_ACK			0xBF
// �����������ѯ����֮������Ϣ
#define ECB_GET_DEV_CTRL			0xCA
#define ECB_GET_DEV_CTRL_ACK		0xCB
#define ECB_GET_DEV_CTRL_ACK2		0xCC	// �������յ�Ӧ��󣬲��ܹ�������һ�������
// ���������ECB��������������ʼ��
#define ECB_INIT_BY_ADDR			0x0C
#define ECB_INIT_BY_ADDR_ACK		0x0D
#define ECB_INIT_BY_ID				0xAE
#define ECB_INIT_BY_ID_AKC			0xAF
// ���������ECB��������������λ
#define ECB_RESET_BY_ADDR			0x0A
#define ECB_RESET_BY_ADDR_ACK		0x0B
#define ECB_RESET_BY_ID				0xAC
#define ECB_RESET_BY_ID_ACK			0xAD

// ������������������ַ����������������
#define ECB_CTRL_DEV				0x01
#define ECB_CTRL_DEV_ACK			0x02


// �������������������ͬ������/����/ʱ��
#define ECB_SYNC_TIME				0x30
#define ECB_SYNC_TIME_ACK			0x31
// ��ѯ��������״̬(���ַ)
#define ECB_GET_STATUS				0x80
#define ECB_GET_STATUS_ACK			0x81

// 2018.2.26��� ��ѯ�յ�״̬
#define ECB_GET_STATUS_AIR          0x82
#define ECB_GET_STATUS_AIR_ACK      0x83

// �����������ϱ�ͨ·����״̬(���ַ)
#define ECB_SYNC_STATUS				0x91
#define ECB_SYNC_STATUS_ACK			0x92

// �������������������������ʼ��
#define ECB_INIT_ALL				0x01
#define ECB_INIT_ALL_ACK			0x02
// �������������������������λ
#define ECB_RESET_ALL				0x03
#define ECB_RESET_ALL_ACK			0x04


// ҳ��
#define MAX_PAGE_NUM				16
// ÿһҳ��ͼ������
#define MAX_ICON_NUM				6
// ״̬����
#define MSG_SMART_UPDATE			1
// ��Ч��ַ
#define INVALID_ECB_ADDR			0xFFFF
// ������ַ
#define SCENE_GRUOP_ADDR			0xFFFF

#define AIR_TYPE                    1   // 2018.2.24��ӣ�����˵�������ǿյ�
#define MUSIC_TYPE                  2   // 2018.2.24��ӣ�����˵������������
#define ALL_TYPE                    3   // 2018.2.26��ӣ�����˵������������

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
   
//	ST_POINT = 301,   
	
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
	SCMD_SCENE,							// ��������(1��255, ���ַΪ0xFFFF)
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

/******* �յ�״̬ *****************************************************************************************************/
#define AC_MODE_MAX				14
#define AC_FUNC_MAX				4
#define AC_SPEED_MAX			7
#define AC_TEMPRATRUE_MAX		22
#define AC_TEMPRATURE_INDEX		14

#define AC_STATUS_ON			1
#define AC_STATUS_OFF			2

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

#define MUSIC_VOICE_MAX         31

/**********************************************************************************************************************/

/******* �·�״̬ *****************************************************************************************************/
#define NEWWIND_CO2           0
#define NEWWIND_TEMPRATURE    14
/**********************************************************************************************************************/

void InitSmartDev();
void ResetSmartDev();
void UpdateSmartDevSet();

void DeleteSmartTimer();


DWORD GetSmartAddr();
void SetSmartAddr(DWORD addr);
void AddSmartDev(ECB_DATA* pECB);
void AddSceneCtrlList(ECB_DATA* pECB);

void AddSmartTimer(ECB_DATA* pECB);     // 2018.1.15���      
void Get_Local_Time();                  // 2018.1.25��� �������Զ���ȡ����ʱ�书��



void AddSmartAddrList(ECB_DATA* pECB);
/*const*/ SmartDev* GetSmartDev(DWORD* dwCount);
void SetStatusByScene(BYTE index);
void SetStatusByList(WORD addr, BYTE scene);
void SetStatusByAck(DWORD addr, DWORD cmd, DWORD param);
void SetStatusBySync(DWORD addr, DWORD cmd, DWORD param);

char* GetSmartPng(DWORD type, DWORD status);
char* GetSmartPngMusic(DWORD status);
char* GetSmartPngPoint(DWORD status);
char* GetSmartPngOnOff(DWORD status);

void* MSG_TIMR(void* pParam);




