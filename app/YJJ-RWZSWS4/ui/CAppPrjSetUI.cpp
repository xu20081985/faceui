#include "CCtrlModules.h"

class CPrjSetUIApp : public CAppBase
{
public:
    CPrjSetUIApp(DWORD hWnd) : CAppBase(hWnd)
    {
    }

    ~CPrjSetUIApp()
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
            case TOUCH_SLIDE:
                m_dwTimeout = 0;
                break;
            case TOUCH_MESSAGE:
                m_dwTimeout = 0;
                if (wParam == m_idButton[0])
                {
                    m_pSmartDev->type = m_type;
					SetSmartUi();
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                else if (wParam == m_idButton[1])
                {
                    m_pSmartDev->type = m_type + 1;
					SetSmartUi();
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                else if (wParam == m_idButton[2])
                {
                    m_pSmartDev->type = m_type + 2;
					SetSmartUi();
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
                else if (wParam == m_idButton[3])
                {
                    m_pSmartDev->type = m_type + 3;
					SetSmartUi();
                    DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
                }
            default:
                break;
        }
        return TRUE;
    }

    void show(DWORD index)
    {
        m_type = index;
        for (DWORD i = 0; i < 4; i++)
        {
            m_pButton[i]->SetSrcpng(GetSmartPng(i + index, ICON_ON_NORMAL));
            m_pButton[i]->Show(STATUS_NORMAL);
        }
    }

    void OnCreate()
    {
        switch (m_pSmartDev->type)
        {
            case ST_LIGHT_A ... ST_LIGHT_D:						   // 灯光
                show(ST_LIGHT_A);
                break;
            case ST_DIMMER_A ... ST_DIMMER_D:                        //调光
                show(ST_DIMMER_A);
                break;
            case ST_CURTAIN_A ... ST_CURTAIN_D:                      //窗帘
                show(ST_CURTAIN_A);
                break;
            case ST_AC_A ... ST_AC_D:                           	   //空调
                show(ST_AC_A);
                break;
            case ST_IRAIR_A ... ST_IRAIR_D:						   //红外空调
                show(ST_IRAIR_A);
                break;
            case ST_HEAT_A ... ST_HEAT_D:                            //地暖
                show(ST_HEAT_A);
                break;
            case ST_WINDOW_A ... ST_WINDOW_D:                         //窗户
                show(ST_WINDOW_A);
                break;
            case ST_OUTLET_A ... ST_OUTLET_D:                         //插座
                show(ST_OUTLET_A);
                break;
			case ST_WIND_A ... ST_WIND_D:    							// 新风
                show(ST_WIND_A);
                break;		
            case ST_TV_A ... ST_TV_D:    								// 红外电视
                show(ST_TV_A);
                break;			
            case ST_MUSIC_A ... ST_MUSIC_D: 							// 背景音乐
                show(ST_MUSIC_A);
                break;
            case ST_LOCK_A ... ST_LOCK_D:
                show(ST_LOCK_A);
                break;
            default:
                break;
        }


    }

    BOOL Create(DWORD lParam, DWORD zParam)
    {
        InitFrame("setui.xml");
        m_pButton[0] = (CDPButton *)GetCtrlByName("icon1", &m_idButton[0]);
        m_pButton[1] = (CDPButton *)GetCtrlByName("icon2", &m_idButton[1]);
        m_pButton[2] = (CDPButton *)GetCtrlByName("icon3", &m_idButton[2]);
        m_pButton[3] = (CDPButton *)GetCtrlByName("icon4", &m_idButton[3]);

        m_pSmartDev = (SmartDev *)lParam;
        OnCreate();

        return TRUE;
    }

private:
    DWORD m_idButton[4];
    CDPButton *m_pButton[4];
    SmartDev *m_pSmartDev;
    DWORD m_type;
};

CAppBase *CreatePrjSetUIApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CPrjSetUIApp *pApp = new CPrjSetUIApp(wParam);
    if(!pApp->Create(lParam, zParam))
    {
        delete pApp;
        pApp = NULL;
    }
    return pApp;
}