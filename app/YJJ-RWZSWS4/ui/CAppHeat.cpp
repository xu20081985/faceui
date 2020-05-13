#include "CCtrlModules.h"

class CHeatApp : public CAppBase
{
public:
    CHeatApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CHeatApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch (uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                break;
            case MSG_BROADCAST:
                if (wParam == SMART_STATUS_SYNC)
                {
                    if (zParam != 0)
                    {
                        m_pSmartDev->status = zParam;
                        ShowAircStatus();
                    }
                }
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
                else if (wParam == m_idSub)
                {
                    if (--m_pHeat->temp < 6)
                    {
                        m_pHeat->temp = 26;
                    }
                    sprintf(m_buf, "%d", m_pHeat->temp + 9);
                    m_pSetTemp->SetSrc(m_buf);
                    m_pSetTemp->Show(TRUE);
                    m_pHeat->onoff = AC_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    SendTempCmd(m_pHeat->temp);
                }
                else if(wParam == m_idAdd)
                {
                    if (++m_pHeat->temp > 26)
                    {
                        m_pHeat->temp = 6;
                    }
                    sprintf(m_buf, "%d", m_pHeat->temp + 9);
                    m_pSetTemp->SetSrc(m_buf);
                    m_pSetTemp->Show(TRUE);
                    m_pHeat->onoff = AC_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    SendTempCmd(m_pHeat->temp);
                }
                else if (wParam == m_idOnOff)
                {
                    if (m_pHeat->onoff == AC_STATUS_OFF)
                    {
                        m_pHeat->onoff = AC_STATUS_ON;
                        m_pSmartDev->cmd = SCMD_OPEN;
                    }
                    else
                    {
                        m_pHeat->onoff = AC_STATUS_OFF;
                        m_pSmartDev->cmd = SCMD_CLOSE;
                    }
                    SendTempCmd(m_pHeat->temp);
                }
                break;
        }

        ShowAircStatus();
        return TRUE;
    }

    void SendTempCmd(int temp)
    {
        WORD status = m_pSmartDev->status;
        AC_DATA *pHeat = (AC_DATA *)&status;
        pHeat->temp = temp;

        SendSmartCmd(&m_pSmartDev->device, SCMD_AC, status);
    }

    void ShowAircStatus()
    {
        char buf[128];

        // 设置温度 地暖最低温度为15℃
        if (m_pHeat->temp > 0)
        {
            sprintf(buf, "%02d", m_pHeat->temp + 9);
            m_pSetTemp->SetSrc(buf);
            m_pSetTemp->Show(TRUE);
        }

        // 室内温度
        sprintf(m_buf, "%s: %02d%s", GetStringByID(10003), GetEnvTemp(), GetStringByID(10006));
        m_pCurTemp->SetSrc(m_buf);
        m_pCurTemp->Show(TRUE);

        // 开关机
        if (m_pHeat->onoff > 0)
        {
            if (m_pHeat->onoff == AC_STATUS_ON || m_pSmartDev->cmd == SCMD_OPEN)
                m_pOnOff->SetSrcpng(GetSmartPngOnOff(1));
            else
                m_pOnOff->SetSrcpng(GetSmartPngOnOff(0));
            m_pOnOff->Show(STATUS_NORMAL);
        }

    }

    void show()
    {
        if (m_pHeat->onoff == 0)
        {
            if (m_pSmartDev->cmd == SCMD_OPEN)
                m_pHeat->onoff = 1;
            else
                m_pHeat->onoff = 2;
        }

        if (m_pHeat->temp == 0)
        {
            m_pHeat->temp = 17;
        }

        m_pHeat->mode = 1;

        ShowAircStatus();
    }
    void OnCreate(SmartDev *pSmartDev, DWORD status)
    {
        m_pTitle->SetSrc(pSmartDev->name);
        m_pTitle->Show(TRUE);

        m_pSmartDev = pSmartDev;
        m_pHeat = (AC_DATA *)&m_pSmartDev->status;

        SmartGetStatus(&pSmartDev->device);

        show();
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("heat.xml");
        GetCtrlByName("back", &m_idBack);
        GetCtrlByName("add", &m_idAdd);
        GetCtrlByName("sub", &m_idSub);

        m_pTitle = (CDPStatic *)GetCtrlByName("title");
        m_pSetTemp = (CDPStatic *)GetCtrlByName("set_temp");
        m_pCurTemp = (CDPStatic *)GetCtrlByName("cur_temp");
        m_pOnOff = (CDPButton *)GetCtrlByName("power", &m_idOnOff);
        OnCreate((SmartDev *)lParam, zParam);
        return TRUE;
    }

private:
    DWORD m_idBack;
    DWORD m_idAdd;
    DWORD m_idSub;
    DWORD m_idOnOff;

    CDPStatic *m_pTitle;
    CDPStatic *m_pSetTemp;
    CDPStatic *m_pCurTemp;
    CDPButton *m_pOnOff;

    SmartDev *m_pSmartDev;
    AC_DATA *m_pHeat;
    char m_buf[64];
};

CAppBase *CreateHeatApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CHeatApp *pApp = new CHeatApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}