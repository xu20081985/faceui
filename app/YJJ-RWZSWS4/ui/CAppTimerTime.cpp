#include "CCtrlModules.h"

class CTimerTimeApp : public CAppBase
{
public:
    CTimerTimeApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CTimerTimeApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch (uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                    DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
                m_pEditPwd->FlickSet();
                break;
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idBack)
                {
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                else if (wParam == m_idOK)
                {
                    memset(m_str, 0, sizeof(m_str));
                    m_pItem->hour = atoi(strncpy(m_str, m_time, 2));
                    memset(m_str, 0, sizeof(m_str));
                    m_pItem->minute = atoi(strncpy(m_str, m_time + 3, 2));
                    sprintf(m_pItem->timeStr, "%02d:%02d", m_pItem->hour, m_pItem->minute);
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                else if (wParam == m_idDelete)
                {
                    showDelete();
                    m_pEditPwd->ShowFlick(TRUE);
                }
                break;
            case KBD_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == KBD_CTRL)
                {
                    switch (m_curPos)
                    {
                        case 0:
                            if (lParam == '2')
                            {
                                if (m_time[m_curPos + 1] > '3')
                                    m_time[m_curPos + 1] = '3';
                            }

                            if (lParam >= '0' && lParam <= '2')
                            {
                                m_time[m_curPos] = lParam;
                                m_curPos += 1;
                                m_flickPos += 1;
                            }
                            break;
                        case 1:
                            if (m_time[m_curPos - 1] == '2')
                            {
                                if (lParam >= '0' && lParam <= '3')
                                {
                                    m_time[m_curPos] = lParam;
                                    m_curPos += 2;
                                    m_flickPos += 2;
                                }
                            }
                            else if (m_time[m_curPos - 1] == '0'
                                     || m_time[m_curPos - 1] == '1')
                            {
                                if (lParam >= '0' && lParam <= '9')
                                {
                                    m_time[m_curPos] = lParam;
                                    m_curPos += 2;
                                    m_flickPos += 2;
                                }
                            }
                            break;
                        case 2:
                            break;
                        case 3:
                            if (lParam >= '0' && lParam <= '5')
                            {
                                m_time[m_curPos] = lParam;
                                m_curPos += 1;
                                m_flickPos += 1;
                            }
                            break;
                        case 4:
                            if (lParam >= '0' && lParam <= '9')
                            {
                                m_time[m_curPos] = lParam;
                                m_curPos = 0;
                                m_flickPos = 0;
                            }
                            break;
                        default:
                            break;
                    }
                    m_pEditPwd->Input_show(m_time, m_flickPos);
                    m_pEditPwd->ShowFlick(TRUE);
                }
                break;
            default:
                break;
        }
        return TRUE;
    }

    void showDelete()
    {
        if (m_curPos == 1 || m_curPos == 4)
        {
            m_curPos -= 1;
            m_flickPos -= 1;
        }
        else if (m_curPos == 3)
        {
            m_curPos -= 2;
            m_flickPos -= 2;
        }
        else if (m_curPos == 0)
        {
            m_curPos = 4;
            m_flickPos = 4;
        }
        m_pEditPwd->set_curpos(m_flickPos);
    }

    void OnCreate()
    {
        strcpy(m_time, m_pItem->timeStr);
        m_pEditPwd->Input_show(m_pItem->timeStr, m_flickPos);
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("timer_time.xml");

        GetCtrlByName("back", &m_idBack);
        GetCtrlByName("delete", &m_idDelete);
        GetCtrlByName("ok", &m_idOK);

        m_pEditPwd = (CEditBox *)GetCtrlByName("pwd", &m_idEditBox);

        m_curPos = 0;
        m_flickPos = 0;
        m_pItem = (PSmartTimer)lParam;
        OnCreate();
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idDelete;
    DWORD m_idOK;

    DWORD m_idEditBox;
    CEditBox *m_pEditPwd;
    char m_time[64];
    char m_str[32];
    DWORD m_curPos;
    DWORD m_flickPos;
    PSmartTimer m_pItem;
};

CAppBase *CreateTimerTimeApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CTimerTimeApp *pApp = new CTimerTimeApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
