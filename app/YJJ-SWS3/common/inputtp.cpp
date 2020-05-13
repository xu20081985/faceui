
#include "roomlib.h"
#include "input.h"
#include <sys/time.h>
#define	MAX_MAP				10
const char CALIBRATE_FILE[] = "/UserDev/PenCalib";
int Valid_Flag = 0;         // 滑动标志位

int DIMMER_VALID_FLAG = 0;  // 用于各种进度条的滑动判断标志位

extern int KBD_FLAG;
extern int g_time;

extern int Touch_Valid;     // 2018.1.8添加，优化解决点击变滑屏问题

extern int IR_AIR_FLAG;     // 2018.1.10添加，用于优化二级界面的触摸效果
extern int AIR_FLAG;        // 2018.1.11添加，用于优化二级界面的触摸效果
extern int HEAT_FLAG;		// 2018.1.11添加，用于优化二级界面的触摸效果
extern int TV_FLAG;         // 2018.1.11添加，用于优化二级界面的触摸效果
extern int CURTAIN_FLAG;    // 2018.1.15添加，用于优化二级界面的触摸效果
extern int MUSIC_FALG;		// 2018.1.15添加，用于优化二级界面的触摸效果
extern int mainnn;  
extern int PWD_FLAG;        // 2018.1.24添加，用于优化二级界面的触摸效果 
extern int SET_PWD;			// 2018.1.24添加，用于优化二级界面的触摸效果 
DWORD  u_sec;               // 2018.1.22添加，用于优化滑屏效果
extern int TIMER_TIME;

int slide_flag = 0;
int point_flag = 0;
extern int count_notouch;
extern int main_cpp;

typedef struct
{
	EventMap* pevent;
	DWORD count;
	DWORD layer;
} EventList;

BOOL DoCtrlOp(EventMap* pEvent, DWORD x, DWORD y, DWORD statue);
static BOOL m_bTouchDirect = FALSE;				// 是否直接输出才触摸屏的值，在进行屏幕校准的时候需要设置为TRUE
static int CalibrateParam[6];					// 触摸屏校准参数
static DWORD m_TotalEvent = 0;					// 当前注册的触摸屏响应事件个数
static EventList m_lEventMapList[MAX_MAP];		// 当前注册的触摸屏响应事件内容
static BOOL m_bTouchPressed;					// 当前触摸屏是否被按下
static EventMap m_bLastEventPtr;		        // 上次响应的事件,在触摸屏弹起后被设置为0xffffffff
static DWORD m_dwLastXoff;
static DWORD m_dwLastYoff;

static DWORD cacheval[16];
static DWORD cachetotal;
static DWORD cachewptr;
static DWORD cacherptr;
static DWORD dropbegin;					        // 丢弃掉down开始的4个值
static DWORD g_dropbegin = 0;                   // 丢弃掉down开始的4个值
// 滑屏消息处理
const int OFFSET_LEFT	= -25;
const int OFFSET_RIGHT	= 25;
const int OFFSET_UPSIDE	= -25;
const int OFFSET_DOWN	= 25;

static int g_dwLastXoff;
static int g_dwLastYoff;
static BOOL g_bTouchPressed;

//======================================================
//** 函数名称: InitTouchCalibrate
//** 功能描述: 初始化触摸校准
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
BOOL InitTouchCalibrate(void)
{
	if(!GetSwitch(SWITCH_CALIBRATE))
		return TRUE;

	FILE* fd;

	fd = fopen(CALIBRATE_FILE, "rb");
	if(fd == NULL)
		return FALSE;
	if(fread(CalibrateParam, 1, 24, fd) != 24)
	{
		fclose(fd);
		return FALSE;
	}
	fclose(fd);
	return TRUE;
}

//======================================================
//** 函数名称: SaveTouchCalibrate
//** 功能描述: 保存触摸校准
//** 输　入: val
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void SaveTouchCalibrate(int *val)
{
	FILE* fd;

	fd = fopen(CALIBRATE_FILE, "wb");
	if(fd == NULL)
		return;
	memcpy(CalibrateParam, val, 24);
	fwrite(CalibrateParam, 1, 24, fd);
	fclose(fd);
}

//======================================================
//** 函数名称: SetTouchDirect
//** 功能描述: 设置是否触摸校准
//** 输　入: val
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void SetTouchDirect(BOOL val)
{
	m_bTouchDirect = val;
}

//======================================================
//** 函数名称: RegEventRegion
//** 功能描述: 注册事件注册
//** 输　入: eventcout pMap layer
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
void RegEventRegion(DWORD eventcout, EventMap* pMap, DWORD layer)
{
	DWORD i;

	if(m_TotalEvent < MAX_MAP)
	{
		for(i = 0; i < m_TotalEvent; i++)
		{
			if((m_lEventMapList[i].pevent == pMap)
				&& (m_lEventMapList[i].count == eventcout))
				return;
		}
		for(i = 0; i < m_TotalEvent; i++)
		{
			if(m_lEventMapList[i].layer < layer)
				break;
		}
		memmove(&m_lEventMapList[i + 1], &m_lEventMapList[i], (m_TotalEvent - i) * sizeof(EventList));
		m_lEventMapList[i].pevent = pMap;
		m_lEventMapList[i].count = eventcout;
		m_lEventMapList[i].layer = layer;
		m_TotalEvent++;
	}
}

//======================================================
//** 函数名称: UnRegEventRegion
//** 功能描述: 反注册事件注册
//** 输　入: eventcout pMap
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void UnRegEventRegion(DWORD eventcout, EventMap* pMap)
{
	DWORD i, j;

	DBGMSG(DISP_MOD, "UnRegEventRegion %d %p\r\n", eventcout, pMap);
	for(i = 0; i < m_TotalEvent; i++)
	{
		if((m_lEventMapList[i].pevent == pMap)
			&& (m_lEventMapList[i].count == eventcout))
		{
			for(j = 0; j < eventcout; j++)
			{
				if(pMap[j].pCtrl == m_bLastEventPtr.pCtrl)
					m_bLastEventPtr.pCtrl = NULL;
			}
			memmove(&m_lEventMapList[i], &m_lEventMapList[i + 1], (m_TotalEvent - 1) * sizeof(EventList));
			m_TotalEvent -= 1;
			DBGMSG(DISP_MOD, "UnRegEventRegion %d\r\n", m_TotalEvent);
			return;
		}
	}
	DBGMSG(DISP_MOD, "UnRegEventRegion Dont find %d %p\r\n", eventcout, pMap);
}

//======================================================
//** 函数名称: ClrEventRegion
//** 功能描述: 清理事件注册
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
void ClrEventRegion(void)
{
	m_TotalEvent = 0;
	int i =0;
	memset(m_lEventMapList, 0, sizeof(EventList) * MAX_MAP);
	DBGMSG(DISP_MOD, "ClrEventRegion\r\n");
}

//======================================================
//** 函数名称: TouchEventRemap
//** 功能描述: 触摸事件处理
//** 输　入: pmsg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
BOOL TouchEventRemap(SYS_MSG* pmsg)
{
	int calx, caly;
	DWORD i, j;
	DBGMSG(DISP_MOD, "Get point %d %d %d %d\r\n", DPGetTickCount(), pmsg->wParam, pmsg->lParam, pmsg->zParam);

	if(m_bTouchDirect)
	{
		if(pmsg->zParam == TOUCH_UP)
			PlayWav(KEYPAD_INDEX, GetRingVol());

		DPPostMessage(TOUCH_RAW_MESSAGE, pmsg->wParam, pmsg->lParam, pmsg->zParam);
		return FALSE;
	}
	else
		pmsg->msg = TOUCH_MESSAGE;

#if 0

	if(pmsg->zParam == TOUCH_DOWN)	// 第一个抓到的点将被扔掉，仅修改状态, 某些电容屏会受干扰，会接收到1个点的信息
	{
		m_bTouchPressed = TRUE;
		m_bLastEventPtr.pCtrl = NULL;

		g_bTouchPressed = TRUE;
		g_dwLastXoff = pmsg->wParam;
		g_dwLastYoff = pmsg->lParam;
	}
#endif
	if(IR_AIR_FLAG == 0 && AIR_FLAG == 0 && HEAT_FLAG == 0 && TV_FLAG == 0 && CURTAIN_FLAG == 0 
		&& MUSIC_FALG == 0 && PWD_FLAG == 0 && SET_PWD == 0 && DIMMER_VALID_FLAG == 0 && main_cpp == 0) {
	
	 	if((pmsg->zParam == TOUCH_UP || pmsg->zParam == TOUCH_VALID))
		 {

	/*		 if(DIMMER_VALID_FLAG == 1) {
			
				m_bTouchPressed = TRUE;       // 按下标志位
				m_bLastEventPtr.pCtrl = NULL;
				g_dwLastXoff = pmsg->wParam;
				g_dwLastYoff = pmsg->lParam;
			 }
    */
			if(g_bTouchPressed)
			{
				// 先判断是否为滑屏消息
				
				int	Xoff = pmsg->wParam - g_dwLastXoff;
				int	Yoff = pmsg->lParam - g_dwLastYoff;
					
				if(Xoff < OFFSET_LEFT && KBD_FLAG == 0 )
				{
					Valid_Flag = 1;
					
					if(DIMMER_VALID_FLAG == 0) {

						DPPostMessage(TOUCH_SLIDE, SLIDE_LEFT, 0, 0);	
						g_bTouchPressed = FALSE;
										
					}
					
				}
				
				else if(Xoff > OFFSET_RIGHT && KBD_FLAG == 0)
				{
					Valid_Flag = 1;

					if(DIMMER_VALID_FLAG == 0) {

						DPPostMessage(TOUCH_SLIDE, SLIDE_RIGHT, 0, 0);
						
						g_bTouchPressed = FALSE;
															
					}
					
				//	else if(DIMMER_VALID_FLAG == 1)

				//		DPPostMessage(TOUCH_SLIDE, SLIDE_RIGHT, 0, 0);
				}
				
				else if(Yoff < OFFSET_UPSIDE && KBD_FLAG == 0)
				{
					
					if(DIMMER_VALID_FLAG == 0) {

						Valid_Flag = 1;
						g_bTouchPressed = FALSE;
						DPPostMessage(TOUCH_SLIDE, SLIDE_UPSIDE, 0, 0);
					}							
				}
				
				else if(Yoff > OFFSET_DOWN && KBD_FLAG == 0)
				{
							
					if(DIMMER_VALID_FLAG == 0) {

						Valid_Flag = 1;
						g_bTouchPressed = FALSE;
						DPPostMessage(TOUCH_SLIDE, SLIDE_DOWN, 0, 0);
					}
				}
				else {

			//		DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_DOWN);
					Valid_Flag = 2;
			//		point_flag = 0;
				}				
			}
		}                                                              
	
    	if((m_bTouchPressed /*&& pmsg->zParam == TOUCH_UP */&& Valid_Flag == 2))
		{

			int Xoff = pmsg->wParam - g_dwLastXoff;
			int Yoff = pmsg->lParam - g_dwLastYoff;

			if(Xoff>-3 && Xoff<3 && Yoff>-3 && Yoff<3)	{
				calx = pmsg->wParam;
				caly = pmsg->lParam;
		
			if(m_bLastEventPtr.pCtrl == NULL)
			{
				for(i = 0; i < m_TotalEvent; i++)
				{
					for(j = 0; j < m_lEventMapList[i].count; j++)
					{
						if((calx >  m_lEventMapList[i].pevent[j].xStart)
							&& (calx < m_lEventMapList[i].pevent[j].xEnd) 
							&& (caly >  m_lEventMapList[i].pevent[j].yStart)
							&& (caly <  m_lEventMapList[i].pevent[j].yEnd))
						{
							memcpy(&m_bLastEventPtr, &m_lEventMapList[i].pevent[j], sizeof(EventMap));
							break;
						}
					}
					if(m_bLastEventPtr.pCtrl != NULL)
						break;
				}
				if(m_bLastEventPtr.pCtrl != NULL)
				{
					m_dwLastXoff = calx - m_bLastEventPtr.xStart;
					m_dwLastYoff = caly - m_bLastEventPtr.yStart;
					if(DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_DOWN))
					{
						//if(GetOnOff(SWITCH_KEYTONE))
							PlayWav(KEYPAD_INDEX, GetRingVol());
					}
				}
				else
				{
					m_bTouchPressed = FALSE;
				}
			}

			if(((m_bTouchPressed)
				&& (m_bLastEventPtr.pCtrl != NULL))&& pmsg->zParam == TOUCH_UP)
			{
				m_bTouchPressed = FALSE;
				Valid_Flag = 0;
			    KBD_FLAG = 0;
				point_flag = 0;		
				DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_UP);
				
			}
		 }	
	  }

		if(pmsg->zParam == TOUCH_DOWN)	  // 第一个抓到的点将被扔掉，仅修改状态, 某些电容屏会受干扰，会接收到1个点的信息
		{
#if 1

			struct timeval tv;
			gettimeofday(&tv,NULL);	
			u_sec = tv.tv_sec;
#endif

			m_bTouchPressed = TRUE;       // 按下标志位
			m_bLastEventPtr.pCtrl = NULL;

			g_bTouchPressed = TRUE;       // 滑动标志位
			g_dwLastXoff = pmsg->wParam;
			g_dwLastYoff = pmsg->lParam;
		}

#if 1
		if(m_bTouchPressed  ==  TRUE && g_bTouchPressed == TRUE) {

			 struct timeval tv;
			 gettimeofday(&tv,NULL);

			 if((u_sec - tv.tv_sec) > 2 && DIMMER_VALID_FLAG == 0) {

				g_bTouchPressed = FALSE;
				Valid_Flag = 2;
				DPPostMessage(TOUCH_RAW_MESSAGE, g_dwLastXoff, g_dwLastYoff, TOUCH_UP, MSG_TOUCH_TYPE);

			}

			 else if((u_sec - tv.tv_sec) <= 2 && DIMMER_VALID_FLAG == 0) {

				DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_DOWN);
			 }
		}  
#endif
	}

	else if(TIMER_TIME == 1 || IR_AIR_FLAG == 1 || HEAT_FLAG == 1 || AIR_FLAG == 1 || TV_FLAG == 1 || CURTAIN_FLAG == 1 
			|| MUSIC_FALG ==1 || PWD_FLAG == 1 || SET_PWD == 1 || DIMMER_VALID_FLAG == 1 || main_cpp == 1) {

	if(pmsg->zParam == TOUCH_DOWN)	// 第一个抓到的点将被扔掉，仅修改状态, 某些电容屏会受干扰，会接收到1个点的信息
	{
		m_bTouchPressed = TRUE;
		m_bLastEventPtr.pCtrl = NULL;

		g_bTouchPressed = TRUE;
		g_dwLastXoff = pmsg->wParam;
		g_dwLastYoff = pmsg->lParam;
	}

	else if((pmsg->zParam == TOUCH_UP))
	{
		if(g_bTouchPressed)
		{
			// 先判断是否为滑屏消息
			int Xoff = pmsg->wParam - g_dwLastXoff;
			int Yoff = pmsg->lParam - g_dwLastYoff;

			if(Xoff < OFFSET_LEFT)
			{
				if (m_bLastEventPtr.pCtrl != NULL)
					DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_UP);
				DPPostMessage(TOUCH_SLIDE, SLIDE_LEFT, 0, 0);
			}
			else if(Xoff > OFFSET_RIGHT)
			{
				if (m_bLastEventPtr.pCtrl != NULL)
					DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_UP);
				DPPostMessage(TOUCH_SLIDE, SLIDE_RIGHT, 0, 0);
			}
			else if(Yoff < OFFSET_UPSIDE)
			{
				if (m_bLastEventPtr.pCtrl != NULL)
					DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_UP);				
				DPPostMessage(TOUCH_SLIDE, SLIDE_UPSIDE, 0, 0);
			}
			else if(Yoff > OFFSET_DOWN)
			{
				if (m_bLastEventPtr.pCtrl != NULL)
					DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_UP);				
				DPPostMessage(TOUCH_SLIDE, SLIDE_DOWN, 0, 0);
			}
			else
			{
				if((m_bTouchPressed)
					&& (m_bLastEventPtr.pCtrl != NULL))
				{
					m_bTouchPressed = FALSE;

					if(slide_flag == 0)
						DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_UP);
					else if(slide_flag == 1)
						slide_flag = 0;
				}
			}

			g_bTouchPressed = FALSE;
		}
	}

	else if(m_bTouchPressed)
	{
		calx = pmsg->wParam;
		caly = pmsg->lParam;

		if(m_bLastEventPtr.pCtrl == NULL)
		{
			for(i = 0; i < m_TotalEvent; i++)
			{
				for(j = 0; j < m_lEventMapList[i].count; j++)
				{
					if((calx >=	m_lEventMapList[i].pevent[j].xStart)
						&& (calx <= m_lEventMapList[i].pevent[j].xEnd) 
						&& (caly >=	m_lEventMapList[i].pevent[j].yStart)
						&& (caly <=	m_lEventMapList[i].pevent[j].yEnd))
					{
						memcpy(&m_bLastEventPtr, &m_lEventMapList[i].pevent[j], sizeof(EventMap));
						break;
					}
				}
				if(m_bLastEventPtr.pCtrl != NULL)
					break;
			}
			if(m_bLastEventPtr.pCtrl != NULL)
			{
				m_dwLastXoff = calx - m_bLastEventPtr.xStart;
				m_dwLastYoff = caly - m_bLastEventPtr.yStart;
				if(DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_DOWN))
				{
					//if(GetOnOff(SWITCH_KEYTONE))
						PlayWav(KEYPAD_INDEX, GetRingVol());
						m_bLastEventPtr.pCtrl == NULL;
				}
			}
			else
			{
				m_bTouchPressed = FALSE;
			}
		}
		else
		{
			if((calx >=	m_bLastEventPtr.xStart)
				&& (calx <= m_bLastEventPtr.xEnd) 
				&& (caly >=	m_bLastEventPtr.yStart)
				&& (caly <=	m_bLastEventPtr.yEnd))
			{
				m_dwLastXoff = calx - m_bLastEventPtr.xStart;
				m_dwLastYoff = caly - m_bLastEventPtr.yStart;
				DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_VALID);
			}
			else
			{
				DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_MOVEOUT);
				m_bLastEventPtr.pCtrl = NULL;
				m_bTouchPressed = FALSE;
			}
		}
	}

	 if((pmsg->zParam == TOUCH_VALID)) {

	/*	if(g_bTouchPressed)
		{
			// 先判断是否为滑屏消息
			int Xoff = pmsg->wParam - g_dwLastXoff;
			int Yoff = pmsg->lParam - g_dwLastYoff;

			if(Xoff < OFFSET_LEFT)
			{
				DPPostMessage(TOUCH_SLIDE, SLIDE_LEFT, 0, 0);
			}
			else if(Xoff > OFFSET_RIGHT)
			{
				DPPostMessage(TOUCH_SLIDE, SLIDE_RIGHT, 0, 0);
			}
			else if(Yoff < OFFSET_UPSIDE)
			{
				DPPostMessage(TOUCH_SLIDE, SLIDE_UPSIDE, 0, 0);
			}
			else if(Yoff > OFFSET_DOWN)
			{
				DPPostMessage(TOUCH_SLIDE, SLIDE_DOWN, 0, 0);
			}
		} */

		DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_SLIDE );

	  }

		
	}
	return FALSE;	 	
}

//======================================================
//** 函数名称: TouchEventRemapEx
//** 功能描述: 触摸事件处理
//** 输　入: pmsg
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//====================================================== 
BOOL TouchEventRemapEx(SYS_MSG* pmsg)
{
	int calx, caly;
	DWORD i, j;
	DBGMSG(DISP_MOD, "Get point %d %d %d %d\r\n", DPGetTickCount(), pmsg->wParam, pmsg->lParam, pmsg->zParam);

	if(m_bTouchDirect)
	{
		if(pmsg->zParam == TOUCH_UP)
			PlayWav(KEYPAD_INDEX, GetRingVol());

		DPPostMessage(TOUCH_RAW_MESSAGE, pmsg->wParam, pmsg->lParam, pmsg->zParam);
		return FALSE;
	}
	else
		pmsg->msg = TOUCH_MESSAGE;

	if(pmsg->zParam == TOUCH_DOWN)	// 第一个抓到的点将被扔掉，仅修改状态, 某些电容屏会受干扰，会接收到1个点的信息
	{
		cachetotal = 0;
		dropbegin = 0;
		cacherptr = 0;
		cachewptr = 0;
		m_bTouchPressed = TRUE;
		m_bLastEventPtr.pCtrl = NULL;
	}
	else if(pmsg->zParam == TOUCH_UP)
	{
		if((m_bTouchPressed)
			&& (m_bLastEventPtr.pCtrl != NULL))
		{
			m_bTouchPressed = FALSE;
			DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_UP);
		}
	}
	else if(m_bTouchPressed)
	{
		dropbegin++;
		if(dropbegin < 4)
			return FALSE;

		calx = (CalibrateParam[1] * pmsg->lParam+ CalibrateParam[2] * pmsg->wParam + CalibrateParam[0]) >> 16;
		caly = (CalibrateParam[4] * pmsg->lParam + CalibrateParam[5] * pmsg->wParam + CalibrateParam[3]) >> 16;
		cacheval[cachewptr++] = calx;
		cacheval[cachewptr++] = caly;
		cachewptr &= 0xf;
		cachetotal++;
		if(cachetotal < 4)
			return FALSE;
		calx = cacheval[cacherptr++];
		caly = cacheval[cacherptr++];
		cacherptr &= 0xf;
		if(m_bLastEventPtr.pCtrl == NULL)
		{
			for(i = 0; i < m_TotalEvent; i++)
			{
				for(j = 0; j < m_lEventMapList[i].count; j++)
				{
					if((calx >  m_lEventMapList[i].pevent[j].xStart)
						&& (calx < m_lEventMapList[i].pevent[j].xEnd) 
						&& (caly >  m_lEventMapList[i].pevent[j].yStart)
						&& (caly <  m_lEventMapList[i].pevent[j].yEnd))
					{
						memcpy(&m_bLastEventPtr, &m_lEventMapList[i].pevent[j], sizeof(EventMap));
						break;
					}
				}
				if(m_bLastEventPtr.pCtrl != NULL)
					break;
			}
			if(m_bLastEventPtr.pCtrl != NULL)
			{
				m_dwLastXoff = calx - m_bLastEventPtr.xStart;
				m_dwLastYoff = caly - m_bLastEventPtr.yStart;
				if(DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_DOWN))
				{
					//if(GetOnOff(SWITCH_KEYTONE))
					PlayWav(KEYPAD_INDEX, GetRingVol());
				}
			}
			else
			{
				m_bTouchPressed = FALSE;
			}
		}
		else
		{
			if((calx >	m_bLastEventPtr.xStart)
				&& (calx < m_bLastEventPtr.xEnd) 
				&& (caly >	m_bLastEventPtr.yStart)
				&& (caly <	m_bLastEventPtr.yEnd))
			{
				m_dwLastXoff = calx - m_bLastEventPtr.xStart;
				m_dwLastYoff = caly - m_bLastEventPtr.yStart;
				DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_VALID);
			}
			else
			{
				DoCtrlOp(&m_bLastEventPtr, m_dwLastXoff, m_dwLastYoff, TOUCH_MOVEOUT);
				m_bLastEventPtr.pCtrl = NULL;
				m_bTouchPressed = FALSE;
			}
		}
	}
	return FALSE;
}

  
