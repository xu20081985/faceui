#include "CCtrlModules.h"
#include "SmartConfig.h"

extern int DIMMER_VALID_FLAG;
DWORD curtain_temp;

int CURTAIN_FLAG = 0;                 // 用于修改二级界面触摸逻辑        

class CCurtainApp : public CAppBase
{
public:
	CCurtainApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CCurtainApp()
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:

			if(m_dwTimeout < 30)
			{
				m_dwTimeout++;
				if(m_dwTimeout == 30)
				{
			
					CURTAIN_FLAG = 0;      
					DIMMER_VALID_FLAG = 0;
					m_dwTimeout = 0;
					DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
				}
			}
			break;
/*		case TOUCH_SLIDE:
			 m_dwTimeout = 0;
			 if(wParam == SLIDE_LEFT) {
	
				if(curtain_temp>0) {
					
					if(curtain_temp>3)
						curtain_temp-=3;

					else if(curtain_temp<=3)
						curtain_temp-=1;
				}
				m_pSmartDev->status = curtain_temp;		
				m_pProgress->SetProgressCur(curtain_temp);         	
				m_pProgress->Show();
				DPPostMessage(TOUCH_MESSAGE, m_idProgress, curtain_temp, 0);
			 }

			 else if(wParam == SLIDE_RIGHT) {

				if(curtain_temp<100) {

					if(curtain_temp<97)
						curtain_temp+=3;

					else if(curtain_temp>=97)
						curtain_temp+=1;
				}
				m_pSmartDev->status = curtain_temp;	
				m_pProgress->SetProgressCur(curtain_temp);         	
				m_pProgress->Show();
				DPPostMessage(TOUCH_MESSAGE, m_idProgress, curtain_temp, 0);
			 } 
			break; */
		case MSG_BROADCAST:
			if(wParam == SMART_STATUS_SYNC || wParam == SMART_STATUS_SCENE)
			{
				if(lParam == m_pSmartDev->addr)
				{
					UpdateStatus();
				}
			}
			else if(wParam == SMART_STATUS_GET)
			{
				if(lParam == m_pSmartDev->addr)
				{
					if(zParam != m_pSmartDev->status)
					{
						m_pSmartDev->status = zParam;
						UpdateStatus();
					}
				}
			}
			break;
		case TOUCH_MESSAGE:
			m_dwTimeout = 0;
			
			if(wParam == m_idBack)
			{
				CURTAIN_FLAG = 0;      
				DIMMER_VALID_FLAG = 0;
				DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idOpen)
			{
				// 打开
				char buf[32];
				sprintf(buf, "%d%%", 100);
				m_pPercent->SetSrc(buf);
				m_pPercent->Show(TRUE);

				m_pProgress->SetProgressCur(100);         	
				m_pProgress->Show();
				
			//	m_pOpen->SetSrc(GetStringByID(10208));
			//	m_pOpen->Show(TRUE);
				m_pSmartDev->status = 100;
				SendSmartCmd(m_pSmartDev->addr, SCMD_CURTAIN_OPEN, 100);
			}

			else if (wParam == m_idCurtains) {
				
				if(curtain_temp>0) {
					
					if(curtain_temp>3)
						curtain_temp-=3;

					else if(curtain_temp<=3)
						curtain_temp-=1;
				}
				m_pSmartDev->status = curtain_temp;
				m_pProgress->SetProgressCur(curtain_temp);         	
				m_pProgress->Show();

				DPPostMessage(TOUCH_MESSAGE, m_idProgress, curtain_temp, 0);
	
			}

			else if(wParam == m_idCurtaina) {

				if(curtain_temp<100) {

					if(curtain_temp<97)
						curtain_temp+=3;

					else if(curtain_temp>=97)
						curtain_temp+=1;
				} else {
					curtain_temp = 100;
				}
				m_pSmartDev->status = curtain_temp;
				m_pProgress->SetProgressCur(curtain_temp);         	
				m_pProgress->Show();

				DPPostMessage(TOUCH_MESSAGE, m_idProgress, curtain_temp, 0);

			}
			else if(wParam == m_idStop)
			{
				SendSmartCmd(m_pSmartDev->addr, SCMD_CURTAIN_STOP, 0);
			}
			else if(wParam == m_idClose)
			{
				// 关闭	
			    char buf[32];
				sprintf(buf, "%d%%", 0);
				m_pPercent->SetSrc(buf);
				m_pPercent->Show(TRUE);
				m_pSmartDev->status = 0;
				m_pProgress->SetProgressCur(0);         	
				m_pProgress->Show();
		
			//	m_pClose->SetSrc(GetStringByID(10206));
			//	m_pClose->Show(TRUE);
				
				SendSmartCmd(m_pSmartDev->addr, SCMD_CURTAIN_CLOSE, 0);
			}
			else if(wParam == m_idCurtain)
			{
				if(m_pSmartDev->status == 0)
					SendSmartCmd(m_pSmartDev->addr, SCMD_CURTAIN_OPEN, 100);
				else
					SendSmartCmd(m_pSmartDev->addr, SCMD_CURTAIN_CLOSE, 0);
			}
			else if(wParam == m_idProgress)
			{
				char buf[32];
				
				if (lParam > 95)
					lParam = 100;
				if (lParam >= 93 && lParam <= 95)
					lParam += rand() % 6; 
			
				sprintf(buf, "%d%%", lParam);
				curtain_temp = lParam;
				m_pSmartDev->status = lParam;
				m_pPercent->SetSrc(buf);
				m_pPercent->Show(TRUE);
				
            	// 显示进度条
				m_pProgress->SetProgressCur(m_pSmartDev->status);
				m_pProgress->Show();
				
				if(lParam)
					SendSmartCmd(m_pSmartDev->addr, SCMD_CURTAIN_OPEN, lParam);
				else
					SendSmartCmd(m_pSmartDev->addr, SCMD_CURTAIN_CLOSE, 0);
			}
			break;
		}
		return TRUE;	
	}

	void UpdateStatus()
	{
		char buf[32];
		char buf2[32];
		// 显示窗帘图标
		if(m_pSmartDev->status == 0)
		{
			strcpy(buf, "curtannew.png");
			strcpy(buf2, "curtannew.png");
		}
		else if(m_pSmartDev->status == 100)
		{
			strcpy(buf, "curtannew.png");
			strcpy(buf2, "curtannew.png");
		}
		else
		{
			strcpy(buf, "curtannew.png");
			strcpy(buf2, "curtannew.png");
		}
		m_pCurtain->SetBkpng(buf);
		m_pCurtain->SetBkpng(STATUS_PRESSED, buf2);
		m_pCurtain->Show(STATUS_NORMAL);

		// 显示百分比
		sprintf(buf, "%d%%", m_pSmartDev->status);
		m_pPercent->SetSrc(buf);
		m_pPercent->Show(TRUE);
    
		// 显示进度条
		m_pProgress->SetProgressCur(m_pSmartDev->status);
		m_pProgress->Show();
	}

	void OnCreate(SmartDev* pSmartDev)
	{
		m_pSmartDev = pSmartDev;
		// 显示名称
		m_pTitle->SetSrc(pSmartDev->name);
		m_pTitle->Show(TRUE);
        // 打开
	//	m_pOpen->SetSrc(GetStringByID(10208));
	//	m_pOpen->Show(TRUE);
		// 关闭		
	//	m_pClose->SetSrc(GetStringByID(10206));
	//	m_pClose->Show(TRUE);
		// 暂停
	//	m_pStop->SetSrc(GetStringByID(10207));
	//	m_pStop->Show(TRUE);
		
		// 更新显示
		UpdateStatus();
		// 查询状态
		SmartGetStatus(pSmartDev->addr);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("curtain.xml");
		m_dwTimeout = 0;
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("stop", &m_idStop);
		GetCtrlByName("open", &m_idOpen);
		GetCtrlByName("close", &m_idClose);
		m_pTitle = (CDPStatic *)GetCtrlByName("title");

		m_pOpen  = (CDPStatic *)GetCtrlByName("dakai");
		m_pClose = (CDPStatic *)GetCtrlByName("guanbi");
		m_pStop  = (CDPStatic *)GetCtrlByName("zanting");

		m_pPercent = (CDPStatic *)GetCtrlByName("percent");
		m_pCurtain = (CDPButton *)GetCtrlByName("curtannew", &m_idCurtain);
		m_pProgress = (CDPProgress *)GetCtrlByName("progress", &m_idProgress);

		m_pCurtaina = (CDPButton *)GetCtrlByName("curtain_up", &m_idCurtaina);
		m_pCurtains = (CDPButton *)GetCtrlByName("curtain_down", &m_idCurtains);

		OnCreate((SmartDev *)lParam);
		return TRUE;
	}

private:
	DWORD m_idBack;
	DWORD m_idStop;
	DWORD m_idOpen;
	DWORD m_idClose;
	DWORD m_idCurtain;
	DWORD m_idProgress;

	DWORD m_idCurtaina;
	DWORD m_idCurtains;
	
	CDPStatic* m_pTitle;
	CDPStatic* m_pPercent;
	

	CDPStatic* m_pOpen;
	CDPStatic* m_pClose;
	CDPStatic* m_pStop;
		
	CDPButton* m_pCurtain;
	CDPProgress* m_pProgress;

	SmartDev* m_pSmartDev;

	CDPButton* m_pCurtaina;
	CDPButton* m_pCurtains;
};

CAppBase* CreateCurtainApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CCurtainApp* pApp = new CCurtainApp(wParam);
	CURTAIN_FLAG = 1;
	DIMMER_VALID_FLAG = 1;
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}
