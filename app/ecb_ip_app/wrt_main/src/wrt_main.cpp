/*
 * wrt_main.cpp -- Gateway Master Program Processing
 *
 * Copyright (c) Wrt Intelligent Technology Co Ltd. 2017. All Rights Reserved.
 *
 * See the Project file for usage and redistribution requirements
 *
 *	$Id: wrt_main.cpp 	2017/06/20   Siny $
 */
 
/******************************** Description *********************************/
 
/*
 *  This module provides main functions for management
 *  Provides access keys, factory recovery, service initialization, 
 *  and button control, watchdog, etc.
 */
 
/********************************* Includes ***********************************/

#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <net/if.h>
#include <signal.h>
#include <sys/ioctl.h>
#include "openssl/rc4.h"
#include "wrt_crypt.h"
#include "wrt_common.h"
#include "wrt_MsgQueue.h"
#include "wrt_log.h"
#include "wrt_cfg.h"
#include "wrt_gpio.h"
#include "wrt_devHandler.h"
#include "wrt_msgHandler.h"
#include "wrt_broadcast.h"
#include "wrt_ntp.h"
#include "webmain.h"

/********************************* Defines ************************************/

unsigned int key_up;   						/* Press key */
unsigned int key_down; 						/* Bounce key */
bool led_flag = false;      				/* Identifies the status of the LED light */
bool start_key = false;						/* Press the button key for the first time */
int run_status;								/* When running, enter different status flag */
int g_led_gpio;								/* led gpio handle */
int g_wdt_gpio;								/* wdt gpio handle */
int g_buzzer_gpio;							/* buzzer gpio handle */
int g_key_gpio;								/* key gpio handle */
CWRTLogManage *WRT_log_g;					/* log moudle handle */

extern int g_hWatchDog;
extern int get_variable_key_flag;
extern T_SYSTEMINFO* pSystemInfo;
extern CWRTMsgQueue *WRTSockMsgQueue_g;
extern CWRTMsgQueue *WRTCmdMsgQueue_g;
extern CWRTMsgQueue *WRTUartRecvMsgQueue_g;

extern void *socket_listen_task_gateway(void *arg);
extern void *wrt_msgSend_thread(void *arg);
extern void *wrt_msgHandler_thread(void *arg);
extern void *wrt_uartRecvHandler_thread(void *arg);
extern void *dealLinkageFunc(void *arg);
extern void *remoteControlInit(void *arg);
extern void *check_keepalive(void *arg);
extern void *check_sensor_timeout(void *arg);

/*********************************** Code *************************************/
/*
 *   Gets the key flag bit and sets the timing false
 */
//======================================================
//** 函数名称: resetVariableKeyTimer
//** 功能描述: 复位可变密钥标志
//** 输　入: data
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
static bool resetVariableKeyHandle(void *data)
{
	get_variable_key_flag = false;
	
	return false;
}

/******************************************************************************/
/*
 *   Gets the key flag reset
 */
//======================================================
//** 函数名称: resetVariableKeyTimer
//** 功能描述: 重新设置可变密钥定时器
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
static void resetVariableKeyTimer()
{
	DEBUG_MESSAGE("====================>>resetVariableKeyTimer\n");
	if (false == get_variable_key_flag)
	{
		get_variable_key_flag = true;
		alarm(30);
	}
}

/******************************************************************************/
/*
 *   rm the key flag timer
 */
//======================================================
//** 函数名称: rmVariableKeyTimer
//** 功能描述: 移除可变密钥定时器
//** 输　入: arg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
static void rmVariableKeyTimer(int arg)
{
	get_variable_key_flag = false;
}

/******************************************************************************/
/*
 *   Restore factory settings and delete configuration files, and then restart
 */
//======================================================
//** 函数名称: reset_factory
//** 功能描述: 恢复出厂设置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
static void reset_factory()
{
/*
 *	 Delete log and system file
 */
	system("rm -rf /UserDev/log");
	system("rm -rf /UserDev/config");
/*
 *	 System reboot
 */
	system("sync");
	system("reboot -f");
}

/******************************************************************************/
/*
 *   System completion ringing
 */
//======================================================
//** 函数名称: bellRingtimes
//** 功能描述: 蜂鸣器控制声音
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
void bellRingtimes()
{
	SetGpioVal(g_buzzer_gpio, PULL_UP);
	usleep(100*1000);
	SetGpioVal(g_buzzer_gpio, PULL_DOWN);
	usleep(100*1000);
}

/******************************************************************************/
/*
 *   Initializing communication queues and threads
 */
//======================================================
//** 函数名称: initSocketServer
//** 功能描述: 初始化网络服务
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int initSocketServer()
{
/*
 *  Creating a network message cache queue	
 */
	if (NULL == WRTSockMsgQueue_g)
	{
		WRTSockMsgQueue_g = new CWRTMsgQueue((key_t)1233);
		if (false == WRTSockMsgQueue_g->initMsgQueue(WRT_MSG_QUEUE_MAX * 5))
		{
			DEBUG_ERROR("[CWRTMsgQueue]---init WRTSockMsgQueue_g err\n");
			return -1;
		}
	}
	
/*
 *  Creating a cmd message cache queue	
 */	
 	if (NULL == WRTCmdMsgQueue_g)
	{
		WRTCmdMsgQueue_g = new CWRTMsgQueue((key_t)1234);
		if (false == WRTCmdMsgQueue_g->initMsgQueue(WRT_MSG_QUEUE_MAX * 20))
		{
			DEBUG_ERROR("[CWRTMsgQueue]---init WRTCmdMsgQueue_g err\n");
			return -1;
		}
	}
	
/*
 *	LAN message receive thread 
 */
	pthread_create_t(socket_listen_task_gateway);
/*
 *  Message sending thread	
 */
	pthread_create_t(wrt_msgSend_thread);
/*
 *  Message processing thread	
 */
	pthread_create_t(wrt_msgHandler_thread);
/*
 *  Uart receive data processing thread
 */
	pthread_create_t(wrt_uartRecvHandler_thread);
/*
 *  Sensor linkage thread	
 */
	pthread_create_t(dealLinkageFunc);
/*
 *  Timer linkage thread	
 */
	pthread_create_t(dealTimerFunc);
/*
 *  Cloud processing threads	
 */
	pthread_create_t(remoteControlInit);
/*
 *	Check TCP active connections threads	
 */
	pthread_create_t(check_keepalive);
/*
 *	Check sensor status timeout
 */
	pthread_create_t(check_sensor_timeout);

	return 0;
}

/******************************************************************************/
/*
 *   Initializing use gpio for system
 */
//======================================================
//** 函数名称: initSystemGpio
//** 功能描述: 初始化系统GPIO
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int initSystemGpio()
{
	GPIO_CFG gpio_cfg;

/*
 *  Initializes LED lights, associated GPIO, control status
 */	
	memset(&gpio_cfg, 0, sizeof(gpio_cfg));
	gpio_cfg.gpio_num = LED_GPIO;
	gpio_cfg.gpio_cfg = OUTPUT;
	if (InitGpio(&g_led_gpio, gpio_cfg))
	{
		DEBUG_ERROR("InitGpio led gpio fail\n");
		return -1;
	}	
	SetGpioVal(g_led_gpio, PULL_UP);

/*
 *  Initializes watchdog lights, associated GPIO, control status
 */	
	memset(&gpio_cfg, 0, sizeof(gpio_cfg));
	gpio_cfg.gpio_num = WATCHDOG_GPIO;
	gpio_cfg.gpio_cfg = OUTPUT;
	if (InitGpio(&g_wdt_gpio, gpio_cfg))
	{
		DEBUG_ERROR("InitGpio led gpio fail\n");
		return -1;
	}
	SetGpioVal(g_wdt_gpio, PULL_UP);

/*
 *  Initializes buzzer ring, associated GPIO, control status
 */		
	memset(&gpio_cfg, 0, sizeof(gpio_cfg));
	gpio_cfg.gpio_num = BUZZER_GPIO;
	gpio_cfg.gpio_cfg = OUTPUT;
	if (InitGpio(&g_buzzer_gpio, gpio_cfg))
	{
		DEBUG_ERROR("InitGpio buzzer gpio fail\n");
		return -1;
	}
	SetGpioVal(g_buzzer_gpio, PULL_DOWN);

/*
 *  Initializes buzzer ring, associated GPIO, control status
 */		
	memset(&gpio_cfg, 0, sizeof(gpio_cfg));
	gpio_cfg.gpio_num = SET_KEY_GPIO;
	gpio_cfg.gpio_cfg = INPUT;
	if (InitGpio(&g_key_gpio, gpio_cfg))
	{
		DEBUG_ERROR("InitGpio key gpio fail\n");
		return -1;
	}

	return 0;
}

/******************************************************************************/
/*
 *   net check is run
 */
 /*
int sock_check()  
{    
    int fd = -1;
	int err = 0;  
    struct ifreq ifr;  
    struct ethtool_value edata;
	
    memset(&ifr, 0, sizeof(ifr));  
    strcpy(ifr.ifr_name, "eth0"); 
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{  
		perror("Cannot get control socket");  
		return -1;  
    }  

    edata.cmd = 0x0000000a;  
    ifr.ifr_data = (caddr_t)&edata;  
    err = ioctl(fd, 0x8946, &ifr);  
    if (err == 0 && edata.data) 
	{
		printf("Link detected: yes\n");  
		return 0;
    } 

	printf("Link detected: no\n"); 
	if (fd != NULL)
		close(fd);
	return -1;
}  
*/

/******************************************************************************/
/*
 *   Gateway configuration file and service initialization
 */
//======================================================
//** 函数名称: WRT_gateWay_init
//** 功能描述: 网关初始化
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
int WRT_gateWay_init()
{
	int ret;

/*
 *	Turn on the hardware watchdog
 */	
	ret = StartWatchDog();
	CHECK_RET(ret, "StartWatchDog error");

//	ret = setLocalTime();
//	CHECK_RET(ret, "setLocalTime error");
	getRtcTime();
/*
 *	start NTP time
 */ 
	//ret = StartNtpTime();
	//CHECK_RET(ret, "StartNtpTime error");
/*
 *	Initialize system configuration information	
 */
	ret = init_system_info();
	CHECK_RET(ret, "init_system_info error");
/*
 *	Initialize device processing module 
 */
	ret = init_dev_handler();
	CHECK_RET(ret, "init_dev_handler error");
/*
 *	Initialize the message processing module
 */
	ret = initMsgHandler();
	CHECK_RET(ret, "initMsgHandler error");
/*
 *	Initializing network services 
 */
	ret = initSocketServer();
	CHECK_RET(ret, "initSocketServer error");
/*
 *	Initializing gpio services 
 */
	ret = initSystemGpio();
	CHECK_RET(ret, "initSystemGpio error");
/*
 *	Initialize the log management module 
 */
	WRT_log_g = &CWRTLogManage::WRTLogManage();
	WRT_log_g->Init();
/*
 *	Register timing signal
 */
	signal(SIGALRM, rmVariableKeyTimer);
/*
 *	Get the gateway id and version
 */	
	ret = get_id_version();
	CHECK_RET(ret, "get_id_version error");	
/*
 *	Initial broadcast service
 */
	ret = initGroupBroadcast();
	CHECK_RET(ret, "initGroupBroadcast error");
/*
 *	Initialize finish ring
 */
	bellRingtimes();

	return 0;
}

/******************************************************************************/
/*
 *   Led status display, 500ms control cycle lights out
 */
//======================================================
//** 函数名称: led_status_ctrl
//** 功能描述: 运行灯状态控制
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================   
void led_status_ctrl()
{
	static int loop_times = 10;				/* Cycle times for light on */

	if (0 == uartGetIsSearching())
	{
		if (0 == --loop_times || READY_REBOOT_STATUS == run_status)
		{
			if (true == led_flag)
			{
				SetGpioVal(g_led_gpio, PULL_DOWN);//light on
				led_flag = false;		
			}
			else
			{
				SetGpioVal(g_led_gpio, PULL_UP);//light off
				led_flag = true;
			}
			loop_times = 10;
		}
	}
	else
		SetGpioVal(g_led_gpio, PULL_UP);
}

/******************************************************************************/
/*
 *   key controls gateway status
 */
//======================================================
//** 函数名称: key_status_ctrl
//** 功能描述: set按键状态控制
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//======================================================  
void key_status_ctrl()
{
	unsigned char key_val = 1;//Press output 0,default 1
	unsigned int interval = 0;//time difference
	
	GetGpioVal(g_key_gpio, &key_val);

	if (!key_val)
	{
/*
 *  	Stop searching, two rings
 */		 
		if (true == start_key && uartGetIsSearching())
		{
			bellRingtimes();
			bellRingtimes();
			uartSetIsSearching(0);
		}
		
		if (true == start_key)
		{
			key_up = time(NULL);
			start_key = false;
			WRTGateWayRate();
		}
		else
		{
			key_down = time(NULL);
			interval = key_down - key_up;
		}
	}
	else
	{
/*
 *  The flag States in a normal state
 */			
		start_key = true;
		run_status = NORMAL_STATUS;
		SetGpioVal(g_buzzer_gpio, PULL_DOWN);
	}

/*
 *  key status define, get key and restore factory
 */		
	if ((interval > GET_KEY_STATUS_TIME) && (run_status == NORMAL_STATUS))
	{
		bellRingtimes();	
		resetVariableKeyTimer();
		run_status = GET_KEY_STATUS;
	}
	else if ((interval > READY_REBOOT_TIME) && (run_status == GET_KEY_STATUS))
	{
		SetGpioVal(g_buzzer_gpio, PULL_UP);
		run_status = READY_REBOOT_STATUS;
	}
	else if ((interval > RESTORY_FACTORY_TIME) && (run_status == READY_REBOOT_STATUS))
	{
		SetGpioVal(g_buzzer_gpio, PULL_DOWN);
		reset_factory();
	}
	else
	{
	}
}

//======================================================
//** 函数名称: wdt_status_ctrl
//** 功能描述: watchdog状态控制
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
void wdt_status_ctrl()
{
	static int timeout = 100;
	static bool status = false;
	
	if (0 == --timeout) {
		if (false == status) 
		{
			SetGpioVal(g_wdt_gpio, PULL_DOWN);//wdt
			status = true;		
		}
		else
		{
			SetGpioVal(g_wdt_gpio, PULL_UP);//wdt
			status = false;
		}
		timeout = 100;
	}
}

//======================================================
//** 函数名称: FeedWatchDog
//** 功能描述: 喂watchdog
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
void FeedWatchDog()
{
	unsigned int timeout = 16000;	/* watchdog timeout 16000ms */
	static unsigned int refreshtime = 40;	/* feed watchdog time, 2s times*/

	/* Software wdt */	
	if (0 == --refreshtime)
	{
		refreshtime = 40;
		if (ioctl(g_hWatchDog, IOCTL_REFRESH_WATCHDOG, &timeout))
		{
			DEBUG_ERROR("ioctl IOCTL_REFRESH_WATCHDOG error\n");
		}	
	}
}

/******************************************************************************/
/*
 *  Gateway entry --- main
 */
//======================================================
//** 函数名称: main
//** 功能描述: WRT主函数入口
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月29日
//====================================================== 
int main(int argc, char *argv[])
{	
	DEBUG_MESSAGE("************* Wrt_main start *************\n");
	//sleep(5);
	//pthread_create_t(remoteControlInit);
	//while(1);
	Web_Init();
	sleep(8);
	if (WRT_gateWay_init())
	{
		DEBUG_ERROR("WRT_gateWay_init error!!!");
		exit(-1);
	}

	while (true)
	{	
		led_status_ctrl();
		key_status_ctrl();
		wdt_status_ctrl();
		FeedWatchDog();
		usleep(50*1000);
	}

	return 0;
}

