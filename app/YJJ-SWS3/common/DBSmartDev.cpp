#include "roomlib.h"
#include "SmartConfig.h"
#include "CCtrlModules.h"
#include <string.h>

#define	SMART_DEV_FILE0		"SmartDev0.ext"
#define	SMART_DEV_FILE1		"SmartDev1.ext"
#define	SMART_DEV_ENDID		0x86459358

extern Time_Link_List* g_TimeHead;          // 2018.1.15ͷ������ݵ�ָ��(��ʱ�¼�)
extern char *g_pData[12];      
extern BOOL Flag[12];
extern int clear_open_flag; 				// 2018.3.9������ڽ��0x01����100%������

typedef struct
{
	DWORD	count;
	WORD	phy_addr[MAX_PAGE_NUM * MAX_ICON_NUM];      //�����ַ�ռ䡣
	WORD	group_addr[MAX_PAGE_NUM * MAX_ICON_NUM];    //�������ַ�ռ䡣
}SmartAddrList;

typedef struct
{
	DWORD		addr;                                   //��ַ
	DWORD		count;                                  //�豸����                       
	SmartDev	device[MAX_PAGE_NUM * MAX_ICON_NUM];    //�豸
	DWORD		dwSceneCtrl;
	BOOL		bDefault;
	DWORD		VERSION;                                //�汾
	DWORD		Endid;
}SmartDevSet;

// 2018.1.16��ӣ����ڶ�ʱ�¼����νṹ��
typedef struct
{
	BYTE hour;
	BYTE minute;
	BYTE data;
	WORD device;
	BYTE ctr_mode;
	WORD ctr_param;
	int  num;
}SetTimer;

void Creat_Node(SetTimer*  setTimer);
static StaticLock		g_CS;
static SmartAddrList*	g_pAddrList;
static DWORD			g_dwMaxScene;
static SceneCtrl*		g_pSceneCtrl;
SmartDevSet*			g_pSmartDevSet;                  //���ܼҾ��豸���á�

//======================================================
//** ��������: UpdateSmartDevSet
//** ��������: ���¼Ҿ��豸����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
void UpdateSmartDevSet()
{
	SmartDevSet* pSet;
	BOOL ret;

	pSet = (SmartDevSet*)malloc(sizeof(SmartDevSet) + sizeof(SceneCtrl) * g_pSmartDevSet->dwSceneCtrl);
	if(NULL == pSet)
		return;

	memcpy(pSet, g_pSmartDevSet, sizeof(SmartDevSet));
	memcpy((char*)pSet + sizeof(SmartDevSet), g_pSceneCtrl, sizeof(SceneCtrl) * g_pSmartDevSet->dwSceneCtrl);

	if(g_pSmartDevSet->VERSION & 1)
		ret = WriteServerFile(SMART_DEV_FILE1, sizeof(SmartDevSet) + sizeof(SceneCtrl) * g_pSmartDevSet->dwSceneCtrl, (char*)pSet);
	else
		ret = WriteServerFile(SMART_DEV_FILE0, sizeof(SmartDevSet) + sizeof(SceneCtrl) * g_pSmartDevSet->dwSceneCtrl, (char*)pSet);
	if(!ret)
		free(pSet);
	else
	{
		g_pSmartDevSet->VERSION++;
	}
}

//======================================================
//** ��������: InitDefaultSmartDevSet
//** ��������: ��ʼ��Ĭ�ϼҾ��豸����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void InitDefaultSmartDevSet()
{
#if 0	
	DWORD page;

	page = 0 * MAX_ICON_NUM;
	// �ƹ�
	g_pSmartDevSet->device[page + 2].exist = TRUE;
	g_pSmartDevSet->device[page + 2].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 2].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[page + 2].name, GetStringByID(2003));

	page = 1 * MAX_ICON_NUM;
	// �ƹ�
	g_pSmartDevSet->device[page + 0].exist = TRUE;
	g_pSmartDevSet->device[page + 0].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 0].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[page + 0].name, GetStringByID(2003));	
	// ����
	g_pSmartDevSet->device[page + 1].exist = TRUE;
	g_pSmartDevSet->device[page + 1].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 1].type = ST_DIMMER_A;
	strcpy(g_pSmartDevSet->device[page + 1].name, GetStringByID(2000));

	page = 2 * MAX_ICON_NUM;
	// �ƹ�
	g_pSmartDevSet->device[page + 0].exist = TRUE;
	g_pSmartDevSet->device[page + 0].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 0].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[page + 0].name, GetStringByID(2003));	
	// ����
	g_pSmartDevSet->device[page + 1].exist = TRUE;
	g_pSmartDevSet->device[page + 1].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 1].type = ST_DIMMER_A;
	strcpy(g_pSmartDevSet->device[page + 1].name, GetStringByID(2000));
	// ����
	g_pSmartDevSet->device[page + 2].exist = TRUE;
	g_pSmartDevSet->device[page + 2].type = ST_CURTAIN_A;
	g_pSmartDevSet->device[page + 2].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 2].name, GetStringByID(2016));

	page = 3 * MAX_ICON_NUM;
	// �ƹ�
	g_pSmartDevSet->device[page + 0].exist = TRUE;
	g_pSmartDevSet->device[page + 0].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 0].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[page + 0].name, GetStringByID(2003));	
	// ����
	g_pSmartDevSet->device[page + 2].exist = TRUE;
	g_pSmartDevSet->device[page + 2].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 2].type = ST_DIMMER_A;
	strcpy(g_pSmartDevSet->device[page + 2].name, GetStringByID(2000));
	// ����
	g_pSmartDevSet->device[page + 4].exist = TRUE;
	g_pSmartDevSet->device[page + 4].type = ST_CURTAIN_A;
	g_pSmartDevSet->device[page + 4].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 4].name, GetStringByID(2016));
	// ��ů
	g_pSmartDevSet->device[page + 5].exist = TRUE;
	g_pSmartDevSet->device[page + 5].type = ST_HEAT_A;
	g_pSmartDevSet->device[page + 5].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 5].name, GetStringByID(2009));

	page = 5 * MAX_ICON_NUM;
	// �ƹ�
	g_pSmartDevSet->device[page + 0].exist = TRUE;
	g_pSmartDevSet->device[page + 0].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 0].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[page + 0].name, GetStringByID(2003));	
	// ����
	g_pSmartDevSet->device[page + 1].exist = TRUE;
	g_pSmartDevSet->device[page + 1].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 1].type = ST_DIMMER_A;
	strcpy(g_pSmartDevSet->device[page + 1].name, GetStringByID(2000));
	// ����
	g_pSmartDevSet->device[page + 2].exist = TRUE;
	g_pSmartDevSet->device[page + 2].type = ST_CURTAIN_A;
	g_pSmartDevSet->device[page + 2].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 2].name, GetStringByID(2016));
	// ��ů
	g_pSmartDevSet->device[page + 3].exist = TRUE;
	g_pSmartDevSet->device[page + 3].type = ST_HEAT_A;
	g_pSmartDevSet->device[page + 3].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 3].name, GetStringByID(2009));
	// �յ�
	g_pSmartDevSet->device[page + 4].exist = TRUE;
	g_pSmartDevSet->device[page + 4].type = ST_AC_A;
	g_pSmartDevSet->device[page + 4].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 4].name, GetStringByID(2007));

	page = 8 * MAX_ICON_NUM;
	// �ƹ�
	g_pSmartDevSet->device[page + 0].exist = TRUE;
	g_pSmartDevSet->device[page + 0].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 0].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[page + 0].name, GetStringByID(2003));	
	// ����
	g_pSmartDevSet->device[page + 1].exist = TRUE;
	g_pSmartDevSet->device[page + 1].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 1].type = ST_DIMMER_A;
	strcpy(g_pSmartDevSet->device[page + 1].name, GetStringByID(2000));
	// ����
	g_pSmartDevSet->device[page + 2].exist = TRUE;
	g_pSmartDevSet->device[page + 2].type = ST_CURTAIN_A;
	g_pSmartDevSet->device[page + 2].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 2].name, GetStringByID(2016));
	// �յ�
	g_pSmartDevSet->device[page + 3].exist = TRUE;
	g_pSmartDevSet->device[page + 3].type = ST_AC_A;
	g_pSmartDevSet->device[page + 3].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 3].name, GetStringByID(2007));
	// ����
	g_pSmartDevSet->device[page + 4].exist = TRUE;
	g_pSmartDevSet->device[page + 4].type = ST_MUSIC_A;
	g_pSmartDevSet->device[page + 4].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 4].name, GetStringByID(2012));
    // �������
	g_pSmartDevSet->device[page + 5].exist = TRUE;
	g_pSmartDevSet->device[page + 5].type = ST_TV_A;
	g_pSmartDevSet->device[page + 5].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 5].name, GetStringByID(2013));

	page = 12 * MAX_ICON_NUM;
	g_pSmartDevSet->device[page + 0].exist = TRUE;
	g_pSmartDevSet->device[page + 0].type = ST_SCENE_F;
	g_pSmartDevSet->device[page + 0].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 0].name, GetStringByID(3000));

	g_pSmartDevSet->device[page + 1].exist = TRUE;
	g_pSmartDevSet->device[page + 1].type = ST_SCENE_G;
	g_pSmartDevSet->device[page + 1].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 1].name, GetStringByID(3001));

	g_pSmartDevSet->count = 23;
	g_pSmartDevSet->addr = INVALID_ECB_ADDR;

	g_pSmartDevSet->bDefault = TRUE;
	g_pSmartDevSet->Endid = SMART_DEV_ENDID;

	memset(g_pAddrList, 0, sizeof(SmartAddrList));	

#else	
	// ��һҳ �����
	g_pSmartDevSet->device[1].exist = TRUE;
	g_pSmartDevSet->device[1].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[1].type = ST_DIMMER_A;
	strcpy(g_pSmartDevSet->device[1].name, GetStringByID(2000));
#if 0
	g_pSmartDevSet->device[1].exist = TRUE;
	g_pSmartDevSet->device[1].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[1].type = ST_DIMMER_B;
	strcpy(g_pSmartDevSet->device[1].name, GetStringByID(2001)); 

	g_pSmartDevSet->device[2].exist = TRUE;
	g_pSmartDevSet->device[2].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[2].type = ST_DIMMER_C;
	strcpy(g_pSmartDevSet->device[2].name, GetStringByID(2002)); 
#endif
   // �ƹ�
	g_pSmartDevSet->device[0].exist = TRUE;
	g_pSmartDevSet->device[0].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[0].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[0].name, GetStringByID(2003));
#if 0
	g_pSmartDevSet->device[4].exist = TRUE;
	g_pSmartDevSet->device[4].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[4].type = ST_LIGHT_B;
	strcpy(g_pSmartDevSet->device[4].name, GetStringByID(2004));

	g_pSmartDevSet->device[5].exist = TRUE;
	g_pSmartDevSet->device[5].type = ST_LIGHT_C;
	g_pSmartDevSet->device[5].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[5].name, GetStringByID(2005)); 

	// �ڶ�ҳ
	g_pSmartDevSet->device[6].exist = TRUE;
	g_pSmartDevSet->device[6].type = ST_LIGHT_D;
	g_pSmartDevSet->device[6].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[6].name, GetStringByID(2006));
#endif
	// �յ�
	g_pSmartDevSet->device[3].exist = TRUE;
	g_pSmartDevSet->device[3].type = ST_AC_A;
	g_pSmartDevSet->device[3].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[3].name, GetStringByID(2007));

#if 1
	g_pSmartDevSet->device[8].exist = TRUE;
	g_pSmartDevSet->device[8].type = ST_AC_B;
	g_pSmartDevSet->device[8].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[8].name, GetStringByID(2008));
#endif
    // ��ů
	g_pSmartDevSet->device[4].exist = TRUE;
	g_pSmartDevSet->device[4].type = ST_HEAT_A;
	g_pSmartDevSet->device[4].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[4].name, GetStringByID(2009));
    // �·�
//	g_pSmartDevSet->device[10].exist = TRUE;
//	g_pSmartDevSet->device[10].type = ST_WIND_A;
//	g_pSmartDevSet->device[10].addr = INVALID_ECB_ADDR;
//	strcpy(g_pSmartDevSet->device[10].name, GetStringByID(2010));
#if 0
	g_pSmartDevSet->device[11].exist = TRUE;
	g_pSmartDevSet->device[11].type = ST_WIND_B;
	g_pSmartDevSet->device[11].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[11].name, GetStringByID(2011));
#endif
	// ����ҳ
	// ����
	g_pSmartDevSet->device[6].exist = TRUE;
	g_pSmartDevSet->device[6].type = ST_MUSIC_A;
	g_pSmartDevSet->device[6].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[6].name, GetStringByID(2012));
    // �������
	g_pSmartDevSet->device[7].exist = TRUE;
	g_pSmartDevSet->device[7].type = ST_TV_A;
	g_pSmartDevSet->device[7].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[7].name, GetStringByID(2013));
#if 0
	g_pSmartDevSet->device[14].exist = TRUE;
	g_pSmartDevSet->device[14].type = ST_LIGHT_A;
	g_pSmartDevSet->device[14].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[14].name, GetStringByID(2014));

	g_pSmartDevSet->device[15].exist = TRUE;
	g_pSmartDevSet->device[15].type = ST_WINDOW_A;
	g_pSmartDevSet->device[15].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[15].name, GetStringByID(2015));
#endif
    // ����
	g_pSmartDevSet->device[2].exist = TRUE;
	g_pSmartDevSet->device[2].type = ST_CURTAIN_A;
	g_pSmartDevSet->device[2].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[2].name, GetStringByID(2016));
#if 0
	g_pSmartDevSet->device[17].exist = TRUE;
	g_pSmartDevSet->device[17].type = ST_CURTAIN_D;
	g_pSmartDevSet->device[17].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[17].name, GetStringByID(2017));
#endif
	// ����ҳ
	// ����
//	g_pSmartDevSet->device[18].exist = TRUE;
//	g_pSmartDevSet->device[18].type = ST_OUTLET_A;
//	g_pSmartDevSet->device[18].addr = INVALID_ECB_ADDR;
//	strcpy(g_pSmartDevSet->device[18].name, GetStringByID(2018));

	g_pSmartDevSet->device[9].exist = TRUE;
	g_pSmartDevSet->device[9].type = ST_SCENE_F;
	g_pSmartDevSet->device[9].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[9].name, GetStringByID(3000));

	g_pSmartDevSet->device[10].exist = TRUE;
	g_pSmartDevSet->device[10].type = ST_SCENE_G;
	g_pSmartDevSet->device[10].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[10].name, GetStringByID(3001));
		
	g_pSmartDevSet->device[5].exist = TRUE;
	g_pSmartDevSet->device[5].type = ST_WIND_A;
	g_pSmartDevSet->device[5].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[5].name, GetStringByID(2010));
	
	g_pSmartDevSet->count = 11;
	g_pSmartDevSet->addr = INVALID_ECB_ADDR;

	g_pSmartDevSet->bDefault = TRUE;
	g_pSmartDevSet->Endid = SMART_DEV_ENDID;

	memset(g_pAddrList, 0, sizeof(SmartAddrList));

	//g_pSmartDevSet->device[19].exist = TRUE;
	//g_pSmartDevSet->device[19].type = ST_SCENE_A;
	//g_pSmartDevSet->device[19].addr = INVALID_ECB_ADDR;
	//strcpy(g_pSmartDevSet->device[19].name, GetStringByID(3000));

	//g_pSmartDevSet->device[20].exist = TRUE;
	//g_pSmartDevSet->device[20].type = ST_SCENE_B;
	//g_pSmartDevSet->device[20].addr = INVALID_ECB_ADDR;
	//strcpy(g_pSmartDevSet->device[20].name, GetStringByID(3001));

	//g_pSmartDevSet->device[21].exist = TRUE;
	//g_pSmartDevSet->device[21].type = ST_SCENE_C;
	//g_pSmartDevSet->device[21].addr = INVALID_ECB_ADDR;
	//strcpy(g_pSmartDevSet->device[21].name, GetStringByID(3002));

	//g_pSmartDevSet->device[22].exist = TRUE;
	//g_pSmartDevSet->device[22].type = ST_SCENE_D;
	//g_pSmartDevSet->device[22].addr = INVALID_ECB_ADDR;
	//strcpy(g_pSmartDevSet->device[22].name, GetStringByID(3003));

	//g_pSmartDevSet->device[23].exist = TRUE;
	//g_pSmartDevSet->device[23].type = ST_SCENE_E;
	//g_pSmartDevSet->device[23].addr = INVALID_ECB_ADDR;
	//strcpy(g_pSmartDevSet->device[23].name, GetStringByID(3004));

	//g_pSmartDevSet->count = 24;
	//g_pSmartDevSet->addr = INVALID_ECB_ADDR;
	//g_pSmartDevSet->Endid = SMART_DEV_ENDID;
#endif	
}

//======================================================
//** ��������: InitSmartDev
//** ��������: ��ʼ���Ҿ��豸
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
void InitSmartDev()
{
	FILE* fd;
	char filename[64];
	SmartDevSet* pSet0 = NULL;
	SmartDevSet* pSet1 = NULL;
	SceneCtrl* pSceneCtrl0 = NULL;    // ����
	SceneCtrl* pSceneCtrl1 = NULL;

	g_CS.lockon();
	// �����������ַ�����ַ��Ӧ��
	g_pAddrList = (SmartAddrList *)malloc(sizeof(SmartAddrList));
	if(g_pAddrList)   
	{
		memset(g_pAddrList, 0, sizeof(SmartAddrList));
	}
	// ��ȡ�����ļ�
	sprintf(filename, "%s/%s", USERDIR, SMART_DEV_FILE0);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet0 = (SmartDevSet*)malloc(sizeof(SmartDevSet));
		if(pSet0)
		{
			memset(pSet0, 0, sizeof(SmartDevSet));
			int ret = fread(pSet0, 1, sizeof(SmartDevSet), fd);
			if(ret != sizeof(SmartDevSet))
			{
				free(pSet0);
				pSet0 = NULL;
			}
			else
			{
				if(pSet0->Endid != SMART_DEV_ENDID)
				{
					free(pSet0);
					pSet0 = NULL;
				}
				else
				{
					if(pSet0->dwSceneCtrl > 0)
					{
						pSceneCtrl0 = (SceneCtrl *)malloc(sizeof(SceneCtrl) * pSet0->dwSceneCtrl);
						if(pSceneCtrl0)
						{
							int ret = fread(pSceneCtrl0, 1, sizeof(SceneCtrl) * pSet0->dwSceneCtrl, fd);
							if(ret != sizeof(SceneCtrl) * pSet0->dwSceneCtrl)
							{
								free(pSceneCtrl0);
								pSceneCtrl0 = NULL;
								free(pSet0);
								pSet0 = NULL;
							}
						}
					}
				}
			}
		}
		fclose(fd);
	}

	sprintf(filename, "%s/%s", USERDIR, SMART_DEV_FILE1);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet1 = (SmartDevSet*)malloc(sizeof(SmartDevSet));
		if(pSet1)
		{
			memset(pSet1, 0, sizeof(SmartDevSet));
			int ret = fread(pSet1, 1, sizeof(SmartDevSet), fd);
			if(ret != sizeof(SmartDevSet))
			{
				free(pSet1);
				pSet1 = NULL;
			}
			else
			{
				if(pSet1->Endid != SMART_DEV_ENDID)
				{
					free(pSet1);
					pSet1 = NULL;
				}
				else
				{
					if(pSet1->dwSceneCtrl > 0)
					{
						pSceneCtrl1 = (SceneCtrl *)malloc(sizeof(SceneCtrl) * pSet1->dwSceneCtrl);
						if(pSceneCtrl1)
						{
							int ret = fread(pSceneCtrl1, 1, sizeof(SceneCtrl) * pSet1->dwSceneCtrl, fd);
							if(ret != sizeof(SceneCtrl) * pSet1->dwSceneCtrl)
							{
								free(pSceneCtrl1);
								pSceneCtrl1 = NULL;
								free(pSet1);
								pSet1 = NULL;
							}
						}
					}
				}
			}
		}
		fclose(fd);
	}
    // ����ò����ϵͳ�ļ�ʲô��û�е�ʱ���ʼ��Ĭ�ϵ��豸  
	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		g_pSmartDevSet = (SmartDevSet *)malloc(sizeof(SmartDevSet));
		if(g_pSmartDevSet)
		{
			memset(g_pSmartDevSet, 0, sizeof(SmartDevSet));
			InitDefaultSmartDevSet();     //����ò�ƾ��Ǻ�APP�ϵ��豸ͬ���ˡ�
			UpdateSmartDevSet();          //������д�뵽�ļ�����
		}
	}
	else
	{
		if((pSet0 != NULL) && (pSet1 != NULL))
		{
			if(pSet0->VERSION > pSet1->VERSION)
			{
				g_pSmartDevSet = pSet0;
				g_pSceneCtrl = pSceneCtrl0;
				free(pSet1);
				free(pSceneCtrl1);
			}
			else
			{
				g_pSmartDevSet = pSet1;
				g_pSceneCtrl = pSceneCtrl1;
				free(pSet0);
				free(pSceneCtrl0);
			}
		}
		else if(pSet0 != NULL)
		{
			g_pSmartDevSet = pSet0;
			g_pSceneCtrl = pSceneCtrl0;
		}
		else
		{
			g_pSmartDevSet = pSet1;
			g_pSceneCtrl = pSceneCtrl1;
		}
		g_pSmartDevSet->VERSION++;
	}

	g_dwMaxScene = g_pSmartDevSet->dwSceneCtrl;
	// ����״̬����
	for(int i = 0; i < MAX_PAGE_NUM * MAX_ICON_NUM; i++) {

		if(g_pSmartDevSet->device[i].type >= ST_AC_A 
					&&g_pSmartDevSet->device[i].type<=ST_MUSIC_D) {

			g_pSmartDevSet->device[i].status = 0x8000;
			g_pSmartDevSet->device[i].scene_status = 0;

		}

		else {

			g_pSmartDevSet->device[i].status = 0;
			g_pSmartDevSet->device[i].scene_status = 0;
		}
			
	}
		

	g_CS.lockoff();
}

//======================================================
//** ��������: ResetSmartDev
//** ��������: ��λ�Ҿ��豸����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
void ResetSmartDev()                         //ò����һ���ָ�����������صĺ�����
{
	g_CS.lockon();
	memset(g_pSmartDevSet, 0, sizeof(SmartDevSet));
	InitDefaultSmartDevSet();
	UpdateSmartDevSet();
	DeleteServerFile(SMART_DEV_FILE1);
	g_CS.lockoff();
}

//======================================================
//** ��������: SetSmartAddr
//** ��������: ���üҾ������ַ
//** �䡡��: addr
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
void SetSmartAddr(DWORD addr)
{
	g_CS.lockon();
	g_pSmartDevSet->addr = addr;
	UpdateSmartDevSet();
	g_CS.lockoff();
}

//======================================================
//** ��������: GetSmartAddr
//** ��������: ��ȡ�Ҿ������ַ
//** �䡡��: addr
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
DWORD GetSmartAddr()
{
	return g_pSmartDevSet->addr;
}

//======================================================
//** ��������: GetPhyByGruop
//** ��������: ͨ�����ַ��ȡ�����ַ
//** �䡡��: addr
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static WORD GetPhyByGruop(WORD addr)
{
	for(int i = 0; i < g_pAddrList->count; i++)
	{
		if(addr == g_pAddrList->group_addr[i])
		{
			return g_pAddrList->phy_addr[i];
		}
	}

	DBGMSG(DPERROR, "GetPhyAddr fail\r\n");
	return 0;
}

/*
 *2018.1.16����
 *����:������λ�ظ�����ѡ���־λ
 */
//======================================================
//** ��������: Data_Choice
//** ��������: ��ʱ������ѡ
//** �䡡��: flag
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void Data_Choice(BYTE flag)
{
	if((flag & 0x80) == 0) { // �ǵ�������

		if ((flag & 0x7F) == 0x7F) {
			memset(Flag, FALSE, sizeof(Flag));
			Flag[7] = TRUE;
			return;
		}

		if ((flag & 0x1F) == 0x1F) {
			memset(Flag, FALSE, sizeof(Flag));
			Flag[8] = TRUE;
			for (int i = 6; i < 7; i++) {	
				if (flag & (1<<i))		
					Flag[i] = TRUE;
				else		
					Flag[i] = FALSE;
			}
			return;
		}

		if ((flag & 0x60) == 0x60) {
			memset(Flag, FALSE, sizeof(Flag));
			Flag[9] = TRUE;
			for (int i = 0; i < 5; i++) {	
				if (flag & (1<<i))		
					Flag[i] = TRUE;
				else		
					Flag[i] = FALSE;
			}
			return;
		}

		for(int i = 0; i<7; i++) {
			if(flag & (1<<i))
				Flag[i] = TRUE;
			else
				Flag[i] = FALSE;
		}
	}

	else if(flag & 0x80) {

		Flag[10] = TRUE;
	}
}

/* 
 * 2018.1.15 ��ʼ
 * ����:����һ���½ڵ�
 *
 *minute,second:��ʱʱ�䣬data:�ظ�ʱ�䣬device:�����豸,
 *ctr_mode:���Ʒ�ʽ��ctr_param:���Ʋ���
 */
//======================================================
//** ��������: Creat_Node
//** ��������: ��������ڵ�
//** �䡡��: setTimer
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void Creat_Node(SetTimer*  setTimer)
{  
	char buf[20];
	pNode P_Head = (pNode)malloc(sizeof(LNode)); //Ϊͷ�ڵ�����ڴ�ռ䣬�����뵽���ڴ浱���½ڵ�
	pNode P_Mid;								  //Ϊ�м����

	if(NULL == P_Head) {						  //��Ϊ��ָ��������ڴ�ʧ��

		return ;
	}
    memset(P_Head, 0, sizeof(LNode ));            //�������뵽�Ķ��ڴ�

	P_Head->next = NULL;						  //����һָ�븳ֵΪ��(��Ϊβ�ڵ�),����ָ����һ�ڵ��׵�ַ		
    P_Head->show = TRUE;                          //��ʱ��Ĭ�ϱ�����     
		
/*************�������ݸ�ֵ����,�����Ч����***************/ 
	           
	/* ʱ���ַ�����ֵ */
	if(setTimer->hour >=10) {
	
		if(setTimer->minute >= 10)
			sprintf(P_Head->Time,"%d:%d",setTimer->hour,setTimer->minute); 
		else if(setTimer->minute < 10)
			sprintf(P_Head->Time,"%d:0%d",setTimer->hour,setTimer->minute); 
	}
	
	else if(setTimer->hour <10) {

		if(setTimer->minute >= 10)
			sprintf(P_Head->Time,"0%d:%d",setTimer->hour,setTimer->minute); 
		else if(setTimer->minute < 10)
			sprintf(P_Head->Time,"0%d:0%d",setTimer->hour,setTimer->minute); 
	}

#if 1	
	/* �ظ����ڹ�ѡ��ǣ�������ʾ�ظ����� */
	Data_Choice(setTimer->data);
	
	for(int k = 0; k<12; k++) {                       

		P_Head->choose_data[k] = Flag[k];

		if(Flag[k]) {

			g_pData[k] = GetStringByID(13200+k);
			strcat(P_Head->p_Data, g_pData[k]);   // �ظ����ں�����ʾ����
		} 
			
	}
#endif

#if 1
	/* �����豸�ַ�����ֵ */ 
	for(int i = 0; i < MAX_PAGE_NUM * MAX_ICON_NUM; i++) {

		// ��ͨ�豸������ʾ
		if(setTimer->device != 0xffff) {
			
			if((setTimer->device == g_pSmartDevSet->device[i].addr) ) {

				P_Head->device = setTimer->device;   // �������ַ
				strcpy(P_Head->Device, g_pSmartDevSet->device[i].name);
				break;
			}
		}
		// �龰�豸������ʾ
		else if(setTimer->device == 0xffff) {
			if(setTimer->ctr_param == g_pSmartDevSet->device[i].addr) {
				if(g_pSmartDevSet->device[i].type >= ST_SCENE_A
					 && g_pSmartDevSet->device[i].type <= ST_SCENE_Z) { 
					 
					P_Head->device = setTimer->device;   // �������ַ
					strcpy(P_Head->Device, g_pSmartDevSet->device[i].name);
					break;

				}
			}
		}
	}        
	 
	/* �豸���Ʒ�ʽ��ֵ */

	switch(setTimer->ctr_mode) {

	case SCMD_OPEN:   // ��
		strcpy(P_Head->CTL_Type,GetStringByID(13401));
		break;
	
	case SCMD_CLOSE:  // ��
		strcpy(P_Head->CTL_Type,GetStringByID(13402));
		break;
		
		default:
			break;
	}
	P_Head->ctr_mode = setTimer->ctr_mode;   	// ��������
	
	/* ���Ʋ�����ֵ */
	P_Head->ctr_param = setTimer->ctr_param;    
#endif
/**********************************************************/

	if(NULL == g_TimeHead) {					  //�ڵ�Ϊͷ���(֤���ڴ�֮ǰû�ж�ʱ�¼�)

		g_TimeHead = P_Head;					  //������һ����ʱ�¼�			
	}

	else {										  //���ǵ�һ����ʱ�¼�

		P_Mid = g_TimeHead;

		while(NULL != P_Mid->next) {

			P_Mid = P_Mid->next;		
		}

		P_Mid->next = P_Head;			 
	}
   
	UpdatSetTimer();                               //������д���ļ�����			 
}

/*
 *2018.1.15��ӣ���ʱ����
 *����ԭ��:AddSmartTimer(ECB_DATA* pECB)
 *����:�����ֻ�APP���õĶ�ʱ��Ϣ
 *��ڲ���:ECB_DATA* pECB
 *���ڲ���:��
 *����ֵ:��
 */
//======================================================
//** ��������: AddSmartTimer
//** ��������: ��ӼҾӶ�ʱ
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================  
void AddSmartTimer(ECB_DATA* pECB)
{
	SetTimer Timer;
	Timer.hour      = pECB->data[2];             // Сʱ
	Timer.minute    = pECB->data[3];             // ����
	Timer.data      = pECB->data[4];             // �ظ�����
	memcpy(&Timer.device, &pECB->data[5], 2);    // �豸����
	Timer.ctr_mode  = pECB->data[7];             // ���Ʒ�ʽ
	memcpy(&Timer.ctr_param, &pECB->data[8], 2); // ���Ʋ���

	for(int k = 0; k<12; k++) {       // ����־λ������

		Flag[k] = FALSE;
	}

	Creat_Node(&Timer);  // �����¶�ʱ�¼��Ľڵ�

}

/*
 *2018.1.19���
 *����ԭ�ͣ�FlashStatusTimer()
 *����:���ݶ�ʱʱ�䵽��ˢ��ͼ��״̬
 */
//======================================================
//** ��������: FlashStatusTimer
//** ��������: ˢ�¶�ʱ״̬
//** �䡡��: TimerEvent
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================   
void FlashStatusTimer(pNode TimerEvent)
{	
	for(int i = 0; i < g_pSmartDevSet->count; i++) {

		if(TimerEvent->device == g_pSmartDevSet->device[i].addr) {

			if(TimerEvent->ctr_mode == SCMD_OPEN) {

				if(g_pSmartDevSet->device[i].type >= ST_AC_A 
					&& g_pSmartDevSet->device[i].type <= ST_MUSIC_D
					 && g_pSmartDevSet->device[i].type != ST_AC_B) {

					g_pSmartDevSet->device[i].status &= 0x7fff;
					g_pSmartDevSet->device[i].status |= 0x4000;

				}

				else 
				   g_pSmartDevSet->device[i].status = 100;	
			}

		    else if(TimerEvent->ctr_mode == SCMD_CLOSE) {

				if(g_pSmartDevSet->device[i].type >= ST_AC_A 
					&& g_pSmartDevSet->device[i].type <= ST_MUSIC_D
					 &&g_pSmartDevSet->device[i].type != ST_AC_B) {

					g_pSmartDevSet->device[i].status &= 0xbfff;
					g_pSmartDevSet->device[i].status |= 0x8000;

				}

				else 
				   g_pSmartDevSet->device[i].status = 0;
		    }
		}
	}
}

/*
 *2018.1.18��� 
 *����ԭ��:DeleteSmartTimer()
 *��������:ɾ����ʱ��������
 */
//======================================================
//** ��������: DeleteSmartTimer
//** ��������: ɾ���ҾӶ�ʱ
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void DeleteSmartTimer()
{
	pNode p_Mid,p_Mid1;
	if(NULL == g_TimeHead)
		return;

	else {

		p_Mid = g_TimeHead;

		while (NULL != p_Mid) {
			
			p_Mid1 = p_Mid->next;
			free(p_Mid);
			p_Mid = p_Mid1;
		}

		g_TimeHead = NULL;
	}

}

//======================================================
//** ��������: AddSmartDev
//** ��������: ��ӼҾ��豸
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void AddSmartDev(ECB_DATA* pECB)
{
	WORD group_addr, phy_addr;
	// ������ӱ������
	WORD IR_TV_OL,IR_TV_CHANNEL1,IR_TV_CHANNEL2,
		 IR_TV_VOICE1,IR_TV_VOICE2,IR_TV_SWITCH;
	// ����յ��������
	WORD IR_AIR_H,IR_AIR_M,IR_AIR_L,IR_AIR_T,
		 IR_AIR_CODE,IR_AIR_HOT,IR_AIR_ONOFF;
	
	char szName[32] = "";                                   // �豸������
	BYTE num = pECB->data[1] ;								// ���(1 Byte)
	BYTE page = pECB->data[2] - 1;							// ҳ��(1 Byte)
	BYTE bit = pECB->data[3] - 1;							// λ��(1 Byte)
	BYTE type = pECB->data[4];								// ����(1 Byte)
   
	if(type == ST_TV_A) {/* ����Ǻ�������(�������) */
	
		memcpy(&group_addr, &pECB->data[5], 2);				// ���� (2��5 Byte)��������

		switch(pECB->data[7]) {
		
		case 53:  // ���ӿ�/��
			IR_TV_OL = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_TV_CODE(IR_TV_OL,0);
			break;
		case 54:  // Ƶ��+
			IR_TV_CHANNEL1 = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_TV_CODE(IR_TV_CHANNEL1,1);
			break;
		case 55:  // Ƶ��-
			IR_TV_CHANNEL2 = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_TV_CODE(IR_TV_CHANNEL2,2);
			break;
		case 56:  // ����+
			IR_TV_VOICE1 = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_TV_CODE(IR_TV_VOICE1,3);
			break;
		case 57:  // ����-
			IR_TV_VOICE2 = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_TV_CODE(IR_TV_VOICE2,4);
			break;
		case 58:  // TV/AV�л�
			IR_TV_SWITCH = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_TV_CODE(IR_TV_SWITCH,5);
			break;

		default:
			break;

		} 

		if(pECB->length - 17 <= 16 && pECB->length - 17 > 0)	{ // ��Ϣ(1��16 Byte) �ַ�ת�� Ӣ->�� 

			char szGBK[16];
			WORD szUnicode[16];

			memcpy(szGBK, &pECB->data[10], pECB->length - 17);
			szGBK[pECB->length - 17] = 0;

			GbConvert(szUnicode, (BYTE*)szGBK);
			unicode2utf8((BYTE*)szName, (wchar_t*)szUnicode);
		}
		
	} 
    // ����Ǻ���յ�����
   else	if(type == ST_AC_B) {

		memcpy(&group_addr, &pECB->data[5], 2);                 // ���ַ 

		switch(pECB->data[7]) {  

		case 1:
		case 2:
		case 3:  // ��/��
			IR_AIR_ONOFF = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_ONOFF, 0);
			break;

		case 8:  // ����
			IR_AIR_HOT = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_HOT, 1);
			break;

		case 9:  // ����
			IR_AIR_CODE = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_CODE, 2);
			break;

		case 10: // ͨ��
			IR_AIR_T = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_T, 3);
			break;

		case 24: // ���ٵ�
			IR_AIR_L = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_L, 4);
			break;

		case 25: // ������
			IR_AIR_M = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_M, 5);
			break;

		case 26: // ���ٸ� 
			IR_AIR_H = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_H, 6);
			break;

		default:
			break;
		
		}

		if(pECB->length - 17 <= 16 && pECB->length - 17 > 0)	{ // ��Ϣ(1��16 Byte) �ַ�ת�� Ӣ->�� 

			char szGBK[16];
			WORD szUnicode[16];

			memcpy(szGBK, &pECB->data[10], pECB->length - 17);
			szGBK[pECB->length - 17] = 0;

			GbConvert(szUnicode, (BYTE*)szGBK);
			unicode2utf8((BYTE*)szName, (wchar_t*)szUnicode);
		}
	}
		
	else { /* �Ǻ����豸���� */

		memcpy(&group_addr, &pECB->data[5], 2);				    // ����(2��5 Byte) ������Ͳ��Ǻ��⣬��Ϊ2���ֽڵ����ַ

		if(pECB->length - 14 <= 16 && pECB->length - 14 > 0)								// ��Ϣ(1��16 Byte)
		{
			char szGBK[16];
			WORD szUnicode[16];

			memcpy(szGBK, &pECB->data[7], pECB->length - 14);
			szGBK[pECB->length - 14] = 0;

			GbConvert(szUnicode, (BYTE*)szGBK);
			unicode2utf8((BYTE*)szName, (wchar_t*)szUnicode);
		}
	}
		
	// ͨ�����ַ��ȡ�����ַ, ������龰�������
	if(type < ST_SCENE_A)
		phy_addr = GetPhyByGruop(group_addr);                // ��ȡ�����ַ    

	if(page >= MAX_PAGE_NUM || bit >= MAX_ICON_NUM)
		return;

	g_CS.lockon();
	int index = page * MAX_ICON_NUM + bit;

	// �����ǰ��Ĭ������,�����֮ǰ������
	if(g_pSmartDevSet->bDefault)
 	{
 		g_pSmartDevSet->bDefault = FALSE;
 
 		g_pSmartDevSet->count = 0;
 		memset(g_pSmartDevSet->device, 0, sizeof(g_pSmartDevSet->device));
 	}

	if(!g_pSmartDevSet->device[index].exist)
	{
		g_pSmartDevSet->count++;
		g_pSmartDevSet->device[index].exist = TRUE;	
	}

	g_pSmartDevSet->device[index].param0 = num;
	g_pSmartDevSet->device[index].type = type;            // ����
	g_pSmartDevSet->device[index].addr = group_addr;      // ���ַ
	g_pSmartDevSet->device[index].phy_addr = phy_addr;    
	strcpy(g_pSmartDevSet->device[index].name, szName);   // �豸����

	UpdateSmartDevSet();
	g_CS.lockoff();
}

//======================================================
//** ��������: AddSceneCtrlList
//** ��������: ����龰�����б�
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void AddSceneCtrlList(ECB_DATA* pECB)
{
	WORD param;
	BYTE num =	pECB->data[0];				// �豸���(1 Byte)
	BYTE scene = pECB->data[2];				// �������(1 Byte)
	BYTE cmd =	pECB->data[3];				// ���Ʒ�ʽ(1 Byte)
	memcpy(&param, &pECB->data[4], 2);		// ���Ʋ���(2 Byte)	
	param = htons(param);
	if(cmd == 1 && param == 0)
	{
		// ��cmdΪ1ʱ�����Ʋ���0����򿪵�100%
		param = 100;
	}

	g_CS.lockon();
	if(g_pSmartDevSet->dwSceneCtrl + 1 > g_dwMaxScene)
	{
		g_dwMaxScene += 255;
		SceneCtrl* pSceneCtrl = (SceneCtrl *)realloc(g_pSceneCtrl, sizeof(SceneCtrl) * g_dwMaxScene);
		if(pSceneCtrl == NULL)
		{
			g_CS.lockoff();
			return;
		}

		g_pSceneCtrl = pSceneCtrl;
	}

	g_pSceneCtrl[g_pSmartDevSet->dwSceneCtrl].num = num;
	g_pSceneCtrl[g_pSmartDevSet->dwSceneCtrl].scene = scene;
	g_pSceneCtrl[g_pSmartDevSet->dwSceneCtrl].cmd = cmd;
	g_pSceneCtrl[g_pSmartDevSet->dwSceneCtrl].param = param;

	g_pSmartDevSet->dwSceneCtrl++;
	UpdateSmartDevSet();
	g_CS.lockoff();
}


//======================================================
//** ��������: AddSmartAddrList
//** ��������: ��ӼҾӵ�ַ�б�
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void AddSmartAddrList(ECB_DATA* pECB)
{
	g_CS.lockon();
	g_pAddrList->phy_addr[g_pAddrList->count] = pECB->dst;						// ���������ַ
	memcpy(&g_pAddrList->group_addr[g_pAddrList->count], &pECB->data[1], 2);	// ���ַ1
	g_pAddrList->count++;
	g_CS.lockoff();
}

//======================================================
//** ��������: GetSmartDev
//** ��������: ��ȡ�Ҿ��豸�б�
//** �䡡��: count
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
/*const*/ SmartDev* GetSmartDev(DWORD* count)
{
	/*const*/ SmartDev* pSmartDev = NULL;        // ����ָ������ָ��
	g_CS.lockon();
	*count = g_pSmartDevSet->count;
	pSmartDev = g_pSmartDevSet->device;
	g_CS.lockoff();
	return pSmartDev;
}

//======================================================
//** ��������: SetStatusByAck
//** ��������: ����״̬ͨ��ack
//** �䡡��: addr cmd param
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void SetStatusByAck(DWORD addr, DWORD cmd, DWORD param)
{
	g_CS.lockon();
		
	for(int i = 0; i < MAX_PAGE_NUM * MAX_ICON_NUM; i++)
	{
		if(addr == g_pSmartDevSet->device[i].addr)
		{
			int status;
			switch(cmd)
			{
				case SCMD_OPEN:

					if(g_pSmartDevSet->device[i].type >= ST_AC_A && 
					   g_pSmartDevSet->device[i].type <= ST_MUSIC_D) {

						if(param != 0)
							g_pSmartDevSet->device[i].status = param;

						else  {

							g_pSmartDevSet->device[i].status &= 0x7fff;
							g_pSmartDevSet->device[i].status |= 0x4000;
						}
							

						break;
					}

					else {

						if(param == 0) {
		
							g_pSmartDevSet->device[i].status = 100;
							clear_open_flag = 1;
						}
							
						else
							g_pSmartDevSet->device[i].status = param;
					}
					break;
					
				case SCMD_DIMMER_OPEN:
					g_pSmartDevSet->device[i].status = param;
					break;
					
				case SCMD_CLOSE:

					if(g_pSmartDevSet->device[i].type >= ST_AC_A && 
					   g_pSmartDevSet->device[i].type <= ST_MUSIC_D) {

						if(param != 0)
							g_pSmartDevSet->device[i].status = param;

						else {

							g_pSmartDevSet->device[i].status &= 0xbfff;
							g_pSmartDevSet->device[i].status |= 0x8000;
						} 
							
						break;
					}

					else {

						g_pSmartDevSet->device[i].status = 0;
					}	
					break;
				
				/* �����޸İ汾:���������ˢ�� */
				case SCMD_CURTAIN_OPEN:

					if(g_pSmartDevSet->device[i].type >= ST_AC_A && 
					   g_pSmartDevSet->device[i].type <= ST_MUSIC_D) {

						break;
					}

					else {

						g_pSmartDevSet->device[i].status = 100;
					}	
					break;

				case SCMD_CURTAIN_CLOSE:

					if(g_pSmartDevSet->device[i].type >= ST_AC_A && 
					   g_pSmartDevSet->device[i].type <= ST_MUSIC_D) {

						break;
					}

					else {

						g_pSmartDevSet->device[i].status = 0;
					}	

					break;

			    
				case SCMD_AC:

					g_pSmartDevSet->device[i].status = param;
					break;

				case SCMD_MUSIC:

					g_pSmartDevSet->device[i].status = param;
					break;		
			
				default:
					break;
			}
		//	break;
		}
	} 
	g_CS.lockoff();
}

//======================================================
//** ��������: SetStatusBySync
//** ��������: ����״̬ͨ��ͬ��
//** �䡡��: addr cmd param
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void SetStatusBySync(DWORD addr, DWORD cmd, DWORD param)
{
	g_CS.lockon();
	for(int i = 0; i < MAX_PAGE_NUM * MAX_ICON_NUM; i++)
	{
		if(addr == g_pSmartDevSet->device[i].addr)
		{
			int status;
			switch(cmd)
			{
			case SCMD_OPEN:
				g_pSmartDevSet->device[i].status = 100;
				break;
			case SCMD_DIMMER_OPEN:
				g_pSmartDevSet->device[i].status = param;
				break;
			case SCMD_CLOSE:
				g_pSmartDevSet->device[i].status = 0;
				break;
			case SCMD_CURTAIN_OPEN:
				g_pSmartDevSet->device[i].status = 100;//param;
				break;
			case SCMD_CURTAIN_CLOSE:
				g_pSmartDevSet->device[i].status = 0;
				break;
			case SCMD_CURTAIN_STOP:
				g_pSmartDevSet->device[i].status = param;
				break;
	
			default:
				break;
			}
		//	break;
		}
	}
	g_CS.lockoff();
}

//======================================================
//** ��������: SetStatusByList
//** ��������: ����״̬ͨ���б�
//** �䡡��: addr scene
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void SetStatusByList(WORD addr, BYTE scene)
{
	g_CS.lockon();
	for(int i = 0; i < g_pSmartDevSet->dwSceneCtrl; i++)
	{
		// �ҵ���Ӧ�ı��
		if(scene == g_pSceneCtrl[i].scene)
		{
			// �ҵ���Ӧ�ĵ�ַ
			if(addr == g_pSmartDevSet->device[g_pSceneCtrl[i].num].phy_addr)
			{
				if(g_pSceneCtrl[i].cmd == 0x01)
				{
					g_pSmartDevSet->device[g_pSceneCtrl[i].num].status = g_pSceneCtrl[i].param;
				}
				else if(g_pSceneCtrl[i].cmd == 0x03)
				{
					g_pSmartDevSet->device[g_pSceneCtrl[i].num].status = 0;
				}
				else
				{
					DBGMSG(DPINFO, "SetStatusByList UnProcess cmd:%x\r\n", g_pSceneCtrl[i].cmd);
				}
				DPPostMessage(MSG_BROADCAST, SMART_STATUS_SCENE, g_pSmartDevSet->device[g_pSceneCtrl[i].num].addr, scene);
				break;
			}
		}
	}
	g_CS.lockoff();
}

//======================================================
//** ��������: SetStatusByScene
//** ��������: ����״̬ͨ���龰
//** �䡡��: scene
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void SetStatusByScene(BYTE scene)
{
	g_CS.lockon();
	for(int i = 0; i < g_pSmartDevSet->dwSceneCtrl; i++)
	{
		if(scene == g_pSceneCtrl[i].scene)
		{
			for(int k = 0; k < MAX_PAGE_NUM * MAX_ICON_NUM; k++)
			{
				if(g_pSceneCtrl[i].num == g_pSmartDevSet->device[k].param0) 
				{
					if(g_pSceneCtrl[i].cmd == 0x01) 	  // kai 
					{				
						if(g_pSmartDevSet->device[k].type >= ST_AC_A 
							&&g_pSmartDevSet->device[k].type<=ST_MUSIC_D) {
					
							g_pSmartDevSet->device[k].status =
								g_pSmartDevSet->device[k].status & 0x7fff;
					
							g_pSmartDevSet->device[k].status =
								g_pSmartDevSet->device[k].status | 0x4000;
					
						} else 
							g_pSmartDevSet->device[k].status = g_pSceneCtrl[i].param;
					} 
					else if(g_pSceneCtrl[i].cmd == 0x03)  // guan
					{
						if(g_pSmartDevSet->device[k].type >= ST_AC_A 
							&&g_pSmartDevSet->device[k].type<=ST_MUSIC_D) {

							g_pSmartDevSet->device[k].status =
								g_pSmartDevSet->device[k].status & 0xbfff;	

							g_pSmartDevSet->device[k].status =
								g_pSmartDevSet->device[k].status |0x8000;	    
					
						}
						else	
							g_pSmartDevSet->device[k].status = 0;
					} 
					else 
					{
						DBGMSG(DPINFO, "SetStatusByList UnProcess cmd:%x\r\n", g_pSceneCtrl[i].cmd);
					}			
				}	
			}
		}
	}

	for(int i = 0; i < MAX_PAGE_NUM * MAX_ICON_NUM; i++) {
					// �ҵ����龰��
		if (htons(g_pSmartDevSet->device[i].addr) == scene) {
			// ���ǳ�ʼ״̬
			if (g_pSmartDevSet->device[i].addr != 0xffff)
				g_pSmartDevSet->device[i].scene_status = 1;
		} else {
			// ���ǳ�ʼ״̬
			if (g_pSmartDevSet->device[i].addr != 0xffff)
				g_pSmartDevSet->device[i].scene_status = 0;
		}			
	}
//	DPPostMessage(MSG_BROADCAST, SMART_STATUS_SCENE, g_pSmartDevSet->device[g_pSceneCtrl[i].num].addr, scene);
	DPPostMessage(MSG_BROADCAST, SMART_STATUS_SCENE, 0, scene);
	g_CS.lockoff();
}                                                                                                                                                                                                         
