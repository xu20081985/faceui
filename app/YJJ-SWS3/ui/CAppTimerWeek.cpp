#include "CCtrlModules.h"
#include <string.h>
BOOL Flag[12] = {FALSE};                     // 定义勾选项的标志位数组
char *g_pData[12] ={NULL};                   // 定义存储重复日期的字符串指针

extern BOOL E_Tim;

extern pNode p_Edit;       					//定时事件编辑时参数的结构体指针变量

class CTimerWeekApp : public CAppBase
{
public:
	CTimerWeekApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CTimerWeekApp()
	{
	}

	void Select_Week(int i) 
	{
		if(Flag[i]) {

			Flag[i]  = FALSE;
			g_pData[i] = NULL;
		}
							
		else {

			Flag[i]  = TRUE;
			g_pData[i] = GetStringByID(13200+i);  //工作日   
		}
	}

	void Edit_Timer(int i)
	{

		if(Flag[i]) {

			Flag[i] = FALSE;
			g_pData[i] = NULL;
		}
		else {

			Flag[i] = TRUE;
			g_pData[i]= GetStringByID(13200+i); 
		}	
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
					m_dwTimeout = 0;
					DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);			
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}			
			break;				
		case TOUCH_MESSAGE: //返回按键
			m_dwTimeout = 0;
			if(wParam == m_idBack)
   			{   
				
				memset(p_Edit->p_Data,0,128);

				for(int i=0; i < 12; i++) {                         //重复日期字符串赋值
		
					if(NULL != g_pData[i]) {
      
		   				strcat(p_Edit->p_Data, g_pData[i]);     
					}		   
				}

			   for(int k = 0; k < 12; k++) {                        //重复日期勾选标记

					p_Edit->choose_data[k] = Flag[k];
			    }

			   for(int p = 0; p < 12; p++)                          //将重复的时间置为空
		  			 g_pData[p] = NULL;
			   	
				DPPostMessage(MSG_START_APP, PRJ_SET_TIMER_EDIT, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			
			}

			else if(wParam == m_idselect_1) {
			
				switch(m_dwPage) {

				case 0:		
					Edit_Timer(0);
				break;
				
				case 1:
					 Edit_Timer(4);
				break;

				case 2:
					 Edit_Timer(8);
					 break;
					
				default:
					break;									
				} 
		
			}
			
			else if(wParam == m_idselect_2) {
							
				switch(m_dwPage) {
			
				case 0:	  	 
					Edit_Timer(1);
				break;
				
				case 1:
				   	 Edit_Timer(5);
				break;

				case 2:
					Edit_Timer(9);
					break;
				default:
					 break;
														
				}
			}

		    else if(wParam == m_idselect_3) {
					
				switch(m_dwPage) {
			
				case 0:
					Edit_Timer(2);
				break;
				 
				case 1:
					Edit_Timer(6);
				break;

				case 2:
					Edit_Timer(10);
					break;
				
				default:
					 break;

				}
		    }

			else if(wParam == m_idselect_4) {
						
				switch(m_dwPage) {
			
				case 0:
				 	Edit_Timer(3);
				break;

				case 1:
				  	Edit_Timer(7);
							 
				break;

				case 2:
					Edit_Timer(11);
					break;
				
				default:
					 break;

				}
			}

			if(Flag[7] == TRUE) {         // 如果选择了每天的选项

				Flag[0] = FALSE;  // 周一
				Flag[1] = FALSE;  // 周二
				Flag[2] = FALSE;  // 周三
				Flag[3] = FALSE;  // 周四
				Flag[4] = FALSE;  // 周五
				Flag[5] = FALSE;  // 周六
				Flag[6] = FALSE;  // 周日
				Flag[8] = FALSE;  // 周日
				Flag[9] = FALSE;  // 周日
				Flag[10] = FALSE; // 周日
				Flag[11] = FALSE; // 周日
				
				g_pData[0] = NULL;       
				g_pData[1] = NULL;
				g_pData[2] = NULL;
				g_pData[3] = NULL;
				g_pData[4] = NULL;
				g_pData[5] = NULL;
				g_pData[6] = NULL;
				g_pData[8] = NULL;
				g_pData[9] = NULL;
				g_pData[10] = NULL;
				g_pData[11] = NULL;
			}

			else if(Flag[8] == TRUE) {     //  如果选择了工作日
		
				Flag[0] = FALSE;
				Flag[1] = FALSE;
				Flag[2] = FALSE;
				Flag[3] = FALSE;
				Flag[4] = FALSE;
				Flag[10] = FALSE;
				Flag[11] = FALSE;
				
				g_pData[0] = NULL;       
				g_pData[1] = NULL;
				g_pData[2] = NULL;
				g_pData[3] = NULL;
				g_pData[4] = NULL;
				g_pData[10] = NULL;
				g_pData[11] = NULL;
			}

			else if(Flag[9] == TRUE) {      // 如果选择了周末

				Flag[5] = FALSE;  // 周六
				Flag[6] = FALSE;  // 周日
				Flag[10] = FALSE;
				Flag[11] = FALSE;
				
				g_pData[5] = NULL;
				g_pData[6] = NULL;
				g_pData[10] = NULL;
				g_pData[11] = NULL;
			}

			else if(Flag[10]) {             // 如果选择了单次

				Flag[0] = FALSE;  // 周一
				Flag[1] = FALSE;  // 周二
				Flag[2] = FALSE;  // 周三
				Flag[3] = FALSE;  // 周四
				Flag[4] = FALSE;  // 周五
				Flag[5] = FALSE;  // 周六
				Flag[6] = FALSE;  // 周日
				Flag[7] = FALSE;  // 周五
				Flag[8] = FALSE;  // 周六
				Flag[9] = FALSE;  // 周日
				Flag[11] = FALSE;  // 周日
				
				g_pData[0] = NULL;       
				g_pData[1] = NULL;
				g_pData[2] = NULL;
				g_pData[3] = NULL;
				g_pData[4] = NULL;
				g_pData[5] = NULL;
				g_pData[6] = NULL;
				g_pData[7] = NULL;
				g_pData[8] = NULL;
				g_pData[9] = NULL;
				g_pData[11] = NULL;
			}
			
			else if(Flag[11]) {             // 如果选择了永不

				Flag[0] = FALSE;  // 周一
				Flag[1] = FALSE;  // 周二
				Flag[2] = FALSE;  // 周三
				Flag[3] = FALSE;  // 周四
				Flag[4] = FALSE;  // 周五
				Flag[5] = FALSE;  // 周六
				Flag[6] = FALSE;  // 周日
				Flag[7] = FALSE;  // 周五
				Flag[8] = FALSE;  // 周六
				Flag[9] = FALSE;  // 周日
				Flag[10] = FALSE; // 周日
				
				g_pData[0] = NULL;       
				g_pData[1] = NULL;
				g_pData[2] = NULL;
				g_pData[3] = NULL;
				g_pData[4] = NULL;
				g_pData[5] = NULL;
				g_pData[6] = NULL;
				g_pData[7] = NULL;
				g_pData[8] = NULL;
				g_pData[9] = NULL;
				g_pData[10] = NULL;
			}
		OnPage(m_dwPage);
		break;
		case TOUCH_SLIDE:   //滑动屏幕操作
 			m_dwTimeout = 0;
			if(wParam == SLIDE_DOWN)  {   
            
				OnPage((m_dwPage + 2) % 3);
			}

			if(wParam == SLIDE_UPSIDE) {

				OnPage((m_dwPage + 1) % 3);
			}
     
			break;
			
		}
		return TRUE;	
	}

	void OnPage(DWORD dwPage)
	{
		for (int i = 0; i < 5; i++) {
			if (Flag[i] == FALSE)
				break;
			if (i == 4) {
				Flag[0] = FALSE;
				Flag[1] = FALSE;
				Flag[2] = FALSE;
				Flag[3] = FALSE;
				Flag[4] = FALSE;
				Flag[5] = FALSE;
				Flag[8] = TRUE;

				g_pData[0] = NULL;       
				g_pData[1] = NULL;
				g_pData[2] = NULL;
				g_pData[3] = NULL;
				g_pData[4] = NULL;
				g_pData[8] = GetStringByID(13200+8); 
			}
		}

		for (int i = 5; i < 7; i++) {
			if (Flag[i] == FALSE)
				break;
			if (i == 6) {
				Flag[5] = FALSE;
				Flag[6] = FALSE;
				Flag[9] = TRUE;

				g_pData[5] = NULL;
				g_pData[6] = NULL;
				g_pData[9] = GetStringByID(13200+9); 				
			}
		}

		for (int i = 8; i < 10; i++) {
			if (Flag[i] == FALSE)
				break;
			if (i == 9) {
				Flag[8] = FALSE;
				Flag[9] = FALSE;
				Flag[7] = TRUE;

				g_pData[8] = NULL;
				g_pData[9] = NULL;
				g_pData[7] = GetStringByID(13200+7);

				g_pData[0] = NULL;       
				g_pData[1] = NULL;
				g_pData[2] = NULL;
				g_pData[3] = NULL;
				g_pData[4] = NULL;
				g_pData[5] = NULL;
				g_pData[6] = NULL;
				g_pData[7] = NULL;
				g_pData[8] = NULL;
				g_pData[9] = NULL;
				g_pData[10] = NULL;	
				g_pData[11] = NULL;	

				g_pData[7] = GetStringByID(13200+7); 
			}
		}

		for (int i = 0; i < 7; i++) {
			if (Flag[i] == FALSE)
				break;
			if (i == 6) {
				memset(Flag, FALSE, sizeof(Flag));
				Flag[7] = TRUE;
			}
		}

		
		if(dwPage == 0) {
			m_pWeek[0]->SetSrc(GetStringByID(13200));		// 周一
			m_pWeek[0]->Show(TRUE);
			m_pWeek[1]->SetSrc(GetStringByID(13201));		// 周二 
			m_pWeek[1]->Show(TRUE);
			m_pWeek[2]->SetSrc(GetStringByID(13202));		// 周三
			m_pWeek[2]->Show(TRUE);
			m_pWeek[3]->SetSrc(GetStringByID(13203));		// 周四
			m_pWeek[3]->Show(TRUE);

			m_pSelect[0]->Show(Flag[0]);
			m_pSelect[1]->Show(Flag[1]);
			m_pSelect[2]->Show(Flag[2]);
			m_pSelect[3]->Show(Flag[3]);
	    }

		else if(dwPage == 1) {

			m_pWeek[0]->SetSrc(GetStringByID(13204));		// 周五
			m_pWeek[0]->Show(TRUE);
			m_pWeek[1]->SetSrc(GetStringByID(13205));		// 周六
			m_pWeek[1]->Show(TRUE);
			m_pWeek[2]->SetSrc(GetStringByID(13206));		// 周日
			m_pWeek[2]->Show(TRUE);
			m_pWeek[3]->SetSrc(GetStringByID(13207));		// 每天
			m_pWeek[3]->Show(TRUE);
		
			m_pSelect[0]->Show(Flag[4]);
			m_pSelect[1]->Show(Flag[5]);
			m_pSelect[2]->Show(Flag[6]);
			m_pSelect[3]->Show(Flag[7]);
		}

		else if(dwPage == 2) {

			m_pWeek[0]->SetSrc(GetStringByID(13208));		// 工作日
			m_pWeek[0]->Show(TRUE);
			m_pWeek[1]->SetSrc(GetStringByID(13209));		// 周末
			m_pWeek[1]->Show(TRUE);
			m_pWeek[2]->SetSrc(GetStringByID(13210));		// 单次
			m_pWeek[2]->Show(TRUE);
			m_pWeek[3]->SetSrc(GetStringByID(13211));		// 永不
			m_pWeek[3]->Show(TRUE);
		
			m_pSelect[0]->Show(Flag[8]);
			m_pSelect[1]->Show(Flag[9]);
			m_pSelect[2]->Show(Flag[10]);
			m_pSelect[3]->Show(Flag[11]);
		}
	         
	      m_dwPage = dwPage;
	}
	
	void OnCreate()
	{
		
		OnPage(0);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("timer_week.xml");
		GetCtrlByName("back", &m_idBack);

    /************************重复日期选项*****************************/
	
		m_pWeek[0] = (CDPStatic *)GetCtrlByName("week_1");
		m_pWeek[1] = (CDPStatic *)GetCtrlByName("week_2");
		m_pWeek[2] = (CDPStatic *)GetCtrlByName("week_3");
		m_pWeek[3] = (CDPStatic *)GetCtrlByName("week_4");
		
    /****************************************************************/

	
    /************************重复日期勾选*****************************/	

		GetCtrlByName("ctl_data1", &m_idselect_1);
		GetCtrlByName("ctl_data2", &m_idselect_2);
		GetCtrlByName("ctl_data3", &m_idselect_3);
		GetCtrlByName("ctl_data4", &m_idselect_4);
		
		m_pSelect[0] = (CDPStatic *)GetCtrlByName("select_1");
		m_pSelect[1] = (CDPStatic *)GetCtrlByName("select_2");
		m_pSelect[2] = (CDPStatic *)GetCtrlByName("select_3");
		m_pSelect[3] = (CDPStatic *)GetCtrlByName("select_4");
		
    /****************************************************************/   

		for(int k = 0; k<12; k++) {                        //重复日期勾选标记

			Flag[k] = p_Edit->choose_data[k];

			if(Flag[k])
				g_pData[k]= GetStringByID(13200+k);
		    
		}
		OnCreate();
		return TRUE;
	  
	}

private:
	DWORD m_idBack;
	DWORD m_idselect_1;
	DWORD m_idselect_2;
	DWORD m_idselect_3;
	DWORD m_idselect_4;
	CDPStatic* m_pWeek[4];
	CDPStatic* m_pSelect[4];

	DWORD m_dwPage;
};

CAppBase* CreateTimerWeekApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CTimerWeekApp* pApp = new CTimerWeekApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}
