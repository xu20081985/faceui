#include "CCtrlModules.h"
#include "SmartConfig.h"

#include <roomlib.h>

int  t_Year_Main;
int  t_Month_Main;
int  t_Day_Main;
int  t_Hour_Main;
int  t_Minutes_Main;
int  t_Second_Main;
char time_set_main[]="2017-02-01-00:00:00";
int g_First;

int scene_flag = 0;

int frash_flag = 0;       // 2018.1.8添加，用于修复相同图标状态不同步问题
extern BOOL Flag_Show[4];
extern BOOL TimerTouch;
extern int TIME_FLAG;     // 2018.1.26添加，用于增加校准时间提醒
extern int Ctl_Flag;      // 2018.2.24添加，用于修改APP控制问题   
int main_cpp = 1;
//extern SmartDevSet *		g_pSmartDevSet;
class CMainApp : public CAppBase
{
public:
	CMainApp(DWORD hWnd) : CAppBase(hWnd)
	{

	}

	~CMainApp()
	{

	}

	void ResumeAck(void)
	{
		m_dwTimeout = 0;
		OnPage(m_mapPage[m_nPage]);
		return CAppBase::ResumeAck();
	}

	void ScreenoffShow()
	{
		// 刷新屏保时间
		if (m_Show[0] == TRUE)
			m_screenoff = 1 * 60;
		else if (m_Show[1] == TRUE)
			m_screenoff = 5 * 60;
		else if (m_Show[2] == TRUE)
			m_screenoff = 10 * 60;
		else if (m_Show[3] == TRUE)
			return;
		else 
			m_screenoff = 60;
		
		if (m_dwTimeout < m_screenoff) {  
			m_dwTimeout++;								  // 一秒钟加一次（注：可以加一个屏幕变暗的函数)。
			if (m_dwTimeout == m_screenoff) {
				DPPostMessage(MSG_START_APP, CLOCK_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
		}
	}
	
	void TimeShow()
	{
		DPGetLocalTime(&m_tm);
		sprintf(m_buf,"%04d-%02d-%02d  %02d:%02d", 
				m_tm.wYear, m_tm.wMonth, m_tm.wDay, m_tm.wHour, m_tm.wMinute);
		m_pTitle[0]->SetSrc(m_buf); 
		m_pTitle[0]->Show(TRUE);
				
		m_pTitle[1]->SetSrc(GetStringByID(1000 + m_tm.wDayOfWeek)); 
		m_pTitle[1]->Show(TRUE);
		
		if (TIME_FLAG == 1) {
			m_pTitle[2]->SetSrc(GetStringByID(107)); 
			m_pTitle[2]->Show(FALSE);
		} else {
			m_pTitle[2]->SetSrc(GetStringByID(107)); 
			m_pTitle[2]->Show(TRUE);
		}
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:                          //在主界面经过多长时间后，回到那个时钟信息页（设置的时间应该是60秒钟）。
			TimeShow(); 										// 刷新时间
			ScreenoffShow();									// 屏保显示
			break;
		case MSG_BROADCAST: 										
#if 1
		//	if()

			if(Ctl_Flag == AIR_TYPE) {

				SmartDev* pSmartDev = m_pSmartDev;    	  // 空调类型的指针变量

				//这里就不处理了
				if(zParam >> 8 == 0)
					break; 
				
				for(int i = 0; i < MAX_PAGE_NUM * MAX_ICON_NUM; i++) {      // 遍历设备列表
					
					if(lParam == pSmartDev[i].addr && lParam != 0xFFFF) {     // 这里要将状态保存一下

						pSmartDev[i].status = zParam;

						pSmartDev[i].param1 = (pSmartDev[i].status & 0x001F); // 温度参数
						pSmartDev[i].param1 -= 6;

					}
				} 
			}
				
			else if(Ctl_Flag == MUSIC_TYPE) {              // 这里要将状态保存一下

				SmartDev* pSmartDevm = m_pSmartDev;   	   // 音乐类型的指针变量

				for(int i = 0; i < MAX_PAGE_NUM * MAX_ICON_NUM; i++) {       // 遍历设备列表
				
					if(lParam == pSmartDevm[i].addr && lParam != 0xFFFF) {
								
						pSmartDevm[i].status = zParam;
					}				
				} 
			}
			
			else if(Ctl_Flag == ALL_TYPE) {

				SmartDev* pSmartDevm = m_pSmartDev;   	   

				for(int i = 0; i < MAX_PAGE_NUM * MAX_ICON_NUM; i++) {       // 遍历设备列表
				
					if(lParam == pSmartDevm[i].addr && lParam != 0xFFFF) {
								
						if(wParam == SMART_STATUS_S) {
							if(pSmartDevm[i].type >= ST_AC_A && 
							   		pSmartDevm[i].type <= ST_MUSIC_D)
							{

								if (zParam == 0x01) {							
									pSmartDevm[i].status &= 0x7fff;
									pSmartDevm[i].status |= 0x4000;
								} else if(zParam == 0x03) {
									pSmartDevm[i].status &= 0xbfff;
									pSmartDevm[i].status |= 0x8000;
								}
							}
						}
					}				
				} 
			}		
#endif					
			frash_flag = 0;

	//		if(Ctl_Flag == 0 && (wParam != SMART_STATUS_ACK))
	//			UpdateStatus(lParam, zParam);                 //更新设备的列表。

			Ctl_Flag = 0;
			break;
			
		case TOUCH_SLIDE:                                     //执行的滑动动作的内容。
			m_dwTimeout = 0; 
			printf("sssss\n");
			switch (wParam) {                                         
				case SLIDE_LEFT:
					if (m_nPage < m_lPage - 1) {
						DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, m_nPage + 1, 0);
					} else {
						DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
					}
					break;
				case SLIDE_RIGHT:
					if (m_nPage > 0) {
						DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, m_nPage - 1, 0);
					} else {
						DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, m_lPage - 1, 0);
					}
					break;	
				case SLIDE_UPSIDE:
					DPPostMessage(MSG_START_FROM_ROOT, PWD_INPUT_APPID, 0, 0);
					//DPPostMessage(MSG_START_APP, PWD_INPUT_APPID, 0, 0);
					//DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
					break;
				case SLIDE_DOWN:
					//DPPostMessage(MSG_START_APP, CLOCK_APPID, 0, 0);
					//DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
					break;
			}
			break;
		case MSG_PRIVATE:                                  //这个是什么消息呢？
			if(wParam == m_IdBase)
			{
				if(lParam == MSG_SMART_UPDATE)
				{
					//m_pSmartDev = GetSmartDev(&m_dwCount);
					//OnPage(m_mapPage[m_nPage]);
					DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);	
				}
			}
			break;

		case TOUCH_MESSAGE:                                //这里应该是一个点触摸执行。

			frash_flag = 0;
			m_dwTimeout = 0;
			if ((wParam >= m_idButton[0]) && (wParam <= m_idButton[m_nCount - 1])) {
				int index = wParam - m_idButton[0];			
				SmartDev* pSmartDev = &m_pSmartDev[m_mapPage[m_nPage] * MAX_ICON_NUM + m_mapButton[index]]; 
				//printf("status = %d" , pSmartDev->status);	
                // 注:pSmartDev为当前点击后设备的一个参数结构                   
				switch(pSmartDev->type)
				{
				case ST_LIGHT_A:
				case ST_LIGHT_B:
				case ST_LIGHT_C:
				case ST_LIGHT_D:
					DPPostMessage(MSG_START_FROM_OVER, LIGHT_APPID, (DWORD)pSmartDev, 0);     //这就是一个执行命令的消息了。
					break;
				case ST_DIMMER_A:                       //调光
				case ST_DIMMER_B:
				case ST_DIMMER_C:
				case ST_DIMMER_D:
					DPPostMessage(MSG_START_FROM_OVER, DIMMER_APPID, (DWORD)pSmartDev, 0);
					break;
				case ST_CURTAIN_A:                      //窗帘
				case ST_CURTAIN_B:
				case ST_CURTAIN_C:
				case ST_CURTAIN_D:
					DPPostMessage(MSG_START_FROM_OVER, CURTAIN_APPID, (DWORD)pSmartDev, 0);
					break;
				case ST_AC_A:                           //空调
				case ST_AC_C:
				case ST_AC_D:
					DPPostMessage(MSG_START_FROM_OVER, AIRC_APPID, (DWORD)pSmartDev, 0);
					break;
				case ST_AC_B:
					DPPostMessage(MSG_START_FROM_OVER, Ir_Air_APPID, (DWORD)pSmartDev, 0);
					break;
				case ST_HEAT_A:                         //地暖
				case ST_HEAT_B:
				case ST_HEAT_C:
				case ST_HEAT_D:
					DPPostMessage(MSG_START_FROM_OVER, HEAT_APPID, (DWORD)pSmartDev, 0);
					break;
					
				case ST_WIND_A:
				case ST_WIND_B:
				case ST_WIND_C:
				case ST_WIND_D:
					DPPostMessage(MSG_START_FROM_OVER, NewWind_APPID, (DWORD)pSmartDev, 0);
					break;
				  
				case ST_WINDOW_A:                        //窗户
				case ST_WINDOW_B:
				case ST_WINDOW_C:
				case ST_WINDOW_D:
					DPPostMessage(MSG_START_FROM_OVER, WINDOW_APPID, (DWORD)pSmartDev, 0);
					break;
				case ST_OUTLET_A:                        //插座
				case ST_OUTLET_B:
				case ST_OUTLET_C:
				case ST_OUTLET_D:
					DPPostMessage(MSG_START_FROM_OVER, OUTLET_APPID, (DWORD)pSmartDev, 0);
					break;
				case ST_SCENE_A:                         //情景
				case ST_SCENE_B:
				case ST_SCENE_C:
				case ST_SCENE_D:
				case ST_SCENE_E:
				case ST_SCENE_F:
				case ST_SCENE_G:
				case ST_SCENE_H:
				case ST_SCENE_I:
				case ST_SCENE_J:
				case ST_SCENE_K:
				case ST_SCENE_L:
				case ST_SCENE_M:
				case ST_SCENE_N:
				case ST_SCENE_O:
				case ST_SCENE_P:
				case ST_SCENE_Q:
				case ST_SCENE_R:
				case ST_SCENE_S:
				case ST_SCENE_T:
				case ST_SCENE_U:
				case ST_SCENE_V:
				case ST_SCENE_W:
				case ST_SCENE_X:
				case ST_SCENE_Y:
				case ST_SCENE_Z:					
					// 初始化状态
					if (pSmartDev->addr == 0xffff) {
						if (pSmartDev->type == ST_SCENE_F) {
							pSmartDev->scene_status = 1;
							(pSmartDev + 1)->scene_status = 0;
						} else {
							pSmartDev->scene_status = 1;
							(pSmartDev - 1)->scene_status = 0;
						}
					}
					DPPostMessage(MSG_START_FROM_OVER, SCENE_APPID, (DWORD)pSmartDev, 0);
					break;
				case ST_TV_A:    // 电视
				case ST_TV_B:	
				case ST_TV_C:	 
				case ST_TV_D:
					DPPostMessage(MSG_START_FROM_OVER, TV_APPID, (DWORD)pSmartDev, 0);
					break;			
				case ST_MUSIC_A:	//背景音乐	
				case ST_MUSIC_B:
	            case ST_MUSIC_C:
	            case ST_MUSIC_D:
					DPPostMessage(MSG_START_FROM_OVER, MUSIC_APPID, (DWORD)pSmartDev, 0);
				   break;
				}
			}
		
			OnPage(m_mapPage[m_nPage]);
			break;
		}
		
		if(TimerTouch == 1) {    // 定时 

			TimerTouch = 0;
			frash_flag = 0;
			OnPage(m_mapPage[m_nPage]);
		}
  
		if(frash_flag <= 3)      // 主界面刷新
			frash_flag++;

		if(frash_flag <= 2) {    // 主界面刷新
			OnPage(m_mapPage[m_nPage]);
		}
				
		return TRUE;	
	}

	void UpdateStatus(DWORD addr, DWORD status)
	{
		SmartDev* pSmartDev = &m_pSmartDev[m_mapPage[m_nPage] * MAX_ICON_NUM];
		for (int k = 0; k < m_nCount; k++) {
			int i = m_mapButton[k];
			if(pSmartDev[i].addr == addr) {
				if (pSmartDev[i].status)
				{
					m_pButton[k]->SetSrcpng(GetSmartPng(pSmartDev[i].type, 0));
					m_pButton[k]->SetTextColor(STATUS_NORMAL, GetUIConfig(COLOR_TEXT_NORMAL));
					m_pButton[k]->SetTextColor(STATUS_PRESSED, GetUIConfig(COLOR_TEXT_NORMAL));
				} else {
					m_pButton[k]->SetSrcpng(GetSmartPng(pSmartDev[i].type, 1));
					m_pButton[k]->SetTextColor(STATUS_NORMAL, GetUIConfig(COLOR_TEXT_NORMAL));
					m_pButton[k]->SetTextColor(STATUS_PRESSED, GetUIConfig(COLOR_TEXT_NORMAL));
				}	
				m_pButton[k]->SetSrcpng(STATUS_NORMAL, GetSmartPng(pSmartDev[i].type, pSmartDev[i].status));
				m_pButton[k]->Show(STATUS_NORMAL); 
			}
		}
	}

	void OnPage(DWORD nPage)
	{
		// 显示页数点状态
		for (int i = 0; i < m_lPage; i++) {	
			if (m_mapPage[i] == nPage) {
				m_ppoint[m_nPoint + i]->SetSrcpng(GetSmartPngPoint(1));
				m_ppoint[m_nPoint + i]->Show(STATUS_NORMAL);
			} else {
				m_ppoint[m_nPoint + i]->SetSrcpng(GetSmartPngPoint(0));
				m_ppoint[m_nPoint + i]->Show(STATUS_NORMAL);
			}
		}

		SmartDev* pSmartDev = &m_pSmartDev[nPage * MAX_ICON_NUM];
		for (int k = 0; k < m_nCount; k++) {
			int i = m_mapButton[k];
			if (pSmartDev[i].type >= ST_AC_A  
					&& pSmartDev[i].type <= ST_MUSIC_D 
					 && pSmartDev[i].type != ST_AC_B
					 && pSmartDev[i].type != ST_TV_A
					 && pSmartDev[i].type != ST_TV_B
					 && pSmartDev[i].type != ST_TV_C
					 && pSmartDev[i].type != ST_TV_D) {						
				if ((pSmartDev[i].status & 0x8000) > 0) { // 关机
					m_pButton[k]->SetSrcpng(STATUS_NORMAL,GetSmartPng(pSmartDev[i].type, 1));
					m_pButton[k]->SetSrcpng(STATUS_PRESSED,GetSmartPng(pSmartDev[i].type, 2));
				} else {
					m_pButton[k]->SetSrcpng(STATUS_NORMAL,GetSmartPng(pSmartDev[i].type, 0));
					m_pButton[k]->SetSrcpng(STATUS_PRESSED,GetSmartPng(pSmartDev[i].type, 3));	
				} 
			} else if (pSmartDev[i].type >= ST_SCENE_A   
					&& pSmartDev[i].type <= ST_SCENE_Z) {
				if(pSmartDev[i].scene_status == 0) {
					m_pButton[k]->SetSrcpng(STATUS_NORMAL,GetSmartPng(pSmartDev[i].type, 1));
					m_pButton[k]->SetSrcpng(STATUS_PRESSED,GetSmartPng(pSmartDev[i].type, 2));
				} else if (pSmartDev[i].scene_status == 1) {
					m_pButton[k]->SetSrcpng(STATUS_NORMAL,GetSmartPng(pSmartDev[i].type, 0));
					m_pButton[k]->SetSrcpng(STATUS_PRESSED,GetSmartPng(pSmartDev[i].type, 3));
				}
			} else if (pSmartDev[i].type == ST_AC_B 
					|| pSmartDev[i].type == ST_TV_A
					|| pSmartDev[i].type == ST_TV_B
					|| pSmartDev[i].type == ST_TV_C
					|| pSmartDev[i].type == ST_TV_D) {
				m_pButton[k]->SetSrcpng(STATUS_NORMAL,GetSmartPng(pSmartDev[i].type, 0));
				m_pButton[k]->SetSrcpng(STATUS_PRESSED,GetSmartPng(pSmartDev[i].type, 3));		
			} else {							
				// 开光灯、调光灯、窗帘根据状态显示图标
				if (pSmartDev[i].status) {		
					m_pButton[k]->SetSrcpng(STATUS_NORMAL,GetSmartPng(pSmartDev[i].type, 0));
					m_pButton[k]->SetSrcpng(STATUS_PRESSED,GetSmartPng(pSmartDev[i].type, 2));
				} else {		// 开
					m_pButton[k]->SetSrcpng(STATUS_NORMAL,GetSmartPng(pSmartDev[i].type, 1));	
					m_pButton[k]->SetSrcpng(STATUS_PRESSED,GetSmartPng(pSmartDev[i].type, 3));
				}	
			}
			m_pButton[k]->SetSrcText(pSmartDev[i].name);     //名字的显示；
			m_pButton[k]->Show(STATUS_NORMAL);               //文字以及图片的显示。
		}						
		GetPrjShow(m_Show);
		TimeShow();
	}

	BOOL Create(DWORD lParam, DWORD zParam)            //lParam应该是当前第几页。
	{
		m_nCount = 0;
		m_lPage = 0;
		
		if (lParam >= 0 && lParam < MAX_PAGE_NUM)
			m_nPage = lParam;
		else 
			m_nPage = 0;
		
		m_pSmartDev = GetSmartDev(&m_dwCount);

		// 最大页数
		for (int k = 0; k < MAX_PAGE_NUM; k++) {
			SmartDev* pSmartDev = &m_pSmartDev[k * MAX_ICON_NUM];
			for (int i = 0; i < MAX_ICON_NUM; i++) {
				if (pSmartDev[i].exist) {
					m_mapPage[m_lPage++] = k;
					break;
				}
			}
		}
		
		// 当前页设备个数	
		for (int i = 0; i < MAX_ICON_NUM; i++) {
			SmartDev* pSmartDev = &m_pSmartDev[m_mapPage[m_nPage] * MAX_ICON_NUM];
			if (pSmartDev[i].exist) {
				m_mapButton[m_nCount++] = i;
			}
		}

		// 加载XML文件
		if (m_nCount >= 1 && m_nCount <= 6) {
			if (m_lPage % 2 == 0) {
				// 双页点
				sprintf(m_buf, "main%d_2.xml", m_nCount);
				InitFrame(m_buf);
			} else {
				// 单页点
				sprintf(m_buf, "main%d_1.xml", m_nCount);
				InitFrame(m_buf);
			}
		}
		else
			InitFrame("main6_2.xml");

		/* 加载图标位置 */
		for (int i = 0; i < m_nCount; i++) {
			sprintf(m_buf, "button%d", i + 1);
			m_pButton[i] = (CDPButton *)GetCtrlByName(m_buf, &m_idButton[i]);
		}

		/* 加载页数点位置 */
		m_nPoint = MAX_PAGE_NUM / 2 - (m_lPage + 1) / 2;
		for (int i = 0; i < m_lPage; i++) {
			sprintf(m_buf, "point%d", m_nPoint + i + 1);
			m_ppoint[m_nPoint + i] = (CDPButton *)GetCtrlByName(m_buf, &m_idpoint[m_nPoint + i]);
		}
		
		/* 加载文本位置 */
		for (int i = 0; i < 3; i++) {
			sprintf(m_buf, "title%d", i + 1);
			m_pTitle[i] = (CDPStatic *)GetCtrlByName(m_buf);
		}
			
		OnPage(m_mapPage[m_nPage]);

		return TRUE;
		
	}

private:
	DWORD m_nPage;									  // 当前页数
	DWORD m_lPage;									  // 最大页数
	DWORD m_nPoint;									  // 起始点编号
	DWORD m_nCount;									  // 当前页设备数量

	DWORD m_dwCount;                                  // 设备数量
	SmartDev* m_pSmartDev;    						  // 设备列表

	DWORD m_idButton[MAX_ICON_NUM];					  // 图标id	
	DWORD m_idpoint[MAX_PAGE_NUM]; 					  // 页数点id 
	CDPButton* m_pButton[MAX_ICON_NUM];				  // 图标句柄-this指针
	CDPButton* m_ppoint[MAX_PAGE_NUM];                  // 页数点句柄-this指针
	CDPStatic* m_pTitle[3];							  // 文本句柄-this指针
	BYTE m_mapPage[MAX_PAGE_NUM];						  // 页映射(显示→存储)
	BYTE m_mapButton[MAX_ICON_NUM];					  // 图标映射(显示→存储)

	SYSTEMTIME m_tm;                                  // 系统时间
	char m_buf[64];
	BOOL m_Show[4];
};

CAppBase* CreateMainApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CMainApp* pApp = new CMainApp(wParam);   //将指针指向主菜单界面。
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;
	}
	return pApp;
}
