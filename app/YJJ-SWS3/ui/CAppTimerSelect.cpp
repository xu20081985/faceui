#include "CCtrlModules.h"
#include <string.h>

BOOL E_Tim = FALSE;                 //编辑定时事件的状态标志位 

extern char dstime[9];
extern char timer_set[9];

extern char *g_pData[12];
extern BOOL Flag[12];

extern BOOL Light_select;
extern char* p_Light;

extern BOOL Select_Open ;           //控制动作外部引用变量
extern BOOL Select_Close ;
extern char *p_action;

extern int timer;

pNode Head1;                      	// 遍历指针的中间变量
pNode Head2; 
pNode Head3; 

pNode p_Edit;
extern Time_Link_List* g_TimeHead;  // 头结点数据的指针

class CTimerSelectApp : public CAppBase
{
public:
	CTimerSelectApp (DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CTimerSelectApp ()
	{
	}
/*
 *功能:遍历显示链表数据
 */
	BOOL Translate_List_Onpage0() {

		pNode pSet;
		char buf[128];
		int Timer_Pos = 0;

		if(NULL == g_TimeHead) {         // 如果头结点是空指针证明没有定时数据则返回 

			return 0;
		} 

		else {

			pSet = g_TimeHead;

			while(NULL != pSet && Timer_Pos<4) {

				m_pTime[Timer_Pos]->SetSrc(pSet->Time);
				m_pTime[Timer_Pos]->Show(TRUE);

				sprintf(buf,"%s%s %s",pSet->Device,pSet->CTL_Type,pSet->p_Data);
				m_pAction[Timer_Pos]->SetSrc(buf);
				m_pAction[Timer_Pos]->Show(TRUE);
				m_pEdit[Timer_Pos]->SetSrcpng(STATUS_NORMAL,"next.png");
				m_pEdit[Timer_Pos]->SetSrcpng(STATUS_PRESSED,"next.png");
				m_pEdit[Timer_Pos]->Show(STATUS_NORMAL);
				m_pEdit[Timer_Pos]->Show(STATUS_PRESSED);
				
				pSet = pSet->next;			
				Timer_Pos++;     
			}

			Head1 =pSet;
		}
	  return 1;
	}

	BOOL Translate_List_Onpage1() {

		pNode pSet;
		char buf[128];
		int Timer_Pos = 0;

		if(NULL == Head1) {         // 如果头结点是空指针证明没有定时数据则返回 

			return 0;
		} 

		else {

			pSet = Head1;

			while(NULL != pSet && Timer_Pos<4) {

				m_pTime[Timer_Pos]->SetSrc(pSet->Time);
				m_pTime[Timer_Pos]->Show(TRUE);

				sprintf(buf,"%s%s %s",pSet->Device,pSet->CTL_Type,pSet->p_Data);
				m_pAction[Timer_Pos]->SetSrc(buf);
				m_pAction[Timer_Pos]->Show(TRUE);
				m_pEdit[Timer_Pos]->SetSrcpng(STATUS_NORMAL,"next.png");
				m_pEdit[Timer_Pos]->SetSrcpng(STATUS_PRESSED,"next.png");
				m_pEdit[Timer_Pos]->Show(STATUS_NORMAL);
				m_pEdit[Timer_Pos]->Show(STATUS_PRESSED);

				pSet = pSet->next;			
				Timer_Pos++;     
			}
			Head2 = pSet;
		}
	  return 1;
	}
	
	BOOL Translate_List_Onpage2() {

		pNode pSet;
		char buf[128];
		int Timer_Pos = 0;

		if(NULL == Head2) {         // 如果头结点是空指针证明没有定时数据则返回 

			return 0;
		} 

		else {

			pSet = Head2;

			while(NULL != pSet && Timer_Pos<4) {

				m_pTime[Timer_Pos]->SetSrc(pSet->Time);
				m_pTime[Timer_Pos]->Show(TRUE);

				sprintf(buf,"%s%s %s",pSet->Device,pSet->CTL_Type,pSet->p_Data);
				m_pAction[Timer_Pos]->SetSrc(buf);
				m_pAction[Timer_Pos]->Show(TRUE);
				m_pEdit[Timer_Pos]->SetSrcpng(STATUS_NORMAL,"next.png");
				m_pEdit[Timer_Pos]->SetSrcpng(STATUS_PRESSED,"next.png");
				m_pEdit[Timer_Pos]->Show(STATUS_NORMAL);
				m_pEdit[Timer_Pos]->Show(STATUS_PRESSED);		
				
				pSet = pSet->next;			
				Timer_Pos++;     
			}
			Head3 = pSet;
		}
	  return 1;
	}

	BOOL Translate_List_Onpage3() {
	
			pNode pSet;
			char buf[128];
			int Timer_Pos = 0;
	
			if(NULL == Head3) { 		// 如果头结点是空指针证明没有定时数据则返回 
	
				return 0;
			} 
	
			else {
	
				pSet = Head3;
	
				while(NULL != pSet && Timer_Pos<4) {
	
					m_pTime[Timer_Pos]->SetSrc(pSet->Time);
					m_pTime[Timer_Pos]->Show(TRUE);
	
					sprintf(buf,"%s%s %s",pSet->Device,pSet->CTL_Type,pSet->p_Data);
					m_pAction[Timer_Pos]->SetSrc(buf);
					m_pAction[Timer_Pos]->Show(TRUE);
					m_pEdit[Timer_Pos]->SetSrcpng(STATUS_NORMAL,"next.png");
					m_pEdit[Timer_Pos]->SetSrcpng(STATUS_PRESSED,"next.png");
					m_pEdit[Timer_Pos]->Show(STATUS_NORMAL);
					m_pEdit[Timer_Pos]->Show(STATUS_PRESSED);
	
					pSet = pSet->next;			
					Timer_Pos++;	 
				}
			}
		  return 1;
	}
	
	void Edit_Timer_List(pNode &point,int pos)
	{

		pNode p_Mid;
		
		if(NULL != point) {

			if(pos == 0) {

				p_Edit = point;                       //获得将要编辑的指针
				DPPostMessage(MSG_START_APP, PRJ_SET_TIMER_EDIT, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			if(pos>0) {

				p_Mid = point;
				for(int i = 0; i<pos; i++) {
			
					p_Mid = p_Mid->next;

					if(NULL != p_Mid)
						continue;

					else 
						return;
				}

				p_Edit = p_Mid;
				DPPostMessage(MSG_START_APP, PRJ_SET_TIMER_EDIT, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);			
			}

			E_Tim = TRUE;         //编辑状态标志位置位为1	
		}

		else
			return;
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
		case TOUCH_MESSAGE:
			m_dwTimeout = 0;
			if(wParam == m_idBack)
			{
				DPPostMessage(MSG_START_APP, PRJ_SET_TIMER_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			/******************编辑定时事件***********************/

			else if(wParam == m_idEdit[0])
			{
		
				switch(m_dwPage) {
		
				case 0:
					Edit_Timer_List(g_TimeHead,0);
					break;
				case 1:
					Edit_Timer_List(Head1,0);
					break;
				case 2:
					Edit_Timer_List(Head2,0);
					break;
				case 3:
					Edit_Timer_List(Head3,0);
					break;
				}				
			}
			else if(wParam == m_idEdit[1])
			{

				switch(m_dwPage) {

				case 0:
					Edit_Timer_List(g_TimeHead,1);
					break;
				case 1:
					Edit_Timer_List(Head1,1);
					break;
				case 2:
					Edit_Timer_List(Head2,1);
					break;
				case 3:
					Edit_Timer_List(Head3,1);
					break;
				}
			}
			else if(wParam == m_idEdit[2])
			{

				switch(m_dwPage) {

				case 0:
					Edit_Timer_List(g_TimeHead,2);
					break;
				case 1:
					Edit_Timer_List(Head1,2);
					break;
				case 2:
					Edit_Timer_List(Head2,2);
					break;
				case 3:
					Edit_Timer_List(Head3,2);
					break;
				}
			}
			else if(wParam == m_idEdit[3])
			{
				switch(m_dwPage) {

				case 0:
					Edit_Timer_List(g_TimeHead,3);
					break;
				case 1:
					Edit_Timer_List(Head1,3);
					break;
				case 2:
					Edit_Timer_List(Head2,3);
					break;
				case 3:
					Edit_Timer_List(Head3,3);
					break;
				}
			}
							
		  break;

		  case TOUCH_SLIDE:
		  	m_dwTimeout = 0;
			if(wParam == SLIDE_DOWN)   {

				for(int i=0;i<4;i++) {

					m_pTime[i]->Show(FALSE);
					m_pAction[i]->Show(FALSE);
					m_pEdit[i]->Show(STATUS_UNACK);
				}
				
				OnCreate((m_dwPage + 3) % 4);		
			}
            if(wParam == SLIDE_UPSIDE) {

				for(int i=0;i<4;i++) {

					m_pTime[i]->Show(FALSE);				
					m_pAction[i]->Show(FALSE);
					m_pEdit[i]->Show(STATUS_UNACK);
				}
				
				OnCreate((m_dwPage + 1) % 4);							
			} 
			break;
		}
		return TRUE;	
	}

	void OnCreate(int page)
	{

		 int i;
		 if(page == 0) {
		 	
            for(i =0;i<4;i++) {
				
				m_pTime[i]->SetSrc("");
				m_pTime[i]->Show(TRUE);

				m_pAction[i]->SetSrc("");
				m_pAction[i]->Show(TRUE);

            }
			Translate_List_Onpage0();
		}

		else if(page == 1) {
		
			for(i = 0;i<4;i++) {

			 	m_pTime[i]->SetSrc("");
				m_pTime[i]->Show(TRUE);

				m_pAction[i]->SetSrc("");
				m_pAction[i]->Show(TRUE);

			 }
			Translate_List_Onpage1();
		 }

		else if(page == 2) {

			for(i = 0;i<4;i++) {

			 	m_pTime[i]->SetSrc("");
				m_pTime[i]->Show(TRUE);

				m_pAction[i]->SetSrc("");
				m_pAction[i]->Show(TRUE);
		
			 }
			Translate_List_Onpage2();
		}

		else if(page == 3) {

			for(i = 0;i<4;i++) {

				m_pTime[i]->SetSrc("");	
				m_pTime[i]->Show(TRUE);

				m_pAction[i]->SetSrc("");
				m_pAction[i]->Show(TRUE);

			} 
			Translate_List_Onpage3();
		}

		m_dwPage = page;	
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("timer_select.xml");
		GetCtrlByName("back", &m_idBack);

		m_pTime[0] = (CDPStatic *)GetCtrlByName("time_1");
		m_pTime[1] = (CDPStatic *)GetCtrlByName("time_2");
		m_pTime[2] = (CDPStatic *)GetCtrlByName("time_3");
		m_pTime[3] = (CDPStatic *)GetCtrlByName("time_4");

		m_pAction[0] = (CDPStatic *)GetCtrlByName("action_1");
		m_pAction[1] = (CDPStatic *)GetCtrlByName("action_2");
		m_pAction[2] = (CDPStatic *)GetCtrlByName("action_3");
		m_pAction[3] = (CDPStatic *)GetCtrlByName("action_4");

		m_pEdit[0] = (CDPButton *)GetCtrlByName("switch_1", &m_idEdit[0]);
		m_pEdit[1] = (CDPButton *)GetCtrlByName("switch_2", &m_idEdit[1]);
		m_pEdit[2] = (CDPButton *)GetCtrlByName("switch_3", &m_idEdit[2]);
		m_pEdit[3] = (CDPButton *)GetCtrlByName("switch_4", &m_idEdit[3]);

		Translate_List_Onpage0();
		Translate_List_Onpage1();
		Translate_List_Onpage2();
		Translate_List_Onpage3();

		OnCreate(0);
		return TRUE;
	}

private:
	DWORD m_idBack;
	DWORD m_idEdit[4];
	DWORD m_dwPage;

	CDPStatic* m_pTime[4];          //定时事件的时间摆放位置
	CDPStatic* m_pAction[4];        //定时时间执行动作摆放位置
	CDPButton* m_pEdit[4];          //定时事件编辑
};

CAppBase* CreateTimerSelectApp (DWORD wParam, DWORD lParam, DWORD zParam)
{
	CTimerSelectApp * pApp = new CTimerSelectApp (wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}
