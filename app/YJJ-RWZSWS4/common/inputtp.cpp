#include "roomlib.h"
#include "input.h"
#include <sys/time.h>

#define	MAX_MAP					10
const char CALIBRATE_FILE[] = "/UserDev/PenCalib";

int DIMMER_VALID_FLAG = 0;  // ���ڸ��ֽ������Ļ����жϱ�־λ

int point_flag = 0;

typedef struct
{
    EventMap *pevent;
    DWORD count;
    DWORD layer;
} EventList;

BOOL DoCtrlOp(EventMap *pEvent, DWORD x, DWORD y, DWORD statue);
static BOOL m_bTouchDirect = FALSE;				// �Ƿ�ֱ������Ŵ�������ֵ���ڽ�����ĻУ׼��ʱ����Ҫ����ΪTRUE
static int CalibrateParam[6];					// ������У׼����
static DWORD m_TotalEvent = 0;					// ��ǰע��Ĵ�������Ӧ�¼�����
static EventList m_lEventMapList[MAX_MAP];		// ��ǰע��Ĵ�������Ӧ�¼�����
static EventMap m_bLastEventPtr;		        // �ϴ���Ӧ���¼�,�ڴ��������������Ϊ0xffffffff
static DWORD m_dwLastXoff;						// ��ǰ�ؼ���Xƫ��
static DWORD m_dwLastYoff;						// ��ǰ�ؼ���Yƫ��
static BOOL m_bTouchPressed;					// ��ǰ�������Ƿ񱻰���(���)
static int g_dwLastXoff;						// ��һ�ε�X����
static int g_dwLastYoff;						// ��һ�ε�Y����
static BOOL g_bTouchPressed;					// ��ǰ�������Ƿ񱻰���(����)

static DWORD cacheval[16];
static DWORD cachetotal;
static DWORD cachewptr;
static DWORD cacherptr;
static DWORD dropbegin;					        // ������down��ʼ��4��ֵ

//======================================================
//** ��������: InitTouchCalibrate
//** ��������: ��ʼ������У׼ֵ
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
BOOL InitTouchCalibrate(void)
{
    if(!GetSwitch(SWITCH_CALIBRATE))
        return TRUE;

    FILE *fd;

    fd = fopen(CALIBRATE_FILE, "rb");
    if(fd == NULL)
        return FALSE;
    if(fread(CalibrateParam, 1, 24, fd) != 24)
    {
        fclose(fd);
        return FALSE;
    }
    fclose(fd);
    return TRUE;
}

//======================================================
//** ��������: SaveTouchCalibrate
//** ��������: ���津��У׼ֵ
//** �䡡��: val
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SaveTouchCalibrate(int *val)
{
    FILE *fd;

    fd = fopen(CALIBRATE_FILE, "wb");
    if(fd == NULL)
        return;
    memcpy(CalibrateParam, val, 24);
    fwrite(CalibrateParam, 1, 24, fd);
    fclose(fd);
}

//======================================================
//** ��������: SetTouchDirect
//** ��������: ���ô�������
//** �䡡��: val
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void SetTouchDirect(BOOL val)
{
    m_bTouchDirect = val;
}

//======================================================
//** ��������: RegEventRegion
//** ��������: ע��ָ���¼�ע��
//** �䡡��: eventcout pMap layer
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void RegEventRegion(DWORD eventcout, EventMap *pMap, DWORD layer)
{
    DWORD i;

    if (m_TotalEvent < MAX_MAP)
    {
        for (i = 0; i < m_TotalEvent; i++)
        {
            if ((m_lEventMapList[i].pevent == pMap)
                    && (m_lEventMapList[i].count == eventcout))
                return;
        }
        for (i = 0; i < m_TotalEvent; i++)
        {
            if (m_lEventMapList[i].layer < layer)
                break;
        }
        memmove(&m_lEventMapList[i + 1], &m_lEventMapList[i], (m_TotalEvent - i) * sizeof(EventList));
        m_lEventMapList[i].pevent = pMap;
        m_lEventMapList[i].count = eventcout;
        m_lEventMapList[i].layer = layer;
        m_TotalEvent++;
    }
}

//======================================================
//** ��������: UnRegEventRegion
//** ��������: ɾ��ָ���¼�ע��
//** �䡡��: eventcout pMap
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void UnRegEventRegion(DWORD eventcout, EventMap *pMap)
{
    DWORD i, j;

    DBGMSG(DISP_MOD, "UnRegEventRegion %d %p\r\n", eventcout, pMap);
    for (i = 0; i < m_TotalEvent; i++)
    {
        if ((m_lEventMapList[i].pevent == pMap)
                && (m_lEventMapList[i].count == eventcout))
        {
            for (j = 0; j < eventcout; j++)
            {
                if (pMap[j].pCtrl == m_bLastEventPtr.pCtrl)
                    m_bLastEventPtr.pCtrl = NULL;
            }
            memmove(&m_lEventMapList[i], &m_lEventMapList[i + 1], (m_TotalEvent - 1) * sizeof(EventList));
            m_TotalEvent -= 1;
            DBGMSG(DISP_MOD, "UnRegEventRegion %d\r\n", m_TotalEvent);
            return;
        }
    }
    DBGMSG(DISP_MOD, "UnRegEventRegion Dont find %d %p\r\n", eventcout, pMap);
}

//======================================================
//** ��������: ClrEventRegion
//** ��������: ����¼�ע��
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void ClrEventRegion(void)
{
    m_TotalEvent = 0;
    memset(m_lEventMapList, 0, sizeof(EventList) * MAX_MAP);
    DBGMSG(DISP_MOD, "ClrEventRegion\r\n");
}

//======================================================
//** ��������: TouchEventRemap
//** ��������: �����¼�����
//** �䡡��: pmsg
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
BOOL TouchEventRemap(SYS_MSG *pmsg)
{
    DWORD i, j;
    DBGMSG(DISP_MOD, "Get point %d %d %d %d\r\n", DPGetTickCount(), pmsg->wParam, pmsg->lParam, pmsg->zParam);

    if (m_bTouchDirect)
    {
        if (pmsg->zParam == TOUCH_UP)
            PlayWav(KEYPAD_INDEX, GetRingVol());

        DPPostMessage(TOUCH_RAW_MESSAGE, pmsg->wParam, pmsg->lParam, pmsg->zParam);
        return FALSE;
    }
    else
        pmsg->msg = TOUCH_MESSAGE;

    if (pmsg->zParam == TOUCH_DOWN)
    {
        m_bTouchPressed = TRUE;
        m_bLastEventPtr.pCtrl = NULL;

        g_bTouchPressed = TRUE;
        g_dwLastXoff = pmsg->wParam;
        g_dwLastYoff = pmsg->lParam;
    }
    else if(pmsg->zParam == TOUCH_UP)
    {
        if (g_bTouchPressed)
        {
            // ���ж��Ƿ�Ϊ������Ϣ
            int Xoff = pmsg->wParam - g_dwLastXoff;
            int Yoff = pmsg->lParam - g_dwLastYoff;

            const int OFFSET_LEFT	= -100;
            const int OFFSET_RIGHT	= 100;
            const int OFFSET_UPSIDE	= -100;
            const int OFFSET_DOWN	= 100;

            if ((abs(Xoff) >= abs(Yoff)) && (abs(Xoff) > OFFSET_RIGHT))
            {
                if (Xoff < OFFSET_LEFT)
                {
                    DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_SLIDE);
                    DPPostMessage(TOUCH_SLIDE, SLIDE_LEFT, 0, 0);
                }
                else
                {
                    DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_SLIDE);
                    DPPostMessage(TOUCH_SLIDE, SLIDE_RIGHT, 0, 0);
                }
            }
            else if ((abs(Yoff) > abs(Xoff)) && (abs(Yoff) > OFFSET_DOWN))
            {
                if (Yoff < OFFSET_UPSIDE)
                {
                    DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_SLIDE);
                    DPPostMessage(TOUCH_SLIDE, SLIDE_UPSIDE, 0, 0);
                }
                else
                {
                    DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_SLIDE);
                    DPPostMessage(TOUCH_SLIDE, SLIDE_DOWN, 0, 0);
                }
            }
            else
            {
                if ((m_bTouchPressed) && (m_bLastEventPtr.pCtrl != NULL))
                {
                    m_bTouchPressed = FALSE;
                    DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_UP);
                }
            }

            m_bLastEventPtr.pCtrl = NULL;
            g_bTouchPressed = FALSE;
            m_bTouchPressed = FALSE;
        }
    }

    if (m_bTouchPressed)
    {
        int calx = pmsg->wParam;
        int caly = pmsg->lParam;
        if (m_bLastEventPtr.pCtrl == NULL)
        {
            for (i = 0; i < m_TotalEvent; i++)
            {
                for (j = 0; j < m_lEventMapList[i].count; j++)
                {
                    if ((calx > m_lEventMapList[i].pevent[j].xStart)
                            && (calx < m_lEventMapList[i].pevent[j].xEnd)
                            && (caly > m_lEventMapList[i].pevent[j].yStart)
                            && (caly < m_lEventMapList[i].pevent[j].yEnd))
                    {
                        memcpy(&m_bLastEventPtr, &m_lEventMapList[i].pevent[j], sizeof(EventMap));
                        break;
                    }
                }
                if (m_bLastEventPtr.pCtrl != NULL)
                    break;
            }

            if (m_bLastEventPtr.pCtrl != NULL)
            {
                m_dwLastXoff = calx - m_bLastEventPtr.xStart;
                m_dwLastYoff = caly - m_bLastEventPtr.yStart;
                DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_DOWN);
            }
            else
            {
                m_bTouchPressed = FALSE;
            }
        }
        else
        {
            if ((calx >	m_bLastEventPtr.xStart)
                    && (calx < m_bLastEventPtr.xEnd)
                    && (caly >	m_bLastEventPtr.yStart)
                    && (caly <	m_bLastEventPtr.yEnd))
            {
                m_dwLastXoff = calx - m_bLastEventPtr.xStart;
                m_dwLastYoff = caly - m_bLastEventPtr.yStart;
                DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_VALID);
            }
            else
            {
                DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_MOVEOUT);
                m_bLastEventPtr.pCtrl = NULL;
                m_bTouchPressed = FALSE;
            }
        }
    }

    return FALSE;
}

BOOL TouchEventRemapEx(SYS_MSG *pmsg)
{
    int calx, caly;
    DWORD i, j;
    DBGMSG(DISP_MOD, "Get point %d %d %d %d\r\n", DPGetTickCount(), pmsg->wParam, pmsg->lParam, pmsg->zParam);

    if(m_bTouchDirect)
    {
        if(pmsg->zParam == TOUCH_UP)
            PlayWav(KEYPAD_INDEX, GetRingVol());

        DPPostMessage(TOUCH_RAW_MESSAGE, pmsg->wParam, pmsg->lParam, pmsg->zParam);
        return FALSE;
    }
    else
        pmsg->msg = TOUCH_MESSAGE;

    if(pmsg->zParam == TOUCH_DOWN)	// ��һ��ץ���ĵ㽫���ӵ������޸�״̬, ĳЩ���������ܸ��ţ�����յ�1�������Ϣ
    {
        cachetotal = 0;
        dropbegin = 0;
        cacherptr = 0;
        cachewptr = 0;
        m_bTouchPressed = TRUE;
        m_bLastEventPtr.pCtrl = NULL;
    }
    else if(pmsg->zParam == TOUCH_UP)
    {
        if((m_bTouchPressed)
                && (m_bLastEventPtr.pCtrl != NULL))
        {
            m_bTouchPressed = FALSE;
            DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_UP);
        }
    }
    else if(m_bTouchPressed)
    {
        dropbegin++;
        if(dropbegin < 4)
            return FALSE;

        calx = (CalibrateParam[1] * pmsg->lParam + CalibrateParam[2] * pmsg->wParam + CalibrateParam[0]) >> 16;
        caly = (CalibrateParam[4] * pmsg->lParam + CalibrateParam[5] * pmsg->wParam + CalibrateParam[3]) >> 16;
        cacheval[cachewptr++] = calx;
        cacheval[cachewptr++] = caly;
        cachewptr &= 0xf;
        cachetotal++;
        if(cachetotal < 4)
            return FALSE;
        calx = cacheval[cacherptr++];
        caly = cacheval[cacherptr++];
        cacherptr &= 0xf;
        if(m_bLastEventPtr.pCtrl == NULL)
        {
            for(i = 0; i < m_TotalEvent; i++)
            {
                for(j = 0; j < m_lEventMapList[i].count; j++)
                {
                    if((calx >  m_lEventMapList[i].pevent[j].xStart)
                            && (calx < m_lEventMapList[i].pevent[j].xEnd)
                            && (caly >  m_lEventMapList[i].pevent[j].yStart)
                            && (caly <  m_lEventMapList[i].pevent[j].yEnd))
                    {
                        memcpy(&m_bLastEventPtr, &m_lEventMapList[i].pevent[j], sizeof(EventMap));
                        break;
                    }
                }
                if(m_bLastEventPtr.pCtrl != NULL)
                    break;
            }
            if(m_bLastEventPtr.pCtrl != NULL)
            {
                m_dwLastXoff = calx - m_bLastEventPtr.xStart;
                m_dwLastYoff = caly - m_bLastEventPtr.yStart;
                if(DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_DOWN))
                {
                    //if(GetOnOff(SWITCH_KEYTONE))
                    PlayWav(KEYPAD_INDEX, GetRingVol());
                }
            }
            else
            {
                m_bTouchPressed = FALSE;
            }
        }
        else
        {
            if((calx >	m_bLastEventPtr.xStart)
                    && (calx < m_bLastEventPtr.xEnd)
                    && (caly >	m_bLastEventPtr.yStart)
                    && (caly <	m_bLastEventPtr.yEnd))
            {
                m_dwLastXoff = calx - m_bLastEventPtr.xStart;
                m_dwLastYoff = caly - m_bLastEventPtr.yStart;
                DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_VALID);
            }
            else
            {
                DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_MOVEOUT);
                m_bLastEventPtr.pCtrl = NULL;
                m_bTouchPressed = FALSE;
            }
        }
    }
    return FALSE;
}
