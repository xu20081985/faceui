#include "CCtrlModules.h"
#include <string.h>

int  timer;                             // 创建节点的个数
char dstime[] = "00:00";                // 中间变量
char DW[] = "";                         // 空字符串
extern Time_Link_List* g_TimeHead;      // 头结点数据的指针
extern char *g_pData[8];      
extern BOOL Flag[8];
extern BOOL Light_select;
extern char* p_Light;

extern char *p_action;
extern BOOL Select_Open;
extern BOOL Select_Close;

class CTimerObjectApp : public CAppBase
{
public:
	CTimerObjectApp(DWORD hWnd) : CAppBase(hWnd)
	{
	}

	~CTimerObjectApp()
	{
	}

	/*
	 *功能:创建一个新的节点
	 */
	void Creat_Node()
	{
		int i = 0,j = 0,k = 0;
		pNode P_Head = (pNode)malloc(sizeof(LNode )); //为头节点分配内存空间，把申请到的内存当作新节点
		pNode P_Mid;								  //为中间变量
	
		if(NULL == P_Head) {						  //若为空指针则分配内存失败
	
			return ;
		}
        memset(P_Head, 0, sizeof(LNode ) );           //清理申请到的对内存
        
		
	/*************进行数据赋值操作,填充有效数据***************/ 
		P_Head->next = NULL;						  //其下一指针赋值为空(作为尾节点),将来指向下一节点首地址
		
        P_Head->show = TRUE;                          //定时器默认被开启
        
        strcpy(P_Head->Time, dstime);                 //时间字符串赋值


		for(i=0; i<8; i++) {                          //重复日期字符串赋值
			
			if(NULL != g_pData[i]) {
          
               j++;
			   strcat(P_Head->p_Data, g_pData[i]);     
			}		   
		}

		for(k = 0; k<8; k++) {                        //重复日期勾选标记

			P_Head->choose_data[k] = Flag[k];
		}
        /* 控制设备字符串赋值 */
		if(NULL !=p_Light)                            
			strcpy(P_Head->Device, p_Light);        
		else
		 strcpy(P_Head->Device, DW);
		 	
		 P_Head->Device_Status = Light_select;        //控制设备勾选标记
		 /* 设备控制方式赋值 */
		 if(NULL != p_action)                         
			 strcpy(P_Head->CTL_Type, p_action);
		 else 
		 	 strcpy(P_Head->CTL_Type, DW);
		 
		 P_Head->CTL_OPEN  = Select_Open;             //控制方式勾选项
		 P_Head->CTL_CLOSE = Select_Close;
		 
	/**********************************************************/
	
		if(NULL == g_TimeHead) {					  //节点为头结点(证明在此之前没有定时事件)
	
			g_TimeHead = P_Head;					  //建立第一个定时事件
				
		}
	
		else {										  //不是第一个定时事件
	
			P_Mid = g_TimeHead;
	
			while(NULL != P_Mid->next) {
	
				P_Mid = P_Mid->next;		
			}
	
			P_Mid->next = P_Head;			 
		}
       
		UpdatSetTimer();                               //将数据写入文件当中
			 
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
			if(wParam == m_idCancel)
			{   //取消按键
				DPPostMessage(MSG_START_APP, TIMER_SELECT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			

			else if(wParam == m_idSave)
			{   //保存按键 在此添加功能:创建链表,并将数据写到文件当中
              
			   GetTimercount(timer);       //此时获得定时事件的个数
			   timer += 1;                 //定时事件保存后加1
			   SetTimercount(timer);       //将定时事件的个数设置后重新写入文件当中

			   if(timer <= 16)	
			   	  Creat_Node();            //创建一个节点
               
			   DPPostMessage(MSG_START_APP, TIMER_SELECT_APPID, 0, 0);
			   DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				
			}

          /*********************定时事件时钟编辑************************/
			else if(wParam == m_idTime)
			{   //时间编辑处
				DPPostMessage(MSG_START_APP, TIMER_TIME_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}

		  /*******************定时事件重复时间编辑*********************/
			else if(wParam == m_idDate)
			{   //重复编辑处
				DPPostMessage(MSG_START_APP, TIMER_WEEK_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}		 

		   /*******************定时事件设备控制编辑*********************/
			else if(wParam == m_idObject)
			{   //控制对象
				DPPostMessage(MSG_START_APP, TIMER_DEVICE_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
		   	
           /*******************定时事件控制状态编辑*********************/
			else if(wParam == m_idAction)
			{   
				DPPostMessage(MSG_START_APP, TIMER_ACTION_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
		   
			break;
		}
		return TRUE;	
	}


	void OnCreate()
	{		
		char buf[128];
        char buf1[20];
        int i;
	  /**************显示定时时间**************/
		sprintf(buf, "%s", dstime);      //显示时间
		m_pTime->SetSrc(buf);
		m_pTime->Show(TRUE);
    

	  /**************显示重复时间**************/
		memset(buf,0,sizeof(buf));
		for(i =0; i<8; i++)	{

			if(NULL != g_pData[i])
				strcat(buf,g_pData[i]);			
		}			
		m_pDate->SetSrc(buf);
		m_pDate->Show(TRUE);


	  /*************显示控制设备**************/
	  	memset(buf,0,sizeof(buf));

		if(NULL !=  p_Light)
			strcpy(buf,p_Light);
		
		m_pObject->SetSrc(buf);
		m_pObject->Show(TRUE);
		

	  /**************显示控制方式**************/
		memset(buf,0,sizeof(buf));
	  
		if(NULL != p_action)
	  		strcpy(buf,p_action);
		else
			strcpy(buf,DW);
					
		m_pAction->SetSrc(buf);
		m_pAction->Show(TRUE);
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("timer_object.xml");
		GetCtrlByName("save", &m_idSave);
		GetCtrlByName("cancel", &m_idCancel);
		GetCtrlByName("btn_time", &m_idTime);
		GetCtrlByName("btn_date", &m_idDate);
		GetCtrlByName("btn_object", &m_idObject);
		GetCtrlByName("btn_action", &m_idAction);
		m_pTime = (CDPStatic *)GetCtrlByName("time");
		m_pDate = (CDPStatic *)GetCtrlByName("date");
		m_pObject = (CDPStatic *)GetCtrlByName("object");
		m_pAction = (CDPStatic *)GetCtrlByName("action");

		OnCreate();
		return TRUE;
	}

private:
	DWORD m_idSave;
	DWORD m_idTime;
	DWORD m_idDate;
	DWORD m_idCancel;
	DWORD m_idObject;
	DWORD m_idAction;

	CDPStatic* m_pTime;
	CDPStatic* m_pDate;
	CDPStatic* m_pObject;
	CDPStatic* m_pAction;

	
};

CAppBase* CreateTimerObjectApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CTimerObjectApp* pApp = new CTimerObjectApp(wParam);
	if(!pApp->Create(lParam, zParam))
	{
		delete pApp;
		pApp = NULL;		
	}
	return pApp;
}
 