#include "roomlib.h"
#include "SmartConfig.h"
#include "CCtrlModules.h"
#include "list.h"

#define	SMART_DEV_FILE0		"SmartDev0.ext"
#define	SMART_DEV_FILE1		"SmartDev1.ext"
#define	SMART_DEV_ENDID		0x86459358
#define SMART_TIMER_FILE    "SmartTimer.ext"

typedef struct
{
    DWORD		addr;                                   // 物理地址
    DWORD		pwd;                                    // 系统密码
    DWORD		route;                                  // 路由模式
    DWORD		id;										// 传感器ID-温度
    DWORD		count;                                  // 设备数量
    SmartDev	device[MAX_PAGE_NUM * MAX_ICON_NUM];      // 设备信息
    DWORD		dwSceneCtrl;							// 情景控制数量
    LightStudy	study[MAX_CHAN_NUM];						// 灯光学习信息
    BOOL		bDefault;								// 默认配置标识
    DWORD		VERSION;                                // 写文件标识
    DWORD		Endid;									// 结束标志
} SmartDevSet;

static StaticLock		g_CS;							 // 文件互斥锁
static CMyList 			m_timerlist;					 // 定时列表
static DWORD			g_dwMaxScene;					 // 最大情景设备个数
static SceneCtrl		*g_pSceneCtrl;					 // 情景模式控制信息
static SmartDevSet		*g_pSmartDevSet;                  // 智能家居设备信息
static DWORD			g_dwStudyChan;					 // 灯光当前学习通路
static DWORD			g_dwStudyNum;					 // 灯光当前学习个数


const UINT8 WRTBindAsInitiatorTable[] =
{
    0x01,//《八键遥控器》（没低压）
    0x02,//《门窗磁传感器》（高4位清空）
    0x03,//《振动传感器》（没低压）
    0x04,//《人体移动传感器》（高4位清空）
    0x08,//《开关》
    0x09,//《出入》
    0x11,//《亮暗》
    0x23,//（没低压）
    0x24, 0x25, //（高4位清空）
    0x26,//（没低压）
    0x2c, 0x2d, 0x2e, 0x2F, 0x30, 0x31, 0x32, 0x33, //《光能开关面板》
    0x3C, 0x3d, 0x3e, 0x3F, 0x40, 0x41, 0x42, 0x43, //<机械开关传感器》
    0x44,//四路触发器
    0x45,//无线被动红外（高4位清空）
    0x50, 0x52, //《X键遥控器》
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, //《键遥控器》（高4位清空）
    0x6B,//《门窗磁传感器》（高4位清空）
    0x6C,//《红外探测器》（高4位清空）
    0x6D, 0x6F, 0x70, 0x71, //《红外光照传感器》（高4位清空）
    0x2A, 0x2B, //《一/二路光能窗帘开关面板》
    0X80,//《二路窗帘面板》
};

const UINT8 WRTBindDataClearH4[] =
{
    0x02,//《门窗磁传感器》（高4位清空）
    0x04,//《人体移动传感器》（高4位清空）
    0x24, 0x25, //（高4位清空）
    0x45,//无线被动红外（高4位清空）
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, //《键遥控器》（高4位清空）
    0x6B,//《门窗磁传感器》（高4位清空）
    0x6C,//《红外探测器》（高4位清空）
    0x6D, 0x6F, 0x70, 0x71 //《红外光照传感器》（高4位清空）
};

//======================================================
//** 函数名称: UpdateSmartDevSet
//** 功能描述: 更新设备列表
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void UpdateSmartDevSet()
{
    SmartDevSet *pSet;
    BOOL ret;

    pSet = (SmartDevSet *)malloc(sizeof(SmartDevSet) + sizeof(SceneCtrl) * g_pSmartDevSet->dwSceneCtrl);
    if(NULL == pSet)
        return;

    memcpy(pSet, g_pSmartDevSet, sizeof(SmartDevSet));
    memcpy((char *)pSet + sizeof(SmartDevSet), g_pSceneCtrl, sizeof(SceneCtrl) * g_pSmartDevSet->dwSceneCtrl);

    if(g_pSmartDevSet->VERSION & 1)
        ret = WriteServerFile(SMART_DEV_FILE1, sizeof(SmartDevSet) + sizeof(SceneCtrl) * g_pSmartDevSet->dwSceneCtrl, (char *)pSet);
    else
        ret = WriteServerFile(SMART_DEV_FILE0, sizeof(SmartDevSet) + sizeof(SceneCtrl) * g_pSmartDevSet->dwSceneCtrl, (char *)pSet);
    if(!ret)
        free(pSet);
    else
    {
        g_pSmartDevSet->VERSION++;
    }
}

//======================================================
//** 函数名称: UpdatSmartTimerSet
//** 功能描述: 更新定时列表
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void UpdatSmartTimerSet()
{
    char filename[64];
    FILE *fd = NULL;

    g_CS.lockon();
    sprintf(filename, "%s/%s", USERDIR, SMART_TIMER_FILE);

    fd = fopen(filename, "wb");
    if (NULL == fd)
    {
        g_CS.lockoff();
        return;
    }

    PSmartTimer pItem = (PSmartTimer) m_timerlist.Head();
    while (pItem)
    {
        fwrite(pItem, 1, sizeof(SmartTimer), fd);
        pItem = (PSmartTimer) m_timerlist.Next(&(pItem->lpObj));
    }
    fclose(fd);
    g_CS.lockoff();
}

//======================================================
//** 函数名称: InitDefaultSmartDevSet
//** 功能描述: 初始化默认设备列表
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void InitDefaultSmartDevSet()
{
    DWORD page = 0;
    DWORD index = 0;
    DWORD id = ReadCpuID();
    WORD type = htons(ECB_DEV_TYPE);

    memset(g_pSmartDevSet, 0, sizeof(SmartDevSet));

    // 第一页
    page = 0 * MAX_ICON_NUM;
    index = 0;

    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = 1;
    g_pSmartDevSet->device[page + index].device.id = id;
    g_pSmartDevSet->device[page + index].device.type = type;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_LIGHT_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2007));

    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = 2;
    g_pSmartDevSet->device[page + index].device.id = id;
    g_pSmartDevSet->device[page + index].device.type = type;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_LIGHT_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2008));

    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = 3;
    g_pSmartDevSet->device[page + index].device.id = id;
    g_pSmartDevSet->device[page + index].device.type = type;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_LIGHT_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2009));

    // 第二页
    page = 1 * MAX_ICON_NUM;
    index = 0;

    // 主灯
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_LIGHT_B;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2000));

    // 窗帘
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_CURTAIN_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2110));

    // 调光灯
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_DIMMER_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2010));

    // 插座
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_OUTLET_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2130));

    // 音乐
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_MUSIC_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2070));

    // 第三页
    page = 2 * MAX_ICON_NUM;
    index = 0;
	// 新风
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_WIND_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2050));

    // 空调
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_AC_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2020));

    // 地暖
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_HEAT_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2040));

    // 红外电视
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_TV_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2080));

    // 红外空调
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_IRAIR_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2030));

    // 第四页
    page = 3 * MAX_ICON_NUM;
    index = 0;

    // 在家
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = 4;
    g_pSmartDevSet->device[page + index].device.id = id;
    g_pSmartDevSet->device[page + index].device.type = type;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_SCENE_F;
    g_pSmartDevSet->device[page + index].scene = 3;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(3005));

    // 离家
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = 4;
    g_pSmartDevSet->device[page + index].device.id = id;
    g_pSmartDevSet->device[page + index].device.type = type;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_SCENE_G;
    g_pSmartDevSet->device[page + index].scene = 4;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(3006));

    // 就餐
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = 4;
    g_pSmartDevSet->device[page + index].device.id = id;
    g_pSmartDevSet->device[page + index].device.type = type;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_SCENE_B;
    g_pSmartDevSet->device[page + index].scene = 5;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(3001));

    // 影院
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = 4;
    g_pSmartDevSet->device[page + index].device.id = id;
    g_pSmartDevSet->device[page + index].device.type = type;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_SCENE_C;
    g_pSmartDevSet->device[page + index].scene = 6;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(3002));

    // 温馨
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = 4;
    g_pSmartDevSet->device[page + index].device.id = id;
    g_pSmartDevSet->device[page + index].device.type = type;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_SCENE_E;
    g_pSmartDevSet->device[page + index].scene = 7;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(3004));

    g_pSmartDevSet->count = 18;
    g_pSmartDevSet->addr = id & 0xFFFF;
    g_pSmartDevSet->pwd = INIT_DEV_PWD;
    g_pSmartDevSet->route = 0;

    g_pSmartDevSet->dwSceneCtrl = 0;
    g_pSmartDevSet->bDefault = TRUE;
    g_pSmartDevSet->Endid = SMART_DEV_ENDID;
}

//======================================================
//** 函数名称: testtimer
//** 功能描述: 定时功能添加定时-测试用
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void testtimer(BYTE hour, BYTE minute, BYTE week,
               BYTE chan, DWORD id, WORD type,
               BYTE way, WORD param)
{
    PSmartTimer pobj = new SmartTimer;
    memset(pobj, 0, sizeof(SmartTimer));
    pobj->hour = hour;
    pobj->minute = minute;
    pobj->week = week;
    pobj->device.channel = chan;
    pobj->device.id = id;
    pobj->device.type = type;
    pobj->way = way;
    pobj->param = param;
    pobj->onoff = FALSE;
    GetTimerShow(pobj);

    m_timerlist.AddTail(&pobj->lpObj);
    printf("timer count = %d\r\n", m_timerlist.GetCount());
}

//======================================================
//** 函数名称: SmartTimerTest
//** 功能描述: 定时功能测试用例-测试用
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SmartTimerTest()
{
    DWORD id = ReadCpuID();
    WORD type = htons(ECB_DEV_TYPE);

    testtimer(9,  0, 0x9f, 0x01, id, type, 0x01, 0);
    testtimer(18, 0, 0x9f, 0x01, id, type, 0x03, 0);
    testtimer(9,  0, 0x9f, 0x02, id, type, 0x01, 0);
    testtimer(18, 0, 0x9f, 0x02, id, type, 0x03, 0);
    testtimer(9,  0, 0x9f, 0x03, id, type, 0x01, 0);
    testtimer(18, 0, 0x9f, 0x03, id, type, 0x03, 0);
}

//======================================================
//** 函数名称: InitDefaultSmartTimer
//** 功能描述: 初始化默认定时
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void InitDefaultSmartTimer()
{
    PSmartTimer pItemNext;
    PSmartTimer pItem = (PSmartTimer) m_timerlist.Head();
    while (pItem)
    {
        pItemNext = (PSmartTimer) m_timerlist.Next(&(pItem->lpObj));
        m_timerlist.Disconnect(&(pItem->lpObj));
        delete pItem;
        pItem = pItemNext;
    }
}

//======================================================
//** 函数名称: InitSmartDev
//** 功能描述: 初始化设备
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void InitSmartDev()
{
    FILE *fd;
    char filename[64];
    SmartDevSet *pSet0 = NULL;
    SmartDevSet *pSet1 = NULL;
    SceneCtrl *pSceneCtrl0 = NULL;    // 场景
    SceneCtrl *pSceneCtrl1 = NULL;

    g_CS.lockon();
    // 读取配置文件
    sprintf(filename, "%s/%s", USERDIR, SMART_DEV_FILE0);
    fd = fopen(filename, "rb");
    if(fd != NULL)
    {
        pSet0 = (SmartDevSet *)malloc(sizeof(SmartDevSet));
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
                            DWORD ret = fread(pSceneCtrl0, 1, sizeof(SceneCtrl) * pSet0->dwSceneCtrl, fd);
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
        pSet1 = (SmartDevSet *)malloc(sizeof(SmartDevSet));
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
                            DWORD ret = fread(pSceneCtrl1, 1, sizeof(SceneCtrl) * pSet1->dwSceneCtrl, fd);
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

    // 不存在设备配置文件，初始化默认设备
    if((pSet0 == NULL) && (pSet1 == NULL))
    {
        g_pSmartDevSet = (SmartDevSet *)malloc(sizeof(SmartDevSet));
        if(g_pSmartDevSet)
        {
            memset(g_pSmartDevSet, 0, sizeof(SmartDevSet));
            InitDefaultSmartDevSet();     // 初始化默认设备
            UpdateSmartDevSet();          // 配置信息写入文件
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

#if 0
    // 所有状态清零
    for(int i = 0; i < MAX_DEV_NUM; i++)
    {
        if (g_pSmartDevSet->device[i].type >= ST_AC_A
                && g_pSmartDevSet->device[i].type <= ST_MUSIC_D)
        {
            g_pSmartDevSet->device[i].cmd = SCMD_CLOSE;
            g_pSmartDevSet->device[i].status = 0x8000;
        }
        else
        {
            g_pSmartDevSet->device[i].cmd = SCMD_CLOSE;
            g_pSmartDevSet->device[i].status = 0;
        }
    }
#endif
    g_CS.lockoff();
}

//======================================================
//** 函数名称: InitSmartTimer
//** 功能描述: 初始化定时
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void InitSmartTimer()
{
    char filename[64];
    FILE *fin;
    DWORD count;
    struct stat statbuff;

    sprintf(filename, "%s/%s", USERDIR, SMART_TIMER_FILE);
    fin = fopen(filename, "rb");
    if (fin == NULL)
    {
        //SmartTimerTest();
        UpdatSmartTimerSet();
        return;
    }

    if (m_timerlist.GetCount() > 0)
        InitDefaultSmartTimer();

    if (stat(filename, &statbuff))
    {
        fclose(fin);
        printf("stat file error\r\n");
        return;
    }

    if (0 == statbuff.st_size)
    {
        fclose(fin);
        printf("file size = 0\r\n");
        return;
    }

    count = statbuff.st_size / sizeof(SmartTimer);
    while (count--)
    {
        PSmartTimer pobj = new SmartTimer;
        int ret = fread(pobj, 1, sizeof(SmartTimer), fin);
        if (ret != sizeof(SmartTimer))
        {
            fclose(fin);
            break;
        }
        m_timerlist.AddTail(&pobj->lpObj);
    }
    fclose(fin);
    printf("InitSmartTimer add %d timer\r\n", m_timerlist.GetCount());
}

//======================================================
//** 函数名称: ResetSmartDev
//** 功能描述: 恢复出厂设备
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void ResetSmartDev()
{
    g_CS.lockon();
    InitDefaultSmartDevSet();
    UpdateSmartDevSet();
    DeleteServerFile(SMART_DEV_FILE1);
    g_CS.lockoff();
}

//======================================================
//** 函数名称: ResetSmartTimer
//** 功能描述: 恢复出厂定时
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void ResetSmartTimer()
{
    g_CS.lockon();
    InitDefaultSmartTimer();
    DeleteServerFile(SMART_TIMER_FILE);
    g_CS.lockoff();
}

//======================================================
//** 函数名称: AddSmartTimer
//** 功能描述: 添加定时
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void AddSmartTimer(ECB_DATA *pECB)
{
    PSmartTimer pobj = new SmartTimer;

    pobj->hour = pECB->data[8];
    pobj->minute = pECB->data[9];
    pobj->week = pECB->data[10];
    memcpy(&pobj->device, &pECB->data[11], 7);
    pobj->way = pECB->data[18];
    memcpy(&pobj->param, &pECB->data[19], 2);
    pobj->param = htons(pobj->param);
    pobj->onoff = TRUE;

    GetTimerShow(pobj);

    if (m_timerlist.GetCount() < 16)
    {
        m_timerlist.AddTail(&pobj->lpObj);
        UpdatSmartTimerSet();
    }
    else
    {
        delete pobj;
    }
}

//======================================================
//** 函数名称: AddSmartDev
//** 功能描述: 添加设备列表
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void AddSmartDev(ECB_DATA *pECB)
{
    DEVICE device;
    BYTE scene = 0;
    // 红外电视编码参数
    WORD IR_TV_OL, IR_TV_CHANNEL1, IR_TV_CHANNEL2,
         IR_TV_VOICE1, IR_TV_VOICE2, IR_TV_SWITCH;

    // 红外空调编码参数
    WORD IR_AIR_H, IR_AIR_M, IR_AIR_L, IR_AIR_T,
         IR_AIR_CODE, IR_AIR_HOT, IR_AIR_ONOFF;

    char szName[32] = "";                                   // 设备名称
    //BYTE num = pECB->data[7];								// 序号(1 Byte)
    BYTE page = pECB->data[8] - 1;							// 页号(1 Byte)
    BYTE bit = pECB->data[9] - 1;							// 位号(1 Byte)
    BYTE type = pECB->data[10];								// 类型(1 Byte)

    // 如果是红外类型(红外电视)
    if (type >= ST_TV_A && type <= ST_TV_D)  					// 参数 (7～10 Byte)
    {
        memcpy(&device, &pECB->data[11], 7);
        switch(pECB->data[18])
        {
            case 53:  // 电视开/关
                IR_TV_OL = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_TV_CODE(IR_TV_OL, 0);
                break;
            case 54:  // 频道+
                IR_TV_CHANNEL1 = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_TV_CODE(IR_TV_CHANNEL1, 1);
                break;
            case 55:  // 频道-
                IR_TV_CHANNEL2 = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_TV_CODE(IR_TV_CHANNEL2, 2);
                break;
            case 56:  // 音量+
                IR_TV_VOICE1 = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_TV_CODE(IR_TV_VOICE1, 3);
                break;
            case 57:  // 音量-
                IR_TV_VOICE2 = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_TV_CODE(IR_TV_VOICE2, 4);
                break;
            case 58:  // TV/AV切换
                IR_TV_SWITCH = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_TV_CODE(IR_TV_SWITCH, 5);
                break;
            default:
                break;
        }

        if (pECB->length - 32 <= 16 && pECB->length - 32 > 0)	  // 信息(1～16 Byte) 字符转换 英->汉
        {
            char szGBK[16];
            WORD szUnicode[16];

            memcpy(szGBK, &pECB->data[21], pECB->length - 32);
            szGBK[pECB->length - 32] = 0;

            GbConvert(szUnicode, (BYTE *)szGBK);
            unicode2utf8((BYTE *)szName, (wchar_t *)szUnicode);
        }
    }
    // 如果是红外空调类型
    else if (type >= ST_IRAIR_A && type <= ST_IRAIR_D)
    {
        memcpy(&device, &pECB->data[11], 7);
        switch(pECB->data[18])
        {
            case 1:	 // 开
                IR_AIR_ONOFF = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_ONOFF, 0);
				break;
            case 2:	 // 关
                IR_AIR_ONOFF = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_ONOFF, 1);
                break;
            case 8:  // 制热
                IR_AIR_HOT = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_HOT, 2);
                break;
            case 9:  // 制冷
                IR_AIR_CODE = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_CODE, 3);
                break;
            case 10: // 通风
                IR_AIR_T = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_T, 4);
                break;
            case 24: // 风速低
                IR_AIR_L = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_L, 5);
                break;
            case 25: // 风速中
                IR_AIR_M = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_M, 6);
                break;
            case 26: // 风速高
                IR_AIR_H = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_H, 7);
                break;
            default:
                break;
        }

        if(pECB->length - 32 <= 16 && pECB->length - 32 > 0)	  // 信息(1～16 Byte) 字符转换 英->汉
        {
            char szGBK[16];
            WORD szUnicode[16];

            memcpy(szGBK, &pECB->data[21], pECB->length - 32);
            szGBK[pECB->length - 32] = 0;

            GbConvert(szUnicode, (BYTE *)szGBK);
            unicode2utf8((BYTE *)szName, (wchar_t *)szUnicode);
        }
    }
    else if (type >= ST_SCENE_A && type <= ST_SCENE_Z)
    {
        scene = pECB->data[11];
        device.channel = 4;
        device.id = GetDevID();
        device.type = GetDevType();
        if(pECB->length - 23 <= 16 && pECB->length - 23 > 0)	  // 信息(1～16 Byte)
        {
            char szGBK[16];
            WORD szUnicode[16];

            memcpy(szGBK, &pECB->data[12], pECB->length - 23);
            szGBK[pECB->length - 23] = 0;

            GbConvert(szUnicode, (BYTE *)szGBK);
            unicode2utf8((BYTE *)szName, (wchar_t *)szUnicode);
        }
    }
    // 非红外设备类型
    else
    {
        memcpy(&device, &pECB->data[11], 7); 			          // 设备标识
        if(pECB->length - 29 <= 16 && pECB->length - 29 > 0)	  // 信息(1～16 Byte)
        {
            char szGBK[16];
            WORD szUnicode[16];

            memcpy(szGBK, &pECB->data[18], pECB->length - 29);
            szGBK[pECB->length - 29] = 0;

            GbConvert(szUnicode, (BYTE *)szGBK);
            unicode2utf8((BYTE *)szName, (wchar_t *)szUnicode);
        }
    }

    if (page >= MAX_PAGE_NUM || bit >= MAX_ICON_NUM)
        return;

    g_CS.lockon();
    WORD index = page * MAX_ICON_NUM + bit;
    // 如果当前是默认配置,清空配置
    if (g_pSmartDevSet->bDefault)
    {
        g_pSmartDevSet->bDefault = FALSE;
        g_pSmartDevSet->count = 0;
        g_pSmartDevSet->dwSceneCtrl = 0;
        memset(g_pSmartDevSet->device, 0, sizeof(g_pSmartDevSet->device));
    }

    // 设备未存在,设备数量 +1
    if (!g_pSmartDevSet->device[index].exist)
    {
        g_pSmartDevSet->count++;
        g_pSmartDevSet->device[index].exist = TRUE;
    }

    g_pSmartDevSet->device[index].type = type;            		// 设备类型
    memcpy(&g_pSmartDevSet->device[index].device, &device, 7);	// 设备标识
    strcpy(g_pSmartDevSet->device[index].name, szName);   		// 设备名字
    if (type >= ST_SCENE_A && type <= ST_SCENE_Z)
        g_pSmartDevSet->device[index].scene = scene;			// 情景编号

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
//** 日　期: 2018年11月19日
//======================================================
void AddSceneCtrlList(ECB_DATA *pECB)
{
    WORD param;
    DEVICE device;

    memcpy(&device, &pECB->data[7], 7);		// 控制设备(7 Byte)
    BYTE num =	pECB->data[14];				// 存储编号(1 Byte)
    BYTE scene = pECB->data[15];			// 场景编号(1 Byte)
    BYTE cmd =	pECB->data[16];				// 控制方式(1 Byte)
    memcpy(&param, &pECB->data[17], 2);		// 控制参数(2 Byte)
    param = htons(param);

    g_CS.lockon();
    if (g_pSmartDevSet->dwSceneCtrl + 1 > g_dwMaxScene)
    {
        g_dwMaxScene += 255;
        SceneCtrl *pSceneCtrl = (SceneCtrl *)realloc(g_pSceneCtrl, sizeof(SceneCtrl) * g_dwMaxScene);
        if (pSceneCtrl == NULL)
        {
            g_CS.lockoff();
            return;
        }
        g_pSceneCtrl = pSceneCtrl;
    }

	for (int i = 0; i < g_pSmartDevSet->dwSceneCtrl; i++)
	{
		if (num == g_pSceneCtrl[i].num && 
			scene == g_pSceneCtrl[i].scene)
		{
			g_CS.lockoff();
            return;
		}
	}

    memcpy(&g_pSceneCtrl[g_pSmartDevSet->dwSceneCtrl].device, &device, 7);
    g_pSceneCtrl[g_pSmartDevSet->dwSceneCtrl].num = num;
    g_pSceneCtrl[g_pSmartDevSet->dwSceneCtrl].scene = scene;
    g_pSceneCtrl[g_pSmartDevSet->dwSceneCtrl].cmd = cmd;
    g_pSceneCtrl[g_pSmartDevSet->dwSceneCtrl].param = param;
    g_pSmartDevSet->dwSceneCtrl++;
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** 函数名称: AddLightStudy
//** 功能描述: 添加灯光学习
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void AddLightStudy(ECB_DATA *pECB)
{
    static SENSOR dwsensor = {0};
    LightStudy *plightStudy = NULL;
    BYTE bindFlag = FALSE;


    int size = sizeof(WRTBindAsInitiatorTable);
    for (int i = 0; i < size; i++)
    {
        if (pECB->data[5] == WRTBindAsInitiatorTable[i])
        {
            bindFlag = TRUE;
            break;
        }
    }

    if (bindFlag != TRUE)
        return;

    size = sizeof(WRTBindDataClearH4);
    for (int i = 0; i < size; i++)
    {
        if (pECB->data[6] == WRTBindDataClearH4[i])
        {
            pECB->data[6] &= 0x0f;
            break;
        }
    }

    if (g_dwStudyChan < 3)
        plightStudy = GetLightStudy(g_dwStudyChan);

    for (int i = 0; i < 16; i++)
    {
        if (!memcmp(&plightStudy->senosr[i], &pECB->data[1], sizeof(SENSOR)))
        {
            if (plightStudy->type == 1 && i < 8)
            {
                DPPostMessage(MSG_BROADCAST, SMART_LIGHT_STUDY, plightStudy->type, 1);
            }
            else if ((plightStudy->type == 1 && i >= 8))
            {
                DPPostMessage(MSG_BROADCAST, SMART_LIGHT_STUDY, plightStudy->type, 2);
            }
            else if ((plightStudy->type == 2 && i < 8))
            {
                DPPostMessage(MSG_BROADCAST, SMART_LIGHT_STUDY, plightStudy->type, 2);
            }
            else if ((plightStudy->type == 2 && i >= 8))
            {
                DPPostMessage(MSG_BROADCAST, SMART_LIGHT_STUDY, plightStudy->type, 1);
            }
            return;
        }
    }

    if (g_dwStudyNum == 1 && plightStudy->type == 1)
    {
        if (plightStudy->singleNum < 8)
        {
            memcpy(&plightStudy->senosr[plightStudy->singleNum], &pECB->data[1], sizeof(SENSOR));
            plightStudy->singleNum++;
        }
        else
        {
            memcpy(&plightStudy->senosr[7], &pECB->data[1], sizeof(SENSOR));
            plightStudy->singleNum = 8;
        }
        UpdateSmartDevSet();
        DPPostMessage(MSG_BROADCAST, SMART_LIGHT_STUDY, plightStudy->type, 0);
    }
    else if (plightStudy->type == 2)
    {
        if (g_dwStudyNum == 2)
        {
            memcpy(&dwsensor, &pECB->data[1], sizeof(SENSOR));
            g_dwStudyNum--;
        }
        else if (g_dwStudyNum == 1)
        {
            if (plightStudy->doubleNum < 8)
            {
                if (!memcmp(&dwsensor, &pECB->data[1], sizeof(SENSOR)))
                {
                    DBGMSG(DPINFO, "key1 and key2 is same\r\n");
                    return;
                }
                memcpy(&plightStudy->senosr[8 + plightStudy->doubleNum], &dwsensor, sizeof(SENSOR));
                memcpy(&plightStudy->senosr[8 + plightStudy->doubleNum + 1], &pECB->data[1], sizeof(SENSOR));

                plightStudy->doubleNum += 2;
                if (plightStudy->doubleNum % 2)
                    plightStudy->doubleNum--;
            }
            else
            {
                if (!memcmp(&dwsensor, &pECB->data[1], sizeof(SENSOR)))
                {
                    DBGMSG(DPINFO, "key1 and key2 is same\r\n");
                    return;
                }
                memcpy(&plightStudy->senosr[14], &dwsensor, sizeof(SENSOR));
                memcpy(&plightStudy->senosr[15], &pECB->data[1], sizeof(SENSOR));
                plightStudy->doubleNum = 8;
            }
            UpdateSmartDevSet();
            g_dwStudyNum = 0;
            DPPostMessage(MSG_BROADCAST, SMART_LIGHT_STUDY, plightStudy->type, 0);
        }
    }
}

//======================================================
//** 函数名称: SetLightStudySync
//** 功能描述: 设置灯光学习同步
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SetLightStudySync(ECB_DATA *pECB)
{
    BYTE bindFlag = FALSE;

    int size = sizeof(WRTBindAsInitiatorTable);
    for (int i = 0; i < size; i++)
    {
        if (pECB->data[5] == WRTBindAsInitiatorTable[i])
        {
            bindFlag = TRUE;
            break;
        }
    }

    if (bindFlag != TRUE)
        return;

    size = sizeof(WRTBindDataClearH4);
    for (int i = 0; i < size; i++)
    {
        if (pECB->data[6] == WRTBindDataClearH4[i])
        {
            pECB->data[6] &= 0x0f;
            break;
        }
    }

    for (int i = 0; i < 3; i++)
    {
        LightStudy *plightStudy = GetLightStudy(i);
        for (int j = 0; j < 8; j++)
        {
            if (!memcmp(&plightStudy->senosr[j], &pECB->data[1], sizeof(SENSOR)))
            {
                g_pSmartDevSet->device[i].cmd =
                    (g_pSmartDevSet->device[i].cmd == SCMD_OPEN) ? SCMD_CLOSE : SCMD_OPEN;

                if (g_pSmartDevSet->device[i].cmd == SCMD_OPEN)
                    SetLightGpioVal(i + 1, FALSE);	// 亮
                else
                    SetLightGpioVal(i + 1, TRUE);	// 灭
                    
                SendStatusCmd(&g_pSmartDevSet->device[i].device, g_pSmartDevSet->device[i].cmd, 0);    
            }
        }
        for (int j = 8; j < 16; j++)
        {
            if (!memcmp(&plightStudy->senosr[j], &pECB->data[1], sizeof(SENSOR)))
            {

                g_pSmartDevSet->device[i].cmd = (j % 2 == 1) ? SCMD_CLOSE : SCMD_OPEN;

                if (g_pSmartDevSet->device[i].cmd == SCMD_OPEN)
                    SetLightGpioVal(i + 1, FALSE);	// 亮
                else
                    SetLightGpioVal(i + 1, TRUE);	// 灭

				SendStatusCmd(&g_pSmartDevSet->device[i].device, g_pSmartDevSet->device[i].cmd, 0);	
            }
        }
        DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, (DWORD)&g_pSmartDevSet->device[i], 0);
    }

}

//======================================================
//** 函数名称: SetStatusBySync
//** 功能描述: 设备设备状态设备同步
//** 输　入: device cmd param
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SetStatusBySync(DEVICE *device, BYTE cmd, WORD param)
{
    g_CS.lockon();
    for (int i = 0; i < MAX_DEV_NUM; i++)
    {
        SmartDev *pSmartDev = &g_pSmartDevSet->device[i];
        if (pSmartDev->exist != TRUE
                || (pSmartDev->device.id == INVALID_DEV_ID
                    && pSmartDev->device.type == INVALID_DEV_TYPE))
            continue;
        if (!memcmp(&pSmartDev->device, device, 7))
        {
            switch (cmd)
            {
                case SCMD_OPEN:
                    pSmartDev->cmd = cmd;
                    pSmartDev->status = param;
                    break;
                case SCMD_DIMMER_OPEN:
                    if (param)
                    {
                        pSmartDev->cmd = SCMD_OPEN;
                        pSmartDev->status = param;
                    }
                    else
                    {
                        pSmartDev->cmd = SCMD_CLOSE;
                        pSmartDev->status = 0;
                    }
                    break;
                case SCMD_CLOSE:
                    pSmartDev->cmd = cmd;
                    pSmartDev->status = param;
                    break;
                case SCMD_DELAY_OPEN:
                    pSmartDev->cmd = SCMD_OPEN;
                    pSmartDev->status = param;
                    break;
                case SCMD_DELAY_CLOSE:
                    pSmartDev->cmd = SCMD_CLOSE;
                    pSmartDev->status = 0;
                    break;
                case SCMD_DELAY_OPEN_CLOSE:
                    pSmartDev->cmd = SCMD_CLOSE;
                    pSmartDev->status = 0;
                    break;
                case SCMD_CURTAIN_OPEN:
                    if (param)
                    {
                        pSmartDev->cmd = SCMD_OPEN;
                        pSmartDev->status = param;
                    }
                    else
                    {
                        pSmartDev->cmd = SCMD_CLOSE;
                        pSmartDev->status = 0;
                    }
                    break;
                case SCMD_CURTAIN_CLOSE:
                    pSmartDev->cmd = SCMD_CLOSE;
                    pSmartDev->status = 0;
                    break;
                case SCMD_OPEN_CLOSE:
                    if (pSmartDev->cmd == SCMD_CLOSE)
                    {
                        pSmartDev->cmd = SCMD_OPEN;
                        pSmartDev->status = 0;
                    }
                    else
                    {
                        pSmartDev->cmd = SCMD_CLOSE;
                        pSmartDev->status = 0;
                    }
                case SCMD_AC:
                case SCMD_MUSIC:
				case 0x80://特殊处理新风	
                    if (param & 0x4000)
                    {
                        pSmartDev->cmd = SCMD_OPEN;
                        pSmartDev->status = param;
                    }
                    else
                    {
                        pSmartDev->cmd = SCMD_CLOSE;
                        pSmartDev->status = param;
                    }
                    break;
                default:
                    break;
            }
        }
    }
    g_CS.lockoff();
    //DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, (DWORD)device, param);
}

//======================================================
//** 函数名称: SetStatusByScene
//** 功能描述: 设备设备状态情景同步
//** 输　入: scene
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SetStatusByScene(BYTE scene)
{
    for (DWORD i = 0; i < g_pSmartDevSet->dwSceneCtrl; i++)
    {
        if (scene == g_pSceneCtrl[i].scene)
        {
            SetStatusBySync(&g_pSceneCtrl[i].device,
                            g_pSceneCtrl[i].cmd,
                            g_pSceneCtrl[i].param);
        }
    }

    g_CS.lockon();
    for (int i = 0; i < MAX_DEV_NUM; i++)
    {
        if (g_pSmartDevSet->device[i].exist == TRUE
                && g_pSmartDevSet->device[i].scene > 0)
        {
            if (g_pSmartDevSet->device[i].scene == scene)
            {
                g_pSmartDevSet->device[i].cmd = SCMD_OPEN;
            }
            else if (g_pSmartDevSet->device[i].scene != scene)
            {
                g_pSmartDevSet->device[i].cmd = SCMD_CLOSE;
            }
        }
    }
    g_CS.lockoff();
    //DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, 0, 0);

}

//======================================================
//** 函数名称: SetStatusByAll
//** 功能描述: 设置所有设备状态
//** 输　入: cmd
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SetStatusByAll(BYTE cmd)
{
    g_CS.lockon();
    for (int i = 0; i < MAX_DEV_NUM; i++)
    {
        SmartDev *pSmartDev = &g_pSmartDevSet->device[i];
        if (pSmartDev->exist && pSmartDev->scene == 0)
        {
            pSmartDev->cmd = cmd;
        }
    }
    g_CS.lockoff();

    for (int i = 0; i < 3; i++)
    {
        if (cmd == SCMD_OPEN)
        {
            SetLightGpioVal(i + 1, FALSE);
        }
        else
        {
            SetLightGpioVal(i + 1, TRUE);
        }
    }

    DPPostMessage(MSG_BROADCAST, SMART_STATUS_SCENE, (DWORD)&g_pSmartDevSet->device[0], 0);
}

//======================================================
//** 函数名称: GetSmartDevByDev
//** 功能描述: 获取设备表中设备
//** 输　入: device
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
DEVICE *GetSmartDevByDev(DEVICE *device)
{
    DEVICE *SmartDev = NULL;
    g_CS.lockon();
    for (int i = 0; i < MAX_DEV_NUM; i++)
    {
        if (!memcmp(&g_pSmartDevSet->device[i].device, device, 7))
        {
            SmartDev = &g_pSmartDevSet->device[i].device;
            break;
        }
    }
    g_CS.lockoff();
    return SmartDev;
}

//======================================================
//** 函数名称: GetTimerShow
//** 功能描述: 获取定时中文字符串
//** 输　入: pItem
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void GetTimerShow(PSmartTimer pItem)
{
    memset(pItem->timeStr, 0, sizeof(pItem->timeStr));
    sprintf(pItem->timeStr, "%02d:%02d", pItem->hour, pItem->minute);

    memset(pItem->wayStr, 0, sizeof(pItem->wayStr));
    switch (pItem->way)
    {
        case SCMD_OPEN: 				// 打开
            strcpy(pItem->wayStr, GetStringByID(13401));
            break;
        case SCMD_CLOSE:				// 关闭
            strcpy(pItem->wayStr, GetStringByID(13402));
            break;
        case SCMD_DELAY_OPEN:			// 延时开
            sprintf(pItem->wayStr, "%s(%ds)", GetStringByID(13403), pItem->param);
            break;
        case SCMD_DELAY_CLOSE:		// 延时关
            sprintf(pItem->wayStr, "%s(%ds)", GetStringByID(13404), pItem->param);
            break;
        case SCMD_DELAY_OPEN_CLOSE: 	// 开延时关
            sprintf(pItem->wayStr, "%s(%ds)", GetStringByID(13405), pItem->param);
            break;
        case SCMD_SCENE: 			// 场景
            strcpy(pItem->wayStr, GetStringByID(13407));
            break;
        default:
            break;
    }

    memset(pItem->devStr, 0, sizeof(pItem->devStr));
    for (int i = 0; i < MAX_DEV_NUM; i++)
    {
        if (pItem->way == SCMD_SCENE)
        {
            if (g_pSmartDevSet->device[i].exist == TRUE
                    && pItem->param == g_pSmartDevSet->device[i].scene)
            {
                if (g_pSmartDevSet->device[i].type >= ST_SCENE_A
                        && g_pSmartDevSet->device[i].type <= ST_SCENE_Z)
                {
                    strcpy(pItem->devStr, g_pSmartDevSet->device[i].name);
                    break;
                }
            }
        }
        else
        {
            if (g_pSmartDevSet->device[i].exist == TRUE
                    && !memcmp(&g_pSmartDevSet->device[i].device, &pItem->device, 7))  	// 普通设备名字显示
            {
                strcpy(pItem->devStr, g_pSmartDevSet->device[i].name);
                break;
            }
        }
    }

    memset(pItem->weekStr, 0, sizeof(pItem->weekStr));
    if ((pItem->week & 0x80) == 0)
    {
        if ((pItem->week & 0x7f) == 0x7f)
        {
            strcpy(pItem->weekStr, GetStringByID(13207));
        }
        else if ((pItem->week & 0x1f) == 0x1f)
        {
            for (int i = 5; i < 7; i++)
            {
                if (pItem->week & (1 << i))
                {
                    strcat(pItem->weekStr, GetStringByID(13200 + i));
                    strcat(pItem->weekStr, ",");
                }
            }
            strcat(pItem->weekStr, GetStringByID(13208));
        }
        else if ((pItem->week & 0x60) == 0x60)
        {
            for (int i = 0; i < 5; i++)
            {
                if (pItem->week & (1 << i))
                {
                    strcat(pItem->weekStr, GetStringByID(13200 + i));
                    strcat(pItem->weekStr, ",");
                }
            }
            strcat(pItem->weekStr, GetStringByID(13209));
        }
        else if ((pItem->week & 0x80) != 0)
        {
            strcpy(pItem->weekStr, GetStringByID(13210));
        }
        else
        {
            for (int i = 0; i < 7; i++)
            {
                if (pItem->week & (1 << i))
                {
                    strcat(pItem->weekStr, GetStringByID(13200 + i));
                    strcat(pItem->weekStr, ",");
                }
            }
            if (strlen(pItem->weekStr) > 0)
                pItem->weekStr[strlen(pItem->weekStr) - 1] = '\0';
        }
    }
    else
    {
        strcpy(pItem->weekStr, GetStringByID(13210));
    }
}

//======================================================
//** 函数名称: SetSmartInit
//** 功能描述: 设置设备表默认标识
//** 输　入: flag
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SetSmartInit(DWORD flag)
{
    g_CS.lockon();
    //g_pSmartDevSet->bDefault = flag;
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** 函数名称: SetSmartAddr
//** 功能描述: 设置物理地址
//** 输　入: addr
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
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
//** 功能描述: 获取物理地址
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
DWORD GetSmartAddr()
{
    DWORD addr;
    g_CS.lockon();
    addr = g_pSmartDevSet->addr;
    g_CS.lockoff();
    return addr;
}

//======================================================
//** 函数名称: SetSmartPwd
//** 功能描述: 设置系统密码
//** 输　入: pwd
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SetSmartPwd(DWORD pwd)
{
    g_CS.lockon();
    g_pSmartDevSet->pwd = pwd;
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** 函数名称: GetSmartPwd
//** 功能描述: 获取系统密码
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
DWORD GetSmartPwd()
{
    DWORD pwd;
    g_CS.lockon();
    pwd = g_pSmartDevSet->pwd;
    g_CS.lockoff();
    return pwd;
}

//======================================================
//** 函数名称: SetSmartRoute
//** 功能描述: 设置ZIGBEE路由开关
//** 输　入: route
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SetSmartRoute(DWORD route)
{
    g_CS.lockon();
    g_pSmartDevSet->route = route;
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** 函数名称: SetSmartEnvId
//** 功能描述: 获取ZIGBEE路由开关
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
DWORD GetSmartRoute()
{
    DWORD route;
    g_CS.lockon();
    route = g_pSmartDevSet->route;
    g_CS.lockoff();
    return route;
}

//======================================================
//** 函数名称: SetSmartEnvId
//** 功能描述: 设置传感器ID(温度)
//** 输　入: id
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SetSmartEnvId(DWORD id)
{
    g_CS.lockon();
    g_pSmartDevSet->id = id;
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** 函数名称: GetSmartEnvId
//** 功能描述: 获取传感器ID(温度)
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
DWORD GetSmartEnvId()
{
    DWORD id;
    g_CS.lockon();
    id = g_pSmartDevSet->id;
    g_CS.lockoff();
    return id;
}

//======================================================
//** 函数名称: GetLightStudy
//** 功能描述: 获取灯光学习
//** 输　入: i
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
LightStudy *GetLightStudy(int i)
{
    LightStudy *plightStudy;
    g_CS.lockon();
    plightStudy = &g_pSmartDevSet->study[i];
    g_CS.lockoff();
    return plightStudy;
}

//======================================================
//** 函数名称: SetLightStudy
//** 功能描述: 设置灯光学习
//** 输　入: lightstudy i
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SetLightStudy(LightStudy lightstudy, int i)
{
    g_CS.lockon();
    memcpy(&g_pSmartDevSet->study[i], &lightstudy, sizeof(LightStudy));
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** 函数名称: SetSmartUi
//** 功能描述: ui设置保存
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年12月27日
//======================================================
void SetSmartUi()
{
    g_CS.lockon();
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** 函数名称: SetStudyChan
//** 功能描述: 设置学习通路
//** 输　入: i
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SetStudyChan(DWORD i)
{
    g_CS.lockon();
    g_dwStudyChan = i;
    g_CS.lockoff();
}

//======================================================
//** 函数名称: SetStudyNum
//** 功能描述: 设置学习个数
//** 输　入: i
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SetStudyNum(DWORD i)
{
    g_CS.lockon();
    g_dwStudyNum = i;
    g_CS.lockoff();
}

//======================================================
//** 函数名称: GetSmartDev
//** 功能描述: 获取设备表指针
//** 输　入: count
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
SmartDev *GetSmartDev(DWORD *count)
{
    SmartDev *pSmartDev;
    g_CS.lockon();
    *count = g_pSmartDevSet->count;
    pSmartDev = g_pSmartDevSet->device;
    g_CS.lockoff();
    return pSmartDev;
}

//======================================================
//** 函数名称: GetTimerNext
//** 功能描述: 获取定时器头
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
PSmartTimer GetTimerHead()
{
    PSmartTimer pItem;
    g_CS.lockon();
    pItem = (PSmartTimer) m_timerlist.Head();
    g_CS.lockoff();
    return pItem;
}

//======================================================
//** 函数名称: GetTimerNext
//** 功能描述: 获取定时器下一条
//** 输　入: pobject
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
PSmartTimer GetTimerNext(LPLISTOBJ pobject)
{
    PSmartTimer pItem;
    g_CS.lockon();
    pItem = (PSmartTimer) m_timerlist.Next(pobject);
    g_CS.lockoff();
    return pItem;
}

//======================================================
//** 函数名称: GetTimerCount
//** 功能描述: 获取定时器条数
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
DWORD GetTimerCount()
{
    DWORD count;
    g_CS.lockon();
    count = m_timerlist.GetCount();
    g_CS.lockoff();
    return count;
}
