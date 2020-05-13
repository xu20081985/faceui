#include "CCtrlModules.h"
#include "SmartConfig.h"

class CPrjReportApp : public CAppBase
{
public:
	CPrjReportApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CPrjReportApp()
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:           								    //这里是不是总线上没有数据？
			if(m_dwTimeout > 0)     								   //当其减到零后，退出该界面。
			{
				m_dwTimeout--;
				if(m_dwTimeout == 0)
					DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}

			if(m_dwTick > 0)                                       //如果那个协议没有发送MSG_PRIVATE消息，那么就是上报失败。
			{
				if(DPGetTickCount() > m_dwTick)                   //貌似是多长时间没有反应。
				{
					m_pPng->SetSrc("tip_ok_big.png");
					m_pPng->Show(TRUE);

					m_pText->SetSrc(GetStringByID(14001));	   // 上报ID失败，请检查网络连接！
					m_pText->Show(TRUE);

					m_dwTick = 0;
					m_dwTimeout = TIP_TIMEOUT_FAIL;	
				}
			}
			break;
		case MSG_PRIVATE:
			if(wParam == m_IdBase)
			{
				m_pPng->SetSrc("tip_ok_big.png");
				m_pPng->Show(TRUE);

				m_pText->SetSrc(GetStringByID(14001));		// 上报ID成功！
				m_pText->Show(TRUE);

				m_dwTimeout = TIP_TIMEOUT_OK;
				m_dwTick = 0;
			}
			break;
		}
		return TRUE;	
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("prj_report.xml");
		m_pPng = (CDPStatic *)GetCtrlByName("png");
		m_pText = (CDPStatic *)GetCtrlByName("text");

		SmartReportID();                 //点一下上报ID功能界面后直接发送该协议。
		m_dwTick = DPGetTickCount() + TIP_TIMEOUT_ACK * 1000;  //那么问题来了，这里为什么都乘1000？
		return TRUE;
	}

private:
	CDPStatic* m_pPng;
	CDPStatic* m_pText;
	DWORD m_dwTick;
};

CAppBase* CreatePrjReportApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjReportApp* pApp = new CPrjReportApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}