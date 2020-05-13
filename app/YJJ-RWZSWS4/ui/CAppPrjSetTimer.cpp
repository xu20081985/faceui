#include "CCtrlModules.h"
#include "list.h"

class CPrjSetTimerApp : public CAppBase
{
public:
    CPrjSetTimerApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CPrjSetTimerApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch (uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                break;
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                if (wParam == SLIDE_UPSIDE)
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PRJ_SET_TIMER_APPID, 0, 0);
                    if (m_dwPage < m_mPage - 1)
                        DPPostMessage(MSG_START_FROM_ROOT, PRJ_SET_TIMER_APPID, m_dwPage + 1, 0);
                    else
                        DPPostMessage(MSG_START_FROM_ROOT, PRJ_SET_TIMER_APPID, 0, 0);
                }
                else if (wParam == SLIDE_DOWN)
                {
                    if (m_dwPage > 0)
                        DPPostMessage(MSG_START_FROM_ROOT, PRJ_SET_TIMER_APPID, m_dwPage - 1, 0);
                    else
                        DPPostMessage(MSG_START_FROM_ROOT, PRJ_SET_TIMER_APPID, m_mPage - 1, 0);
                }
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)
                {
                    DPPostMessage(MSG_START_FROM_ROOT, PROJECT_APPID, 0, 0);
                }
                else if (wParam >= m_idEdit[0] && wParam <= m_idEdit[m_dwCount - 1])
                {
                    DWORD index = wParam - m_idEdit[0];
                    DPPostMessage(MSG_START_FROM_ROOT, TIMER_EDIT_APPID, (DWORD)m_dwpItem[index], m_dwPage);
                }
                else if (wParam >= m_idSwitch[0] && wParam <= m_idSwitch[m_dwCount - 1])
                {
                    DWORD index = wParam - m_idSwitch[0];
                    m_dwpItem[index]->onoff = (m_dwpItem[index]->onoff == TRUE) ? FALSE : TRUE;
                    m_pSwitch[index]->SetSrcpng(GetSmartAutoOnOff(m_dwpItem[index]->onoff));
                    m_pSwitch[index]->Show(STATUS_NORMAL);
                    UpdatSmartTimerSet();
                }
                break;
            default:
                break;
        }
        return TRUE;
    }

    void OnCreate()
    {
        DWORD dwCount = 0;
        DWORD index = 0;

        m_pItem = GetTimerHead();

        while (m_pItem)
        {
            if (dwCount >= (m_dwPage * m_size)
                    && dwCount < (m_dwPage * m_size + m_dwCount))
            {
                index = dwCount % m_size;

                m_pTime[index]->SetSrc(m_pItem->timeStr);
                m_pTime[index]->Show(TRUE);

                m_pEdit[index]->Show(STATUS_NORMAL);
                //m_pDelete[index]->Show(STATUS_NORMAL);
                m_pLine[index]->Show(TRUE);

                sprintf(m_buf, "%s%s,  %s", m_pItem->wayStr, m_pItem->devStr, m_pItem->weekStr);
                m_pAction[index]->SetSrc(m_buf);
                m_pAction[index]->Show(TRUE);

                m_pSwitch[index]->SetSrcpng(GetSmartAutoOnOff(m_pItem->onoff));
                m_pSwitch[index]->Show(STATUS_NORMAL);

                m_dwpItem[index] = m_pItem;
            }
            else if (dwCount >= (m_dwPage * m_size + m_dwCount))
            {
                break;
            }
            dwCount++;
            m_pItem = GetTimerNext(&(m_pItem->lpObj));
        }
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("timer.xml");

        GetCtrlByName("back", &m_idBack);

        //InitSmartTimer();						// 每次重新加载定时文件

        m_size = 5;								// 每页个数
        m_dwPage = lParam;						// 当前页数
        m_count = GetTimerCount();				// 定时个数

        if ((m_dwPage + 1) * m_size <= m_count)	// 当前页个数
            m_dwCount = m_size;
        else
            m_dwCount = m_count - m_dwPage * m_size;

        if (m_count == 0)						// 最大页数
            m_mPage = 1;
        else
            m_mPage = ((m_count - 1) / m_size) + 1;

        for (DWORD i = 0; i < m_dwCount; i++)
        {
            sprintf(m_buf, "edit%d", i + 1);
            m_pEdit[i] = (CDPButton *)GetCtrlByName(m_buf, &m_idEdit[i]);

            sprintf(m_buf, "delete%d", i + 1);
            m_pDelete[i] = (CDPButton *)GetCtrlByName(m_buf, &m_idDelete[i]);

            sprintf(m_buf, "time%d", i + 1);
            m_pTime[i] = (CDPStatic *)GetCtrlByName(m_buf);

            sprintf(m_buf, "text%d", i + 1);
            m_pAction[i] = (CDPStatic *)GetCtrlByName(m_buf);

            sprintf(m_buf, "switch%d", i + 1);
            m_pSwitch[i] = (CDPButton *)GetCtrlByName(m_buf, &m_idSwitch[i]);

            sprintf(m_buf, "line%d", i + 1);
            m_pLine[i] = (CDPStatic *)GetCtrlByName(m_buf);
        }

        OnCreate();
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_dwPage;
    DWORD m_dwCount;
    DWORD m_mPage;
    DWORD m_count;
    DWORD m_size;

    DWORD m_idEdit[5];
    DWORD m_idDelete[5];
    DWORD m_idSwitch[5];

    CDPStatic *m_pLine[5];
    CDPStatic *m_pTime[5];
    CDPStatic *m_pAction[5];

    CDPButton *m_pEdit[5];
    CDPButton *m_pDelete[5];

    CDPButton *m_pSwitch[5];
    PSmartTimer m_pItem;
    PSmartTimer m_dwpItem[5];

    char m_buf[64];
};

CAppBase *CreatePrjSetTimerApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CPrjSetTimerApp *pApp = new CPrjSetTimerApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
