#include "roomlib.h"

#define	SETFILE0		"SystemSet0.ext"
#define	SETFILE1		"SystemSet1.ext"
#define SETTIME         "Timer.ext"
#define	SETENDID		0x55595301
extern int BK1;
extern int BK2;


/***********************ϵͳ��������*******************************/
typedef struct
{
	char	szPrjPwd[16];				// ��������
	char	szBakJpg[256];				// ����ͼƬ     
	BYTE	reserved[2040 - 240];		// 256
    int     Timer_Num;                  // ��ʱ�¼�����


	BOOL    flow[4];

	WORD    TV_Code[6];                 // ������ӵĺ������
	WORD    IR_Air_Code[7];             // ����յ��ĺ������
		
	DWORD	VERSION;					// 2040
	DWORD	Endid;						// 2044
} SystemSet_t;							// 2048

/*******************************************************************/



/********************��ʱ����Ҫд��Ľṹ��*************************/
/*
typedef struct LNode      
{
	char   Time[9];                    // �洢��ʱʱ��
   	char   p_Data[128];                // �ظ���ʱ�� 
   	BOOL   choose_data[8];             // �ظ����ڹ�ѡ   

	char   Device[10];                 // ���ƶ���
    BOOL   Device_Status; 

	char   CTL_Type[10];               // ���Ʒ�ʽ
	BOOL   CTL_OPEN;
	BOOL   CTL_CLOSE;

	int    NUM;                        // ��ʱ�¼������
	
    BOOL   show;                       // ��ʱ������/��ֹ��־
	
	struct LNode  *next;               // ����ָ��		
}Time_Link_List,*pNode;                // ��ʱ���ýṹ�����
*/
/******************************************************************/


static SystemSet_t* m_gSystemSet = NULL;       //�������ｫ��̬ȫ�ֱ���ȡ�����˱����ͨ����
static StaticLock g_SystemSetCS;
Time_Link_List* g_TimeHead = NULL;             //��ʱ�¼���ͷָ��

//======================================================
//** ��������: UpdataSystemSet
//** ��������: ����ϵͳ����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void UpdataSystemSet(void)              //ϵͳ����
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
 *����:������д���ļ�����
 */
//======================================================
//** ��������: UpdatSetTimer
//** ��������: �������ö�ʱ
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
void UpdatSetTimer(void)                //����ʱ��
{
   Time_Link_List* pset;
   char filename[64];                   
   FILE* fd;  
   sprintf(filename, "%s/%s", USERDIR, SETTIME);                                         
   pset = g_TimeHead;                        
   fd = fopen(filename, "wb");                 //ֻд�򿪻��½�һ���������ļ���ֻ����д����

   if(NULL == fd) {                            //���ļ�ʧ��              

	  return;
   }         
   
   while(NULL != pset) {

	  fwrite(pset,1,sizeof(Time_Link_List),fd);
	  pset = pset->next;                       //ָ�븳ֵ                    
   }

   fclose(fd);                                                      
}

//======================================================
//** ��������: InitDefaultSystemSet
//** ��������: ��ʼ��Ĭ��ϵͳ����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
void InitDefaultSystemSet(void)       //�ҽ����ﾲ̬������Ϊȫ�ֺ���
{
	memset(m_gSystemSet, 0, sizeof(SystemSet_t));
	strcpy(m_gSystemSet->szPrjPwd, "12345678");
	strcpy(m_gSystemSet->szBakJpg, "/FlashDev/wallpaper/3.jpg");

	for(int i = 0; i<6; i++) { // ��ʼ������������ֵ       
	
		m_gSystemSet->TV_Code[i] = 0xffff;
	}

	for(int j = 0; j<7; j++) {

		m_gSystemSet->IR_Air_Code[j] = 0xffff;
	}
   	m_gSystemSet->Timer_Num = 0;      //��ʼʱ�Ķ�ʱ�¼�����Ϊ0��
	m_gSystemSet->Endid = SETENDID;
}

/*
 *����:��ʼ����ʱ��������
 */
//======================================================
//** ��������: InitTimerFILE
//** ��������: ��ʼ����ʱ���б�
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void InitTimerFILE()                  //���ļ�����,��ʼ����ʱ���б�
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
	
	 if(filesize == 0){                                      //����ļ�Ϊ�����˳�

		fclose(fd);
	 	return;
	 }
 
   pset = (Time_Link_List*)malloc(sizeof(Time_Link_List));   //�����ڴ�
   
   if(NULL == pset) {                   

		return;
   }

   g_TimeHead = pset;                                        //ͷָ�벻�ܱ�

  while(1) {
   
	
	memset(pset, 0, sizeof(Time_Link_List));
	ret = fread(pset, 1, sizeof(Time_Link_List), fd);

   	

	if((ret == 0)) {

		free(pset);                                          //��ʧ���ͷ��ڴ�
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
//** ��������: InitSystemSet
//** ��������: ��ʼ��ϵͳ����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
			// �ļ����󣬱��浽�����·��
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
			// �ļ����󣬱��浽�����·��
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
//** ��������: ResetSystemSet
//** ��������: ��λϵͳ����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
void ResetSystemSet(void)
{
	g_SystemSetCS.lockon();
	InitDefaultSystemSet();		
	UpdataSystemSet();
	DeleteServerFile(SETFILE1); 
	InitSystemSet();             //���������ָ���������ʱ��������ʵʱ�任
	g_SystemSetCS.lockoff(); 
}

/*
 *����ԭ��:void GetPrjPwd(char* szPasswd)
 *��������:���ļ��л�ȡ�洢������
 *��ڲ���:char* szPasswd
 *���ڲ���:��
 */
//======================================================
//** ��������: GetPrjPwd
//** ��������: ��ȡϵͳ����
//** �䡡��: szPasswd
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
void GetPrjPwd(char* szPasswd)
{
	if(NULL == szPasswd)
		return;

	g_SystemSetCS.lockon();
	strcpy(szPasswd, m_gSystemSet->szPrjPwd);
	g_SystemSetCS.lockoff(); 
}



/*   2017.12.08 ��Ӷ�д�ļ��Ĳ���  */

/*
 *����ԭ��:void SetIR_TV_CODE(WORD Status,int i)
 *��������:��ECB�����ϻ�õ�����д�뵽�ļ�����
 *��ڲ���:WORD Status,int i
 *���ڲ���:��
 */

//======================================================
//** ��������: SetIR_TV_CODE
//** ��������: ���õ��Ӻ������
//** �䡡��: Status i
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
void SetIR_TV_CODE(WORD Status,int i)
{
	g_SystemSetCS.lockon();

	m_gSystemSet->TV_Code[i] = Status;
	UpdataSystemSet();           // ������д�뵽�ļ�����
	
	g_SystemSetCS.lockoff();
	
}

/*
 *����ԭ��: GetIR_TV_CODE(WORD Status,int i)
 *��������:��ú�����ӵĺ������
 *��ڲ���:i
 *���ڲ���:Status
 */
#if 1
//======================================================
//** ��������: GetIR_TV_CODE
//** ��������: ��ȡ���Ӻ������
//** �䡡��: Status i
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
 *����ԭ��: GetIR_AIR_CODE(WORD Status,int i)
 *��������:��ú���յ��ĺ������
 *��ڲ���:i
 *���ڲ���:Status
 */

//======================================================
//** ��������: GetIR_AIR_CODE
//** ��������: ��ȡ�յ��������
//** �䡡��: Status i
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================

WORD GetIR_AIR_CODE(WORD Status,int i)
{
	
	g_SystemSetCS.lockon();
	Status = m_gSystemSet->IR_Air_Code[i];
	g_SystemSetCS.lockoff();

	return Status;
}

/*
 *����ԭ��:SetIR_AIR_CODE(WORD Status,int i)
 *��������:���ECB�����Ϻ���յ����벢д�뵽�ļ�����
 *��ڲ���:i
 *���ڲ���:Status
 */
//======================================================
//** ��������: SetIR_AIR_CODE
//** ��������: ���ÿյ��������
//** �䡡��: Status i
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================

void SetIR_AIR_CODE(WORD Status,int i)
{
	g_SystemSetCS.lockon();

	m_gSystemSet->IR_Air_Code[i] = Status;
	UpdataSystemSet();           // ������д�뵽�ļ�����
	
	g_SystemSetCS.lockoff();

}

/**********************************/


/*
 *����: ��ö�ʱ�¼��Ĵ���
 */
//======================================================
//** ��������: GetTimercount
//** ��������: ��ȡ��ʱ����
//** �䡡��: szTimer
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void GetTimercount(int szTimer)
{
	g_SystemSetCS.lockon();
	szTimer = m_gSystemSet->Timer_Num;
	g_SystemSetCS.lockoff(); 
}

/*
 *����: ����ʱ�¼��ĸ���д�뵽�����ļ�����ȥ
 */
//======================================================
//** ��������: SetTimercount
//** ��������: ���ö�ʱ����
//** �䡡��: szTimer
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//====================================================== 
void SetTimercount(int szTimer)
{
	g_SystemSetCS.lockon();
	m_gSystemSet->Timer_Num = szTimer;
	UpdataSystemSet(); 
	g_SystemSetCS.lockoff(); 
}

/*
 *����ԭ��:void SetPrjPwd(char* szPasswd)
 *����:�������õ�����д�뵽ϵͳ�ļ�����
 *��ڲ���:char* szPasswd
 *���ڲ���:��
 */

//======================================================
//** ��������: SetPrjPwd
//** ��������: ����ϵͳ����
//** �䡡��: szPasswd
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
//** ��������: SetPrjShow
//** ��������: ��������ʱ��
//** �䡡��: show
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
//** ��������: GetPrjShow
//** ��������: ��ȡ����ʱ��
//** �䡡��: show
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
//** ��������: SetPrjbkp
//** ��������: ��������ͼƬ
//** �䡡��: szbkp
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
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
//** ��������: SetScreenOnOff
//** ��������: ������Ļ����
//** �䡡��: bOn
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
BOOL SetScreenOnOff(BOOL bOn)
{
	DWORD bright = 100;
	if(!bOn)
		bright = 0;

	return AdjustScreen(bright, 100, 100);
}

//======================================================
//** ��������: GetDelay
//** ��������: ����Ĭ����ʱ60s
//** �䡡��: type
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
DWORD GetDelay(DWORD type)
{
		return 60;	
}
