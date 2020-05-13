#include "roomlib.h"
#include "SmartConfig.h"
#include "CCtrlModules.h"
#include <string.h>

#define	SMART_DEV_FILE0		"SmartDev0.ext"
#define	SMART_DEV_FILE1		"SmartDev1.ext"
#define	SMART_DEV_ENDID		0x86459358

extern Time_Link_List* g_TimeHead;          // 2018.1.15头结点数据的指针(定时事件)
extern char *g_pData[12];      
extern BOOL Flag[12];
extern int clear_open_flag; 				// 2018.3.9添加用于解决0x01开到100%的问题

typedef struct
{
	DWORD	count;
	WORD	phy_addr[MAX_PAGE_NUM * MAX_ICON_NUM];      //物理地址空间。
	WORD	group_addr[MAX_PAGE_NUM * MAX_ICON_NUM];    //申请组地址空间。
}SmartAddrList;

typedef struct
{
	DWORD		addr;                                   //地址
	DWORD		count;                                  //设备数量                       
	SmartDev	device[MAX_PAGE_NUM * MAX_ICON_NUM];    //设备
	DWORD		dwSceneCtrl;
	BOOL		bDefault;
	DWORD		VERSION;                                //版本
	DWORD		Endid;
}SmartDevSet;

// 2018.1.16添加，用于定时事件传参结构体
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
SmartDevSet*			g_pSmartDevSet;                  //智能家居设备设置。

//======================================================
//** 函数名称: UpdateSmartDevSet
//** 功能描述: 更新家居设备设置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
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
//** 函数名称: InitDefaultSmartDevSet
//** 功能描述: 初始化默认家居设备设置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void InitDefaultSmartDevSet()
{
#if 0	
	DWORD page;

	page = 0 * MAX_ICON_NUM;
	// 灯光
	g_pSmartDevSet->device[page + 2].exist = TRUE;
	g_pSmartDevSet->device[page + 2].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 2].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[page + 2].name, GetStringByID(2003));

	page = 1 * MAX_ICON_NUM;
	// 灯光
	g_pSmartDevSet->device[page + 0].exist = TRUE;
	g_pSmartDevSet->device[page + 0].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 0].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[page + 0].name, GetStringByID(2003));	
	// 调光
	g_pSmartDevSet->device[page + 1].exist = TRUE;
	g_pSmartDevSet->device[page + 1].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 1].type = ST_DIMMER_A;
	strcpy(g_pSmartDevSet->device[page + 1].name, GetStringByID(2000));

	page = 2 * MAX_ICON_NUM;
	// 灯光
	g_pSmartDevSet->device[page + 0].exist = TRUE;
	g_pSmartDevSet->device[page + 0].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 0].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[page + 0].name, GetStringByID(2003));	
	// 调光
	g_pSmartDevSet->device[page + 1].exist = TRUE;
	g_pSmartDevSet->device[page + 1].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 1].type = ST_DIMMER_A;
	strcpy(g_pSmartDevSet->device[page + 1].name, GetStringByID(2000));
	// 窗帘
	g_pSmartDevSet->device[page + 2].exist = TRUE;
	g_pSmartDevSet->device[page + 2].type = ST_CURTAIN_A;
	g_pSmartDevSet->device[page + 2].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 2].name, GetStringByID(2016));

	page = 3 * MAX_ICON_NUM;
	// 灯光
	g_pSmartDevSet->device[page + 0].exist = TRUE;
	g_pSmartDevSet->device[page + 0].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 0].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[page + 0].name, GetStringByID(2003));	
	// 调光
	g_pSmartDevSet->device[page + 2].exist = TRUE;
	g_pSmartDevSet->device[page + 2].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 2].type = ST_DIMMER_A;
	strcpy(g_pSmartDevSet->device[page + 2].name, GetStringByID(2000));
	// 窗帘
	g_pSmartDevSet->device[page + 4].exist = TRUE;
	g_pSmartDevSet->device[page + 4].type = ST_CURTAIN_A;
	g_pSmartDevSet->device[page + 4].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 4].name, GetStringByID(2016));
	// 地暖
	g_pSmartDevSet->device[page + 5].exist = TRUE;
	g_pSmartDevSet->device[page + 5].type = ST_HEAT_A;
	g_pSmartDevSet->device[page + 5].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 5].name, GetStringByID(2009));

	page = 5 * MAX_ICON_NUM;
	// 灯光
	g_pSmartDevSet->device[page + 0].exist = TRUE;
	g_pSmartDevSet->device[page + 0].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 0].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[page + 0].name, GetStringByID(2003));	
	// 调光
	g_pSmartDevSet->device[page + 1].exist = TRUE;
	g_pSmartDevSet->device[page + 1].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 1].type = ST_DIMMER_A;
	strcpy(g_pSmartDevSet->device[page + 1].name, GetStringByID(2000));
	// 窗帘
	g_pSmartDevSet->device[page + 2].exist = TRUE;
	g_pSmartDevSet->device[page + 2].type = ST_CURTAIN_A;
	g_pSmartDevSet->device[page + 2].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 2].name, GetStringByID(2016));
	// 地暖
	g_pSmartDevSet->device[page + 3].exist = TRUE;
	g_pSmartDevSet->device[page + 3].type = ST_HEAT_A;
	g_pSmartDevSet->device[page + 3].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 3].name, GetStringByID(2009));
	// 空调
	g_pSmartDevSet->device[page + 4].exist = TRUE;
	g_pSmartDevSet->device[page + 4].type = ST_AC_A;
	g_pSmartDevSet->device[page + 4].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 4].name, GetStringByID(2007));

	page = 8 * MAX_ICON_NUM;
	// 灯光
	g_pSmartDevSet->device[page + 0].exist = TRUE;
	g_pSmartDevSet->device[page + 0].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 0].type = ST_LIGHT_A;
	strcpy(g_pSmartDevSet->device[page + 0].name, GetStringByID(2003));	
	// 调光
	g_pSmartDevSet->device[page + 1].exist = TRUE;
	g_pSmartDevSet->device[page + 1].addr = INVALID_ECB_ADDR;
	g_pSmartDevSet->device[page + 1].type = ST_DIMMER_A;
	strcpy(g_pSmartDevSet->device[page + 1].name, GetStringByID(2000));
	// 窗帘
	g_pSmartDevSet->device[page + 2].exist = TRUE;
	g_pSmartDevSet->device[page + 2].type = ST_CURTAIN_A;
	g_pSmartDevSet->device[page + 2].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 2].name, GetStringByID(2016));
	// 空调
	g_pSmartDevSet->device[page + 3].exist = TRUE;
	g_pSmartDevSet->device[page + 3].type = ST_AC_A;
	g_pSmartDevSet->device[page + 3].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 3].name, GetStringByID(2007));
	// 音乐
	g_pSmartDevSet->device[page + 4].exist = TRUE;
	g_pSmartDevSet->device[page + 4].type = ST_MUSIC_A;
	g_pSmartDevSet->device[page + 4].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[page + 4].name, GetStringByID(2012));
    // 红外电视
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
	// 第一页 调光灯
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
   // 灯光
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

	// 第二页
	g_pSmartDevSet->device[6].exist = TRUE;
	g_pSmartDevSet->device[6].type = ST_LIGHT_D;
	g_pSmartDevSet->device[6].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[6].name, GetStringByID(2006));
#endif
	// 空调
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
    // 地暖
	g_pSmartDevSet->device[4].exist = TRUE;
	g_pSmartDevSet->device[4].type = ST_HEAT_A;
	g_pSmartDevSet->device[4].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[4].name, GetStringByID(2009));
    // 新风
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
	// 第三页
	// 音乐
	g_pSmartDevSet->device[6].exist = TRUE;
	g_pSmartDevSet->device[6].type = ST_MUSIC_A;
	g_pSmartDevSet->device[6].addr = INVALID_ECB_ADDR;
	strcpy(g_pSmartDevSet->device[6].name, GetStringByID(2012));
    // 红外电视
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
    // 窗帘
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
	// 第四页
	// 插座
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
//** 函数名称: InitSmartDev
//** 功能描述: 初始化家居设备
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void InitSmartDev()
{
	FILE* fd;
	char filename[64];
	SmartDevSet* pSet0 = NULL;
	SmartDevSet* pSet1 = NULL;
	SceneCtrl* pSceneCtrl0 = NULL;    // 场景
	SceneCtrl* pSceneCtrl1 = NULL;

	g_CS.lockon();
	// 先申请物理地址、组地址对应表
	g_pAddrList = (SmartAddrList *)malloc(sizeof(SmartAddrList));
	if(g_pAddrList)   
	{
		memset(g_pAddrList, 0, sizeof(SmartAddrList));
	}
	// 读取配置文件
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
    // 这里貌似是系统文件什么都没有的时候初始化默认的设备  
	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		g_pSmartDevSet = (SmartDevSet *)malloc(sizeof(SmartDevSet));
		if(g_pSmartDevSet)
		{
			memset(g_pSmartDevSet, 0, sizeof(SmartDevSet));
			InitDefaultSmartDevSet();     //这里貌似就是和APP上的设备同步了。
			UpdateSmartDevSet();          //将数据写入到文件当中
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
	// 所有状态清零
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
//** 函数名称: ResetSmartDev
//** 功能描述: 复位家居设备设置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void ResetSmartDev()                         //貌似是一个恢复出厂设置相关的函数。
{
	g_CS.lockon();
	memset(g_pSmartDevSet, 0, sizeof(SmartDevSet));
	InitDefaultSmartDevSet();
	UpdateSmartDevSet();
	DeleteServerFile(SMART_DEV_FILE1);
	g_CS.lockoff();
}

//======================================================
//** 函数名称: SetSmartAddr
//** 功能描述: 设置家居物理地址
//** 输　入: addr
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void SetSmartAddr(DWORD addr)
{
	g_CS.lockon();
	g_pSmartDevSet->addr = addr;
	UpdateSmartDevSet();
	g_CS.lockoff();
}

//======================================================
//** 函数名称: GetSmartAddr
//** 功能描述: 获取家居物理地址
//** 输　入: addr
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
DWORD GetSmartAddr()
{
	return g_pSmartDevSet->addr;
}

//======================================================
//** 函数名称: GetPhyByGruop
//** 功能描述: 通过组地址获取物理地址
//** 输　入: addr
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
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
 *2018.1.16创建
 *功能:用于置位重复日期选项标志位
 */
//======================================================
//** 函数名称: Data_Choice
//** 功能描述: 定时数据挑选
//** 输　入: flag
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void Data_Choice(BYTE flag)
{
	if((flag & 0x80) == 0) { // 非单次周期

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
 * 2018.1.15 开始
 * 功能:创建一个新节点
 *
 *minute,second:定时时间，data:重复时间，device:控制设备,
 *ctr_mode:控制方式，ctr_param:控制参数
 */
//======================================================
//** 函数名称: Creat_Node
//** 功能描述: 创建链表节点
//** 输　入: setTimer
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void Creat_Node(SetTimer*  setTimer)
{  
	char buf[20];
	pNode P_Head = (pNode)malloc(sizeof(LNode)); //为头节点分配内存空间，把申请到的内存当作新节点
	pNode P_Mid;								  //为中间变量

	if(NULL == P_Head) {						  //若为空指针则分配内存失败

		return ;
	}
    memset(P_Head, 0, sizeof(LNode ));            //清理申请到的对内存

	P_Head->next = NULL;						  //其下一指针赋值为空(作为尾节点),将来指向下一节点首地址		
    P_Head->show = TRUE;                          //定时器默认被开启     
		
/*************进行数据赋值操作,填充有效数据***************/ 
	           
	/* 时间字符串赋值 */
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
	/* 重复日期勾选标记，汉字显示重复日期 */
	Data_Choice(setTimer->data);
	
	for(int k = 0; k<12; k++) {                       

		P_Head->choose_data[k] = Flag[k];

		if(Flag[k]) {

			g_pData[k] = GetStringByID(13200+k);
			strcat(P_Head->p_Data, g_pData[k]);   // 重复周期汉字显示出来
		} 
			
	}
#endif

#if 1
	/* 控制设备字符串赋值 */ 
	for(int i = 0; i < MAX_PAGE_NUM * MAX_ICON_NUM; i++) {

		// 普通设备名字显示
		if(setTimer->device != 0xffff) {
			
			if((setTimer->device == g_pSmartDevSet->device[i].addr) ) {

				P_Head->device = setTimer->device;   // 控制组地址
				strcpy(P_Head->Device, g_pSmartDevSet->device[i].name);
				break;
			}
		}
		// 情景设备名字显示
		else if(setTimer->device == 0xffff) {
			if(setTimer->ctr_param == g_pSmartDevSet->device[i].addr) {
				if(g_pSmartDevSet->device[i].type >= ST_SCENE_A
					 && g_pSmartDevSet->device[i].type <= ST_SCENE_Z) { 
					 
					P_Head->device = setTimer->device;   // 控制组地址
					strcpy(P_Head->Device, g_pSmartDevSet->device[i].name);
					break;

				}
			}
		}
	}        
	 
	/* 设备控制方式赋值 */

	switch(setTimer->ctr_mode) {

	case SCMD_OPEN:   // 开
		strcpy(P_Head->CTL_Type,GetStringByID(13401));
		break;
	
	case SCMD_CLOSE:  // 关
		strcpy(P_Head->CTL_Type,GetStringByID(13402));
		break;
		
		default:
			break;
	}
	P_Head->ctr_mode = setTimer->ctr_mode;   	// 控制类型
	
	/* 控制参数赋值 */
	P_Head->ctr_param = setTimer->ctr_param;    
#endif
/**********************************************************/

	if(NULL == g_TimeHead) {					  //节点为头结点(证明在此之前没有定时事件)

		g_TimeHead = P_Head;					  //建立第一个定时事件			
	}

	else {										  //不是第一个定时事件

		P_Mid = g_TimeHead;

		while(NULL != P_Mid->next) {

			P_Mid = P_Mid->next;		
		}

		P_Mid->next = P_Head;			 
	}
   
	UpdatSetTimer();                               //将数据写入文件当中			 
}

/*
 *2018.1.15添加，定时功能
 *函数原型:AddSmartTimer(ECB_DATA* pECB)
 *功能:接收手机APP配置的定时信息
 *入口参数:ECB_DATA* pECB
 *出口参数:无
 *返回值:无
 */
//======================================================
//** 函数名称: AddSmartTimer
//** 功能描述: 添加家居定时
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================  
void AddSmartTimer(ECB_DATA* pECB)
{
	SetTimer Timer;
	Timer.hour      = pECB->data[2];             // 小时
	Timer.minute    = pECB->data[3];             // 分钟
	Timer.data      = pECB->data[4];             // 重复日期
	memcpy(&Timer.device, &pECB->data[5], 2);    // 设备名称
	Timer.ctr_mode  = pECB->data[7];             // 控制方式
	memcpy(&Timer.ctr_param, &pECB->data[8], 2); // 控制参数

	for(int k = 0; k<12; k++) {       // 将标志位先清零

		Flag[k] = FALSE;
	}

	Creat_Node(&Timer);  // 创建新定时事件的节点

}

/*
 *2018.1.19添加
 *函数原型：FlashStatusTimer()
 *功能:根据定时时间到后刷新图标状态
 */
//======================================================
//** 函数名称: FlashStatusTimer
//** 功能描述: 刷新定时状态
//** 输　入: TimerEvent
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
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
 *2018.1.18添加 
 *函数原型:DeleteSmartTimer()
 *函数功能:删除定时数据链表
 */
//======================================================
//** 函数名称: DeleteSmartTimer
//** 功能描述: 删除家居定时
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
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
//** 函数名称: AddSmartDev
//** 功能描述: 添加家居设备
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void AddSmartDev(ECB_DATA* pECB)
{
	WORD group_addr, phy_addr;
	// 红外电视编码参数
	WORD IR_TV_OL,IR_TV_CHANNEL1,IR_TV_CHANNEL2,
		 IR_TV_VOICE1,IR_TV_VOICE2,IR_TV_SWITCH;
	// 红外空调编码参数
	WORD IR_AIR_H,IR_AIR_M,IR_AIR_L,IR_AIR_T,
		 IR_AIR_CODE,IR_AIR_HOT,IR_AIR_ONOFF;
	
	char szName[32] = "";                                   // 设备的名字
	BYTE num = pECB->data[1] ;								// 序号(1 Byte)
	BYTE page = pECB->data[2] - 1;							// 页号(1 Byte)
	BYTE bit = pECB->data[3] - 1;							// 位号(1 Byte)
	BYTE type = pECB->data[4];								// 类型(1 Byte)
   
	if(type == ST_TV_A) {/* 如果是红外类型(红外电视) */
	
		memcpy(&group_addr, &pECB->data[5], 2);				// 参数 (2～5 Byte)红外类型

		switch(pECB->data[7]) {
		
		case 53:  // 电视开/关
			IR_TV_OL = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_TV_CODE(IR_TV_OL,0);
			break;
		case 54:  // 频道+
			IR_TV_CHANNEL1 = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_TV_CODE(IR_TV_CHANNEL1,1);
			break;
		case 55:  // 频道-
			IR_TV_CHANNEL2 = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_TV_CODE(IR_TV_CHANNEL2,2);
			break;
		case 56:  // 音量+
			IR_TV_VOICE1 = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_TV_CODE(IR_TV_VOICE1,3);
			break;
		case 57:  // 音量-
			IR_TV_VOICE2 = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_TV_CODE(IR_TV_VOICE2,4);
			break;
		case 58:  // TV/AV切换
			IR_TV_SWITCH = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_TV_CODE(IR_TV_SWITCH,5);
			break;

		default:
			break;

		} 

		if(pECB->length - 17 <= 16 && pECB->length - 17 > 0)	{ // 信息(1～16 Byte) 字符转换 英->汉 

			char szGBK[16];
			WORD szUnicode[16];

			memcpy(szGBK, &pECB->data[10], pECB->length - 17);
			szGBK[pECB->length - 17] = 0;

			GbConvert(szUnicode, (BYTE*)szGBK);
			unicode2utf8((BYTE*)szName, (wchar_t*)szUnicode);
		}
		
	} 
    // 如果是红外空调类型
   else	if(type == ST_AC_B) {

		memcpy(&group_addr, &pECB->data[5], 2);                 // 组地址 

		switch(pECB->data[7]) {  

		case 1:
		case 2:
		case 3:  // 开/关
			IR_AIR_ONOFF = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_ONOFF, 0);
			break;

		case 8:  // 制热
			IR_AIR_HOT = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_HOT, 1);
			break;

		case 9:  // 制冷
			IR_AIR_CODE = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_CODE, 2);
			break;

		case 10: // 通风
			IR_AIR_T = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_T, 3);
			break;

		case 24: // 风速低
			IR_AIR_L = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_L, 4);
			break;

		case 25: // 风速中
			IR_AIR_M = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_M, 5);
			break;

		case 26: // 风速高 
			IR_AIR_H = (pECB->data[8]<<8 | pECB->data[9]);
			SetIR_AIR_CODE(IR_AIR_H, 6);
			break;

		default:
			break;
		
		}

		if(pECB->length - 17 <= 16 && pECB->length - 17 > 0)	{ // 信息(1～16 Byte) 字符转换 英->汉 

			char szGBK[16];
			WORD szUnicode[16];

			memcpy(szGBK, &pECB->data[10], pECB->length - 17);
			szGBK[pECB->length - 17] = 0;

			GbConvert(szUnicode, (BYTE*)szGBK);
			unicode2utf8((BYTE*)szName, (wchar_t*)szUnicode);
		}
	}
		
	else { /* 非红外设备类型 */

		memcpy(&group_addr, &pECB->data[5], 2);				    // 参数(2～5 Byte) 如果类型不是红外，则为2个字节的组地址

		if(pECB->length - 14 <= 16 && pECB->length - 14 > 0)								// 信息(1～16 Byte)
		{
			char szGBK[16];
			WORD szUnicode[16];

			memcpy(szGBK, &pECB->data[7], pECB->length - 14);
			szGBK[pECB->length - 14] = 0;

			GbConvert(szUnicode, (BYTE*)szGBK);
			unicode2utf8((BYTE*)szName, (wchar_t*)szUnicode);
		}
	}
		
	// 通过组地址获取物理地址, 如果是情景，则忽略
	if(type < ST_SCENE_A)
		phy_addr = GetPhyByGruop(group_addr);                // 获取物理地址    

	if(page >= MAX_PAGE_NUM || bit >= MAX_ICON_NUM)
		return;

	g_CS.lockon();
	int index = page * MAX_ICON_NUM + bit;

	// 如果当前是默认配置,先清空之前的数量
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
	g_pSmartDevSet->device[index].type = type;            // 类型
	g_pSmartDevSet->device[index].addr = group_addr;      // 组地址
	g_pSmartDevSet->device[index].phy_addr = phy_addr;    
	strcpy(g_pSmartDevSet->device[index].name, szName);   // 设备名字

	UpdateSmartDevSet();
	g_CS.lockoff();
}

//======================================================
//** 函数名称: AddSceneCtrlList
//** 功能描述: 添加情景控制列表
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void AddSceneCtrlList(ECB_DATA* pECB)
{
	WORD param;
	BYTE num =	pECB->data[0];				// 设备序号(1 Byte)
	BYTE scene = pECB->data[2];				// 场景编号(1 Byte)
	BYTE cmd =	pECB->data[3];				// 控制方式(1 Byte)
	memcpy(&param, &pECB->data[4], 2);		// 控制参数(2 Byte)	
	param = htons(param);
	if(cmd == 1 && param == 0)
	{
		// 当cmd为1时，控制参数0代表打开到100%
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
//** 函数名称: AddSmartAddrList
//** 功能描述: 添加家居地址列表
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void AddSmartAddrList(ECB_DATA* pECB)
{
	g_CS.lockon();
	g_pAddrList->phy_addr[g_pAddrList->count] = pECB->dst;						// 器件物理地址
	memcpy(&g_pAddrList->group_addr[g_pAddrList->count], &pECB->data[1], 2);	// 组地址1
	g_pAddrList->count++;
	g_CS.lockoff();
}

//======================================================
//** 函数名称: GetSmartDev
//** 功能描述: 获取家居设备列表
//** 输　入: count
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
/*const*/ SmartDev* GetSmartDev(DWORD* count)
{
	/*const*/ SmartDev* pSmartDev = NULL;        // 形容指向常量的指针
	g_CS.lockon();
	*count = g_pSmartDevSet->count;
	pSmartDev = g_pSmartDevSet->device;
	g_CS.lockoff();
	return pSmartDev;
}

//======================================================
//** 函数名称: SetStatusByAck
//** 功能描述: 设置状态通过ack
//** 输　入: addr cmd param
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
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
				
				/* 窗帘修改版本:点击后立即刷新 */
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
//** 函数名称: SetStatusBySync
//** 功能描述: 设置状态通过同步
//** 输　入: addr cmd param
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
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
//** 函数名称: SetStatusByList
//** 功能描述: 设置状态通过列表
//** 输　入: addr scene
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void SetStatusByList(WORD addr, BYTE scene)
{
	g_CS.lockon();
	for(int i = 0; i < g_pSmartDevSet->dwSceneCtrl; i++)
	{
		// 找到对应的编号
		if(scene == g_pSceneCtrl[i].scene)
		{
			// 找到对应的地址
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
//** 函数名称: SetStatusByScene
//** 功能描述: 设置状态通过情景
//** 输　入: scene
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
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
					// 找到该情景了
		if (htons(g_pSmartDevSet->device[i].addr) == scene) {
			// 不是初始状态
			if (g_pSmartDevSet->device[i].addr != 0xffff)
				g_pSmartDevSet->device[i].scene_status = 1;
		} else {
			// 不是初始状态
			if (g_pSmartDevSet->device[i].addr != 0xffff)
				g_pSmartDevSet->device[i].scene_status = 0;
		}			
	}
//	DPPostMessage(MSG_BROADCAST, SMART_STATUS_SCENE, g_pSmartDevSet->device[g_pSceneCtrl[i].num].addr, scene);
	DPPostMessage(MSG_BROADCAST, SMART_STATUS_SCENE, 0, scene);
	g_CS.lockoff();
}                                                                                                                                                                                                         
