#include "CCtrlModules.h"
//#include "Dpdef_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int INPUT_TIME = 0;                         // ʱ�������־λ
int Year;
int Month;
int Day;
int Hour;
int Minutes;
int Second;
int i = 0,j = 0,k;
char dest[15];

int yue = 0;
int riq = 0;
int shi = 0;

char JG_TM[15];                             // 2018.4.23��ӣ���������ʱ���ж� 
char JG_YEAR[5];

char time_set[]="2017-02-01-00:00:00";      // �ַ��������ʼ��

/* ����ʱ���ʵʱ���� */
int  t_Year;
int  t_Month;
int  t_Day;
int  t_Hour;
int  t_Minutes;
int  t_Second;

extern int TIME_FLAG;

//const char src[512];
class CPrjSetDateApp : public CAppBase
{
public:
	CPrjSetDateApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CPrjSetDateApp()
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
			break;
		case TOUCH_SLIDE:
			m_dwTimeout = 0;
			break;
        case TOUCH_MESSAGE:
			m_dwTimeout = 0;
            if(wParam == m_idBack)                     //����ǵ�����ذ������򷵻���һ��
			{
				i = 0;
				j = 0;
				k = 0;
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idOK)     {            //������ȷ�ϰ�����ִ�����²���

				INPUT_TIME = TRUE;                     //����־λ���֤���Ѿ��������

				TIME_FLAG = 1;                         //ȡ��У׼ʱ����ʾ
				/* ���ַ�������ת�������������鲢��ֵ����Ӧ���� */     
				Year    = atoi(strncpy(dest,time_set,4));
				Month   = atoi(strncpy(dest+4,time_set+5,2));
				Day     = atoi(strncpy(dest+6,time_set+8,2));
				Hour    = atoi(strncpy(dest+8,time_set+11,2));
				Minutes = atoi(strncpy(dest+10,time_set+14,2));
				Second  = atoi(strncpy(dest+12,time_set+17,2));

                tm.wYear   = Year;
				tm.wMonth  = Month;
				tm.wDay    = Day ;
				tm.wHour   = Hour ;
				tm.wMinute = Minutes;
				tm.wSecond = Second;
				DPSetLocalTime(&tm);

				memset(dest,0,15);                     //Դ��ַ��Ŀ���ַ���ص�
                i = 0;
				j = 0;
				k = 0;
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idDelete) {            //������ɾ��������ִ�����²���
 		
				m_pEditPwd->Deletejmp();       		   //ǰ�ƹ��

				if(j >= 0) {

					if(k == 5 ||k == 7 ||k == 9 ||k== 11 ||k== 13)	{

						j-=2;
						k--;
					}
						

					else {

						if(j > 0) {

							k--;
							j--;
						}		
					}			
				}		
			}

			break;
		case KBD_MESSAGE:                              //������������
			m_dwTimeout = 0;
			if(wParam == KBD_CTRL) {
            	
           		if(m_bShow) {
					
					m_pEditPwd->SetIsPwd(FALSE);       //��ʾ����
					m_pEditPwd->SetString("");
					m_bShow = FALSE;
				}
					
				  i ++;                                //����������ַ�����1
				  k ++;                                //�ж�ʱ���ַ����ı���
				  j ++;                                //ʱ���ַ�����1

				//printf("the num is %d\n\r",lParam);
				
/*---------------------------------------------------------------------------------*/     
                // ����������
				if(k == 1) {

					if((lParam - 48) != 2) {

						k--;
						j--;
						i--;
						m_pEditPwd->Input_get(lParam);
						break;
					}
						
				}


				// �·�
				if(k == 5) {

					if((lParam - 48) >= 2) {

						k--;
						j--;
						i--;
						m_pEditPwd->Input_get(lParam);
						break;
					}

					else {

						yue = (lParam - 48);

						if(yue == 1) {

							if(atoi(strncpy(dest+5,time_set+6,1)) > 2) {

								strncpy(time_set+6,"2",1);
							
							}
						}
					} 			
				}

				if(k == 6) {
				
					if(yue == 1) {

						if((lParam - 48) > 2) {

							k--;
							j--;
							i--;
							m_pEditPwd->Input_get(lParam);
							break;
						}

						else 
							atoi(strncpy(time_set+8,"01",2));
					}

					else  {

						if((lParam - 48) == 0) {

							k--;
							j--;
							i--;
							m_pEditPwd->Input_get(lParam);
							break;
						}

						else 
							atoi(strncpy(time_set+8,"01",2));

					}			
					
				}

				// ����
				if(k == 7) {

					// ����2�·�
					if(atoi(strncpy(JG_TM+4,time_set+5,2)) != 2) {

						if((lParam - 48) > 3) {

								k--;
								j--;
								i--;
								m_pEditPwd->Input_get(lParam);
								break;
						}

						else {

							riq = (lParam - 48);

							if(riq == 3) {

								atoi(strncpy(time_set+9,"0",1));
							}
						}
							
					}
                    // 2�·�
					else {

						if((lParam - 48) > 2) {

								k--;
								j--;
								i--;
								m_pEditPwd->Input_get(lParam);
								break;
						}

						else 
							riq = (lParam - 48);
					}
						
				}
				
				if(k == 8) {

					if(atoi(strncpy(JG_TM+4,time_set+5,2)) == 1
						|| atoi(strncpy(JG_TM+4,time_set+5,2)) ==3 
						|| atoi(strncpy(JG_TM+4,time_set+5,2)) ==5
						|| atoi(strncpy(JG_TM+4,time_set+5,2)) == 7
						|| atoi(strncpy(JG_TM+4,time_set+5,2)) == 8
						|| atoi(strncpy(JG_TM+4,time_set+5,2)) == 10
						|| atoi(strncpy(JG_TM+4,time_set+5,2)) == 12) {

						if(riq == 3) {

							if((lParam - 48) > 1) {

								k--;
								j--;
								i--;
								m_pEditPwd->Input_get(lParam);
								break;
							}

						}

						if(riq == 0) {

							if((lParam - 48) == 0) {

								k--;
								j--;
								i--;
								m_pEditPwd->Input_get(lParam);
								break;
				
							}
						}
					}

					else {

						if(atoi(strncpy(JG_TM+4,time_set+5,2)) == 2) {

							if(riq == 2) {

								if((atoi(strncpy(JG_YEAR,time_set,4))%4) != 0) {

									if((lParam - 48) > 8) {

										k--;
										j--;
										i--;
										m_pEditPwd->Input_get(lParam);
										break;
									}
								}
							}
						}

						else {

							if(riq == 3) {

								if((lParam - 48) != 0) {
									
									k--;
									j--;
									i--;
									m_pEditPwd->Input_get(lParam);
									break;
								}
							}

							if(riq == 0) {

								if((lParam - 48) == 0) {

									k--;
									j--;
									i--;
									m_pEditPwd->Input_get(lParam);
									break;
					
								}
							}
						}
					}
				}
                // Сʱ
				if(k == 9) {

					if((lParam - 48) >2) {

						k--;
						j--;
						i--;
						m_pEditPwd->Input_get(lParam);
						break;
					}

					else {

						shi = (lParam - 48);

						if(shi == 2) {

							if(atoi(strncpy(dest+9,time_set+12,1)) > 3) {

								strncpy(time_set+12,"3",1);
							}
						}
					}
						
				}

				if(k == 10) {

					if(shi == 2) {

						if((lParam - 48) > 3) {
				
							k--;
							j--;
							i--;
							m_pEditPwd->Input_get(lParam);
							break;
						}
					}
				}

				// ����
				if(k == 11) {

					if((lParam - 48) > 5) {

						k--;
						j--;
						i--;
						m_pEditPwd->Input_get(lParam);
						break;
					}
				}
                // ����
				if(k == 13) {

					if((lParam - 48) > 5) {
					
						k--;
						j--;
						i--;
						m_pEditPwd->Input_get(lParam);
						break;
					}
				}
/* -------------------------------------------------------------------- */				
				m_pEditPwd->Input_settim(lParam);      //����ַ���
				if(k == 5 ||k == 7 ||k == 9 ||k== 11 ||k== 13) {

					j++;
					strncpy(time_set+(j-1),m_pEditPwd->GetString()+(i-1),1); 
				}

				else {
				
					strncpy(time_set+(j-1),m_pEditPwd->GetString()+(i-1),1);	
				}
               
			    if(k == 14) {

					 j = 0;
					 k = 0;

				} 
					
			    m_pEditPwd->Input_get(lParam);         //�����ַ���
		    }
			break;      
		}
		return TRUE;	
	}


	void ShowTip(char* buf)                            //��ʾ���������
	{
	    m_bShow = TRUE;
		m_pEditPwd->SetIsPwd(FALSE);
		m_pEditPwd->SetString(buf);
		m_pEditPwd->set_curpos();					   //�������Ϊ����һλ 
		m_pEditPwd->Show_Time(TRUE);                   //��ʾ����	
       	
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{	                                               //��ʵʱʱ����صı�������
		InitFrame("prj_setdate.xml");                  //���ڼ�ʱ�����ÿ��
 	    GetCtrlByName("back", &m_idBack);              //�õ����ذ����Ŀ���
		GetCtrlByName("ok", &m_idOK);                  //�õ�ȷ�ϼ��Ŀ���
		GetCtrlByName("delete", &m_idDelete);          //�õ�ɾ�����Ŀ���
		m_pEditPwd = (CEditBox *)GetCtrlByName("pwd", &m_idEditPwd);
		m_pLayOutTip = (CLayOut *)GetCtrlByName("tip_frame");             

		DPGetLocalTime(&tm);

		t_Year    = tm.wYear;
		t_Month   = tm.wMonth;
		t_Day     = tm.wDay;
		t_Hour    = tm.wHour;
		t_Minutes = tm.wMinute;
		t_Second  = tm.wSecond;
         
		sprintf(time_set,"%04d-%02d-%02d-%02d:%02d:%02d",t_Year, t_Month,t_Day
				  ,t_Hour,t_Minutes,t_Second);

		ShowTip(time_set);                           
 		m_bShow = TRUE;
		return TRUE;  
	}

private:
	DWORD m_idBack;                                    //���ذ������Ʊ���
	DWORD m_idOK;                                      //ȷ�ϼ����Ʊ���
	DWORD m_idDelete;                                  //ɾ�������Ʊ���
	DWORD m_idEditPwd;                                 //���ְ������Ʊ���
	BOOL m_bShow;
	CEditBox* m_pEditPwd;                              //���ְ���ָ����Ʊ���
	CLayOut* m_pLayOutTip;                             //��ʾ���Ʊ���   
	CTimeDate* m_pTime;                                //ʱ�Ӹ���
	SYSTEMTIME tm; 

	
};

CAppBase* CreatePrjSetDateApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjSetDateApp* pApp = new CPrjSetDateApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}  
 