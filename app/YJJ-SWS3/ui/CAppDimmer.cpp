#include "CCtrlModules.h"
#include "SmartConfig.h"
DWORD temp = 0;
extern int DIMMER_VALID_FLAG;
int dimmer_time = 0;
WORD save_status = 100;
int clear_open_flag = 0;   // 2018.3.9添加用于解决0x01开到100%的问题

class CDimmerApp : public CAppBase
{
public:
	CDimmerApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CDimmerApp()
	{
	}
	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
	
		switch(uMsg)
		{
		
		case TIME_MESSAGE:

			if(dimmer_time < 30)
			{
				dimmer_time++;
				if(dimmer_time == 30)
				{
					DIMMER_VALID_FLAG = 0;
					DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
				}
			}
			break;
		case TOUCH_SLIDE:

	/*		 dimmer_time = 0; 
			 if(wParam == SLIDE_LEFT) {
	
				if(temp>0) {

					if(temp>3)
						temp-=3;

					else if(temp<=3)
						temp-=1;
				}
				
				m_pSmartDev->status = temp;	
				DPPostMessage(TOUCH_MESSAGE, m_idProgress, temp, 0);
			 }

			 else if(wParam == SLIDE_RIGHT) {

				if(temp<100) {
				
					if(temp<97)
						temp+=3;

					else if(temp >=97)
						temp+=1;
				}
				
				m_pSmartDev->status = temp;
				save_status = m_pSmartDev->status;
				DPPostMessage(TOUCH_MESSAGE, m_idProgress, temp, 0);
			 } 
			m_pSmartDev->status = lParam;
			DPPostMessage(TOUCH_MESSAGE, m_idProgress, lParam, 0);
			break;
	 */
		case MSG_BROADCAST:
			if(wParam == SMART_STATUS_ACK)
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
			dimmer_time = 0;
			if(wParam == m_idBack)                //返回上一菜单界面。
			{
				DIMMER_VALID_FLAG = 0;
				DPPostMessage(MSG_END_OVER_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idOpen)         //百分之百打开。   
			{
				char buf[32];

				sprintf(buf, "%d%%", save_status);
				m_pPercent->SetSrc(buf);
				m_pPercent->Show(TRUE);
			
				m_pProgress->SetProgressCur(save_status); 
				m_pProgress->Show();

				m_pSmartDev->status = save_status;
				SendSmartCmd(m_pSmartDev->addr, SCMD_OPEN, 0);
			//	SmartGetStatus(m_pSmartDev->addr);  // 查询状态
			}
			else if(wParam == m_idClose)            //百分之百关闭。
			{
				char buf[32];
				sprintf(buf, "%d%%",0);
				m_pPercent->SetSrc(buf);
				m_pPercent->Show(TRUE);

				if(m_pSmartDev->status != 0)
					save_status = m_pSmartDev->status;
				
				m_pSmartDev->status = 0;
				m_pProgress->SetProgressCur(0);         	
				m_pProgress->Show();
				SendSmartCmd(m_pSmartDev->addr, SCMD_CLOSE, 0);
			}
			
			else if(wParam == m_idProgress)
			{
				char buf[32];
				
				if (lParam > 95)
					lParam = 100;
				if (lParam >= 93 && lParam <= 95)
					lParam += rand() % 6; 
				
				sprintf(buf, "%d%%", lParam);
				m_pPercent->SetSrc(buf);
				m_pPercent->Show(TRUE);
				//temp = lParam;
				m_pSmartDev->status = lParam;   // 改变status状态

				//if(m_pSmartDev->status >= 95)
				//	m_pSmartDev->status = 100;
				
				save_status = m_pSmartDev->status;
				m_pProgress->SetProgressCur(m_pSmartDev->status);         	
				m_pProgress->Show();
				
				sprintf(buf, "%d%%", save_status);
				m_pPercent->SetSrc(buf);
				m_pPercent->Show(TRUE);
				
				m_pProgress->SetProgressCur(m_pSmartDev->status);
				m_pProgress->Show();
				if(lParam > 0)
					SendSmartCmd(m_pSmartDev->addr, SCMD_DIMMER_OPEN, lParam);
				else
					SendSmartCmd(m_pSmartDev->addr, SCMD_CLOSE, 0);  //如果拖拽进度条到零，则关闭。调光灯。
			}
			break;
		}
		return TRUE;	
	}

	void UpdateStatus()
	{
		char buf[32];
		char buf2[32];
		// 显示调光灯图标
		if(m_pSmartDev->status == 0)
		{
			strcpy(buf, "frame_dimmer_0.png");
			strcpy(buf2, "frame_dimmer_100.png");
		}
		else if(m_pSmartDev->status == 100)
		{
			strcpy(buf, "frame_dimmer_100.png");
			strcpy(buf2, "frame_dimmer_0.png");
		}
		else
		{
			strcpy(buf, "frame_dimmer_50.png");
			strcpy(buf2, "frame_dimmer_0.png");
		}
	/*	m_pDimmer->SetBkpng(buf);
		m_pDimmer->SetBkpng(STATUS_PRESSED, buf2);
		m_pDimmer->Show(STATUS_NORMAL);
    */
		// 显示百分比
		if(clear_open_flag == 0) {

			sprintf(buf, "%d%%", m_pSmartDev->status);
			m_pPercent->SetSrc(buf);
			m_pPercent->Show(TRUE);

		// 显示进度条
			m_pProgress->SetProgressCur(m_pSmartDev->status);
			m_pProgress->Show();
		}

		else if(clear_open_flag == 1) {

			clear_open_flag = 0;
			sprintf(buf, "%d%%", save_status);
			m_pPercent->SetSrc(buf);
			m_pPercent->Show(TRUE);

		// 显示进度条
			m_pProgress->SetProgressCur(save_status);
			m_pProgress->Show();		
		}
		
	}

	void OnCreate(SmartDev* pSmartDev)
	{
		m_pSmartDev = pSmartDev;
		// 显示名称
		m_pTitle->SetSrc(pSmartDev->name);
		m_pTitle->Show(TRUE);
		// 更新显示
		UpdateStatus();               // 在这里更新状态
		// 查询状态
		SmartGetStatus(pSmartDev->addr);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("dimmer.xml");
		dimmer_time = 0;
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("open", &m_idOpen);
		GetCtrlByName("close", &m_idClose);

		m_pTitle = (CDPStatic *)GetCtrlByName("title");
	//	m_pDimmer = (CDPButton *)GetCtrlByName("dimmer", &m_idDimmer);
		m_pPercent = (CDPStatic *)GetCtrlByName("percent");
		m_pProgress = (CDPProgress *)GetCtrlByName("progress", &m_idProgress);
		OnCreate((SmartDev *)lParam);
		return TRUE;
	}

private:
	DWORD m_idBack;
	DWORD m_idOpen;
	DWORD m_idClose;
	DWORD m_idDimmer;
	DWORD m_idProgress;

	CDPButton* m_pDimmer;
	CDPStatic* m_pTitle;
	CDPStatic* m_pPercent;
	CDPProgress* m_pProgress;

	SmartDev* m_pSmartDev;
};

CAppBase* CreateDimmerApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CDimmerApp* pApp = new CDimmerApp(wParam);
	clear_open_flag = 0;
	DIMMER_VALID_FLAG = 1;

	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}