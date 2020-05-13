#include "CCtrlModules.h"

class CTimerWeekApp : public CAppBase
{
public:
    CTimerWeekApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CTimerWeekApp()
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
                    if (m_dwPage == 0)
                    {
                        hide();
                        OnCreate(1);
                    }
                    else
                    {
                        hide();
                        OnCreate(0);
                    }
                }
                else if (wParam == SLIDE_DOWN)
                {
                    if (m_dwPage == 0)
                    {
                        hide();
                        OnCreate(1);
                    }
                    else
                    {
                        hide();
                        OnCreate(0);
                    }
                }
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)
                {
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }

                if (m_dwPage == 0)
                {
                    if (wParam >= m_idButton[0] && wParam <= m_idButton[4])
                    {
                        int index = wParam - m_idButton[0];
                        show(index);
                    }
                }
                else if (m_dwPage == 1)
                {
                    if (wParam >= m_idButton[0] && wParam <= m_idButton[1])
                    {
                        int index = wParam - m_idButton[0];
                        show(index);
                    }
                }
                break;
            default:
                break;
        }
        return TRUE;
    }

    void show(DWORD i)
    {
        if (m_dwPage == 0)
        {
            DWORD status = m_pItem->week & (1 << i);
            if (status)
            {
                m_pItem->week &= ~(1 << i);
                m_pButton[i]->SetSrcpng(GetSmartPngSelect(0));
                m_pButton[i]->Show(STATUS_NORMAL);
            }
            else
            {
                m_pItem->week |= (1 << i);
                m_pButton[i]->SetSrcpng(GetSmartPngSelect(1));
                m_pButton[i]->Show(STATUS_NORMAL);
            }
        }
        else if(m_dwPage == 1)
        {
            DWORD status = m_pItem->week & (1 << (i + 5));
            if (status)
            {
                m_pItem->week &= ~(1 << (i + 5));
                m_pButton[i]->SetSrcpng(GetSmartPngSelect(0));
                m_pButton[i]->Show(STATUS_NORMAL);
            }
            else
            {
                m_pItem->week |= (1 << (i + 5));
                m_pButton[i]->SetSrcpng(GetSmartPngSelect(1));
                m_pButton[i]->Show(STATUS_NORMAL);
            }
        }

        if ((m_pItem->week & 0x7f) != 0)
            m_pItem->week &= 0x7f;
        else
            m_pItem->week = 0x80;
        GetTimerShow(m_pItem);
    }

    void hide()
    {
        if (m_dwPage == 0)
        {
            for (int i = 2; i < 5; i++)
            {
                m_pWeek[i]->Show(FALSE);
                m_pButton[i]->Hide();
                m_pLine[i]->Show(FALSE);
            }
        }
    }

    void OnCreate(DWORD page)
    {
        m_dwPage = page;
        if (m_dwPage == 0)
            m_size = 5;
        else
            m_size = 2;

        for (DWORD i = 0; i < m_size; i++)
        {
            sprintf(m_buf, "text%d", i + 1);
            m_pWeek[i] = (CDPStatic *)GetCtrlByName(m_buf);
            sprintf(m_buf, "select%d", i + 1);
            m_pButton[i] = (CDPButton *)GetCtrlByName(m_buf, &m_idButton[i]);
            sprintf(m_buf, "line%d", i + 1);
            m_pLine[i] = (CDPStatic *)GetCtrlByName(m_buf);
        }

        if (m_dwPage == 0)
        {
            for (int i = 0; i < 5; i++)
            {
                m_pWeek[i]->SetSrc(GetStringByID(13501 + i));
                m_pWeek[i]->Show(TRUE);
                if ((m_pItem->week & 0x80) == 0)
                {
                    m_pButton[i]->SetSrcpng(GetSmartPngSelect(m_pItem->week & (1 << i)));
                    m_pButton[i]->Show(STATUS_NORMAL);
                }
                else
                {
                    m_pButton[i]->SetSrcpng(GetSmartPngSelect(0));
                    m_pButton[i]->Show(STATUS_NORMAL);
                }
                m_pLine[i]->Show(TRUE);
            }
        }
        else
        {
            for (int i = 0; i < 2; i++)
            {
                m_pWeek[i]->SetSrc(GetStringByID(13506 + i));
                m_pWeek[i]->Show(TRUE);
                if ((m_pItem->week & 0x80) == 0)
                {
                    m_pButton[i]->SetSrcpng(GetSmartPngSelect(m_pItem->week & (1 << (i + 5))));
                    m_pButton[i]->Show(STATUS_NORMAL);
                }
                else
                {
                    m_pButton[i]->SetSrcpng(GetSmartPngSelect(0));
                    m_pButton[i]->Show(STATUS_NORMAL);
                }
                m_pLine[i]->Show(TRUE);
            }
        }

    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("timer_week.xml");

        GetCtrlByName("back", &m_idBack);

        m_pItem = (PSmartTimer)lParam;

        OnCreate(zParam);
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_dwPage;
    DWORD m_size;

    DWORD m_idButton[5];
    CDPButton *m_pButton[5];
    CDPStatic *m_pLine[5];
    CDPStatic *m_pWeek[5];
    PSmartTimer m_pItem;
    char m_buf[64];
};

CAppBase *CreateTimerWeekApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CTimerWeekApp *pApp = new CTimerWeekApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
