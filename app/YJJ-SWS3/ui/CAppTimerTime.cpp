#include "CCtrlModules.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int TIMER_TIME = 0;
char timer_set[] = "00:00";                  //��ʱ�¼�ʱ������ʱ���֣���
int i_timer = 0,j_timer = 0,k_timer;
extern BOOL E_Tim;
extern char dstime[9];

extern BOOL E_Tim;

extern pNode p_Edit;                            //�༭��ʱ�¼�������ָ��              
class CTimerTimeApp : public CAppBase
{
public:
	CTimerTimeApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CTimerTimeApp()
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
					i_timer = 0;
					j_timer = 0;
					k_timer = 0;
					m_dwTimeout = 0;
					TIMER_TIME = 0;
					DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);			
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}			
			break;
		case TOUCH_SLIDE:
			m_dwTimeout = 0;
			break;				
		case TOUCH_MESSAGE:
			m_dwTimeout = 0;
			if(wParam == m_idCancel)
			{
				i_timer = 0;
				j_timer = 0;
				k_timer = 0;
				if(!E_Tim) {  //��������Ӷ�ʱ�¼�����
					TIMER_TIME = 0;
					DPPostMessage(MSG_START_APP, TIMER_OBJECT_APPID, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}

				else {        //�༭��ʱ�¼� (2018.1.16��ʼ�õ������)

					DPPostMessage(MSG_START_APP, PRJ_SET_TIMER_EDIT, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}

			else if(wParam == m_idOK) {
         
				i_timer = 0;
				j_timer = 0;
				k_timer = 0;

				if(!E_Tim) {

					strcpy(dstime,timer_set);
					TIMER_TIME = 0;
					DPPostMessage(MSG_START_APP, TIMER_OBJECT_APPID, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}

				else {       //�༭��ʱ�¼�

					strcpy(p_Edit->Time,timer_set);
					DPPostMessage(MSG_START_APP, PRJ_SET_TIMER_EDIT, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}

			else if(wParam == m_idDelete) {

				m_pEditPwd->Deletetimerjmp();       		   //ǰ�ƹ��

				if (j_timer > 0) {

					if (k_timer == 3 ||k_timer == 5) {	
						j_timer -= 2;
					} else  {
						j_timer--;
					}
					if (k_timer > 0)
						k_timer--;	
				}

			}
			break;
		case KBD_MESSAGE:
			m_dwTimeout = 0;
			if(wParam == KBD_CTRL)
			{
				if (k_timer == 0) {	
					j_timer = k_timer;
					if (lParam >= '0' && lParam <= '2')
						m_timeVal[0] = lParam;
					else {
						return TRUE;
					}
				}

				if (k_timer == 1) {
					j_timer= k_timer;
					if (m_timeVal[0] == '2') {
						if (lParam >= '0' && lParam <= '3')
							m_timeVal[1] = lParam;
						else {
							return TRUE;
						}
					}
				}
				
				if(k_timer == 2) {	
					j_timer = k_timer;
					if (lParam >= '0' && lParam <= '5')
						m_timeVal[2] = lParam;
					else {
						return TRUE;
					}
				}
			
				if(m_bShow) {
				   m_bShow = FALSE;
				   m_pEditPwd->SetIsPwd(FALSE);
				   m_pEditPwd->SetString("");
				}

				i_timer++;                                //����������ַ�����1
				k_timer++;                                //�ж�ʱ���ַ����ı���
				j_timer++;                                //ʱ���ַ�����1
				m_pEditPwd->Input_settimr(lParam);         //����ַ���

				if(k_timer == 3) {
					
					j_timer = k_timer + 1;
				}
				
				strncpy(timer_set+(j_timer-1),m_pEditPwd->GetString()+(i_timer-1),1);
				if (timer_set[0] == '2') {
					if (timer_set[1] > '3')
						timer_set[1] = '3';
				}
					

				if(k_timer == 4) {

					j_timer = 0;
					k_timer = 0;
				} 

				m_pEditPwd->Input_timerget(lParam);         //�����ַ���				
			}
			break;
		}
		return TRUE;	
	}

	void OnCreate(char* buf)
	{
		m_bShow = TRUE;
		m_pEditPwd->SetIsPwd(FALSE);
		m_pEditPwd->SetString(buf);
		m_pEditPwd->set_curpos();					   				  //�������Ϊ����һλ 
		m_pEditPwd->Show_Time(TRUE);                                  //����ʱ����ʾ�����show����
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("timer_time.xml");                                  //ʱ�����ý���XML�ļ���ȡ            
		//GetCtrlByName("save", &m_idSave);                             //���水������                          
		GetCtrlByName("back", &m_idCancel);                           //���ذ�������
		GetCtrlByName("delete", &m_idDelete);                         //ɾ����������
		GetCtrlByName("ok", &m_idOK);                                 //ȷ����������
		m_pEditPwd = (CEditBox *)GetCtrlByName("pwd", &m_idEditBox);  

		if(E_Tim) {

			strcpy(timer_set,p_Edit->Time);
			OnCreate(timer_set);  
		}
		else
			OnCreate(timer_set);                                       //��ʾʱ��
			
		m_bShow = TRUE;
		return TRUE;
	}

private:
	BYTE m_timeVal[4];
	DWORD m_idSave;
	DWORD m_idCancel;
	DWORD m_idDelete;
	DWORD m_idOK;
	DWORD m_idEditBox;                                                 //���ְ������Ʊ���
	BOOL m_bShow;                      
	CEditBox* m_pEditPwd;                                              //���ְ���ָ����Ʊ���
};

CAppBase* CreateTimerTimeApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CTimerTimeApp* pApp = new CTimerTimeApp(wParam);
	TIMER_TIME = 1;
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}