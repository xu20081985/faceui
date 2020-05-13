#pragma once

typedef struct
{
	DWORD x;
	DWORD y;
	DWORD flag;
} TOUCHDATA;

typedef struct
{
	DWORD key;
	DWORD flag;
} KBDDATA;

typedef struct
{
	DWORD msg;
	DWORD wParam;
	DWORD lParam;
	DWORD zParam;
} SYS_MSG;

#include "appid.h"
#define	MSG_URGENT_TYPE			0
#define	MSG_USER_TYPE			1
#define	MSG_TIME_TYPE			2
#define	MSG_TOUCH_TYPE			3
#define	MSG_KEY_TYPE			4
#define	MSG_MAX_TYPE			5

#define	TIME_MESSAGE			10000		// 时间消息

#define HARDKBD_MESSAGE  		10001		// 按键的原始消息
#define	KBD_MESSAGE				10002
#define	KBD_DOWN				0
#define	KBD_UP					1
#define KBD_CTRL				2			// 键盘控件消息

#define TOUCH_RAW_MESSAGE		10003		// 触摸屏的原始消息，用于屏幕校正或黑屏
#define TOUCH_DOWN				0			// 某个按键被按下
#define TOUCH_VALID				1			// 某个按键按下后移动触摸点
#define TOUCH_UP				2			// 被按下的按键被释放
#define	TOUCH_MOVEOUT			3			// 被按下的按键被移出

#define	TOUCH_MESSAGE			10004		// 某个控件被触发的消息
#define TOUCH_ACTIVE			12345
#define TOUCH_SLIDE				100041		// 滑屏消息
#define SLIDE_LEFT				0			// 左滑
#define SLIDE_RIGHT				1			// 右滑
#define SLIDE_UPSIDE			2			// 上滑
#define SLIDE_DOWN				3			// 下滑

#define	MSG_SYSTEM				10005		// 系统消息
#define	REBOOT_MACH				1			// 重启终端
#define	UPDATE_NETCFG			2			// 更新配置
#define	CODE_CHANGE				3			// 号码变化
#define	RESET_MACH				4			// 恢复出厂设置
#define WATCHDOG_CHANGE			5			// 是否开始看门狗
#define TIME_CHANGE				6			// 时间变化
#define RESET_ZIGBEE			7			// 初始化ZIGBEE

#define	MSG_BROADCAST			10006		// 广播消息，当前存在的所有窗口都可以收到
#define NETWORK_CHANGE			1			// 网络状态变化
#define SMART_STATUS_ACK		2			// 器件收到控制命令后的回复
#define SMART_STATUS_SCENE		3			// 情景控制回复
#define SMART_STATUS_SYNC		4			// 器件主动上报状态
#define SMART_STATUS_GET		5			// 主动获取开关状态
#define SMART_LIGHT_STUDY		6			// 灯光通路学习


#define	MSG_PRIVATE				10007		// 私有消息，发送给指定的窗口, wparam为窗口的id

#define	MSG_START_APP			10020		// 启动新的窗口
#define	MSG_END_APP				10021		// 结束指定的窗口
#define	MSG_START_FROM_ROOT		10022		// 启动指定的窗口，并结束当前所有的窗口
#define	MSG_EXTSTART_APP		10023		// 其他线程往窗口线程发送的窗口启动消息
#define	MSG_START_FROM_OVER		10024		// 启动指定的窗口，叠加到当前窗口上
#define	MSG_END_OVER_APP		10025		// 结束指定的窗口, 并恢复之前的窗口

void InitSysMessage(void);
BOOL DPPostMessage(DWORD msgtype, DWORD wParam, DWORD lParam, DWORD zParam, DWORD type = 1);
DWORD DPGetMessage(SYS_MSG* msg);
void CleanUserInput(void);
