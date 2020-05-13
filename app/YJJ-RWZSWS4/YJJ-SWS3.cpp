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
//** ��������: WatchDogProc
//** ��������: ���Ź�����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void WatchDogProc()
{
#ifndef _DEBUG
    FeedWatchDog();                         // ��һ��ʱ�䣨1���ӣ�ιһ�ι���
#endif
    DWORD dwTotal, dwFree;
    DumpMemory(&dwTotal, &dwFree);
    if (dwFree * 20 < dwTotal)	            // С��5%ʱ �Զ�����
    {
        DBGMSG(DPERROR, "dwMemoryLoad Send Reboot %lu %lu\r\n", dwFree, dwTotal);
        DPPostMessage(MSG_SYSTEM, REBOOT_MACH, 0, 0);
    }
}

//======================================================
//** ��������: TimeSyncProc
//** ��������: ʱ��ͬ������
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: CfgStatusProc
//** ��������: ����״̬����
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: CreateApp
//** ��������: ��������
//** �䡡��: wParam lParam zParam
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static CAppBase *CreateApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
    CAppBase *pApp = NULL;
    switch (wParam)                          // wParam�Ǵ���ID������
    {
        case MAIN_APPID:                        	 // ���˵�
            pApp = CreateMainApp(wParam, lParam, zParam);
            break;
        case PWD_INPUT_APPID:                  	 // ��������
            pApp = CreatePwdInputApp(wParam, lParam, zParam);
            break;
        case PROJECT_APPID:                    	 // ϵͳ����
            pApp = CreateProjectApp(wParam, lParam, zParam);
            break;
        case MECHINE_SET:                      	 // ��������
            pApp = CreateMechineApp(wParam, lParam, zParam);
            break;
        case PRJ_REPORT_APPID:                 	 // �ϱ�ID
            pApp = CreatePrjReportApp(wParam, lParam, zParam);
            break;
        case PRJ_RESET_APPID:                  	 // �ָ�����
            pApp = CreatePrjResetApp(wParam, lParam, zParam);
            break;
        case PRJ_ABOUT_APPID:					 	 // ���ڱ���
            pApp = CreateAboutApp(wParam, lParam, zParam);
            break;
        case PRJ_SET_PWD_APPID:                 	 // �������á�
            pApp = CreatePrjSetPwdApp(wParam, lParam, zParam);
            break;
        case BKGD_SET:                         	 // ��������
            pApp = CreateBKApp(wParam, lParam, zParam);
            break;
        case PRJ_SET_DATE_APPID:                	 // ʱ������
            pApp = CreatePrjSetDateApp(wParam, lParam, zParam);
            break;
        case PRJ_SHOW_APPID:						 // ��ʾ����
            pApp = CreatePrjShow(wParam, lParam, zParam);
            break;
        case PRJ_LOCK_TIME_APPID:					 // ����ʱ��
            pApp = CreateLockTime(wParam, lParam, zParam);
            break;
        case PRJ_SHOW_BRIGHT_APPID:				 // ��Ļ����
            pApp = CreateShowBright(wParam, lParam, zParam);
            break;
        case PRJ_LIGHT_CFG_APPID: 				 // �ƹ�����
            pApp = CreateLightCfgApp(wParam, lParam, zParam);
            break;
        case PRJ_LIGHT_INIT_APPID: 				 // �ƹ��ʼ��
            pApp = CreateLightInitApp(wParam, lParam, zParam);
            break;
        case PRJ_LIGHT_STUDY_APPID: 				 // �ƹ�ѧϰ
            pApp = CreateLightStudyApp(wParam, lParam, zParam);
            break;
        case PRJ_LIGHT_LIST_APPID: 				 // ѧϰ�б�
            pApp = CreateTipApp(wParam, lParam, zParam);
            break;

        case UI_APPID:
            pApp = CreatePrjSetUIApp(wParam, lParam, zParam);
            break;
        case CLOCK_APPID:                      	 // ����ʱ��
            pApp = CreateClockApp(wParam, lParam, zParam);
            break;
        //	case TIP_APPID:
        //		pApp = CreateTipApp(wParam, lParam, zParam);
        //		break;
        case LIGHT_APPID:                      	 //��ͨ�ƹ����
            pApp = CreateLightApp(wParam, lParam, zParam);
            break;
        case DIMMER_APPID:                    	 //������ơ�
            pApp = CreateDimmerApp(wParam, lParam, zParam);
            break;
        case CURTAIN_APPID:                   	 //�������ơ�
            pApp = CreateCurtainApp(wParam, lParam, zParam);
            break;
        case SCENE_APPID:                      	 //�龰����
            pApp = CreateSceneApp(wParam, lParam, zParam);
            break;
        case WINDOW_APPID:
            pApp = CreateWindowApp(wParam, lParam, zParam);
            break;
        case OUTLET_APPID:                        //��������
            pApp = CreateOutletApp(wParam, lParam, zParam);
            break;
		case WIND_APPID:                        	 //�·����
            pApp = CreateWindApp(wParam, lParam, zParam);
            break;	
        case AIRC_APPID:                        	 //�յ�����
            pApp = CreateAirCApp(wParam, lParam, zParam);
            break;
        case HEAT_APPID:                       	 //��ů����
            pApp = CreateHeatApp(wParam, lParam, zParam);
            break;
        case TV_APPID:                            //������ӿ���
            pApp = CreateTVApp(wParam, lParam, zParam);
            break;
        case MUSIC_APPID:                         //�������ֿ���
            pApp = CreateMUSICApp(wParam, lParam, zParam);
            break;
        case IR_AIR_APPID:                        //����յ�����
            pApp = CreateIr_AirApp(wParam, lParam, zParam);
            break;
        case PRJ_SET_TIMER_APPID:                  //��ʱ����
            pApp = CreatePrjSetTimerApp(wParam, lParam, zParam);
            break;

        case PRJ_TEMP_APPID:                      //�¶�У׼����
            pApp = CreatePrjSetTempApp(wParam, lParam, zParam);
            break;
        case TIMER_SELECT_APPID:                   //��ʱ���ñ༭����
            pApp = CreateTimerSelectApp(wParam, lParam, zParam);
            break;
        case TIMER_OBJECT_APPID:                   //��ʱ���ñ༭����
            pApp = CreateTimerObjectApp(wParam, lParam, zParam);
            break;
        case TIMER_TIME_APPID:                     //��ʱ����ʱ��༭
            pApp = CreateTimerTimeApp(wParam, lParam, zParam);
            break;
        case TIMER_WEEK_APPID:                     //��ʱ�������ڱ༭
            pApp = CreateTimerWeekApp(wParam, lParam, zParam);
            break;
        case TIMER_DEVICE_APPID:                   //��ʱ����֮�豸���Ʊ༭
            pApp = CreateTimerDeviceApp(wParam, lParam, zParam);
            break;
        case TIMER_ACTION_APPID:                	 //��ʱ����֮���Ʒ�ʽ�༭
            pApp = CreateTimerActionApp(wParam, lParam, zParam);
            break;
        case TIMER_EDIT_APPID:                  	 //��ʱ���ñ༭����
            pApp = CreateTimerEditApp(wParam, lParam, zParam);
            break;
    }

    if (pApp == NULL)
        DBGMSG(DPWARNING, "CreateApp %x %x %x Fail!\r\n", wParam, lParam, zParam);

    return pApp;
}


//======================================================
//** ��������: DispatchMessage
//** ��������: ��Ϣ����
//** �䡡��: pMsg
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: MessageProcess
//** ��������: ϵͳ��Ϣ��ʼ��
//** �䡡��: msg wParam lParam zParam
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
            dwAppCount++;                     //�����������
            CAppBase::Show();                 //��ʾ
            CleanUserInput();                 //��Ϣ�������

            DWORD dwTotal, dwFree;
            DumpMemory(&dwTotal, &dwFree);    //��ʾ��ص�ϵͳ��Ϣ
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

                    // �ָ�֮ǰ�Ĵ���
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
//** ��������: InitSystem(void)
//** ��������: ϵͳ��Ϣ��ʼ��
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void InitSystem()
{
#ifndef _DEBUG
    //ScanDisk();            				 // release�汾��������Ҫɨ�����
    //PlayWav(KEYPAD_INDEX, 0);		 	 // ����һ���������ܿ���һ������������
#endif
    //SetSystemVolume(0xFFFFFFFF);

    // ��ʼ��ϵͳʱ��
    SYSTEMTIME tm;
    DPGetLocalTime(&tm);
    tm.wYear = 2018;
    DPSetLocalTime(&tm);
    //StopLogo();

    // ��ʾ��������ʱ����״̬
    CheckSpareSpace(WINDOWSDIR);
    CheckSpareSpace(FLASHDIR);
    CheckSpareSpace(USERDIR);

    // ��ʾ�������ڴ�״̬
    DWORD mtotal, mfree;
    DumpMemory(&mtotal, &mfree);
    DBGMSG(DPINFO, "init memory %u%% free:%u total:%u\r\n", mfree * 100 / mtotal, mfree, mtotal);

    // ������������ʹ�õ��ڴ�ռ�
    //SetObjectMemorySpace_GWF(1*1024*1024);

    // �������
    DPBacktrace();
}

//======================================================
//** ��������: InitServer(void)
//** ��������: ϵͳ�����ʼ��
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
static void InitServer()
{
    InitGb2Unicode();                           // ����Unicode�룬����Ļ����ʾ����
    LoadAllString("/FlashDev/str/chinese.txt");

    InitFileServer();							// ��ʼ���ļ�����

    InitSystemSet();                            // ϵͳ�ļ��洢

    SetScreenOnOff(TRUE);                 		// ��Ļ����
    InitSmartGpio();

    InitSmartDev();                             // �Ҿ��豸��ʼ��
    InitSmartTimer();                           // ��ʱ�¼��洢
    StartSmartServer(); 						// �Ҿ�Э���ʼ��
    InitSmartZigbee();							// ZIGBEEģ���ʼ��
    //StartPCServer();                      // PC���Է����ʼ��

#ifndef DEBUG
    StartWatchDog();
#endif
}

//======================================================
//** ��������: main(void)
//** ��������: ���������
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
int main()
{
    printf("\r\n\r\n=====================>>Start<<=====================\r\n\r\n");

    InitConfig();                // UI��������
    InitSystem();                // ϵͳ��Ϣ
    InitSysMessage();            // ��ʼ�������������߳�
    InitServer();                // ���ܼҾ�Э���ʼ��

    DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);	// �������˵�
    DPCreateTimeEvent();        // ʱ����Ϣ�߳�
    SYS_MSG msg;
    while (1)
    {
        if (DispatchMessage(&msg))
            MessageProcess(msg.msg, msg.wParam, msg.lParam, msg.zParam);	// ������Ϣ����
    }

    return 0;
}
