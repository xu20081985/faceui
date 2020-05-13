#pragma once

enum
{
	FRAME_WIDTH,				// 背景宽度
	FRAME_HEIGHT,				// 背景高度
	FRAME_TIP_WIDTH,			// 提示框宽度
	FRAME_TIP_HEIGHT,			// 提示框高度
	LIGHT_PNG_TOP,				// 灯光控制成功，图标的高度
	LIGHT_TEXT_TOP,				// 灯光控制成功，字体的高度
	LIGHT_TEXT_PNG_TOP,			// 灯光控制成功，字体后面笑脸的高度
	DEV_FAIL_PNG_TOP,			// 设备控制失败提示，哭脸的高度
	DEV_FAIL_TEXT_TOP,			// 设备控制失败提示，字体的高度
	SCENE_FAIL_PNG_TOP,			// 场景控制失败提示，哭脸的高度
	SCENE_FAIL_TEXT_TOP1,		// 场景控制失败提示，第一行字的高度
	SCENE_FAIL_TEXT_TOP2,		// 场景控制失败提示，第二行字的高度
	COLOR_TEXT_NORMAL,			// 字体正常颜色
	COLOR_TEXT_FOCUS,			// 字体按下颜色
	TIMER_SELECT_TOP,			// 定时设置勾选高度
	TIMER_SELECT_INTERVAL,		// 定时设置勾选间隔
	FRAME_MAX
};

enum
{
	SWITCH_CALIBRATE,
	SWITCH_KEY,
	SWITCH_MAX
};

enum
{
	TS_BUTTON,
	TS_EDITBOX,
	TS_KEYBOARD,
	TS_KEYBOARD_P,
	TS_PAGE,
	TS_TITLE,
	TS_MKEYBOARD,
	TS_PROGRESS,
	TS_STATIC,
	TS_TEXT,
	TS_TIME,
	TS_DATE,
	TS_WEEK,
	TEXT_SIZE_MAX
};

enum
{
	KEY_RING_MUTE,
	KEY_VALUE_MAX
};

void InitConfig(void);
BOOL GetSwitch(DWORD index);
int GetUIConfig(DWORD index);
int GetKeyConfig(DWORD index);
DWORD GetTextSize(DWORD index);
