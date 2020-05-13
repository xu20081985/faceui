#include <time.h>
#include <sys/time.h>
#include "roomlib.h"
#include "CAppBase.h"
#include "appdef.h"
#include "SmartConfig.h"
#include "CCtrlModules.h"

#ifdef DPCE
#pragma comment(lib, "libjpeg.lib")
#pragma comment(lib,"freetype245.lib")
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "zlib.lib")
#endif

static void test()
{
#if 0
    static WORD cmd = 0x03;
    static DWORD count = 0;
    if (++count == 3)
    {
        cmd = (cmd == 0x03) ? 0x01 : 0x03;
        SendSceneCmd(NULL, cmd, 0);
        count = 0;
    }
#endif
}

//======================================================
//** 函数名称: WatchDogProc
//** 功能描述: 看门狗处理
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void WatchDogProc()
{
#ifndef _DEBUG
    FeedWatchDog();                         // 隔一段时间（1秒钟）喂一次狗。
#endif
    DWORD dwTotal, dwFree;
    DumpMemory(&dwTotal, &dwFree);
    if (dwFree * 20 < dwTotal)	            // 小于5%时 自动重启
    {
        DBGMSG(DPERROR, "dwMemoryLoad Send Reboot %lu %lu\r\n", dwFree, dwTotal);
        DPPostMessage(MSG_SYSTEM, REBOOT_MACH, 0, 0);
    }
}

//======================================================
//** 函数名称: TimeSyncProc
//** 功能描述: 时间同步处理
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void TimeSyncProc()
{
    static DWORD count = 0;

    if (GetTimeSync() == FALSE)
    {
        if (count == 10 || count == 60 || count == 120)
            SmartTimeSync();
        count++;
    }
}

//======================================================
//** 函数名称: CfgStatusProc
//** 功能描述: 配置状态处理
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void CfgStatusProc()
{
    if (GetCfgStatus() == TRUE)
    {
        DWORD count = GetCfgTime();
        if (count++ == 10)
        {
            SetCfgStatus(FALSE);
            DPPostMessage(MSG_START_FROM_ROOT, MAIN_APPID, 0, 0);
        }
        SetCfgTime(count);
    }
}


//======================================================
//** 函数名称: CreateApp
//** 功能描述: 创建界面
//** 输　入: wParam lParam zParam
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static CAppBase *CreateApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CAppBase *pApp = NULL;
    switch (wParam)                          // wParam是窗口ID参数。
    {
        case MAIN_APPID:                        	 // 主菜单
            pApp = CreateMainApp(wParam, lParam, zParam);
            break;
        case PWD_INPUT_APPID:                  	 // 输入密码
            pApp = CreatePwdInputApp(wParam, lParam, zParam);
            break;
        case PROJECT_APPID:                    	 // 系统设置
            pApp = CreateProjectApp(wParam, lParam, zParam);
            break;
        case MECHINE_SET:                      	 // 本机设置
            pApp = CreateMechineApp(wParam, lParam, zParam);
            break;
        case PRJ_REPORT_APPID:                 	 // 上报ID
            pApp = CreatePrjReportApp(wParam, lParam, zParam);
            break;
        case PRJ_RESET_APPID:                  	 // 恢复出厂
            pApp = CreatePrjResetApp(wParam, lParam, zParam);
            break;
        case PRJ_ABOUT_APPID:					 	 // 关于本机
            pApp = CreateAboutApp(wParam, lParam, zParam);
            break;
        case PRJ_SET_PWD_APPID:                 	 // 密码设置。
            pApp = CreatePrjSetPwdApp(wParam, lParam, zParam);
            break;
        case BKGD_SET:                         	 // 背景设置
            pApp = CreateBKApp(wParam, lParam, zParam);
            break;
        case PRJ_SET_DATE_APPID:                	 // 时间设置
            pApp = CreatePrjSetDateApp(wParam, lParam, zParam);
            break;
        case PRJ_SHOW_APPID:						 // 显示设置
            pApp = CreatePrjShow(wParam, lParam, zParam);
            break;
        case PRJ_LOCK_TIME_APPID:					 // 锁屏时间
            pApp = CreateLockTime(wParam, lParam, zParam);
            break;
        case PRJ_SHOW_BRIGHT_APPID:				 // 屏幕亮度
            pApp = CreateShowBright(wParam, lParam, zParam);
            break;
        case PRJ_LIGHT_CFG_APPID: 				 // 灯光配置
            pApp = CreateLightCfgApp(wParam, lParam, zParam);
            break;
        case PRJ_LIGHT_INIT_APPID: 				 // 灯光初始化
            pApp = CreateLightInitApp(wParam, lParam, zParam);
            break;
        case PRJ_LIGHT_STUDY_APPID: 				 // 灯光学习
            pApp = CreateLightStudyApp(wParam, lParam, zParam);
            break;
        case PRJ_LIGHT_LIST_APPID: 				 // 学习列表
            pApp = CreateTipApp(wParam, lParam, zParam);
            break;

        case UI_APPID:
            pApp = CreatePrjSetUIApp(wParam, lParam, zParam);
            break;
        case CLOCK_APPID:                      	 // 待机时间
            pApp = CreateClockApp(wParam, lParam, zParam);
            break;
        //	case TIP_APPID:
        //		pApp = CreateTipApp(wParam, lParam, zParam);
        //		break;
        case LIGHT_APPID:                      	 //普通灯光控制
            pApp = CreateLightApp(wParam, lParam, zParam);
            break;
        case DIMMER_APPID:                    	 //调光控制。
            pApp = CreateDimmerApp(wParam, lParam, zParam);
            break;
        case CURTAIN_APPID:                   	 //窗帘控制。
            pApp = CreateCurtainApp(wParam, lParam, zParam);
            break;
        case SCENE_APPID:                      	 //情景控制
            pApp = CreateSceneApp(wParam, lParam, zParam);
            break;
        case WINDOW_APPID:
            pApp = CreateWindowApp(wParam, lParam, zParam);
            break;
        case OUTLET_APPID:                        //插座控制
            pApp = CreateOutletApp(wParam, lParam, zParam);
            break;
		case WIND_APPID:                        	 //新风控制
            pApp = CreateWindApp(wParam, lParam, zParam);
            break;	
        case AIRC_APPID:                        	 //空调控制
            pApp = CreateAirCApp(wParam, lParam, zParam);
            break;
        case HEAT_APPID:                       	 //地暖控制
            pApp = CreateHeatApp(wParam, lParam, zParam);
            break;
        case TV_APPID:                            //红外电视控制
            pApp = CreateTVApp(wParam, lParam, zParam);
            break;
        case MUSIC_APPID:                         //背景音乐控制
            pApp = CreateMUSICApp(wParam, lParam, zParam);
            break;
        case IR_AIR_APPID:                        //红外空调控制
            pApp = CreateIr_AirApp(wParam, lParam, zParam);
            break;
        case PRJ_SET_TIMER_APPID:                  //定时设置
            pApp = CreatePrjSetTimerApp(wParam, lParam, zParam);
            break;

        case PRJ_TEMP_APPID:                      //温度校准设置
            pApp = CreatePrjSetTempApp(wParam, lParam, zParam);
            break;
        case TIMER_SELECT_APPID:                   //定时设置编辑序列
            pApp = CreateTimerSelectApp(wParam, lParam, zParam);
            break;
        case TIMER_OBJECT_APPID:                   //定时设置编辑界面
            pApp = CreateTimerObjectApp(wParam, lParam, zParam);
            break;
        case TIMER_TIME_APPID:                     //定时设置时间编辑
            pApp = CreateTimerTimeApp(wParam, lParam, zParam);
            break;
        case TIMER_WEEK_APPID:                     //定时设置周期编辑
            pApp = CreateTimerWeekApp(wParam, lParam, zParam);
            break;
        case TIMER_DEVICE_APPID:                   //定时设置之设备控制编辑
            pApp = CreateTimerDeviceApp(wParam, lParam, zParam);
            break;
        case TIMER_ACTION_APPID:                	 //定时设置之控制方式编辑
            pApp = CreateTimerActionApp(wParam, lParam, zParam);
            break;
        case TIMER_EDIT_APPID:                  	 //定时设置编辑设置
            pApp = CreateTimerEditApp(wParam, lParam, zParam);
            break;
    }

    if (pApp == NULL)
        DBGMSG(DPWARNING, "CreateApp %x %x %x Fail!\r\n", wParam, lParam, zParam);

    return pApp;
}


//======================================================
//** 函数名称: DispatchMessage
//** 功能描述: 消息分类
//** 输　入: pMsg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static BOOL DispatchMessage(SYS_MSG *pMsg)
{
    BOOL ret = FALSE;
    DWORD result = DPGetMessage(pMsg);

    switch (result)
    {
        case MSG_TIME_TYPE:
            TimeSyncProc();
            WatchDogProc();
            CfgStatusProc();
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
            TouchEventRemap(pMsg);
            break;
        default:
            break;
    }

    return ret;
}


//======================================================
//** 函数名称: MessageProcess
//** 功能描述: 系统信息初始化
//** 输　入: msg wParam lParam zParam
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void MessageProcess(DWORD msg, DWORD wParam, DWORD lParam, DWORD zParam)
{
    static DWORD dwAppCount = 0;
    static CAppBase *pCurApp = NULL;
    static CAppBase *pAppStack[32] = {0};
    switch(msg)
    {
        case TIME_MESSAGE:
            pCurApp->DoProcess(msg, wParam, lParam, zParam);
            break;
        case TOUCH_RAW_MESSAGE:
        case TOUCH_MESSAGE:
        case TOUCH_ACTIVE:
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
                    AdjustScreen(0, 50, 50);
                    RebootSystem();
                    break;
                case RESET_MACH:
                    ResetSmartDev();
                    ResetSystemSet();
                    ResetSmartTimer();
                    DeinitFileServer();
                    AdjustScreen(0, 50, 50);
                    RebootSystem();
                    break;
                case WATCHDOG_CHANGE:
#ifndef _DEBUG
                    if (lParam)
                        StartWatchDog();
                    else
                        StopWatchDog();
#endif
                    break;
                case RESET_ZIGBEE:
                    InitSmartZigbee();
                    DPSleep(1000);
                    SmartTimeSync();
            }
            break;
        case MSG_START_APP:
        {
            DWORD tick = DPGetTickCount();
            if (pCurApp)
            {
                if (!pCurApp->DoPause())
                    break;
            }
            pCurApp = CreateApp(wParam, lParam, zParam);
            pAppStack[dwAppCount] = pCurApp;
            dwAppCount++;                     //几级界面层数
            CAppBase::Show();                 //显示
            CleanUserInput();                 //消息队列清除

            DWORD dwTotal, dwFree;
            DumpMemory(&dwTotal, &dwFree);    //显示相关的系统信息
            DBGMSG(DPINFO, "MSG_START_APP %x cost %dms, %d%% %u %u\r\n",
                   wParam, DPGetTickCount() - tick, dwFree * 100 / dwTotal, dwFree, dwTotal);
            DumpPhysicalMemory();
        }
        break;
        case MSG_END_APP:
        {
            for (DWORD i = 0; i < dwAppCount; i++)
            {
                if ((DWORD)pAppStack[i] == wParam)
                {
                    DWORD tick = DPGetTickCount();

                    pAppStack[i]->Destroy();
                    delete pAppStack[i];
                    memmove(&pAppStack[i], &pAppStack[i + 1], (dwAppCount - i - 1) * 4);
                    dwAppCount--;

                    DWORD dwTotal, dwFree;
                    DumpMemory(&dwTotal, &dwFree);
                    DBGMSG(DPINFO, "MSG_END_APP cost %dms, %d %u %u\r\n",
                           DPGetTickCount() - tick, dwFree * 100 / dwTotal, dwFree, dwTotal);
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
            for(DWORD i = 0; i < dwAppCount; i++)
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
            DBGMSG(DPINFO, "MSG_START_FROM_ROOT %x cost %dms, %d %u %u\r\n",
                   wParam, DPGetTickCount() - tick, dwFree * 100 / dwTotal, dwFree, dwTotal);
            DumpPhysicalMemory();
        }
        break;
        case MSG_EXTSTART_APP:
            DPPostMessage(MSG_START_FROM_ROOT, wParam, lParam, zParam, MSG_URGENT_TYPE);
        break;
        case MSG_START_FROM_OVER:
        {
            DWORD tick = DPGetTickCount();

            if (pCurApp)
            {
                pCurApp->PauseAck();
            }
            pCurApp = CreateApp(wParam, lParam, zParam);
            pAppStack[dwAppCount] = pCurApp;
            dwAppCount++;
            CAppBase::Show();
            CleanUserInput();

            DWORD dwTotal, dwFree;
            DumpMemory(&dwTotal, &dwFree);
            DBGMSG(DPINFO, "MSG_START_FROM_OVER %x cost %dms, %d %u %u\r\n",
                   wParam, DPGetTickCount() - tick, dwFree * 100 / dwTotal, dwFree, dwTotal);
            DumpPhysicalMemory();
        }
        break;
        case MSG_END_OVER_APP:
            for (DWORD i = 0; i < dwAppCount; i++)
            {
                if ((DWORD)pAppStack[i] == wParam)
                {
                    DWORD tick = DPGetTickCount();

                    pAppStack[i]->Destroy();
                    delete pAppStack[i];
                    memmove(&pAppStack[i], &pAppStack[i + 1], (dwAppCount - i - 1) * 4);
                    dwAppCount--;

                    // 恢复之前的窗口
                    if (dwAppCount > 0)
                        pCurApp = pAppStack[dwAppCount - 1];
                    else
                        pCurApp = pAppStack[0];
                    pCurApp->ResumeAck();
                    CAppBase::Show();
                    CleanUserInput();

                    DWORD dwTotal, dwFree;
                    DumpMemory(&dwTotal, &dwFree);
                    DBGMSG(DPINFO, "MSG_END_OVER_APP cost %dms, %d %u %u\r\n",
                           DPGetTickCount() - tick, dwFree * 100 / dwTotal, dwFree, dwTotal);
                    DumpPhysicalMemory();
                    break;
                }
            }
            break;
    }
}

//======================================================
//** 函数名称: InitSystem(void)
//** 功能描述: 系统信息初始化
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void InitSystem()
{
#ifndef _DEBUG
    //ScanDisk();            				 // release版本开机后需要扫描磁盘
    //PlayWav(KEYPAD_INDEX, 0);		 	 // 播放一下声音，避开第一次声音不正常
#endif
    //SetSystemVolume(0xFFFFFFFF);

    // 初始化系统时间
    SYSTEMTIME tm;
    DPGetLocalTime(&tm);
    tm.wYear = 2018;
    DPSetLocalTime(&tm);
    //StopLogo();

    // 显示程序启动时磁盘状态
    CheckSpareSpace(WINDOWSDIR);
    CheckSpareSpace(FLASHDIR);
    CheckSpareSpace(USERDIR);

    // 显示开机后内存状态
    DWORD mtotal, mfree;
    DumpMemory(&mtotal, &mfree);
    DBGMSG(DPINFO, "init memory %u%% free:%u total:%u\r\n", mfree * 100 / mtotal, mfree, mtotal);

    // 保留升级程序使用的内存空间
    //SetObjectMemorySpace_GWF(1*1024*1024);

    // 错误回溯
    DPBacktrace();
}

//======================================================
//** 函数名称: InitServer(void)
//** 功能描述: 系统服务初始化
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void InitServer()
{
    InitGb2Unicode();                           // 加载Unicode码，在屏幕上显示汉字
    LoadAllString("/FlashDev/str/chinese.txt");

    InitFileServer();							// 初始化文件服务

    InitSystemSet();                            // 系统文件存储

    SetScreenOnOff(TRUE);                 		// 屏幕亮度
    InitSmartGpio();

    InitSmartDev();                             // 家居设备初始化
    InitSmartTimer();                           // 定时事件存储
    StartSmartServer(); 						// 家居协议初始化
    InitSmartZigbee();							// ZIGBEE模块初始化
    //StartPCServer();                      // PC调试服务初始化

#ifndef DEBUG
    StartWatchDog();
#endif
}

//======================================================
//** 函数名称: main(void)
//** 功能描述: 主函数入口
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
int main()
{
    printf("\r\n\r\n=====================>>Start<<=====================\r\n\r\n");

    InitConfig();                // UI界面配置
    InitSystem();                // 系统信息
    InitSysMessage();            // 初始化变量及创建线程
    InitServer();                // 智能家居协议初始化

    DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);	// 创建主菜单
    DPCreateTimeEvent();        // 时间消息线程
    SYS_MSG msg;
    while (1)
    {
        if (DispatchMessage(&msg))
            MessageProcess(msg.msg, msg.wParam, msg.lParam, msg.zParam);	// 接收消息处理
    }

    return 0;
}
