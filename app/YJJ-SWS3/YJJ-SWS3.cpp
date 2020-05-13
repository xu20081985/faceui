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

int g_hour,g_minute,g_second,Week;          // 实时时间
char g_time[9];
const SmartDev* pSmartDev;                  // 定时事件中用的到
int mainnn = 1;

time_t t1 = 0;                              // 开机时的时间
time_t t2 = 0;                              // 当前时间
time_t t3;                                  // 开机后运行了多长时间

struct timeval tBegin;                      // 按下，抬起时间戳
struct timeval tEnd; 
int count_notouch;

int timer_touch_flag = 0;
long time_touch = 0;
extern int xydata[2];
extern BOOL curdown;
extern int Touch_Valid;
                           
BOOL TimerTouch = 0;                        // 定时事件触发标志位

extern Time_Link_List* g_TimeHead;          // 头结点数据的指针
extern int TIME_FLAG;                       // 2018.1.26添加用于定时时间校准

static DWORD g_NUM_VALID = 0;               // 舍弃一些点的计数参数。

int time_mid = 0;                           // 解决定时的延时问题

//======================================================
//** 函数名称: WatchDogProcess
//** 功能描述: 看门狗处理
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void WatchDogProcess()
{
	FeedWatchDog();                         // 隔一段时间（1秒钟）喂一次狗。
	DWORD dwTotal, dwFree;
	DumpMemory(&dwTotal, &dwFree);  
	if(dwFree * 20 < dwTotal)	            // 小于5%时 自动重启
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
//** 函数名称: Send_Time_Ack
//** 功能描述: 创建App
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
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
//** 函数名称: CreateApp
//** 功能描述: 创建App
//** 输　入: wParam lParam zParam
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static CAppBase* CreateApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CAppBase* pApp = NULL;
	switch(wParam)                          //wParam是窗口ID参数。
	{
	case MAIN_APPID:                        //主菜单。
		pApp = CreateMainApp(wParam, lParam, zParam);
		break;      
	case PWD_INPUT_APPID:                   //密码输入界面。
		pApp = CreatePwdInputApp(wParam, lParam, zParam);
		break;
	case PROJECT_APPID:                     //本机设置界面(我改的)
		pApp = CreateProjectApp(wParam, lParam, zParam);
		break;
	case CLOCK_APPID:                       //时钟显示界面
		pApp = CreateClockApp(wParam, lParam, zParam);
		break;
	case MECHINE_SET:                       //本机设置(我加的文件)
		pApp = CreateMechineApp(wParam, lParam, zParam);
		break;
	case BKGD_SET:                          //背景设置(我加的文件)
		pApp = CreateBKApp(wParam, lParam, zParam);
		break;
/*	case TIP_APPID:
		pApp = CreateTipApp(wParam, lParam, zParam);
		break; */
	case LIGHT_APPID:                       //普通灯光控制。
		pApp = CreateLightApp(wParam, lParam, zParam);
		break;
	case DIMMER_APPID:                      //调光控制。
		pApp = CreateDimmerApp(wParam, lParam, zParam);
		break;
	case CURTAIN_APPID:                     //窗帘控制。
		pApp = CreateCurtainApp(wParam, lParam, zParam);
		break;
	case SCENE_APPID:                       //情景控制
		pApp = CreateSceneApp(wParam, lParam, zParam);
		break;
	case WINDOW_APPID:
		pApp = CreateWindowApp(wParam, lParam, zParam);
		break;
	case OUTLET_APPID:                      //插座控制
		pApp = CreateOutletApp(wParam, lParam, zParam);
		break;
	case AIRC_APPID:                        //空调控制(我修改过)
		pApp = CreateAirCApp(wParam, lParam, zParam);
		break;
	case HEAT_APPID:                        //地暖控制(我修改过)
		pApp = CreateHeatApp(wParam, lParam, zParam);
		break;
	case TV_APPID:                          //红外电视控制(我添加的)
		pApp = CreateTVApp(wParam, lParam, zParam);
		break;
	case MUSIC_APPID:                       //背景音乐控制(我添加的)
		pApp = CreateMUSICApp(wParam, lParam, zParam);
		break;
	case Ir_Air_APPID:                      //红外空调控制(我添加的)
		pApp = CreateIr_AirApp(wParam, lParam, zParam);
		break;
	case NewWind_APPID:                //新风
	    pApp = CreateNewWindApp(wParam, lParam, zParam);
	break;
 
	case PRJ_SET_DELAY_APPID:               //延时设置。
		pApp = CreatePrjSetDelayApp(wParam, lParam, zParam);
		break;
	case PRJ_REPORT_APPID:                  //上报本机信息。
		pApp = CreatePrjReportApp(wParam, lParam, zParam);
		break;
	case PRJ_RESET_APPID:                   //恢复出厂设置。
		pApp = CreatePrjResetApp(wParam, lParam, zParam);
		break;
	case PRJ_SET_DATE_APPID:                //日期设置。(我添加的)
		pApp = CreatePrjSetDateApp(wParam, lParam, zParam);
		break;
	case PRJ_SET_PWD_APPID:                 //密码设置。
		pApp = CreatePrjSetPwdApp(wParam, lParam, zParam);
		break;
	case PRJ_SET_TIMER_APPID:               //定时设置。(我添加的)
		pApp = CreatePrjSetTimerApp(wParam, lParam, zParam);
		break;
	case PRJ_SET_UI_APPID:                  //背景设置。(我添加的)
		pApp = CreatePrjSetUIApp(wParam, lParam, zParam);
		break;
	case PRJ_SHOW_APPID:                    //显示设置(我添加的)
		pApp = CreatePrjShow(wParam, lParam, zParam);
		break;
	case PRJ_TEMP_APPID:                    //温度校准设置(我添加的)
		pApp = CreatePrjSetTempApp(wParam, lParam, zParam);
		break;
#if 1
	case TIMER_SELECT_APPID:                //定时设置编辑序列(我加的)
		pApp = CreateTimerSelectApp(wParam, lParam, zParam);
		break; 
	case TIMER_OBJECT_APPID:                //定时设置编辑界面(我加的)               
		pApp = CreateTimerObjectApp(wParam, lParam, zParam);
		break; 
	case TIMER_TIME_APPID:                  //定时设置之时间编辑(我加的)
		pApp = CreateTimerTimeApp(wParam, lParam, zParam);
		break; 
	case TIMER_WEEK_APPID:                  //定时设置之重复时间编辑(我加的)
		pApp = CreateTimerWeekApp(wParam, lParam, zParam);
		break; 
	case TIMER_DEVICE_APPID:                //定时设置之设备控制编辑(我加的)
		pApp = CreateTimerDeviceApp(wParam, lParam, zParam);
		break; 
	case TIMER_ACTION_APPID:                //定时设置之控制方式编辑(我加的)
		pApp = CreateTimerActionApp(wParam, lParam, zParam);
		break;
	case PRJ_SET_TIMER_EDIT:                //定时设置之定时参数编辑设置(我加的)
		pApp = CreateTimerEditApp(wParam, lParam, zParam);
		break; 

#endif
	}

	if(pApp == NULL)
		DBGMSG(DPWARNING, "CreateApp %x %x %x Fail!\r\n", wParam, lParam, zParam);

	return pApp;
}

//======================================================
//** 函数名称: DispatchMessage
//** 功能描述: 解析消息
//** 输　入: pMsg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static BOOL DispatchMessage(SYS_MSG* pMsg)
{
	BOOL ret = FALSE;	
	
	DWORD result = DPGetMessage(pMsg);
	
//	count_notouch ++;

//	printf("end = %d begin = %d\n",tEnd.tv_usec,tBegin.tv_usec);			 	

	switch(result)          // 判断队列类型
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
//** 函数名称: MsgProcess
//** 功能描述: 消息处理
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
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
			DWORD tick = DPGetTickCount();    //貌似这里也应该是个调试信息。
			if(pCurApp)                       //这里是干什么用的？
			{
				if(!pCurApp->DoPause())
					break;
			}
			pCurApp = CreateApp(wParam, lParam, zParam);
			pAppStack[dwAppCount] = pCurApp;
			dwAppCount++;                     //几级界面层数
			CAppBase::Show();                 //显示

			CleanUserInput();                 //将收到的消息队列清除            
			DWORD dwTotal, dwFree;
			DumpMemory(&dwTotal, &dwFree);    //显示相关的系统信息
			DBGMSG(DPINFO, "MSG_START_APP %x cost %dms, %d %u %u\r\n", wParam, DPGetTickCount() - tick, dwFree*100/dwTotal, dwFree, dwTotal);
			DumpPhysicalMemory();              //这里是一个空函数。
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
			CAppBase::Show();          // 这应该是一个调用底层接口显示函数     
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

				// 回复之前的窗口
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
//** 函数名称: InitSystem
//** 功能描述: 初始化系统配置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void InitSystem()
{
#ifndef _DEBUG
	// release版本开机后需要扫描磁盘
	ScanDisk();                      //在linux下貌似应该不需要该函数。
	PlayWav(KEYPAD_INDEX, 0);		 // 播放一下声音，避开第一次声音不正常
#endif

	//SetSystemVolume(0xFFFFFFFF);

	// 初始化为2017年 , 在这里应该可以获得系统的准确时间。
	SYSTEMTIME tm;
	DPGetLocalTime(&tm);            //获得当前系统的时间
	tm.wYear = 2017;              
	DPSetLocalTime(&tm);
	StopLogo();

	// 显示程序启动时磁盘状态
	CheckSpareSpace(WINDOWSDIR);
	CheckSpareSpace(FLASHDIR);
	CheckSpareSpace(USERDIR);

	// 显示开机后内存状态
	DWORD mtotal, mfree;
	DumpMemory(&mtotal, &mfree);
	DBGMSG(DPINFO, "init memory %u%%%% free:%u total:%u\r\n", mfree*100/mtotal, mfree, mtotal);

	// 保留升级程序使用的内存空间
	SetObjectMemorySpace_GWF(1*1024*1024);

	// 错误回溯(什么是回溯?)
	DPBacktrace();
}

//======================================================
//** 函数名称: InitServer
//** 功能描述: 定时主函数
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
static void InitServer()
{
	InitGb2Unicode();                           // 加载Unicode码，在屏幕上显示汉字
	LoadAllString("/FlashDev/str/chinese.txt");

	InitFileServer();	

	InitSystemSet();                            // 系统文件存储             	   
	InitTimerFILE();                            // 定时事件信息存储读写

	//SetScreenOnOff(TRUE);                      // 屏幕亮灭

	InitSmartDev();                             // 家居相关内容初始化
	StartSmartServer();                         // ECB智能家居协议线程
	StartPCServer();                            // 这个暂时用不到

#ifndef DEBUG
	StartWatchDog();
#endif
}

//======================================================
//** 函数名称: MSG_TIMR
//** 功能描述: 定时主函数
//** 输　入: pParam
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void* MSG_TIMR(void* pParam)
{
				
	while(1) {

		pNode p_Mid;
		SYSTEMTIME tm;             // 时钟相关结构体
		DPGetLocalTime(&tm);
		g_hour = tm.wHour;
		g_minute = tm.wMinute;
		Week = tm.wDayOfWeek;
		
		sprintf(g_time,"%02d:%02d",g_hour,g_minute);
		
		if(time_mid != g_minute) {

			if(NULL != g_TimeHead) {   //如果有定时事件

				p_Mid = g_TimeHead;

				while(NULL != p_Mid) {

					time_mid = g_minute;
					
					if(p_Mid->show) {  //判断定时事件是否被开启
						//printf("show is true\n");	
						if(strcmp(g_time,p_Mid->Time) == 0 && TIME_FLAG == 1) {  //判断是否达到时间需求
						//	printf("the time is same\n");						 //并且时间已校准	
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
							//如果勾选每天选项
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
							// 如果勾选工作日选项
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
		                    // 如果勾选周末选项
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
							// 如果勾选单次选项	
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
							// 如果勾选永不选项
							else if(p_Mid->choose_data[11]) {

				
							}
							// 单纯点击星期一到星期六
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
	                                                  
		else //没有定时事件             
			//return NULL;
			{

		}
	}
}


//======================================================
//** 函数名称: main
//** 功能描述: 主函数
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
int main()
{
	printf("\r\n\r\n========================>>Start<<========================\r\n\r\n");

	InitConfig();                // 貌似是和UI界面相对应的东西        
	InitSystem();                // 这里跟以后的工作关系应该不大
	InitSysMessage();            // 初始化变量及创建线程
	InitServer();                // 智能家居协议初始化              

	DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);
	SYS_MSG msg;

	while(1)
	{
		
		//Send_Time_Ack();

		//	MSG_TIMR();              //定时事件扫描

		if(DispatchMessage(&msg))//在这里获取变量,如果没有消息则会一直执行MsgProcess函数。
			MsgProcess(msg.msg, msg.wParam, msg.lParam, msg.zParam); //在这里判断变量.
	}
     
	return 0;
}    
