#include "CCtrlModules.h"
//#include "Dpdef_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int INPUT_TIME = 0;                         // 时间输入标志位
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

char JG_TM[15];                             // 2018.4.23添加，用于输入时间判断 
char JG_YEAR[5];

char time_set[]="2017-02-01-00:00:00";      // 字符串数组初始化

/* 引用时间的实时数据 */
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
            if(wParam == m_idBack)                     //如果是点击返回按键，则返回上一级
			{
				i = 0;
				j = 0;
				k = 0;
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idOK)     {            //如果点击确认按键则执行如下操作

				INPUT_TIME = TRUE;                     //将标志位标记证明已经输入完毕

				TIME_FLAG = 1;                         //取消校准时间提示
				/* 将字符串数组转换成整数型数组并赋值给相应变量 */     
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

				memset(dest,0,15);                     //源地址与目标地址不重叠
                i = 0;
				j = 0;
				k = 0;
				DPPostMessage(MSG_START_APP, PROJECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

			else if(wParam == m_idDelete) {            //如果点击删除按键则执行如下操作
 		
				m_pEditPwd->Deletejmp();       		   //前移光标

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
		case KBD_MESSAGE:                              //键盘输入命令
			m_dwTimeout = 0;
			if(wParam == KBD_CTRL) {
            	
           		if(m_bShow) {
					
					m_pEditPwd->SetIsPwd(FALSE);       //显示数字
					m_pEditPwd->SetString("");
					m_bShow = FALSE;
				}
					
				  i ++;                                //键盘输入的字符串加1
				  k ++;                                //判断时间字符串的变量
				  j ++;                                //时间字符串加1

				//printf("the num is %d\n\r",lParam);
				
/*---------------------------------------------------------------------------------*/     
                // 这里限制年
				if(k == 1) {

					if((lParam - 48) != 2) {

						k--;
						j--;
						i--;
						m_pEditPwd->Input_get(lParam);
						break;
					}
						
				}


				// 月份
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

				// 日期
				if(k == 7) {

					// 除了2月份
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
                    // 2月份
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
                // 小时
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

				// 分钟
				if(k == 11) {

					if((lParam - 48) > 5) {

						k--;
						j--;
						i--;
						m_pEditPwd->Input_get(lParam);
						break;
					}
				}
                // 毫秒
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
				m_pEditPwd->Input_settim(lParam);      //获得字符串
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
					
			    m_pEditPwd->Input_get(lParam);         //载入字符串
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
	{	                                               //与实时时钟相关的变量定义
		InitFrame("prj_setdate.xml");                  //日期及时间设置框架
 	    GetCtrlByName("back", &m_idBack);              //得到返回按键的控制
		GetCtrlByName("ok", &m_idOK);                  //得到确认键的控制
		GetCtrlByName("delete", &m_idDelete);          //得到删除键的控制
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