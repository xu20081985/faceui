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
    DWORD		addr;                                   // �����ַ
    DWORD		pwd;                                    // ϵͳ����
    DWORD		route;                                  // ·��ģʽ
    DWORD		id;										// ������ID-�¶�
    DWORD		count;                                  // �豸����
    SmartDev	device[MAX_PAGE_NUM * MAX_ICON_NUM];      // �豸��Ϣ
    DWORD		dwSceneCtrl;							// �龰��������
    LightStudy	study[MAX_CHAN_NUM];						// �ƹ�ѧϰ��Ϣ
    BOOL		bDefault;								// Ĭ�����ñ�ʶ
    DWORD		VERSION;                                // д�ļ���ʶ
    DWORD		Endid;									// ������־
} SmartDevSet;

static StaticLock		g_CS;							 // �ļ�������
static CMyList 			m_timerlist;					 // ��ʱ�б�
static DWORD			g_dwMaxScene;					 // ����龰�豸����
static SceneCtrl		*g_pSceneCtrl;					 // �龰ģʽ������Ϣ
static SmartDevSet		*g_pSmartDevSet;                  // ���ܼҾ��豸��Ϣ
static DWORD			g_dwStudyChan;					 // �ƹ⵱ǰѧϰͨ·
static DWORD			g_dwStudyNum;					 // �ƹ⵱ǰѧϰ����


const UINT8 WRTBindAsInitiatorTable[] =
{
    0x01,//���˼�ң��������û��ѹ��
    0x02,//���Ŵ��Ŵ�����������4λ��գ�
    0x03,//���񶯴���������û��ѹ��
    0x04,//�������ƶ�������������4λ��գ�
    0x08,//�����ء�
    0x09,//�����롷
    0x11,//��������
    0x23,//��û��ѹ��
    0x24, 0x25, //����4λ��գ�
    0x26,//��û��ѹ��
    0x2c, 0x2d, 0x2e, 0x2F, 0x30, 0x31, 0x32, 0x33, //�����ܿ�����塷
    0x3C, 0x3d, 0x3e, 0x3F, 0x40, 0x41, 0x42, 0x43, //<��е���ش�������
    0x44,//��·������
    0x45,//���߱������⣨��4λ��գ�
    0x50, 0x52, //��X��ң������
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, //����ң����������4λ��գ�
    0x6B,//���Ŵ��Ŵ�����������4λ��գ�
    0x6C,//������̽����������4λ��գ�
    0x6D, 0x6F, 0x70, 0x71, //��������մ�����������4λ��գ�
    0x2A, 0x2B, //��һ/��·���ܴ���������塷
    0X80,//����·������塷
};

const UINT8 WRTBindDataClearH4[] =
{
    0x02,//���Ŵ��Ŵ�����������4λ��գ�
    0x04,//�������ƶ�������������4λ��գ�
    0x24, 0x25, //����4λ��գ�
    0x45,//���߱������⣨��4λ��գ�
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, //����ң����������4λ��գ�
    0x6B,//���Ŵ��Ŵ�����������4λ��գ�
    0x6C,//������̽����������4λ��գ�
    0x6D, 0x6F, 0x70, 0x71 //��������մ�����������4λ��գ�
};

//======================================================
//** ��������: UpdateSmartDevSet
//** ��������: �����豸�б�
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: UpdatSmartTimerSet
//** ��������: ���¶�ʱ�б�
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: InitDefaultSmartDevSet
//** ��������: ��ʼ��Ĭ���豸�б�
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void InitDefaultSmartDevSet()
{
    DWORD page = 0;
    DWORD index = 0;
    DWORD id = ReadCpuID();
    WORD type = htons(ECB_DEV_TYPE);

    memset(g_pSmartDevSet, 0, sizeof(SmartDevSet));

    // ��һҳ
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

    // �ڶ�ҳ
    page = 1 * MAX_ICON_NUM;
    index = 0;

    // ����
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_LIGHT_B;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2000));

    // ����
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_CURTAIN_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2110));

    // �����
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_DIMMER_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2010));

    // ����
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_OUTLET_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2130));

    // ����
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_MUSIC_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2070));

    // ����ҳ
    page = 2 * MAX_ICON_NUM;
    index = 0;
	// �·�
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_WIND_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2050));

    // �յ�
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_AC_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2020));

    // ��ů
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_HEAT_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2040));

    // �������
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_TV_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2080));

    // ����յ�
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = INVALID_DEV_CHAN;
    g_pSmartDevSet->device[page + index].device.id = INVALID_DEV_ID;
    g_pSmartDevSet->device[page + index].device.type = INVALID_DEV_TYPE;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_IRAIR_A;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(2030));

    // ����ҳ
    page = 3 * MAX_ICON_NUM;
    index = 0;

    // �ڼ�
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = 4;
    g_pSmartDevSet->device[page + index].device.id = id;
    g_pSmartDevSet->device[page + index].device.type = type;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_SCENE_F;
    g_pSmartDevSet->device[page + index].scene = 3;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(3005));

    // ���
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = 4;
    g_pSmartDevSet->device[page + index].device.id = id;
    g_pSmartDevSet->device[page + index].device.type = type;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_SCENE_G;
    g_pSmartDevSet->device[page + index].scene = 4;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(3006));

    // �Ͳ�
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = 4;
    g_pSmartDevSet->device[page + index].device.id = id;
    g_pSmartDevSet->device[page + index].device.type = type;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_SCENE_B;
    g_pSmartDevSet->device[page + index].scene = 5;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(3001));

    // ӰԺ
    g_pSmartDevSet->device[page + index].exist = TRUE;
    g_pSmartDevSet->device[page + index].device.channel = 4;
    g_pSmartDevSet->device[page + index].device.id = id;
    g_pSmartDevSet->device[page + index].device.type = type;
    g_pSmartDevSet->device[page + index].phy_addr = INVALID_PHY_ADDR;
    g_pSmartDevSet->device[page + index].type = ST_SCENE_C;
    g_pSmartDevSet->device[page + index].scene = 6;
    strcpy(g_pSmartDevSet->device[page + index++].name, GetStringByID(3002));

    // ��ܰ
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
//** ��������: testtimer
//** ��������: ��ʱ������Ӷ�ʱ-������
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: SmartTimerTest
//** ��������: ��ʱ���ܲ�������-������
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: InitDefaultSmartTimer
//** ��������: ��ʼ��Ĭ�϶�ʱ
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: InitSmartDev
//** ��������: ��ʼ���豸
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void InitSmartDev()
{
    FILE *fd;
    char filename[64];
    SmartDevSet *pSet0 = NULL;
    SmartDevSet *pSet1 = NULL;
    SceneCtrl *pSceneCtrl0 = NULL;    // ����
    SceneCtrl *pSceneCtrl1 = NULL;

    g_CS.lockon();
    // ��ȡ�����ļ�
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

    // �������豸�����ļ�����ʼ��Ĭ���豸
    if((pSet0 == NULL) && (pSet1 == NULL))
    {
        g_pSmartDevSet = (SmartDevSet *)malloc(sizeof(SmartDevSet));
        if(g_pSmartDevSet)
        {
            memset(g_pSmartDevSet, 0, sizeof(SmartDevSet));
            InitDefaultSmartDevSet();     // ��ʼ��Ĭ���豸
            UpdateSmartDevSet();          // ������Ϣд���ļ�
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
    // ����״̬����
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
//** ��������: InitSmartTimer
//** ��������: ��ʼ����ʱ
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: ResetSmartDev
//** ��������: �ָ������豸
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: ResetSmartTimer
//** ��������: �ָ�������ʱ
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void ResetSmartTimer()
{
    g_CS.lockon();
    InitDefaultSmartTimer();
    DeleteServerFile(SMART_TIMER_FILE);
    g_CS.lockoff();
}

//======================================================
//** ��������: AddSmartTimer
//** ��������: ��Ӷ�ʱ
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: AddSmartDev
//** ��������: ����豸�б�
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void AddSmartDev(ECB_DATA *pECB)
{
    DEVICE device;
    BYTE scene = 0;
    // ������ӱ������
    WORD IR_TV_OL, IR_TV_CHANNEL1, IR_TV_CHANNEL2,
         IR_TV_VOICE1, IR_TV_VOICE2, IR_TV_SWITCH;

    // ����յ��������
    WORD IR_AIR_H, IR_AIR_M, IR_AIR_L, IR_AIR_T,
         IR_AIR_CODE, IR_AIR_HOT, IR_AIR_ONOFF;

    char szName[32] = "";                                   // �豸����
    //BYTE num = pECB->data[7];								// ���(1 Byte)
    BYTE page = pECB->data[8] - 1;							// ҳ��(1 Byte)
    BYTE bit = pECB->data[9] - 1;							// λ��(1 Byte)
    BYTE type = pECB->data[10];								// ����(1 Byte)

    // ����Ǻ�������(�������)
    if (type >= ST_TV_A && type <= ST_TV_D)  					// ���� (7��10 Byte)
    {
        memcpy(&device, &pECB->data[11], 7);
        switch(pECB->data[18])
        {
            case 53:  // ���ӿ�/��
                IR_TV_OL = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_TV_CODE(IR_TV_OL, 0);
                break;
            case 54:  // Ƶ��+
                IR_TV_CHANNEL1 = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_TV_CODE(IR_TV_CHANNEL1, 1);
                break;
            case 55:  // Ƶ��-
                IR_TV_CHANNEL2 = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_TV_CODE(IR_TV_CHANNEL2, 2);
                break;
            case 56:  // ����+
                IR_TV_VOICE1 = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_TV_CODE(IR_TV_VOICE1, 3);
                break;
            case 57:  // ����-
                IR_TV_VOICE2 = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_TV_CODE(IR_TV_VOICE2, 4);
                break;
            case 58:  // TV/AV�л�
                IR_TV_SWITCH = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_TV_CODE(IR_TV_SWITCH, 5);
                break;
            default:
                break;
        }

        if (pECB->length - 32 <= 16 && pECB->length - 32 > 0)	  // ��Ϣ(1��16 Byte) �ַ�ת�� Ӣ->��
        {
            char szGBK[16];
            WORD szUnicode[16];

            memcpy(szGBK, &pECB->data[21], pECB->length - 32);
            szGBK[pECB->length - 32] = 0;

            GbConvert(szUnicode, (BYTE *)szGBK);
            unicode2utf8((BYTE *)szName, (wchar_t *)szUnicode);
        }
    }
    // ����Ǻ���յ�����
    else if (type >= ST_IRAIR_A && type <= ST_IRAIR_D)
    {
        memcpy(&device, &pECB->data[11], 7);
        switch(pECB->data[18])
        {
            case 1:	 // ��
                IR_AIR_ONOFF = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_ONOFF, 0);
				break;
            case 2:	 // ��
                IR_AIR_ONOFF = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_ONOFF, 1);
                break;
            case 8:  // ����
                IR_AIR_HOT = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_HOT, 2);
                break;
            case 9:  // ����
                IR_AIR_CODE = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_CODE, 3);
                break;
            case 10: // ͨ��
                IR_AIR_T = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_T, 4);
                break;
            case 24: // ���ٵ�
                IR_AIR_L = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_L, 5);
                break;
            case 25: // ������
                IR_AIR_M = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_M, 6);
                break;
            case 26: // ���ٸ�
                IR_AIR_H = (pECB->data[19] << 8 | pECB->data[20]);
                SetIR_AIR_CODE(IR_AIR_H, 7);
                break;
            default:
                break;
        }

        if(pECB->length - 32 <= 16 && pECB->length - 32 > 0)	  // ��Ϣ(1��16 Byte) �ַ�ת�� Ӣ->��
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
        if(pECB->length - 23 <= 16 && pECB->length - 23 > 0)	  // ��Ϣ(1��16 Byte)
        {
            char szGBK[16];
            WORD szUnicode[16];

            memcpy(szGBK, &pECB->data[12], pECB->length - 23);
            szGBK[pECB->length - 23] = 0;

            GbConvert(szUnicode, (BYTE *)szGBK);
            unicode2utf8((BYTE *)szName, (wchar_t *)szUnicode);
        }
    }
    // �Ǻ����豸����
    else
    {
        memcpy(&device, &pECB->data[11], 7); 			          // �豸��ʶ
        if(pECB->length - 29 <= 16 && pECB->length - 29 > 0)	  // ��Ϣ(1��16 Byte)
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
    // �����ǰ��Ĭ������,�������
    if (g_pSmartDevSet->bDefault)
    {
        g_pSmartDevSet->bDefault = FALSE;
        g_pSmartDevSet->count = 0;
        g_pSmartDevSet->dwSceneCtrl = 0;
        memset(g_pSmartDevSet->device, 0, sizeof(g_pSmartDevSet->device));
    }

    // �豸δ����,�豸���� +1
    if (!g_pSmartDevSet->device[index].exist)
    {
        g_pSmartDevSet->count++;
        g_pSmartDevSet->device[index].exist = TRUE;
    }

    g_pSmartDevSet->device[index].type = type;            		// �豸����
    memcpy(&g_pSmartDevSet->device[index].device, &device, 7);	// �豸��ʶ
    strcpy(g_pSmartDevSet->device[index].name, szName);   		// �豸����
    if (type >= ST_SCENE_A && type <= ST_SCENE_Z)
        g_pSmartDevSet->device[index].scene = scene;			// �龰���

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
//** �ա���: 2018��11��19��
//======================================================
void AddSceneCtrlList(ECB_DATA *pECB)
{
    WORD param;
    DEVICE device;

    memcpy(&device, &pECB->data[7], 7);		// �����豸(7 Byte)
    BYTE num =	pECB->data[14];				// �洢���(1 Byte)
    BYTE scene = pECB->data[15];			// �������(1 Byte)
    BYTE cmd =	pECB->data[16];				// ���Ʒ�ʽ(1 Byte)
    memcpy(&param, &pECB->data[17], 2);		// ���Ʋ���(2 Byte)
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
//** ��������: AddLightStudy
//** ��������: ��ӵƹ�ѧϰ
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: SetLightStudySync
//** ��������: ���õƹ�ѧϰͬ��
//** �䡡��: pECB
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
                    SetLightGpioVal(i + 1, FALSE);	// ��
                else
                    SetLightGpioVal(i + 1, TRUE);	// ��
                    
                SendStatusCmd(&g_pSmartDevSet->device[i].device, g_pSmartDevSet->device[i].cmd, 0);    
            }
        }
        for (int j = 8; j < 16; j++)
        {
            if (!memcmp(&plightStudy->senosr[j], &pECB->data[1], sizeof(SENSOR)))
            {

                g_pSmartDevSet->device[i].cmd = (j % 2 == 1) ? SCMD_CLOSE : SCMD_OPEN;

                if (g_pSmartDevSet->device[i].cmd == SCMD_OPEN)
                    SetLightGpioVal(i + 1, FALSE);	// ��
                else
                    SetLightGpioVal(i + 1, TRUE);	// ��

				SendStatusCmd(&g_pSmartDevSet->device[i].device, g_pSmartDevSet->device[i].cmd, 0);	
            }
        }
        DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, (DWORD)&g_pSmartDevSet->device[i], 0);
    }

}

//======================================================
//** ��������: SetStatusBySync
//** ��������: �豸�豸״̬�豸ͬ��
//** �䡡��: device cmd param
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
				case 0x80://���⴦���·�	
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
//** ��������: SetStatusByScene
//** ��������: �豸�豸״̬�龰ͬ��
//** �䡡��: scene
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: SetStatusByAll
//** ��������: ���������豸״̬
//** �䡡��: cmd
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: GetSmartDevByDev
//** ��������: ��ȡ�豸�����豸
//** �䡡��: device
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: GetTimerShow
//** ��������: ��ȡ��ʱ�����ַ���
//** �䡡��: pItem
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void GetTimerShow(PSmartTimer pItem)
{
    memset(pItem->timeStr, 0, sizeof(pItem->timeStr));
    sprintf(pItem->timeStr, "%02d:%02d", pItem->hour, pItem->minute);

    memset(pItem->wayStr, 0, sizeof(pItem->wayStr));
    switch (pItem->way)
    {
        case SCMD_OPEN: 				// ��
            strcpy(pItem->wayStr, GetStringByID(13401));
            break;
        case SCMD_CLOSE:				// �ر�
            strcpy(pItem->wayStr, GetStringByID(13402));
            break;
        case SCMD_DELAY_OPEN:			// ��ʱ��
            sprintf(pItem->wayStr, "%s(%ds)", GetStringByID(13403), pItem->param);
            break;
        case SCMD_DELAY_CLOSE:		// ��ʱ��
            sprintf(pItem->wayStr, "%s(%ds)", GetStringByID(13404), pItem->param);
            break;
        case SCMD_DELAY_OPEN_CLOSE: 	// ����ʱ��
            sprintf(pItem->wayStr, "%s(%ds)", GetStringByID(13405), pItem->param);
            break;
        case SCMD_SCENE: 			// ����
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
                    && !memcmp(&g_pSmartDevSet->device[i].device, &pItem->device, 7))  	// ��ͨ�豸������ʾ
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
//** ��������: SetSmartInit
//** ��������: �����豸��Ĭ�ϱ�ʶ
//** �䡡��: flag
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SetSmartInit(DWORD flag)
{
    g_CS.lockon();
    //g_pSmartDevSet->bDefault = flag;
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** ��������: SetSmartAddr
//** ��������: ���������ַ
//** �䡡��: addr
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: ��ȡ�����ַ
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: SetSmartPwd
//** ��������: ����ϵͳ����
//** �䡡��: pwd
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SetSmartPwd(DWORD pwd)
{
    g_CS.lockon();
    g_pSmartDevSet->pwd = pwd;
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** ��������: GetSmartPwd
//** ��������: ��ȡϵͳ����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: SetSmartRoute
//** ��������: ����ZIGBEE·�ɿ���
//** �䡡��: route
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SetSmartRoute(DWORD route)
{
    g_CS.lockon();
    g_pSmartDevSet->route = route;
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** ��������: SetSmartEnvId
//** ��������: ��ȡZIGBEE·�ɿ���
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: SetSmartEnvId
//** ��������: ���ô�����ID(�¶�)
//** �䡡��: id
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SetSmartEnvId(DWORD id)
{
    g_CS.lockon();
    g_pSmartDevSet->id = id;
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** ��������: GetSmartEnvId
//** ��������: ��ȡ������ID(�¶�)
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: GetLightStudy
//** ��������: ��ȡ�ƹ�ѧϰ
//** �䡡��: i
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: SetLightStudy
//** ��������: ���õƹ�ѧϰ
//** �䡡��: lightstudy i
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SetLightStudy(LightStudy lightstudy, int i)
{
    g_CS.lockon();
    memcpy(&g_pSmartDevSet->study[i], &lightstudy, sizeof(LightStudy));
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** ��������: SetSmartUi
//** ��������: ui���ñ���
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��12��27��
//======================================================
void SetSmartUi()
{
    g_CS.lockon();
    UpdateSmartDevSet();
    g_CS.lockoff();
}

//======================================================
//** ��������: SetStudyChan
//** ��������: ����ѧϰͨ·
//** �䡡��: i
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SetStudyChan(DWORD i)
{
    g_CS.lockon();
    g_dwStudyChan = i;
    g_CS.lockoff();
}

//======================================================
//** ��������: SetStudyNum
//** ��������: ����ѧϰ����
//** �䡡��: i
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SetStudyNum(DWORD i)
{
    g_CS.lockon();
    g_dwStudyNum = i;
    g_CS.lockoff();
}

//======================================================
//** ��������: GetSmartDev
//** ��������: ��ȡ�豸��ָ��
//** �䡡��: count
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: GetTimerNext
//** ��������: ��ȡ��ʱ��ͷ
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: GetTimerNext
//** ��������: ��ȡ��ʱ����һ��
//** �䡡��: pobject
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: GetTimerCount
//** ��������: ��ȡ��ʱ������
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
DWORD GetTimerCount()
{
    DWORD count;
    g_CS.lockon();
    count = m_timerlist.GetCount();
    g_CS.lockoff();
    return count;
}
