#pragma once

enum
{
	FRAME_WIDTH,				// �������
	FRAME_HEIGHT,				// �����߶�
	FRAME_TIP_WIDTH,			// ��ʾ����
	FRAME_TIP_HEIGHT,			// ��ʾ��߶�
	LIGHT_PNG_TOP,				// �ƹ���Ƴɹ���ͼ��ĸ߶�
	LIGHT_TEXT_TOP,				// �ƹ���Ƴɹ�������ĸ߶�
	LIGHT_TEXT_PNG_TOP,			// �ƹ���Ƴɹ����������Ц���ĸ߶�
	DEV_FAIL_PNG_TOP,			// �豸����ʧ����ʾ�������ĸ߶�
	DEV_FAIL_TEXT_TOP,			// �豸����ʧ����ʾ������ĸ߶�
	SCENE_FAIL_PNG_TOP,			// ��������ʧ����ʾ�������ĸ߶�
	SCENE_FAIL_TEXT_TOP1,		// ��������ʧ����ʾ����һ���ֵĸ߶�
	SCENE_FAIL_TEXT_TOP2,		// ��������ʧ����ʾ���ڶ����ֵĸ߶�
	COLOR_TEXT_NORMAL,			// ����������ɫ
	COLOR_TEXT_FOCUS,			// ���尴����ɫ
	TIMER_SELECT_TOP,			// ��ʱ���ù�ѡ�߶�
	TIMER_SELECT_INTERVAL,		// ��ʱ���ù�ѡ���
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
