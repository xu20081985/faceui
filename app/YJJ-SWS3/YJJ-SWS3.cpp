/*
#include <stdio.h>  
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h> */    

#include "roomlib.h"
#include "CAppBase.h"
#include "appdef.h"
#include "SmartConfig.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "CCtrlModules.h"
#include "SmartConfig.h"


#ifdef DPCE
#pragma comment(lib, "libjpeg.lib")
#pragma comment(lib,"freetype245.lib")
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "zlib.lib")
#endif

int g_hour,g_minute,g_second,Week;          // ʵʱʱ��
char g_time[9];
const SmartDev* pSmartDev;                  // ��ʱ�¼����õĵ�
int mainnn = 1;

time_t t1 = 0;                              // ����ʱ��ʱ��
time_t t2 = 0;                              // ��ǰʱ��
time_t t3;                                  // �����������˶೤ʱ��

struct timeval tBegin;                      // ���£�̧��ʱ���
struct timeval tEnd; 
int count_notouch;

int timer_touch_flag = 0;
long time_touch = 0;
extern int xydata[2];
extern BOOL curdown;
extern int Touch_Valid;
                           
BOOL TimerTouch = 0;                        // ��ʱ�¼�������־λ

extern Time_Link_List* g_TimeHead;          // ͷ������ݵ�ָ��
extern int TIME_FLAG;                       // 2018.1.26������ڶ�ʱʱ��У׼

static DWORD g_NUM_VALID = 0;               // ����һЩ��ļ���������

int time_mid = 0;                           // �����ʱ����ʱ����

//======================================================
//** ��������: WatchDogProcess
//** ��������: ���Ź�����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void WatchDogProcess()
{
	FeedWatchDog();                         // ��һ��ʱ�䣨1���ӣ�ιһ�ι���
	DWORD dwTotal, dwFree;
	DumpMemory(&dwTotal, &dwFree);  
	if(dwFree * 20 < dwTotal)	            // С��5%ʱ �Զ�����
	{
		DBGMSG(DPERROR, "dwMemoryLoad Send Reboot %lu %lu\r\n", dwFree, dwTotal);
		DPPostMessage(MSG_SYSTEM, REBOOT_MACH, 0, 0);
	}
}

static void test()
{
	#if 0
	static WORD cmd = 0x03;
	static DWORD count = 0;
	if (++count == 3) {
		cmd = (cmd == 0x03) ? 0x01: 0x03;
		SendSceneCmd(NULL, cmd, 0);
		count = 0;
	}
	#endif

	static int i = 0;
	
	AdjustScreen(10 * i, 50, 50);	
	if (i++ == 10)
		i = 0;
}

//======================================================
//** ��������: Send_Time_Ack
//** ��������: ����App
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void Send_Time_Ack()
{
	static int delay = 0;
	static int count = 0;
	
	if (!delay) {
		srand(time(NULL));
		delay = rand() % 60;
	}

	if (count++ > 1000)
		 count = 500;
	
	if (count == (30 + (delay % 5)) && TIME_FLAG == 0)
		Get_Local_Time();
	
	if (count == (120 + delay) && TIME_FLAG == 0)
		Get_Local_Time();
	
	if (count == (240 + delay) && TIME_FLAG == 0)
		Get_Local_Time();
}

//======================================================
//** ��������: CreateApp
//** ��������: ����App
//** �䡡��: wParam lParam zParam
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static CAppBase* CreateApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CAppBase* pApp = NULL;
	switch(wParam)                          //wParam�Ǵ���ID������
	{
	case MAIN_APPID:                        //���˵���
		pApp = CreateMainApp(wParam, lParam, zParam);
		break;      
	case PWD_INPUT_APPID:                   //����������档
		pApp = CreatePwdInputApp(wParam, lParam, zParam);
		break;
	case PROJECT_APPID:                     //�������ý���(�Ҹĵ�)
		pApp = CreateProjectApp(wParam, lParam, zParam);
		break;
	case CLOCK_APPID:                       //ʱ����ʾ����
		pApp = CreateClockApp(wParam, lParam, zParam);
		break;
	case MECHINE_SET:                       //��������(�Ҽӵ��ļ�)
		pApp = CreateMechineApp(wParam, lParam, zParam);
		break;
	case BKGD_SET:                          //��������(�Ҽӵ��ļ�)
		pApp = CreateBKApp(wParam, lParam, zParam);
		break;
/*	case TIP_APPID:
		pApp = CreateTipApp(wParam, lParam, zParam);
		break; */
	case LIGHT_APPID:                       //��ͨ�ƹ���ơ�
		pApp = CreateLightApp(wParam, lParam, zParam);
		break;
	case DIMMER_APPID:                      //������ơ�
		pApp = CreateDimmerApp(wParam, lParam, zParam);
		break;
	case CURTAIN_APPID:                     //�������ơ�
		pApp = CreateCurtainApp(wParam, lParam, zParam);
		break;
	case SCENE_APPID:                       //�龰����
		pApp = CreateSceneApp(wParam, lParam, zParam);
		break;
	case WINDOW_APPID:
		pApp = CreateWindowApp(wParam, lParam, zParam);
		break;
	case OUTLET_APPID:                      //��������
		pApp = CreateOutletApp(wParam, lParam, zParam);
		break;
	case AIRC_APPID:                        //�յ�����(���޸Ĺ�)
		pApp = CreateAirCApp(wParam, lParam, zParam);
		break;
	case HEAT_APPID:                        //��ů����(���޸Ĺ�)
		pApp = CreateHeatApp(wParam, lParam, zParam);
		break;
	case TV_APPID:                          //������ӿ���(����ӵ�)
		pApp = CreateTVApp(wParam, lParam, zParam);
		break;
	case MUSIC_APPID:                       //�������ֿ���(����ӵ�)
		pApp = CreateMUSICApp(wParam, lParam, zParam);
		break;
	case Ir_Air_APPID:                      //����յ�����(����ӵ�)
		pApp = CreateIr_AirApp(wParam, lParam, zParam);
		break;
	case NewWind_APPID:                //�·�
	    pApp = CreateNewWindApp(wParam, lParam, zParam);
	break;
 
	case PRJ_SET_DELAY_APPID:               //��ʱ���á�
		pApp = CreatePrjSetDelayApp(wParam, lParam, zParam);
		break;
	case PRJ_REPORT_APPID:                  //�ϱ�������Ϣ��
		pApp = CreatePrjReportApp(wParam, lParam, zParam);
		break;
	case PRJ_RESET_APPID:                   //�ָ��������á�
		pApp = CreatePrjResetApp(wParam, lParam, zParam);
		break;
	case PRJ_SET_DATE_APPID:                //�������á�(����ӵ�)
		pApp = CreatePrjSetDateApp(wParam, lParam, zParam);
		break;
	case PRJ_SET_PWD_APPID:                 //�������á�
		pApp = CreatePrjSetPwdApp(wParam, lParam, zParam);
		break;
	case PRJ_SET_TIMER_APPID:               //��ʱ���á�(����ӵ�)
		pApp = CreatePrjSetTimerApp(wParam, lParam, zParam);
		break;
	case PRJ_SET_UI_APPID:                  //�������á�(����ӵ�)
		pApp = CreatePrjSetUIApp(wParam, lParam, zParam);
		break;
	case PRJ_SHOW_APPID:                    //��ʾ����(����ӵ�)
		pApp = CreatePrjShow(wParam, lParam, zParam);
		break;
	case PRJ_TEMP_APPID:                    //�¶�У׼����(����ӵ�)
		pApp = CreatePrjSetTempApp(wParam, lParam, zParam);
		break;
#if 1
	case TIMER_SELECT_APPID:                //��ʱ���ñ༭����(�Ҽӵ�)
		pApp = CreateTimerSelectApp(wParam, lParam, zParam);
		break; 
	case TIMER_OBJECT_APPID:                //��ʱ���ñ༭����(�Ҽӵ�)               
		pApp = CreateTimerObjectApp(wParam, lParam, zParam);
		break; 
	case TIMER_TIME_APPID:                  //��ʱ����֮ʱ��༭(�Ҽӵ�)
		pApp = CreateTimerTimeApp(wParam, lParam, zParam);
		break; 
	case TIMER_WEEK_APPID:                  //��ʱ����֮�ظ�ʱ��༭(�Ҽӵ�)
		pApp = CreateTimerWeekApp(wParam, lParam, zParam);
		break; 
	case TIMER_DEVICE_APPID:                //��ʱ����֮�豸���Ʊ༭(�Ҽӵ�)
		pApp = CreateTimerDeviceApp(wParam, lParam, zParam);
		break; 
	case TIMER_ACTION_APPID:                //��ʱ����֮���Ʒ�ʽ�༭(�Ҽӵ�)
		pApp = CreateTimerActionApp(wParam, lParam, zParam);
		break;
	case PRJ_SET_TIMER_EDIT:                //��ʱ����֮��ʱ�����༭����(�Ҽӵ�)
		pApp = CreateTimerEditApp(wParam, lParam, zParam);
		break; 

#endif
	}

	if(pApp == NULL)
		DBGMSG(DPWARNING, "CreateApp %x %x %x Fail!\r\n", wParam, lParam, zParam);

	return pApp;
}

//======================================================
//** ��������: DispatchMessage
//** ��������: ������Ϣ
//** �䡡��: pMsg
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static BOOL DispatchMessage(SYS_MSG* pMsg)
{
	BOOL ret = FALSE;	
	
	DWORD result = DPGetMessage(pMsg);
	
//	count_notouch ++;

//	printf("end = %d begin = %d\n",tEnd.tv_usec,tBegin.tv_usec);			 	

	switch(result)          // �ж϶�������
	{
	case MSG_TIME_TYPE:
		WatchDogProcess();
		Send_Time_Ack();
		//test();
		ret = TRUE;
		break;
	case MSG_URGENT_TYPE:
	case MSG_USER_TYPE:
		ret = TRUE;
		break;
	case MSG_KEY_TYPE:
		KeyboardPreprocess(pMsg);
		break;
	case MSG_TOUCH_TYPE:
		count_notouch = 0;
		timer_touch_flag = 1;
		gettimeofday(&tBegin,NULL);   
		ret = TouchEventRemap(pMsg);
		break;
	default:
		break;
	}

#if 0
	if(result != MSG_TOUCH_TYPE ) {
		
		if(timer_touch_flag == 1) {

			timer_touch_flag = 0;
			Touch_Valid = 0;
			curdown = FALSE;
			DPPostMessage(TOUCH_RAW_MESSAGE, xydata[0], xydata[1], TOUCH_UP, MSG_TOUCH_TYPE);
//			DWORD result = DPGetMessage(pMsg);
		}
	} 
#endif

#if 0
	if(timer_touch_flag == 1) {

		gettimeofday(&tEnd,NULL);
		time_touch = tEnd.tv_usec - tBegin.tv_usec;
	}
#endif		

	return ret;
}

//======================================================
//** ��������: MsgProcess
//** ��������: ��Ϣ����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void MsgProcess(DWORD msg, DWORD wParam, DWORD lParam, DWORD zParam)
{
	static DWORD dwAppCount = 0;
	static CAppBase* pCurApp = NULL;
	static CAppBase* pAppStack[32] = {0};
	switch(msg)
	{
	case TIME_MESSAGE:
		pCurApp->DoProcess(msg, wParam, lParam, zParam);
		break;
	case TOUCH_RAW_MESSAGE:
	case TOUCH_MESSAGE:
	case TOUCH_SLIDE:
	case KBD_MESSAGE:
	case MSG_PRIVATE:
		pCurApp->DoProcess(msg, wParam, lParam, zParam);
		break;
	case MSG_BROADCAST:
		pCurApp->DoProcess(msg, wParam, lParam, zParam);
		break;
	case MSG_SYSTEM:
		switch(wParam)
		{
		case REBOOT_MACH:
		case UPDATE_NETCFG:
			DBGMSG(DPINFO, "REBOOT_MACH start\r\n");
			RebootSystem();
			break;
		case RESET_MACH:
			ResetSystemSet();
			DeinitFileServer();
			RebootSystem();
			break;
		case WATCHDOG_CHANGE:
#ifndef _DEBUG
			if(lParam)
				StartWatchDog();
			else
				StopWatchDog();
#endif
			break;
		}
		break;
	case MSG_START_APP:
		{
			DWORD tick = DPGetTickCount();    //ò������ҲӦ���Ǹ�������Ϣ��
			if(pCurApp)                       //�����Ǹ�ʲô�õģ�
			{
				if(!pCurApp->DoPause())
					break;
			}
			pCurApp = CreateApp(wParam, lParam, zParam);
			pAppStack[dwAppCount] = pCurApp;
			dwAppCount++;                     //�����������
			CAppBase::Show();                 //��ʾ

			CleanUserInput();                 //���յ�����Ϣ�������            
			DWORD dwTotal, dwFree;
			DumpMemory(&dwTotal, &dwFree);    //��ʾ��ص�ϵͳ��Ϣ
			DBGMSG(DPINFO, "MSG_START_APP %x cost %dms, %d %u %u\r\n", wParam, DPGetTickCount() - tick, dwFree*100/dwTotal, dwFree, dwTotal);
			DumpPhysicalMemory();              //������һ���պ�����
		}
		break;
	case MSG_END_APP:
		{
			for(int i = 0; i < dwAppCount; i++)
			{
				if((DWORD)pAppStack[i] == wParam)
				{
					DWORD tick = DPGetTickCount();
					pAppStack[i]->Destroy();
					delete pAppStack[i];
					memmove(&pAppStack[i], &pAppStack[i + 1], (dwAppCount - i - 1) * 4);
					dwAppCount--;

					DWORD dwTotal, dwFree;
					DumpMemory(&dwTotal, &dwFree);
					DBGMSG(DPINFO, "MSG_END_APP cost %dms, %d %u %u\r\n", DPGetTickCount() - tick, dwFree*100/dwTotal, dwFree, dwTotal);
					DumpPhysicalMemory();
					break;
				}
			}
		}
		break;
	case MSG_START_FROM_ROOT:
		{
			DWORD tick = DPGetTickCount();

			pCurApp = CreateApp(wParam, lParam, zParam);

			for(int i = 0; i < dwAppCount; i++)
			{
				pAppStack[i]->Destroy();
				delete pAppStack[i];
			}
			
			pAppStack[0] = pCurApp;
			dwAppCount = 1;
			CAppBase::Show();

			CleanUserInput();
			DWORD dwTotal, dwFree;
			DumpMemory(&dwTotal, &dwFree);
			DBGMSG(DPINFO, "MSG_START_FROM_ROOT %x cost %dms, %d %u %u\r\n", wParam, DPGetTickCount() - tick, dwFree*100/dwTotal, dwFree, dwTotal);
			DumpPhysicalMemory();
		}
		break;
	case MSG_EXTSTART_APP:
		DPPostMessage(MSG_START_FROM_ROOT, wParam, lParam, zParam, MSG_URGENT_TYPE);
		break;
	case MSG_START_FROM_OVER:
		{
			DWORD tick = DPGetTickCount();
			if(pCurApp)
			{
				pCurApp->PauseAck();
			}
			pCurApp = CreateApp(wParam, lParam, zParam);
			pAppStack[dwAppCount] = pCurApp;
			dwAppCount++;
			CAppBase::Show();          // ��Ӧ����һ�����õײ�ӿ���ʾ����     
			CleanUserInput();
			DWORD dwTotal, dwFree;
			DumpMemory(&dwTotal, &dwFree);
			DBGMSG(DPINFO, "MSG_START_FROM_OVER %x cost %dms, %d %u %u\r\n", wParam, DPGetTickCount() - tick, dwFree*100/dwTotal, dwFree, dwTotal);
			DumpPhysicalMemory();
		}
		break;
	case MSG_END_OVER_APP:
		for(int i = 0; i < dwAppCount; i++)
		{
			if((DWORD)pAppStack[i] == wParam)
			{
				DWORD tick = DPGetTickCount();

				pAppStack[i]->Destroy();
				delete pAppStack[i];
				memmove(&pAppStack[i], &pAppStack[i + 1], (dwAppCount - i - 1) * 4);
				dwAppCount--;

				// �ظ�֮ǰ�Ĵ���
				pCurApp = pAppStack[0];
				pCurApp->ResumeAck();
				CAppBase::Show();
				CleanUserInput();

				DWORD dwTotal, dwFree;
				DumpMemory(&dwTotal, &dwFree);
				DBGMSG(DPINFO, "MSG_END_OVER_APP cost %dms, %d %u %u\r\n", DPGetTickCount() - tick, dwFree*100/dwTotal, dwFree, dwTotal);
				DumpPhysicalMemory();
				break;
			}
		}
		break;
	}
}

//======================================================
//** ��������: InitSystem
//** ��������: ��ʼ��ϵͳ����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void InitSystem()
{
#ifndef _DEBUG
	// release�汾��������Ҫɨ�����
	ScanDisk();                      //��linux��ò��Ӧ�ò���Ҫ�ú�����
	PlayWav(KEYPAD_INDEX, 0);		 // ����һ���������ܿ���һ������������
#endif

	//SetSystemVolume(0xFFFFFFFF);

	// ��ʼ��Ϊ2017�� , ������Ӧ�ÿ��Ի��ϵͳ��׼ȷʱ�䡣
	SYSTEMTIME tm;
	DPGetLocalTime(&tm);            //��õ�ǰϵͳ��ʱ��
	tm.wYear = 2017;              
	DPSetLocalTime(&tm);
	StopLogo();

	// ��ʾ��������ʱ����״̬
	CheckSpareSpace(WINDOWSDIR);
	CheckSpareSpace(FLASHDIR);
	CheckSpareSpace(USERDIR);

	// ��ʾ�������ڴ�״̬
	DWORD mtotal, mfree;
	DumpMemory(&mtotal, &mfree);
	DBGMSG(DPINFO, "init memory %u%%%% free:%u total:%u\r\n", mfree*100/mtotal, mfree, mtotal);

	// ������������ʹ�õ��ڴ�ռ�
	SetObjectMemorySpace_GWF(1*1024*1024);

	// �������(ʲô�ǻ���?)
	DPBacktrace();
}

//======================================================
//** ��������: InitServer
//** ��������: ��ʱ������
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
static void InitServer()
{
	InitGb2Unicode();                           // ����Unicode�룬����Ļ����ʾ����
	LoadAllString("/FlashDev/str/chinese.txt");

	InitFileServer();	

	InitSystemSet();                            // ϵͳ�ļ��洢             	   
	InitTimerFILE();                            // ��ʱ�¼���Ϣ�洢��д

	//SetScreenOnOff(TRUE);                      // ��Ļ����

	InitSmartDev();                             // �Ҿ�������ݳ�ʼ��
	StartSmartServer();                         // ECB���ܼҾ�Э���߳�
	StartPCServer();                            // �����ʱ�ò���

#ifndef DEBUG
	StartWatchDog();
#endif
}

//======================================================
//** ��������: MSG_TIMR
//** ��������: ��ʱ������
//** �䡡��: pParam
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
void* MSG_TIMR(void* pParam)
{
				
	while(1) {

		pNode p_Mid;
		SYSTEMTIME tm;             // ʱ����ؽṹ��
		DPGetLocalTime(&tm);
		g_hour = tm.wHour;
		g_minute = tm.wMinute;
		Week = tm.wDayOfWeek;
		
		sprintf(g_time,"%02d:%02d",g_hour,g_minute);
		
		if(time_mid != g_minute) {

			if(NULL != g_TimeHead) {   //����ж�ʱ�¼�

				p_Mid = g_TimeHead;

				while(NULL != p_Mid) {

					time_mid = g_minute;
					
					if(p_Mid->show) {  //�ж϶�ʱ�¼��Ƿ񱻿���
						//printf("show is true\n");	
						if(strcmp(g_time,p_Mid->Time) == 0 && TIME_FLAG == 1) {  //�ж��Ƿ�ﵽʱ������
						//	printf("the time is same\n");						 //����ʱ����У׼	
							TimerTouch = 1;
						
							if(p_Mid->choose_data[6]) {

								FlashStatusTimer(p_Mid);
								
								if(p_Mid->ctr_mode == SCMD_SCENE) {

									SendSmartCmd(p_Mid->device, p_Mid->ctr_mode, htons(p_Mid->ctr_param));
									SetStatusByScene(htons(p_Mid->ctr_param));
									
								}
														
								else
									SendSmartCmd(p_Mid->device, p_Mid->ctr_mode, p_Mid->ctr_param);	

						//		sleep(1);
							}
							//�����ѡÿ��ѡ��
							else if(p_Mid->choose_data[7]) {         
										
								if(Week == 1 || Week == 2 || Week == 3
											 || Week == 4 || Week == 5
											 || Week == 6 || Week == 0) {
								
									FlashStatusTimer(p_Mid);

									if(p_Mid->ctr_mode == SCMD_SCENE) {

										SendSmartCmd(p_Mid->device, p_Mid->ctr_mode, htons(p_Mid->ctr_param));
										SetStatusByScene(htons(p_Mid->ctr_param));
									} 
										
								
									else
										SendSmartCmd(p_Mid->device, p_Mid->ctr_mode, p_Mid->ctr_param);	

						//			sleep(1);
								}
							}
							// �����ѡ������ѡ��
							else if(p_Mid->choose_data[8]) {

								if(Week == 1 || Week == 2 || Week == 3
											 || Week == 4 || Week == 5) {

									FlashStatusTimer(p_Mid);

									if(p_Mid->ctr_mode == SCMD_SCENE) {

										SendSmartCmd(p_Mid->device, p_Mid->ctr_mode,  htons(p_Mid->ctr_param));
										SetStatusByScene(htons(p_Mid->ctr_param));
									}
										
									else
										SendSmartCmd(p_Mid->device, p_Mid->ctr_mode, p_Mid->ctr_param);	

						//			sleep(1);
								}
							}
		                    // �����ѡ��ĩѡ��
							else if(p_Mid->choose_data[9]) {

								if(Week == 6 || Week == 0 ) {

									FlashStatusTimer(p_Mid);

									if(p_Mid->ctr_mode == SCMD_SCENE) {

										SendSmartCmd(p_Mid->device, p_Mid->ctr_mode,  htons(p_Mid->ctr_param));
										SetStatusByScene(htons(p_Mid->ctr_param));
									}
											
									else
										SendSmartCmd(p_Mid->device, p_Mid->ctr_mode, p_Mid->ctr_param);

						//			sleep(1);
								}
							}
							// �����ѡ����ѡ��	
							else if(p_Mid->choose_data[10]) {

								FlashStatusTimer(p_Mid);
								p_Mid->show = FALSE;
								UpdatSetTimer();

								if(p_Mid->ctr_mode == SCMD_SCENE) {

									SendSmartCmd(p_Mid->device, p_Mid->ctr_mode,  htons(p_Mid->ctr_param));
									SetStatusByScene(htons(p_Mid->ctr_param));
								}
										
								else
									SendSmartCmd(p_Mid->device, p_Mid->ctr_mode, p_Mid->ctr_param);

						//		sleep(1);
							}
							// �����ѡ����ѡ��
							else if(p_Mid->choose_data[11]) {

				
							}
							// �����������һ��������
							for(int i = 0;i<6;i++) {

								if(p_Mid->choose_data[i]) {

									if(Week == (i+1)) {
									
										FlashStatusTimer(p_Mid);

										if(p_Mid->ctr_mode == SCMD_SCENE) {

											SendSmartCmd(p_Mid->device, p_Mid->ctr_mode,  htons(p_Mid->ctr_param));
											SetStatusByScene(htons(p_Mid->ctr_param));
										}
								
										else
											SendSmartCmd(p_Mid->device, p_Mid->ctr_mode, p_Mid->ctr_param);

						//				sleep(1);
									}
								}
							}
						}
					}
					sleep(1);
					p_Mid = p_Mid->next;
				}
			}
		}
	                                                  
		else //û�ж�ʱ�¼�             
			//return NULL;
			{

		}
	}
}


//======================================================
//** ��������: main
//** ��������: ������
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��28��
//======================================================
int main()
{
	printf("\r\n\r\n========================>>Start<<========================\r\n\r\n");

	InitConfig();                // ò���Ǻ�UI�������Ӧ�Ķ���        
	InitSystem();                // ������Ժ�Ĺ�����ϵӦ�ò���
	InitSysMessage();            // ��ʼ�������������߳�
	InitServer();                // ���ܼҾ�Э���ʼ��              

	DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);
	SYS_MSG msg;

	while(1)
	{
		
		//Send_Time_Ack();

		//	MSG_TIMR();              //��ʱ�¼�ɨ��

		if(DispatchMessage(&msg))//�������ȡ����,���û����Ϣ���һֱִ��MsgProcess������
			MsgProcess(msg.msg, msg.wParam, msg.lParam, msg.zParam); //�������жϱ���.
	}
     
	return 0;
}    
