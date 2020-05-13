#include "CCtrlModules.h"
#include "SmartConfig.h"

class CWindApp : public CAppBase
{
public:
    CWindApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CWindApp()
    {
    }

    BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
    {
        switch(uMsg)
        {
            case TIME_MESSAGE:
                if (m_dwTimeout++ == 30)
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                break;
            case MSG_BROADCAST:
                if (wParam == SMART_STATUS_SYNC)
                {
                    ShowWindStatus();
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
                else if (wParam == m_idHigh)
                {
                    m_pAirC->speed = 4;
                    m_pAirC->onoff = WIND_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    SendTempCmd(m_pAirC->temp);
                }
                else if (wParam == m_idMiddle)
                {
                    m_pAirC->speed = 3;
                    m_pAirC->onoff = WIND_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    SendTempCmd(m_pAirC->temp);
                }
                else if (wParam == m_idLow)
                {
                    m_pAirC->speed = 2;
                    m_pAirC->onoff = WIND_STATUS_ON;
                    m_pSmartDev->cmd = SCMD_OPEN;
                    SendTempCmd(m_pAirC->temp);
                }				
                else if (wParam == m_idOnOff)
                {
                    if (m_pAirC->onoff == WIND_STATUS_OFF)
                    {
                        m_pAirC->onoff = WIND_STATUS_ON;
                        m_pSmartDev->cmd = SCMD_OPEN;
                    }
                    else
                    {
                        m_pAirC->onoff = WIND_STATUS_OFF;
                        m_pSmartDev->cmd = SCMD_CLOSE;
                    }
                    SendTempCmd(m_pAirC->temp);
                }
				ShowWindStatus();
                break;
            default:
                break;
        }

        return TRUE;
    }

    void SendTempCmd(int temp)
    {
        WORD status = m_pSmartDev->status;
		m_pSmartDev->param1 = m_pSmartDev->status;
        AC_DATA *pAirC = (AC_DATA *)&status;
        pAirC->temp = temp;
        SendSmartCmd(&m_pSmartDev->device, SCMD_AC, status);
    }

    void ShowWindStatus()
    {		
		if (m_pSmartDev->status == 0)
		{
			m_pSmartDev->status = m_pSmartDev->param1;
			if (m_pSmartDev->cmd == SCMD_OPEN)
				m_pAirC->onoff = WIND_STATUS_ON;
			else
				m_pAirC->onoff = WIND_STATUS_OFF;
		}
		else
		{
			m_pSmartDev->param1 = m_pSmartDev->status;
		}

        if (m_pAirC->speed == 2)
        {
            m_pLow->SetSrcpng(STATUS_NORMAL, GetSmartPngWind(WIND_LOW, ICON_ON_NORMAL));
            m_pLow->SetSrcpng(STATUS_PRESSED, GetSmartPngWind(WIND_LOW, ICON_ON_LITTLE));
            m_pMiddle->SetSrcpng(STATUS_NORMAL, GetSmartPngWind(WIND_MIDDLE, ICON_OFF_NORMAL));
            m_pMiddle->SetSrcpng(STATUS_PRESSED, GetSmartPngWind(WIND_MIDDLE, ICON_OFF_LITTLE));
            m_pHigh->SetSrcpng(STATUS_NORMAL, GetSmartPngWind(WIND_HIGH, ICON_OFF_NORMAL));
            m_pHigh->SetSrcpng(STATUS_PRESSED, GetSmartPngWind(WIND_HIGH, ICON_OFF_LITTLE));
        }
        else if (m_pAirC->speed == 3)
        {
            m_pLow->SetSrcpng(STATUS_NORMAL, GetSmartPngWind(WIND_LOW, ICON_OFF_NORMAL));
            m_pLow->SetSrcpng(STATUS_PRESSED, GetSmartPngWind(WIND_LOW, ICON_OFF_LITTLE));
            m_pMiddle->SetSrcpng(STATUS_NORMAL, GetSmartPngWind(WIND_MIDDLE, ICON_ON_NORMAL));
            m_pMiddle->SetSrcpng(STATUS_PRESSED, GetSmartPngWind(WIND_MIDDLE, ICON_ON_LITTLE));
            m_pHigh->SetSrcpng(STATUS_NORMAL, GetSmartPngWind(WIND_HIGH, ICON_OFF_NORMAL));
            m_pHigh->SetSrcpng(STATUS_PRESSED, GetSmartPngWind(WIND_HIGH, ICON_OFF_LITTLE));
        }
        else if (m_pAirC->speed == 4)
        {
            m_pLow->SetSrcpng(STATUS_NORMAL, GetSmartPngWind(WIND_LOW, ICON_OFF_NORMAL));
            m_pLow->SetSrcpng(STATUS_PRESSED, GetSmartPngWind(WIND_LOW, ICON_OFF_LITTLE));
            m_pMiddle->SetSrcpng(STATUS_NORMAL, GetSmartPngWind(WIND_MIDDLE, ICON_OFF_NORMAL));
            m_pMiddle->SetSrcpng(STATUS_PRESSED, GetSmartPngWind(WIND_MIDDLE, ICON_OFF_LITTLE));
            m_pHigh->SetSrcpng(STATUS_NORMAL, GetSmartPngWind(WIND_HIGH, ICON_ON_NORMAL));
            m_pHigh->SetSrcpng(STATUS_PRESSED, GetSmartPngWind(WIND_HIGH, ICON_ON_LITTLE));
        }
		
        m_pLow->Show(STATUS_NORMAL);
        m_pMiddle->Show(STATUS_NORMAL);
        m_pHigh->Show(STATUS_NORMAL);

        // 开关机
        if (m_pAirC->onoff > 0)
        {
            if (m_pAirC->onoff == WIND_STATUS_ON || m_pSmartDev->cmd == SCMD_OPEN)
                m_pOnOff->SetSrcpng(GetSmartPngOnOff(1));
            else
                m_pOnOff->SetSrcpng(GetSmartPngOnOff(0));
            m_pOnOff->Show(STATUS_NORMAL);
        }
		
    }

    void OnCreate(SmartDev *pSmartDev)
    {
        m_pSmartDev = pSmartDev;
        // 显示名称
        m_pTitle->SetSrc(pSmartDev->name);
        m_pTitle->Show(TRUE);

        m_pAirC = (AC_DATA *)&m_pSmartDev->status;

        SmartGetStatus(&m_pSmartDev->device);		
        // 更新状态
        ShowWindStatus();
    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("wind.xml");
        GetCtrlByName("back", &m_idBack);
		
		m_pOnOff = (CDPButton *)GetCtrlByName("power", &m_idOnOff);
        m_pTitle = (CDPStatic *)GetCtrlByName("title");

        m_pHigh = (CDPButton *)GetCtrlByName("high", &m_idHigh);
        m_pMiddle = (CDPButton *)GetCtrlByName("middle", &m_idMiddle);
        m_pLow = (CDPButton *)GetCtrlByName("low", &m_idLow);

        OnCreate((SmartDev *)lParam);
        return TRUE;
    }

private:
    DWORD m_idBack;
	DWORD m_idOnOff;
	DWORD m_idHigh;
    DWORD m_idMiddle;
    DWORD m_idLow;
	
	CDPButton *m_pOnOff;
    CDPStatic *m_pTitle;
	AC_DATA *m_pAirC;

    CDPButton *m_pHigh;
    CDPButton *m_pMiddle;
    CDPButton *m_pLow;

    SmartDev *m_pSmartDev;
};

CAppBase *CreateWindApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CWindApp *pApp = new CWindApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}
