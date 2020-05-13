#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "roomlib.h"
#include <linux/input.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include "iotTimer.h"
#include "SmartConfig.h"

#define CLOCKID    CLOCK_REALTIME
#define SIG        SIGRTMIN

static void touch_timer(int a);

extern int Valid_Flag;
extern int KBD_FLAG;

int xydata[2];
BOOL curdown = FALSE;
int Touch_Flag = 0;
int MSG_NUM = 0;

int Touch_Valid = 0;        // 2018.1.8 为了优化，防止有时候点击屏幕的时候，变为滑屏
int Timer_Touch = 0;

/**********************定时函数******************/
static timer_t timerid = 0;
static int cbPar;
static timerCb cbFunction;  // 定义的是一个函数指针 
/************************************************/

static void TimeThread(union sigval v)  
{
	DPPostMessage(TIME_MESSAGE, 0, 0, 0, MSG_TIME_TYPE);   //貌似是隔一段时间发送一次时间消息。
}  
	
static void timerHandler(int sig, siginfo_t *si, void *uc)
{

	cbFunction( cbPar );
}

int timerStop( void )
{
	if ( timerid ) {

		 if (timer_delete(timerid) == -1) {

			return 0;
		 }	

		  timerid = 0;
	}
	printf("timerStop ok\n");

	return 1;
}

int timerStart( int msec,int interval, timerCb cb, int par )
{
	 cbFunction = cb;
	 cbPar      = par;

	 struct sigaction sa;             // 为信号安装处理程序(信号的安装登记)
	 sigset_t mask;                   // 信号集 
	 struct sigevent sev;             // 定时器到期产生的异步通知
	 struct itimerspec its;           // 定时时间结构体

	 if (timerid) {

		printf( "Timer: already busy\n" );
		return 0;
	 }

	 //printf("Establishing handler for signal %d\n", SIG);
	 sa.sa_flags = SA_SIGINFO;         // 设置标志位
	 sa.sa_sigaction = timerHandler;   // 信号处理函数
	 sigemptyset(&sa.sa_mask);

	 if(sigaction(SIG, &sa, NULL) == -1) {

		perror("sigaction");
		return 0;
	 }
	 
	 //printf("Blocking signal %d\n", SIG);
	 sigemptyset(&mask);
	 sigaddset(&mask, SIG);                             // 向mask信号集中添加SIG信号

	 if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {

		perror("sigprocmask");
		return 0;
	 }

	 sev.sigev_notify = SIGEV_SIGNAL;
	 sev.sigev_signo = SIG;
	 sev.sigev_value.sival_ptr = &timerid;

	 if (timer_create(CLOCKID, &sev, &timerid) == -1) {  //  所创建的定时器并未使用 

		perror("timer_create");
		return 0;
	 }
	
	 //printf("timer ID is 0x%lx\n", (long) timerid); 
	 its.it_value.tv_sec = 0;
	 its.it_value.tv_nsec = 90*1000*1000;
	 its.it_interval.tv_sec = interval / 1000;
	 its.it_interval.tv_nsec = ( interval % 1000 ) * 1000;

	 if (timer_settime(timerid, 0, &its, NULL) == -1) {

		perror("timer_settime");
		return 0;
	 }

	 //printf("Unblocking signal %d\n", SIG);
	 if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {

		perror("sigprocmask");
		return 0;
	 }
	 printf("timerStart ok \n");

	 return 1;
}


/*
 *函数名称:DPCreateTimeEvent(void)
 *功能:作为系统时间的发送线程，每秒钟发送一次时间信号
 */


void DPCreateTimeEvent(void)
{
	timer_t timeridd;  
	struct sigevent evp;  
	memset(&evp, 0, sizeof(struct sigevent));

	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = TimeThread;

	int ret = timer_create(CLOCK_REALTIME, &evp, &timeridd);

	if (ret)
		perror("DPCreateTimeEvent fail\r\n");

	else
	{
		struct itimerspec ts;  
		ts.it_interval.tv_sec = 1;
		ts.it_interval.tv_nsec = 0;
		ts.it_value.tv_sec = 1;
		ts.it_value.tv_nsec = 0;
		ret = timer_settime(timeridd, 0, &ts, NULL);
		if (ret)
			perror("timer_settime");
	}
}

static void touch_timer(int a)
{
	DPPostMessage(TOUCH_RAW_MESSAGE, xydata[0], xydata[1], TOUCH_UP, MSG_TOUCH_TYPE);
	curdown = FALSE;
	printf("%d %d up\n", xydata[0], xydata[1]);
}

static void* TouchEvent(void* pParam)
{
	int fd;
	int count = 0;
	static int xyvalue[2];
	int flag = 0;
	struct input_event buf;
	//static BOOL curdown = FALSE;
    
	int xflag = 0;
	int yflag = 0;

	if(GetSwitch(SWITCH_KEY))
		fd = open("/dev/input/event1", O_RDONLY);
	else
		fd = open("/dev/input/event0", O_RDONLY);

	if (fd < 0) {
		printf("open rtp event fail!\n");
		return 0;
	}
	
	int ret = read(fd, &buf, sizeof(struct input_event));
	
	while (1)
	{
		if (buf.type == EV_REL)		// #define EV_REL			0x02
		{
			if (buf.code == REL_X)	// #define REL_X			0x00
			{
				if(yflag == 0)  xflag = 1;
				
				xydata[0] = buf.value;
			}

			if(xflag == 1) {

				if (buf.code == REL_Y)	// #define REL_Y	    0x01
				{
					if(xflag == 1)	yflag = 1;
						
					xydata[1] = buf.value;			
				}
			}
		}
		
		if(xflag == 1 && yflag == 1) {

			timerStop();
			timerStart(2000,0,touch_timer,0);
			
			if (curdown)
			{
				xflag = 0;
			   	yflag = 0;		
				DPPostMessage(TOUCH_RAW_MESSAGE, xydata[0], xydata[1], TOUCH_VALID, MSG_TOUCH_TYPE);			
				printf("valid\n");
			}

			else            //处理关于滑动时有点触碰的问题。
			{
			   xflag = 0;
			   yflag = 0;
			   DPPostMessage(TOUCH_RAW_MESSAGE, xydata[0], xydata[1], TOUCH_DOWN, MSG_TOUCH_TYPE);
			   DPPostMessage(TOUCH_RAW_MESSAGE, xydata[0], xydata[1], TOUCH_VALID, MSG_TOUCH_TYPE);
			   curdown = TRUE;		   
			   printf("down\n");
			}
		}

		ret = read(fd, &buf, sizeof(struct input_event));
		
		printf("TP %d %d %d\r\n", buf.type, buf.code, buf.value);
	}
	close(fd);
	return NULL;
}

void DPCreateTouchEvent()
{
	pthread_t pid0;
	pthread_create(&pid0, NULL, TouchEvent, NULL);
}


void DPCreateTimer()
{

	pthread_t pid0;
	pthread_create(&pid0, NULL, MSG_TIMR, NULL);
}
static void* KeybdThread(void* pParam)
{
	int fd;
	struct input_event buf;

	fd = open("/dev/input/event0", O_RDONLY);
	if(fd < 0)
	{
		printf("open key event fail!\n");
		return NULL;
	}

	while(read(fd, &buf, sizeof(struct input_event)))
	{
		if(buf.type == 1)
		{
			if(buf.value == 1)
			{
				DPPostMessage(HARDKBD_MESSAGE, KBD_DOWN, buf.code, 0, MSG_KEY_TYPE);
			}
			else if(buf.value == 0)
			{
				DPPostMessage(HARDKBD_MESSAGE, KBD_UP, buf.code, 0, MSG_KEY_TYPE);
			}
		}
	}
	close(fd);
	return NULL;
}

void DPCreateKeyEvent()
{
	if(GetSwitch(SWITCH_KEY))
	{
		pthread_t pid0;
		pthread_create(&pid0, NULL, KeybdThread, NULL);
	}
}

//*************************************************************************
//**************** 错误回溯 ***********************************************
//*************************************************************************
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
static void fatal_signal_handler(int signum)
{
	void *buffer[100] = { NULL };
	char **trace = NULL;
	int size = backtrace(buffer, 100);
	trace = backtrace_symbols(buffer, size);
	if (NULL == trace) {
		return;
	}

	size_t name_size = 100;
	char *name = (char*)malloc(name_size);
	for (int i = 0; i < size; ++i) {
		char *begin_name = 0;
		char *begin_offset = 0;
		char *end_offset = 0;
		for (char *p = trace[i]; *p; ++p) { // 利用了符号信息的格式
			if (*p == '(') { // 左括号
				begin_name = p;
			}           
			else if (*p == '+' && begin_name) { // 地址偏移符号
				begin_offset = p;
			}
			else if (*p == ')' && begin_offset) { // 右括号
				end_offset = p;
				break;
			}
		}
		if (begin_name && begin_offset && end_offset ) {
			*begin_name++ = '\0';
			*begin_offset++ = '\0';
			*end_offset = '\0';
			int status = -4; // 0 -1 -2 -3
			char *ret = abi::__cxa_demangle(begin_name, name, &name_size, &status);
			if (0 == status) {
				name = ret;
				printf("%s:%s+%s\n", trace[i], name, begin_offset);
			}
			else {
				printf("%s:%s()+%s\n", trace[i], begin_name, begin_offset);
			}
		}
		else {
			printf("%s\n", trace[i]);
		}
	}
	free(name);
	free(trace);
	exit(1);
}

void DPBacktrace()
{
	signal(SIGSEGV, fatal_signal_handler);
	signal(SIGBUS,  fatal_signal_handler);
	signal(SIGILL,  fatal_signal_handler);
}
