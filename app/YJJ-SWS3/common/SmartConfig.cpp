#include "roomlib.h"
#include "SmartConfig.h"

static char g_szPng[64];
static char g_ofPng[64];

//======================================================
//** 函数名称: GetSmartPngMusic
//** 功能描述: 获取家居音乐图片
//** 输　入: status
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
char* GetSmartPngMusic(DWORD status)
{
	if(status)
		sprintf(g_szPng, "goon.png");
	else 
		sprintf(g_szPng, "pause.png");

	return g_szPng;
}

//======================================================
//** 函数名称: GetSmartPngOnOff
//** 功能描述: 获取家居开关图片
//** 输　入: status
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
char* GetSmartPngOnOff(DWORD status) 
{

	if(status == 0)
		sprintf(g_ofPng, "onoff.png");
	else
		sprintf(g_ofPng, "guanji.png");

	return g_ofPng;
}

//======================================================
//** 函数名称: GetSmartPngPoint
//** 功能描述: 获取家居翻页点图片
//** 输　入: status
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
char* GetSmartPngPoint(DWORD status)
{

	if(status)
		sprintf(g_szPng, "00.png");

	else 
		
		sprintf(g_szPng, "10.png");
	return g_szPng;
}

//======================================================
//** 函数名称: GetSmartPng
//** 功能描述: 获取家居图片
//** 输　入: type status
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2019年01月28日
//======================================================
char* GetSmartPng(DWORD type, DWORD status)
{
	
	switch(type)
	{    //  灯光
	case ST_LIGHT_A:
	case ST_LIGHT_B:
	case ST_LIGHT_C:
	case ST_LIGHT_D:
		if(status == 0)  // 没有被按下
			sprintf(g_szPng, "mainlight.png", type - ST_LIGHT_A + 1);
		else  if(status == 1)           // 被按下
			sprintf(g_szPng, "mainlightp.png", type - ST_LIGHT_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "mainlight2.png", type - ST_LIGHT_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "mainlight3.png", type - ST_LIGHT_A + 1);
		break;
	case ST_DIMMER_A:
	case ST_DIMMER_B:
	case ST_DIMMER_C:
	case ST_DIMMER_D:
		if(status == 0)  // 调光灯状态  
			sprintf(g_szPng, "tiaoguang.png", type - ST_DIMMER_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "tiaoguangp.png", type - ST_DIMMER_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "tiaoguang2.png", type - ST_DIMMER_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "tiaoguang3.png", type - ST_DIMMER_A + 1);
		break;
	case ST_CURTAIN_A:
	case ST_CURTAIN_B:
	case ST_CURTAIN_C:
	case ST_CURTAIN_D:
		if(status == 0)
			sprintf(g_szPng, "chuanglian.png", type - ST_CURTAIN_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "chuanglianp.png", type - ST_CURTAIN_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "chuanglian22.png", type - ST_CURTAIN_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "chuanglian3.png", type - ST_CURTAIN_A + 1);
		break;
	case ST_WINDOW_A:
	case ST_WINDOW_B:
	case ST_WINDOW_C:
	case ST_WINDOW_D:
		if(status == 0)
			sprintf(g_szPng, "window%d.png", type - ST_WINDOW_A + 1);
		else
			sprintf(g_szPng, "window%dp.png", type - ST_WINDOW_A + 1);
		break;
		
	case ST_OUTLET_A: // 插座
	case ST_OUTLET_B:
	case ST_OUTLET_C:
	case ST_OUTLET_D:
		if(status == 0)
			sprintf(g_szPng, "chazuo.png", type - ST_OUTLET_A + 1);
		else
			sprintf(g_szPng, "chazuop.png", type - ST_OUTLET_A + 1);
		break;

	case ST_FAN_A:    // 排气
	case ST_FAN_B:
	case ST_FAN_C:
	case ST_FAN_D:
		if(status == 0)
			sprintf(g_szPng, "paiqi.png", type - ST_FAN_A + 1);
		else
			sprintf(g_szPng, "paiqip.png", type - ST_FAN_A + 1);
		break;

	case ST_AC_A:    // 空调
	case ST_AC_C:
	case ST_AC_D:
		if(status == 0)
			sprintf(g_szPng, "kongtiao.png", type - ST_AC_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "kongtiaop.png", type - ST_AC_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "kongtiao2.png", type - ST_AC_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "kongtiao3.png", type - ST_AC_A + 1);
		break;
	
	case ST_AC_B:   // 红外空调
		if(status == 0)
			sprintf(g_szPng, "irair.png", type - ST_AC_A + 1);
		else
			sprintf(g_szPng, "irair1.png", type - ST_AC_A + 1);
		break;
		
	case ST_HEAT_A:  // 地暖
	case ST_HEAT_B:
	case ST_HEAT_C:
	case ST_HEAT_D:
		if(status == 0)
			sprintf(g_szPng, "dinuan.png", type - ST_HEAT_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "dinuanp.png", type - ST_HEAT_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "dinuan2.png", type - ST_HEAT_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "dinuan3.png", type - ST_HEAT_A + 1);
		break;
	case ST_WIND_A:  // 新风
	case ST_WIND_B:
	case ST_WIND_C:
	case ST_WIND_D:
		if(status == 0)  //开机状态没有被按下
			sprintf(g_szPng, "newwindon.png", type - ST_WIND_A + 1);
		else  if(status == 1)   //关机状态被按下
			sprintf(g_szPng, "newwindoff.png", type - ST_WIND_A + 1);
		else if(status == 2) 	//关机状态没有被按下
			sprintf(g_szPng, "newwindoffp.png", type - ST_WIND_A + 1);
		else if(status == 3)//开机状态被按下
			sprintf(g_szPng, "newwindonp.png", type - ST_WIND_A + 1);
		break;
	case ST_TV_A:    // 电视
	case ST_TV_B:
	case ST_TV_C:
	case ST_TV_D:
		if(status == 0)
			sprintf(g_szPng, "irtv.png", type - ST_TV_A + 1);
		else
			sprintf(g_szPng, "irtv1.png", type - ST_TV_A + 1);
		break;
	case ST_MUSIC_A:
	case ST_MUSIC_B:
	case ST_MUSIC_C:
	case ST_MUSIC_D:
		if(status == 0)
			sprintf(g_szPng, "yinyue.png", type - ST_MUSIC_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "yinyuep.png", type - ST_MUSIC_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "yinyue2.png", type - ST_MUSIC_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "yinyue3.png", type - ST_MUSIC_A + 1);
		break;
	case ST_SCENE_A:   // 情景 会客模式情景
		if(status == 0)
				sprintf(g_szPng, "huike.png", type - ST_SCENE_A + 1);
		else if(status == 1)
				sprintf(g_szPng, "huikep.png", type - ST_SCENE_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "huike2.png", type - ST_SCENE_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "huike3.png", type - ST_SCENE_A + 1);
		break;
		
	case ST_SCENE_B:  // 就餐模式
		if(status == 0)
			sprintf(g_szPng, "jiucan.png", type - ST_SCENE_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "jiucanp.png", type - ST_SCENE_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "jiucan2.png", type - ST_SCENE_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "jiucan3.png", type - ST_SCENE_A + 1);
		break;
		
	case ST_SCENE_C:  // 影院模式
		if(status == 0)
			sprintf(g_szPng, "dianying.png", type - ST_SCENE_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "dianyingp.png", type - ST_SCENE_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "dianying2.png", type - ST_SCENE_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "dianying3.png", type - ST_SCENE_A + 1);
		break;
		
	case ST_SCENE_D:  // 就寝模式
		if(status == 0)
			sprintf(g_szPng, "jiuqin.png", type - ST_SCENE_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "jiuqinp.png", type - ST_SCENE_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "jiuqin2.png", type - ST_SCENE_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "jiuqin3.png", type - ST_SCENE_A + 1);
		break;
		
	case ST_SCENE_E:  // 温馨模式
		if(status == 0)
			sprintf(g_szPng, "wenxin.png", type - ST_SCENE_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "wenxinp.png", type - ST_SCENE_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "wenxin2.png", type - ST_SCENE_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "wenxin3.png", type - ST_SCENE_A + 1);
		break;

	case ST_SCENE_F:   // 在家模式情景
		if(status == 0)
			sprintf(g_szPng, "athome.png", type - ST_SCENE_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "athomep.png", type - ST_SCENE_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "athome2.png", type - ST_SCENE_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "athome3.png", type - ST_SCENE_A + 1);
		break;

	case ST_SCENE_G:   // 离家模式情景
		if(status == 0)
			sprintf(g_szPng, "lijia.png", type - ST_SCENE_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "lijiap.png", type - ST_SCENE_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "lijia2.png", type - ST_SCENE_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "lijia3.png", type - ST_SCENE_A + 1);
		break;
		
	case ST_SCENE_H:
		if(status == 0)
			sprintf(g_szPng, "yeqi.png", type - ST_SCENE_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "yeqip.png", type - ST_SCENE_A + 1);
		else if(status == 2)
			sprintf(g_szPng, "yeqi2.png", type - ST_SCENE_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "yeqi3.png", type - ST_SCENE_A + 1);
		break;
		
	case ST_SCENE_I:
		if(status == 0)
			sprintf(g_szPng, "chenqi.png", type - ST_SCENE_A + 1);
		else if(status == 1)
			sprintf(g_szPng, "chenqip.png", type - ST_SCENE_A + 1);	
		else if(status == 2)
			sprintf(g_szPng, "chenqi2.png", type - ST_SCENE_A + 1);
		else if(status == 3)
			sprintf(g_szPng, "chenqi3.png", type - ST_SCENE_A + 1);
		break;
	case ST_SCENE_J:
	case ST_SCENE_K:
	case ST_SCENE_L:
	case ST_SCENE_M:
	case ST_SCENE_N:
	case ST_SCENE_O:
	case ST_SCENE_P:
	case ST_SCENE_Q:
	case ST_SCENE_R:
	case ST_SCENE_S:
	case ST_SCENE_T:
	case ST_SCENE_U:
	case ST_SCENE_V:
	case ST_SCENE_W:
	case ST_SCENE_X:
	case ST_SCENE_Y:
	case ST_SCENE_Z:
		if(status == 0)
			sprintf(g_szPng, "scene%d.png", type - ST_SCENE_A + 1);
		else
			sprintf(g_szPng, "scenep%dp.png", type - ST_SCENE_A + 1);
		break;
	default:
		DBGMSG(DPERROR, "GetSmartPng unkonw devtype:%d\r\n", type);
		strcpy(g_szPng, "");
		break;
	}

	return g_szPng;
}

void OnSmartDev(int index)
{
	switch(index)
	{
	case ST_DIMMER_A:
	case ST_DIMMER_B:
	case ST_DIMMER_C:
	case ST_DIMMER_D:
		break;
	case ST_LIGHT_A:
	case ST_LIGHT_B:
	case ST_LIGHT_C:
	case ST_LIGHT_D:
	case ST_CURTAIN_A:
	case ST_CURTAIN_B:
	case ST_CURTAIN_C:
	case ST_CURTAIN_D:
	case ST_WINDOW_A:
	case ST_WINDOW_B:
	case ST_WINDOW_C:
	case ST_WINDOW_D:
	case ST_OUTLET_A:
	case ST_OUTLET_B:
	case ST_OUTLET_C:
	case ST_OUTLET_D:
	case ST_FAN_A:
	case ST_FAN_B:
	case ST_FAN_C:
	case ST_FAN_D:
	case ST_AC_A:
	case ST_AC_B:
	case ST_AC_C:
	case ST_AC_D:
	case ST_HEAT_A:
	case ST_HEAT_B:
	case ST_HEAT_C:
	case ST_HEAT_D:
	case ST_WIND_A:
	case ST_WIND_B:
	case ST_WIND_C:
	case ST_WIND_D:
	case ST_TV_A:
	case ST_TV_B:
	case ST_TV_C:
	case ST_TV_D:
	case ST_MUSIC_A:
	case ST_MUSIC_B:
	case ST_MUSIC_C:
	case ST_MUSIC_D:
	case ST_SCENE_A:
	case ST_SCENE_B:
	case ST_SCENE_C:
	case ST_SCENE_D:
	case ST_SCENE_E:
	case ST_SCENE_F:
	case ST_SCENE_G:
	case ST_SCENE_H:
	case ST_SCENE_I:
	case ST_SCENE_J:
	case ST_SCENE_K:
	case ST_SCENE_L:
	case ST_SCENE_M:
	case ST_SCENE_N:
	case ST_SCENE_O:
	case ST_SCENE_P:
	case ST_SCENE_Q:
	case ST_SCENE_R:
	case ST_SCENE_S:
	case ST_SCENE_T:
	case ST_SCENE_U:
	case ST_SCENE_V:
	case ST_SCENE_W:
	case ST_SCENE_X:
	case ST_SCENE_Y:
	case ST_SCENE_Z:
		break;
	default:
		//DBGMSG(DPERROR, "OnSmartDev unkonw type:%d\r\n", m_SmartDev[m_nPage][index].type);
		break;
	}
}
