#include "roomlib.h"
#include "SmartConfig.h"

static char g_szPng[64];

//======================================================
//** ��������: GetSmartPngMusic
//** ��������: ��ȡ���ֲ���/��ͣUI
//** �䡡��: status
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
char *GetSmartPngMusic(DWORD status)
{
    if (status)
        sprintf(g_szPng, "music_play.png");
    else
        sprintf(g_szPng, "music_pause.png");

    return g_szPng;
}

//======================================================
//** ��������: GetSmartAutoOnOff
//** ��������: ��ȡ����ѡ��ť����UI
//** �䡡��: status
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
char *GetSmartAutoOnOff(DWORD status)
{
    if (status)
        sprintf(g_szPng, "auto_on.png");
    else
        sprintf(g_szPng, "auto_off.png");

    return g_szPng;
}

//======================================================
//** ��������: GetSmartPngSelect
//** ��������: ��ȡѡ��/δѡ��UI
//** �䡡��: status
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
char *GetSmartPngSelect(DWORD status)
{
    if (status)
        sprintf(g_szPng, "pub_select.png");
    else
        sprintf(g_szPng, "pub_unselect.png");

    return g_szPng;
}

//======================================================
//** ��������: GetSmartPngOnOff
//** ��������: ��ȡ���ػ�UI
//** �䡡��: status
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
char *GetSmartPngOnOff(DWORD status)
{
    if (status)
        sprintf(g_szPng, "pub_poweron.png");
    else
        sprintf(g_szPng, "pub_poweroff.png");

    return g_szPng;
}

//======================================================
//** ��������: GetSmartPngPoint
//** ��������: ��ȡҳ����UI
//** �䡡��: status
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
char *GetSmartPngPoint(DWORD status)
{
    if (status)
        sprintf(g_szPng, "main_point1.png");
    else
        sprintf(g_szPng, "main_point2.png");

    return g_szPng;
}

//======================================================
//** ��������: GetSmartPngCurtain
//** ��������: ��ȡ����״̬UI
//** �䡡��: type status
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
char *GetSmartPngCurtain(DWORD type, DWORD status)
{
    switch (type)
    {
        case CURTAIN_ALL_CLOSE:
            sprintf(g_szPng, "curtain_close%d.png", status);
            break;
        case CURTAIN_HALF_OPEN:
            sprintf(g_szPng, "curtain_half%d.png", status);
            break;
        case CURTAIN_ALL_OPEN:
            sprintf(g_szPng, "curtain_open%d.png", status);
            break;
        default:
            break;
    }

    return g_szPng;
}

//======================================================
//** ��������: GetSmartPngWind
//** ��������: ��ȡ�·�״̬UI
//** �䡡��: type status
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2019��01��08��
//======================================================
char *GetSmartPngWind(DWORD type, DWORD status)
{
    switch (type)
    {
        case WIND_HIGH:
            sprintf(g_szPng, "wind_high%d.png", status);
            break;
        case WIND_MIDDLE:
            sprintf(g_szPng, "wind_middle%d.png", status);
            break;
        case WIND_LOW:
            sprintf(g_szPng, "wind_low%d.png", status);
            break;
        default:
            break;
    }

    return g_szPng;
}


//======================================================
//** ��������: GetSmartPng
//** ��������: ��ȡ�豸ͼ��UI
//** �䡡��: type status
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
char *GetSmartPng(DWORD type, DWORD status)
{
    switch(type)
    {
        case ST_LIGHT_A: // �ƹ�
            sprintf(g_szPng, "main_light%d_a.png", status);
            break;
        case ST_LIGHT_B:
            sprintf(g_szPng, "main_light%d_b.png", status);
            break;
        case ST_LIGHT_C:
            sprintf(g_szPng, "main_light%d_c.png", status);
            break;
        case ST_LIGHT_D:
            sprintf(g_szPng, "main_light%d_d.png", status);
            break;
        case ST_DIMMER_A: // ����
            sprintf(g_szPng, "main_dimmer%d_a.png", status);
            break;
        case ST_DIMMER_B:
            sprintf(g_szPng, "main_dimmer%d_b.png", status);
            break;
        case ST_DIMMER_C:
            sprintf(g_szPng, "main_dimmer%d_c.png", status);
            break;
        case ST_DIMMER_D:
            sprintf(g_szPng, "main_dimmer%d_d.png", status);
            break;
        case ST_CURTAIN_A: // ����
            sprintf(g_szPng, "main_curtain%d_a.png", status);
            break;
        case ST_CURTAIN_B:
            sprintf(g_szPng, "main_curtain%d_b.png", status);
            break;
        case ST_CURTAIN_C:
            sprintf(g_szPng, "main_curtain%d_c.png", status);
            break;
        case ST_CURTAIN_D:
            sprintf(g_szPng, "main_curtain%d_d.png", status);
            break;
        case ST_WINDOW_A: // ����
            sprintf(g_szPng, "main_curtain%d_a.png", status);
            break;
        case ST_WINDOW_B:
            sprintf(g_szPng, "main_curtain%d_b.png", status);
            break;
        case ST_WINDOW_C:
            sprintf(g_szPng, "main_curtain%d_c.png", status);
            break;
        case ST_WINDOW_D:
            sprintf(g_szPng, "main_curtain%d_d.png", status);
            break;
        case ST_OUTLET_A: // ����
            sprintf(g_szPng, "main_outlet%d_a.png", status);
            break;
        case ST_OUTLET_B:
            sprintf(g_szPng, "main_outlet%d_b.png", status);
            break;
        case ST_OUTLET_C:
            sprintf(g_szPng, "main_outlet%d_c.png", status);
            break;
        case ST_OUTLET_D:
            sprintf(g_szPng, "main_outlet%d_d.png", status);
            break;
        case ST_FAN_A:   // ����
            sprintf(g_szPng, "main_fan%d_a.png", status);
            break;
        case ST_FAN_B:
            sprintf(g_szPng, "main_fan%d_b.png", status);
            break;
        case ST_FAN_C:
            sprintf(g_szPng, "main_fan%d_c.png", status);
            break;
        case ST_FAN_D:
            sprintf(g_szPng, "main_fan%d_d.png", status);
            break;
        case ST_AC_A:    // �յ�
            sprintf(g_szPng, "main_air%d_a.png", status);
            break;
        case ST_AC_B:
            sprintf(g_szPng, "main_air%d_b.png", status);
            break;
        case ST_AC_C:
            sprintf(g_szPng, "main_air%d_c.png", status);
            break;
        case ST_AC_D:
            sprintf(g_szPng, "main_air%d_d.png", status);
            break;
        case ST_IRAIR_A:  // ����յ�
            sprintf(g_szPng, "main_irair%d.png", status);
            break;
        case ST_IRAIR_B:
            sprintf(g_szPng, "main_air%d_a.png", status);
            break;
        case ST_IRAIR_C:
            sprintf(g_szPng, "main_air%d_c.png", status);
            break;
        case ST_IRAIR_D:
            sprintf(g_szPng, "main_air%d_d.png", status);
            break;
        case ST_HEAT_A:  // ��ů
            sprintf(g_szPng, "main_heat%d_a.png", status);
            break;
        case ST_HEAT_B:
            sprintf(g_szPng, "main_heat%d_b.png", status);
            break;
        case ST_HEAT_C:
            sprintf(g_szPng, "main_heat%d_c.png", status);
            break;
        case ST_HEAT_D:
            sprintf(g_szPng, "main_heat%d_d.png", status);
            break;
        case ST_WIND_A:  // �·�
            sprintf(g_szPng, "main_wind%d_a.png", status);
            break;
        case ST_WIND_B:
            sprintf(g_szPng, "main_wind%d_b.png", status);
            break;
        case ST_WIND_C:
            sprintf(g_szPng, "main_wind%d_c.png", status);
            break;
        case ST_WIND_D:
            sprintf(g_szPng, "main_wind%d_d.png", status);
            break;
        case ST_TV_A:    // ����
            sprintf(g_szPng, "main_irtv%d_a.png", status);
            break;
        case ST_TV_B:
            sprintf(g_szPng, "main_irtv%d_b.png", status);
            break;
        case ST_TV_C:
            sprintf(g_szPng, "main_irtv%d_c.png", status);
            break;
        case ST_TV_D:
            sprintf(g_szPng, "main_irtv%d_d.png", status);
            break;
        case ST_MUSIC_A:	// ����
            sprintf(g_szPng, "main_music%d_a.png", status);
            break;
        case ST_MUSIC_B:
            sprintf(g_szPng, "main_music%d_b.png", status);
            break;
        case ST_MUSIC_C:
            sprintf(g_szPng, "main_music%d_c.png", status);
            break;
        case ST_MUSIC_D:
            sprintf(g_szPng, "main_music%d_d.png", status);
            break;
        case ST_LOCK_A:
            sprintf(g_szPng, "main_lock%d_a.png", status);
            break;
        case ST_LOCK_B:
            sprintf(g_szPng, "main_lock%d_b.png", status);
            break;
        case ST_LOCK_C:
            sprintf(g_szPng, "main_lock%d_c.png", status);
            break;
        case ST_LOCK_D:
            sprintf(g_szPng, "main_lock%d_d.png", status);
            break;
        case ST_SCENE_A:   // ���ģʽ
            sprintf(g_szPng, "scene_meeting%d.png", status);
            break;
        case ST_SCENE_B:   // �Ͳ�ģʽ
            sprintf(g_szPng, "scene_eating%d.png", status);
            break;
        case ST_SCENE_C:   // ӰԺģʽ
            sprintf(g_szPng, "scene_movie%d.png", status);
            break;
        case ST_SCENE_D:   // ����ģʽ
            sprintf(g_szPng, "scene_bed%d.png", status);
            break;
        case ST_SCENE_E:   // ��ܰģʽ
            sprintf(g_szPng, "scene_warm%d.png", status);
            break;
        case ST_SCENE_F:   // �ڼ�ģʽ
            sprintf(g_szPng, "scene_home%d.png", status);
            break;
        case ST_SCENE_G:   // ���ģʽ
            sprintf(g_szPng, "scene_leave%d.png", status);
            break;
        case ST_SCENE_H:	 // ҹ��ģʽ
            sprintf(g_szPng, "scene_night%d.png", status);
            break;
        case ST_SCENE_I:	 // ����ģʽ
            sprintf(g_szPng, "scene_morning%d.png", status);
            break;
        case ST_SCENE_J:	 // �뿪ģʽ
            sprintf(g_szPng, "scene_home%d.png", status);
            break;
        case ST_SCENE_K:	 // ���ģʽ
            sprintf(g_szPng, "scene_leave%d.png", status);
            break;
        case ST_SCENE_L:  // �Զ���ģʽ
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
            sprintf(g_szPng, "scene_custom%d.png", status);
            break;
        default:
            DBGMSG(DPERROR, "GetSmartPng unkonw devtype:%d\r\n", type);
            strcpy(g_szPng, "");
            break;
    }

    return g_szPng;
}
