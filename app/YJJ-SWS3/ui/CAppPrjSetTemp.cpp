#include "CCtrlModules.h"
//#include "Dpdef_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//char temp[128];    
char  Tim_Set[20];
DWORD Tem_Cal_Heat;
DWORD Tem_Cal_Air;

extern TEMPTR TEM;
extern TEMPTR TEMP;

class CPrjSetTemApp : public CAppBase
{
public:
	CPrjSetTemApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CPrjSetTemApp()
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
					m_dwTimeout = 0;
					DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);	
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}

		//	printf("The Cal Temp is %d\n",Tem_Cal);
			break;
		case TOUCH_SLIDE:
			m_dwTimeout = 0;
			break;
        case TOUCH_MESSAGE:
			m_dwTimeout = 0;
            if(wParam == m_idBack)                     //如果是点击返回按键，则返回上一级
			{
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idOK)     {            //如果点击确认按键则执行如下操作

				
				strcpy(Tim_Set, m_pEditPwd->GetString());
				Tem_Cal_Heat = atoi(Tim_Set);
				Tem_Cal_Air  = atoi(Tim_Set);

				
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idDelete) {            //如果点击删除按键则执行如下操作
 		
				if(!m_bShow)
				{
					if(m_pEditPwd->GetCurCount() == 0)
					{
						ShowTip(GetStringByID(4012));	
					}
					else
					{
						m_pEditPwd->Delete();
					}
				}
				
			}

			break;
		case KBD_MESSAGE:                              //键盘输入命令

			if(wParam == KBD_CTRL) {
            
           		if(m_bShow) {
					
					m_pEditPwd->SetIsPwd(FALSE);       //显示数字
					m_pEditPwd->SetString("");
					m_bShow = FALSE;
				}
				if(m_pEditPwd->GetCurCount() <=1)
				 	 m_pEditPwd->Inputtemp(lParam);
		    }
			break;      
		}
		return TRUE;	
	}


	void ShowTip(char* buf)                            //显示输入密码框
	{
	    m_bShow = TRUE;
		m_pEditPwd->SetIsPwd(FALSE);
		m_pEditPwd->SetString(buf);
		m_pEditPwd->set_curpos();					   //将光标置为到第一位 
		m_pEditPwd->Show_Time(TRUE);                   //显示数字	
       
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{	                                                             //与实时时钟相关的变量定义
		InitFrame("prj_settemp.xml");                                //日期及时间设置框架
 	    GetCtrlByName("back", &m_idBack);                            //得到返回按键的控制
		GetCtrlByName("ok", &m_idOK);                                //得到确认键的控制
		GetCtrlByName("delete", &m_idDelete);                        //得到删除键的控制
		m_pEditPwd = (CEditBox *)GetCtrlByName("pwd", &m_idEditPwd); //输入框            

		ShowTip(GetStringByID(4012));
		
		return TRUE;  
	}

private:
	DWORD m_idBack;                                    //返回按键控制变量
	DWORD m_idOK;                                      //确认键控制变量
	DWORD m_idDelete;                                  //删除键控制变量
	DWORD m_idEditPwd;                                 //数字按键控制变量
	BOOL m_bShow;
	CEditBox* m_pEditPwd;                              //数字按键指针控制变量
	CLayOut* m_pLayOutTip;                             //提示控制变量   
	CTimeDate* m_pTime;                                //时钟更新
	SYSTEMTIME tm; 

	
};

CAppBase* CreatePrjSetTempApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjSetTemApp* pApp = new CPrjSetTemApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}  
