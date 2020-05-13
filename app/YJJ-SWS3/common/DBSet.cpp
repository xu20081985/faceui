#include "roomlib.h"

#define	SETFILE0		"SystemSet0.ext"
#define	SETFILE1		"SystemSet1.ext"
#define SETTIME         "Timer.ext"
#define	SETENDID		0x55595301
extern int BK1;
extern int BK2;


/***********************系统配置设置*******************************/
typedef struct
{
	char	szPrjPwd[16];				// 管理密码
	char	szBakJpg[256];				// 背景图片     
	BYTE	reserved[2040 - 240];		// 256
    int     Timer_Num;                  // 定时事件个数


	BOOL    flow[4];

	WORD    TV_Code[6];                 // 红外电视的红外编码
	WORD    IR_Air_Code[7];             // 红外空调的红外编码
		
	DWORD	VERSION;					// 2040
	DWORD	Endid;						// 2044
} SystemSet_t;							// 2048

/*******************************************************************/



/********************定时设置要写入的结构体*************************/
/*
typedef struct LNode      
{
	char   Time[9];                    // 存储定时时间
   	char   p_Data[128];                // 重复的时间 
   	BOOL   choose_data[8];             // 重复日期勾选   

	char   Device[10];                 // 控制对象
    BOOL   Device_Status; 

	char   CTL_Type[10];               // 控制方式
	BOOL   CTL_OPEN;
	BOOL   CTL_CLOSE;

	int    NUM;                        // 定时事件的序号
	
    BOOL   show;                       // 定时器开启/禁止标志
	
	struct LNode  *next;               // 链表指针		
}Time_Link_List,*pNode;                // 定时设置结构体变量
*/
/******************************************************************/


static SystemSet_t* m_gSystemSet = NULL;       //我在这里将静态全局变量取消掉了变成普通变量
static StaticLock g_SystemSetCS;
Time_Link_List* g_TimeHead = NULL;             //定时事件的头指针

//======================================================
//** 函数名称: UpdataSystemSet
//** 功能描述: 更新系统设置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void UpdataSystemSet(void)              //系统设置
{
	SystemSet_t* pSet;
	BOOL ret;

	pSet = (SystemSet_t*)malloc(sizeof(SystemSet_t));
	if(NULL == pSet)
	{
		DBGMSG(DPERROR, "UpdataSystemSet malloc fail\r\n");
		return;
	}

	memcpy(pSet, m_gSystemSet, sizeof(SystemSet_t));
	if(m_gSystemSet->VERSION & 1)
		ret = WriteServerFile(SETFILE1, sizeof(SystemSet_t), (char*)pSet);
	else
		ret = WriteServerFile(SETFILE0, sizeof(SystemSet_t), (char*)pSet);
	if(!ret)
		free(pSet);
	else
	{
		m_gSystemSet->VERSION++;
	}
}

/*
 *功能:将数据写入文件当中
 */
//======================================================
//** 函数名称: UpdatSetTimer
//** 功能描述: 更新设置定时
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void UpdatSetTimer(void)                //保存时间
{
   Time_Link_List* pset;
   char filename[64];                   
   FILE* fd;  
   sprintf(filename, "%s/%s", USERDIR, SETTIME);                                         
   pset = g_TimeHead;                        
   fd = fopen(filename, "wb");                 //只写打开或新建一个二进制文件，只允许写数据

   if(NULL == fd) {                            //打开文件失败              

	  return;
   }         
   
   while(NULL != pset) {

	  fwrite(pset,1,sizeof(Time_Link_List),fd);
	  pset = pset->next;                       //指针赋值                    
   }

   fclose(fd);                                                      
}

//======================================================
//** 函数名称: InitDefaultSystemSet
//** 功能描述: 初始化默认系统配置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void InitDefaultSystemSet(void)       //我将这里静态函数改为全局函数
{
	memset(m_gSystemSet, 0, sizeof(SystemSet_t));
	strcpy(m_gSystemSet->szPrjPwd, "12345678");
	strcpy(m_gSystemSet->szBakJpg, "/FlashDev/wallpaper/3.jpg");

	for(int i = 0; i<6; i++) { // 初始化红外编码的数值       
	
		m_gSystemSet->TV_Code[i] = 0xffff;
	}

	for(int j = 0; j<7; j++) {

		m_gSystemSet->IR_Air_Code[j] = 0xffff;
	}
   	m_gSystemSet->Timer_Num = 0;      //开始时的定时事件设置为0个
	m_gSystemSet->Endid = SETENDID;
}

/*
 *功能:初始化定时器的数据
 */
//======================================================
//** 函数名称: InitTimerFILE
//** 功能描述: 初始化定时器列表
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void InitTimerFILE()                  //读文件操作,初始化定时器列表
{
   Time_Link_List* pset;
   Time_Link_List* pMid;
   char filename[64];
   struct stat statbuff;
   unsigned long filesize = -1;
   FILE* fd;  
   sprintf(filename, "%s/%s", USERDIR, SETTIME); 
   int ret;
    
   fd = fopen(filename,"ab+");    
  
   
   if(NULL == fd) {                   

		return;
   }
  
	 
	if(stat(filename,&statbuff) == 0)                  

		filesize = statbuff.st_size;
	
	 if(filesize == 0){                                      //如果文件为空则退出

		fclose(fd);
	 	return;
	 }
 
   pset = (Time_Link_List*)malloc(sizeof(Time_Link_List));   //分配内存
   
   if(NULL == pset) {                   

		return;
   }

   g_TimeHead = pset;                                        //头指针不能变

  while(1) {
   
	
	memset(pset, 0, sizeof(Time_Link_List));
	ret = fread(pset, 1, sizeof(Time_Link_List), fd);

   	

	if((ret == 0)) {

		free(pset);                                          //读失败释放内存
		pset = NULL;
		pMid->next = NULL;
		DBGMSG(DPINFO, "222\r\n");
		break;
	}      		

	pMid = pset;                                                

	if(feof(fd) == 0) {

		pset->next = (Time_Link_List*)malloc(sizeof(Time_Link_List));			
		pset = pset->next;
		DBGMSG(DPINFO, "111\r\n");
	}

	else {
			break;
	 }                                                		
  }
    fclose(fd);
}

//======================================================
//** 函数名称: InitSystemSet
//** 功能描述: 初始化系统设置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void InitSystemSet(void)
{
	FILE* fd;
	char filename[64];
	SystemSet_t* pSet0 = NULL;
	SystemSet_t* pSet1 = NULL;
    
	g_SystemSetCS.lockon();
	sprintf(filename, "%s/%s", USERDIR, SETFILE0);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet0 = (SystemSet_t*)malloc(sizeof(SystemSet_t));
		if(pSet0)
		{
			memset(pSet0, 0, sizeof(SystemSet_t));
			int ret = fread(pSet0, 1, sizeof(SystemSet_t), fd);
			if(ret != sizeof(SystemSet_t))
			{
				free(pSet0);
				pSet0 = NULL;
			}
			else
			{
				if(pSet0->Endid != SETENDID)
				{
					free(pSet0);
					pSet0 = NULL;
				}
			}
		}
		fclose(fd);

		if(pSet0 == NULL)
		{
			// 文件错误，保存到另外的路径
			char bakFile[MAX_PATH];
			sprintf(bakFile, "%s.bak", filename);
			DPMoveFile(bakFile, filename);
		}
	}

	sprintf(filename, "%s/%s", USERDIR, SETFILE1);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		pSet1 = (SystemSet_t*)malloc(sizeof(SystemSet_t));
		if(pSet1)
		{
			memset(pSet1, 0, sizeof(SystemSet_t));
			int ret = fread(pSet1, 1, sizeof(SystemSet_t), fd);
			if(ret != sizeof(SystemSet_t))
			{
				free(pSet1);
				pSet1 = NULL;
			}
			else
			{
				if(pSet1->Endid != SETENDID)
				{
					free(pSet1);
					pSet1 = NULL;
				}
			}
		}
		fclose(fd);

		if(pSet1 == NULL)
		{
			// 文件错误，保存到另外的路径
			char bakFile[MAX_PATH];
			sprintf(bakFile, "%s.bak", filename);
			DPMoveFile(bakFile, filename);
		}
	}

	if((pSet0 == NULL) && (pSet1 == NULL))
	{
		m_gSystemSet = (SystemSet_t*)malloc(sizeof(SystemSet_t));
		if(m_gSystemSet)
		{
			InitDefaultSystemSet();
			UpdataSystemSet();
		}
	}
	else
	{
		if((pSet0 != NULL) && (pSet1 != NULL))
		{
			if(pSet0->VERSION > pSet1->VERSION)
			{
				m_gSystemSet = pSet0;
				free(pSet1);
			}
			else
			{
				m_gSystemSet = pSet1;
				free(pSet0);
			}
		}
		else if(pSet0 != NULL)
			m_gSystemSet = pSet0;
		else
			m_gSystemSet = pSet1;
		m_gSystemSet->VERSION++;
	}

	if(-1 == DPGetFileAttributes(m_gSystemSet->szBakJpg))
	{
		strcpy(m_gSystemSet->szBakJpg, "/FlashDev/wallpaper/3.jpg");
		UpdataSystemSet();
	}

	DPDeleteFile(DEFAULT_BK_JPG);
	DPCopyFile(DEFAULT_BK_JPG, m_gSystemSet->szBakJpg);

	g_SystemSetCS.lockoff();
}

//======================================================
//** 函数名称: ResetSystemSet
//** 功能描述: 复位系统配置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void ResetSystemSet(void)
{
	g_SystemSetCS.lockon();
	InitDefaultSystemSet();		
	UpdataSystemSet();
	DeleteServerFile(SETFILE1); 
	InitSystemSet();             //这里用作恢复出厂设置时，背景的实时变换
	g_SystemSetCS.lockoff(); 
}

/*
 *函数原型:void GetPrjPwd(char* szPasswd)
 *函数功能:从文件中获取存储的密码
 *入口参数:char* szPasswd
 *出口参数:无
 */
//======================================================
//** 函数名称: GetPrjPwd
//** 功能描述: 获取系统密码
//** 输　入: szPasswd
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void GetPrjPwd(char* szPasswd)
{
	if(NULL == szPasswd)
		return;

	g_SystemSetCS.lockon();
	strcpy(szPasswd, m_gSystemSet->szPrjPwd);
	g_SystemSetCS.lockoff(); 
}



/*   2017.12.08 添加读写文件的操作  */

/*
 *函数原型:void SetIR_TV_CODE(WORD Status,int i)
 *函数功能:将ECB总线上获得的数据写入到文件当中
 *入口参数:WORD Status,int i
 *出口参数:无
 */

//======================================================
//** 函数名称: SetIR_TV_CODE
//** 功能描述: 设置电视红外编码
//** 输　入: Status i
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void SetIR_TV_CODE(WORD Status,int i)
{
	g_SystemSetCS.lockon();

	m_gSystemSet->TV_Code[i] = Status;
	UpdataSystemSet();           // 将数据写入到文件当中
	
	g_SystemSetCS.lockoff();
	
}

/*
 *函数原型: GetIR_TV_CODE(WORD Status,int i)
 *函数功能:获得红外电视的红外编码
 *入口参数:i
 *出口参数:Status
 */
#if 1
//======================================================
//** 函数名称: GetIR_TV_CODE
//** 功能描述: 获取电视红外编码
//** 输　入: Status i
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
WORD GetIR_TV_CODE(WORD Status,int i)
{
	
	g_SystemSetCS.lockon();
	Status = m_gSystemSet->TV_Code[i];
	g_SystemSetCS.lockoff();

	return Status;
}
#endif

#if 0
WORD GetIR_TV_CODE(WORD *Status,int i)
{
	g_SystemSetCS.lockon();
	Status = &m_gSystemSet->TV_Code[i];
	g_SystemSetCS.lockoff();

}	
#endif

/*
 *函数原型: GetIR_AIR_CODE(WORD Status,int i)
 *函数功能:获得红外空调的红外编码
 *入口参数:i
 *出口参数:Status
 */

//======================================================
//** 函数名称: GetIR_AIR_CODE
//** 功能描述: 获取空调红外编码
//** 输　入: Status i
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================

WORD GetIR_AIR_CODE(WORD Status,int i)
{
	
	g_SystemSetCS.lockon();
	Status = m_gSystemSet->IR_Air_Code[i];
	g_SystemSetCS.lockoff();

	return Status;
}

/*
 *函数原型:SetIR_AIR_CODE(WORD Status,int i)
 *函数功能:获得ECB总线上红外空调编码并写入到文件当中
 *入口参数:i
 *出口参数:Status
 */
//======================================================
//** 函数名称: SetIR_AIR_CODE
//** 功能描述: 设置空调红外编码
//** 输　入: Status i
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================

void SetIR_AIR_CODE(WORD Status,int i)
{
	g_SystemSetCS.lockon();

	m_gSystemSet->IR_Air_Code[i] = Status;
	UpdataSystemSet();           // 将数据写入到文件当中
	
	g_SystemSetCS.lockoff();

}

/**********************************/


/*
 *功能: 获得定时事件的次数
 */
//======================================================
//** 函数名称: GetTimercount
//** 功能描述: 获取定时条数
//** 输　入: szTimer
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void GetTimercount(int szTimer)
{
	g_SystemSetCS.lockon();
	szTimer = m_gSystemSet->Timer_Num;
	g_SystemSetCS.lockoff(); 
}

/*
 *功能: 将定时事件的个数写入到设置文件当中去
 */
//======================================================
//** 函数名称: SetTimercount
//** 功能描述: 设置定时条数
//** 输　入: szTimer
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void SetTimercount(int szTimer)
{
	g_SystemSetCS.lockon();
	m_gSystemSet->Timer_Num = szTimer;
	UpdataSystemSet(); 
	g_SystemSetCS.lockoff(); 
}

/*
 *函数原型:void SetPrjPwd(char* szPasswd)
 *功能:将新设置的密码写入到系统文件当中
 *入口参数:char* szPasswd
 *出口参数:无
 */

//======================================================
//** 函数名称: SetPrjPwd
//** 功能描述: 设置系统密码
//** 输　入: szPasswd
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void SetPrjPwd(char* szPasswd)
{
	if(NULL == szPasswd)
		return;

	g_SystemSetCS.lockon();
	strcpy(m_gSystemSet->szPrjPwd, szPasswd);
	UpdataSystemSet();                         
	g_SystemSetCS.lockoff(); 
}

//======================================================
//** 函数名称: SetPrjShow
//** 功能描述: 设置屏保时间
//** 输　入: show
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void SetPrjShow(BOOL* show) 
{
	g_SystemSetCS.lockon();
	m_gSystemSet->flow[0] = show[0];
	m_gSystemSet->flow[1] = show[1];
	m_gSystemSet->flow[2] = show[2];
	m_gSystemSet->flow[3] = show[3];
	UpdataSystemSet(); 	
	g_SystemSetCS.lockoff();
}

//======================================================
//** 函数名称: GetPrjShow
//** 功能描述: 获取屏保时间
//** 输　入: show
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void GetPrjShow(BOOL* show)
{
	g_SystemSetCS.lockon();
	show[0] = m_gSystemSet->flow[0];
	show[1] = m_gSystemSet->flow[1];
	show[2] = m_gSystemSet->flow[2];
	show[3] = m_gSystemSet->flow[3];
	g_SystemSetCS.lockoff();
}

//======================================================
//** 函数名称: SetPrjbkp
//** 功能描述: 设置屏保图片
//** 输　入: szbkp
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void SetPrjbkp(char* szbkp)
{
	if(NULL == szbkp)
		return;

	g_SystemSetCS.lockon();
	strcpy(m_gSystemSet->szBakJpg, szbkp);
	UpdataSystemSet();                         
	g_SystemSetCS.lockoff(); 
}

DWORD GetRingVol(void)
{
	return 0xFFFFFFFF;
}

//======================================================
//** 函数名称: SetScreenOnOff
//** 功能描述: 设置屏幕亮暗
//** 输　入: bOn
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
BOOL SetScreenOnOff(BOOL bOn)
{
	DWORD bright = 100;
	if(!bOn)
		bright = 0;

	return AdjustScreen(bright, 100, 100);
}

//======================================================
//** 函数名称: GetDelay
//** 功能描述: 屏保默认延时60s
//** 输　入: type
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
DWORD GetDelay(DWORD type)
{
		return 60;	
}
