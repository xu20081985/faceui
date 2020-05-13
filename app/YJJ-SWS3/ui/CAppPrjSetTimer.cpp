#include "CCtrlModules.h"

int Pos;                                    // ��ʱ�¼���λ����Ϣ
pNode page1;                      	        // ����ָ����м����
pNode page2; 
pNode page3; 

extern Time_Link_List* g_TimeHead;          // ͷ������ݵ�ָ��

class CPrjSetTimerApp : public CAppBase
{
public:
	CPrjSetTimerApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CPrjSetTimerApp()
	{
	}

/*
 *����:������ʱ�¼�����������
 */

	BOOL Translate_List_Onpage3()
	{
			
		pNode pSet;
		char buf[128];
		int Timer_Pos = 0;
			
		if(NULL == page3) { 	   // ���ͷ����ǿ�ָ��֤��û�ж�ʱ�����򷵻� 
	
			return 0;
		}		   
				
		else {					   // ֤���ж�ʱ�¼���ʼ��������ı���
	
			pSet = page3;
			
			while(NULL != pSet && Timer_Pos<4) {									  // ����ѭ��
							
				m_pTime[Timer_Pos]->SetSrc(pSet->Time);
				m_pTime[Timer_Pos]->Show(TRUE);	

				sprintf(buf,"%s %s %s",pSet->Device,pSet->CTL_Type,pSet->p_Data);
				m_pAction[Timer_Pos]->SetSrc(buf);
				m_pAction[Timer_Pos]->Show(TRUE);

				if(pSet->show) {
			
					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_NORMAL,"sign_on.png");
					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_PRESSED,"sign_on.png");
					m_pSwitch[Timer_Pos]->Show(STATUS_NORMAL);
					m_pSwitch[Timer_Pos]->Show(STATUS_PRESSED);
				}

				else {

					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_NORMAL,"sign_off.png");
					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_PRESSED,"sign_off.png");
		   			m_pSwitch[Timer_Pos]->Show(STATUS_NORMAL);
					m_pSwitch[Timer_Pos]->Show(STATUS_PRESSED);
				}
			
				pSet = pSet->next;					
				Timer_Pos++;										  // �ҵ�ָ�� 
			 }	
		  }
		return 1;
	  }

	BOOL Translate_List_Onpage2()
	{
		
		pNode pSet;
		char buf[128];
		int Timer_Pos = 0;
		
		if(NULL == page2) {        // ���ͷ����ǿ�ָ��֤��û�ж�ʱ�����򷵻� 

			return 0;
		}          
			
		else {                     // ֤���ж�ʱ�¼���ʼ��������ı���

			pSet = page2;
		
			while(NULL != pSet && Timer_Pos<4) {                                     // ����ѭ��
						
				m_pTime[Timer_Pos]->SetSrc(pSet->Time);
				m_pTime[Timer_Pos]->Show(TRUE);	

				sprintf(buf,"%s %s %s",pSet->Device,pSet->CTL_Type,pSet->p_Data);
				m_pAction[Timer_Pos]->SetSrc(buf);
				m_pAction[Timer_Pos]->Show(TRUE);

				if(pSet->show) {
			
					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_NORMAL,"sign_on.png");
					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_PRESSED,"sign_on.png");
					m_pSwitch[Timer_Pos]->Show(STATUS_NORMAL);
					m_pSwitch[Timer_Pos]->Show(STATUS_PRESSED);
				}

				else {

					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_NORMAL,"sign_off.png");
					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_PRESSED,"sign_off.png");
	    			m_pSwitch[Timer_Pos]->Show(STATUS_NORMAL);
					m_pSwitch[Timer_Pos]->Show(STATUS_PRESSED);
				}
	
				pSet = pSet->next;		
				Timer_Pos++;                                          // �ҵ�ָ�� 
			 }
			
			page3 = pSet;
	     }
		return 1;
	 }

BOOL Translate_List_Onpage1()
	{
		
		pNode pSet;
		char buf[128];
		int Timer_Pos = 0;
		
		if(NULL == page1) {        // ���ͷ����ǿ�ָ��֤��û�ж�ʱ�����򷵻� 

			return 0;
		}          
			
		else {                           // ֤���ж�ʱ�¼���ʼ��������ı���

			pSet = page1;
		
			while(NULL != pSet&&Timer_Pos<4) {                                     // ����ѭ��
						
				m_pTime[Timer_Pos]->SetSrc(pSet->Time);
				m_pTime[Timer_Pos]->Show(TRUE);

				sprintf(buf,"%s %s %s",pSet->Device,pSet->CTL_Type,pSet->p_Data);
				m_pAction[Timer_Pos]->SetSrc(buf);
				m_pAction[Timer_Pos]->Show(TRUE);

				if(pSet->show) {
			
					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_NORMAL,"sign_on.png");
					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_PRESSED,"sign_on.png");
					m_pSwitch[Timer_Pos]->Show(STATUS_NORMAL);
					m_pSwitch[Timer_Pos]->Show(STATUS_PRESSED);
				}

				else {

					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_NORMAL,"sign_off.png");
					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_PRESSED,"sign_off.png");
		   			m_pSwitch[Timer_Pos]->Show(STATUS_NORMAL);
					m_pSwitch[Timer_Pos]->Show(STATUS_PRESSED);
				}

				pSet = pSet->next;		
				Timer_Pos++;                                          // �ҵ�ָ�� 
			 }

			page2 = pSet;
	     }
		return 1;
	 }

BOOL Translate_List_Onpage0()
	{
		
		pNode pSet;
		char buf[128];
		int Timer_Pos = 0;
		
		if(NULL == g_TimeHead) {         // ���ͷ����ǿ�ָ��֤��û�ж�ʱ�����򷵻� 

			return 0;
		}          
			
		else {                           // ֤���ж�ʱ�¼���ʼ��������ı���

			pSet = g_TimeHead;
		
			while(NULL != pSet && Timer_Pos<4) {                                     // ����ѭ��
									
				m_pTime[Timer_Pos]->SetSrc(pSet->Time);
				m_pTime[Timer_Pos]->Show(TRUE);

				sprintf(buf,"%s %s %s",pSet->Device,pSet->CTL_Type,pSet->p_Data);
				m_pAction[Timer_Pos]->SetSrc(buf);
				m_pAction[Timer_Pos]->Show(TRUE);

				if(pSet->show) {
			
					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_NORMAL,"sign_on.png");
					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_PRESSED,"sign_on.png");
					m_pSwitch[Timer_Pos]->Show(STATUS_NORMAL);
					m_pSwitch[Timer_Pos]->Show(STATUS_PRESSED);
					DBGMSG(DPINFO, "111\r\n");
				}

				else {

					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_NORMAL,"sign_off.png");
					m_pSwitch[Timer_Pos]->SetBkpng(STATUS_PRESSED,"sign_off.png");
		   			m_pSwitch[Timer_Pos]->Show(STATUS_NORMAL);
					m_pSwitch[Timer_Pos]->Show(STATUS_PRESSED);
				}
			
				pSet = pSet->next;			
				Timer_Pos++;                                          // �ҵ�ָ�� 
			 }

			 page1 =pSet;
	     }
		  return 1;
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
			{ // ���ص�����һ�� 
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 1, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idEdit)
			{ // �����˱༭����
				DPPostMessage(MSG_START_APP, TIMER_SELECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			
            else if(wParam == m_idSwitch[0]) {

				pNode p_Mid;

				if(m_dwPage == 0)  {
					p_Mid = g_TimeHead;
				}	
						
				else if(m_dwPage == 1) {
					p_Mid = page1;
				} 			
					
				else if(m_dwPage == 2) {
					p_Mid = page2;
				}	
					
				else if(m_dwPage == 3){

					p_Mid = page3;
				}
					
				if(NULL != p_Mid) {

					if(p_Mid->show)
						p_Mid->show = FALSE;

					else
						p_Mid->show = TRUE;

				   UpdatSetTimer();                          //����
				}

				OnCreate(m_dwPage);
			}
			else if(wParam == m_idSwitch[1]) {

				pNode p_Mid;

				if(m_dwPage == 0)  {
					p_Mid = g_TimeHead;
				}	
						
				else if(m_dwPage == 1) {
					p_Mid = page1;
				} 			
					
				else if(m_dwPage == 2) {
					p_Mid = page2;
				}	
					
				else if(m_dwPage == 3){

					p_Mid = page3;
				}

				if(NULL != p_Mid) {
			
					if(NULL != p_Mid->next) {
					
						if(p_Mid->next->show)
							p_Mid->next->show = FALSE;
					
						else
							p_Mid->next->show = TRUE;
					
						UpdatSetTimer(); 						 //����
					}
				}

				OnCreate(m_dwPage);
			}
			else if(wParam == m_idSwitch[2]) {

				pNode p_Mid;
				DBGMSG(DPINFO, "222\r\n");
				if(m_dwPage == 0)  {
					p_Mid = g_TimeHead;
				}	
						
				else if(m_dwPage == 1) {
					p_Mid = page1;
				} 			
					
				else if(m_dwPage == 2) {
					p_Mid = page2;
				}	
					
				else if(m_dwPage == 3){

					p_Mid = page3;
				}

				if(NULL != p_Mid) {
			
					if(NULL != p_Mid->next) {
			
						if(NULL != p_Mid->next->next) {             //������Ҫ��
					
							if(p_Mid->next->next->show)
								p_Mid->next->next->show = FALSE;
					
							else
								p_Mid->next->next->show = TRUE;
					
						UpdatSetTimer(); 						    //����
						}
					}
				}

				OnCreate(m_dwPage);	
			}
			else if(wParam == m_idSwitch[3]) {

				pNode p_Mid;
				DBGMSG(DPINFO, "333\r\n");
				if(m_dwPage == 0)  {
					p_Mid = g_TimeHead;
				}	
						
				else if(m_dwPage == 1) {
					p_Mid = page1;
				} 			
					
				else if(m_dwPage == 2) {
					p_Mid = page2;
				}	
					
				else if(m_dwPage == 3)  {

					p_Mid = page3;
				}

				if(NULL != p_Mid) {
			
					if(NULL != p_Mid->next) {
			
						if(NULL != p_Mid->next->next) {            
					
							if(NULL != p_Mid->next->next->next)

								if(p_Mid->next->next->next->show)
									p_Mid->next->next->next->show = FALSE;
					
								else
									p_Mid->next->next->next->show = TRUE;
					
						UpdatSetTimer(); 						//����
						}
					}
				}

				OnCreate(m_dwPage);	
			}
			break;

		case TOUCH_SLIDE:
			m_dwTimeout = 0;
			if(wParam == SLIDE_DOWN)   {

				for(int i=0;i<4;i++) {

					m_pTime[i]->Show(FALSE);
					m_pAction[i]->Show(FALSE);
					m_pSwitch[i]->Show(STATUS_UNACK);
				}
				
				OnCreate((m_dwPage + 3) % 4);		
			}
            if(wParam == SLIDE_UPSIDE) {

				for(int i=0;i<4;i++) {

					m_pTime[i]->Show(FALSE);				
					m_pAction[i]->Show(FALSE);
					m_pSwitch[i]->Show(STATUS_UNACK);
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
		InitFrame("prj_settimer.xml");
		GetCtrlByName("back", &m_idBack);                         //���ذ�������
		GetCtrlByName("edit", &m_idEdit);                         //�༭��������

		m_pTime[0] = (CDPStatic *)GetCtrlByName("time_1");        //ʱ���ϵ���ʾλ��
		m_pTime[1] = (CDPStatic *)GetCtrlByName("time_2");
		m_pTime[2] = (CDPStatic *)GetCtrlByName("time_3");
		m_pTime[3] = (CDPStatic *)GetCtrlByName("time_4"); 

		m_pAction[0] = (CDPStatic *)GetCtrlByName("action_1");    //�����ϵ���ʾ��Ϣ  
		m_pAction[1] = (CDPStatic *)GetCtrlByName("action_2");
		m_pAction[2] = (CDPStatic *)GetCtrlByName("action_3");
		m_pAction[3] = (CDPStatic *)GetCtrlByName("action_4"); 

		m_pSwitch[0] = (CDPButton *)GetCtrlByName("switch_1", &m_idSwitch[0]);
		m_pSwitch[1] = (CDPButton *)GetCtrlByName("switch_2", &m_idSwitch[1]);
		m_pSwitch[2] = (CDPButton *)GetCtrlByName("switch_3", &m_idSwitch[2]);
		m_pSwitch[3] = (CDPButton *)GetCtrlByName("switch_4", &m_idSwitch[3]); 

		Translate_List_Onpage0();
		Translate_List_Onpage1();
		Translate_List_Onpage2();
		Translate_List_Onpage3();
		OnCreate(0);
		return TRUE;
	}

private:
	DWORD m_idBack;                          //���ذ������Ʊ���
	DWORD m_idEdit;                          //�༭�������Ʊ���
	DWORD m_idSwitch[16];
	DWORD m_dwPage;                          //ҳ��
	
	CDPStatic* m_pTime[16];                  //ʱ����ʾ
	CDPStatic* m_pAction[16];                //ִ�ж�����ʾ
	CDPButton* m_pSwitch[16];                //��/�ض���������ʾ
};

CAppBase* CreatePrjSetTimerApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjSetTimerApp* pApp = new CPrjSetTimerApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}
