#include "roomlib.h"
#include "zigbee.h"

static ZIGBEE_DEV_INFO smart_zigbee_dev_info;
static WORD zigbee_addr;
static WORD zigbee_ver;

void wrt_zigbee_send(char *send_buff, const int send_len);

WORD GetZigbeeVer()
{
	return zigbee_ver;
}

//======================================================
//** 函数名称: operate_zigbee_return_value
//** 功能描述: ZIGBEE返回值
//** 输　入: code
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
const char *operate_zigbee_return_value(const int code)
{
    switch (code)
    {
        case ZIGBEE_OK:
            return "ZIGBEE_OK";
        case ZIGBEE_LENGTH_FAUSE:
            return "ZIGBEE_LENGTH_FAUSE";
        case ZIGBEE_ADDRESS_FAUSE:
            return "ZIGBEE_ADDRESS_FAUSE";
        case ZIGBEE_CHECK_FAUSE:
            return "ZIGBEE_CHECK_FAUSE";
        case ZIGBEE_WRITE_FAUSE:
            return "ZIGBEE_WRITE_FAUSE";
        case ZIGBEE_OTHER_FAUSE:
            return "ZIGBEE_OTHER_FAUSE";
        default:
            return "unknown code";
    }
}


/****************************************************************/
//串口发送指令部分
//======================================================
//** 函数名称: wrt_modify_zigbee_local_config
//** 功能描述: ZIGBEE修改本地配置
//** 输　入: devinfo
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static int wrt_modify_zigbee_local_config(const ZIGBEE_DEV_INFO devinfo)
{
    if (sizeof(ZIGBEE_DEV_INFO) != 65)
    {
        printf("buf_len:%d != 65", sizeof(ZIGBEE_DEV_INFO));
        return -1;
    }

    int index = 0;
    static const int para_length = 3 + 1 + 2 + sizeof(ZIGBEE_DEV_INFO) + 1;
    unsigned char para[para_length];
    memset(para, 0, para_length);
    para[0] = ZIGBEE_HEAD_AB;
    para[1] = ZIGBEE_HEAD_BC;
    para[2] = ZIGBEE_HEAD_CD;
    para[3] = ZIGBEE_MODIFY_LOCAL_CONFIG;
    //修改本地配置要填写的本地网络地址
    //是要填写zigbee的默认值0x2001
    //还是要填写修改后的目标值0xFFFF
    //para[4] = 0x20;   //local zigbee network address
    //para[5] = 0x01;   //local zigbee network address
    //大小端的关系
    *(unsigned short *)(&(para[4])) = zigbee_addr;

    memcpy(para + 6, &devinfo, sizeof(ZIGBEE_DEV_INFO));

    for (index = 0; index < (para_length - 1); index++)
    {
        para[para_length - 1] += para[index];
    }
    wrt_zigbee_send((char *)para, para_length);

    return 0;
}

//======================================================
//** 函数名称: wrt_zigbee_get_local_dev_info
//** 功能描述: ZIGBEE获取设备信息
//** 输　入: zigbee_dev_info
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static int wrt_zigbee_get_local_dev_info(ZIGBEE_DEV_INFO *zigbee_dev_info)
{
    if (sizeof(ZIGBEE_DEV_INFO) != 65)
    {
        printf("buf_len:%d != 65", sizeof(ZIGBEE_DEV_INFO));
        return -1;
    }

    memset(zigbee_dev_info, 0, sizeof(ZIGBEE_DEV_INFO));
    memset(&smart_zigbee_dev_info, 0, sizeof(smart_zigbee_dev_info));

    unsigned char para[5];
    memset(para, 0, sizeof(para));
    para[0] = ZIGBEE_HEAD_AB;
    para[1] = ZIGBEE_HEAD_BC;
    para[2] = ZIGBEE_HEAD_CD;
    para[3] = ZIGBEE_GET_LOCAL_CONFIG;
    para[4] = para[0] + para[1] + para[2] + para[3];
    wrt_zigbee_send((char *)para, sizeof(para));

    return 0;
}

//======================================================
//** 函数名称: wrt_zigbee_modify_config_ornot
//** 功能描述: ZIGBEE修改设置
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static int wrt_zigbee_modify_config_ornot()
{

    //比较以下6项，若有项不同全修改
    //以下6项必需要修改
    bool modify = false;
    if (strncmp((char *)smart_zigbee_dev_info.dev_name, ZIGBEE_DEV_NAME, strlen(ZIGBEE_DEV_NAME)))
    {
        modify = true;
        printf("dev_name:%s\n", (char *)smart_zigbee_dev_info.dev_name);
    }

    if (GetSmartRoute() != smart_zigbee_dev_info.dev_mode[0])
    {
        modify = true;
        printf("dev_mode:0x%x\n", smart_zigbee_dev_info.dev_mode[0]);
    }

    if (0x19 != smart_zigbee_dev_info.chan[0])
    {
        modify = true;
        printf("chan:0x%x\n", smart_zigbee_dev_info.chan[0]);
    }

    if (0x5457 != (*((unsigned short *)smart_zigbee_dev_info.pan_id)))
    {
        modify = true;
        printf("pan_id:0x%x\n", (*((unsigned short *)smart_zigbee_dev_info.pan_id)));
    }

    if (ZIGBEE_SEND_MODE_BROADCAST != smart_zigbee_dev_info.send_mode[0])
    {
        modify = true;
        printf("send_mode:0x%x\n", smart_zigbee_dev_info.send_mode[0]);
    }

    if (GetSmartAddr() != *((unsigned short *)(smart_zigbee_dev_info.my_addr)))
    {
        modify = true;
        printf("my_addr:0x%x\n", *((unsigned short *)(smart_zigbee_dev_info.my_addr)));
    }

    if (0x03 != smart_zigbee_dev_info.power_level[0])
    {
        modify = true;
        printf("my_addr:0x%x\n", smart_zigbee_dev_info.power_level[0]);
    }

    if (modify)
    {
        //要求修改的zigbee配置
        strncpy((char *)smart_zigbee_dev_info.dev_name, ZIGBEE_DEV_NAME, strlen(ZIGBEE_DEV_NAME));
        smart_zigbee_dev_info.dev_mode[0] = GetSmartRoute();
        smart_zigbee_dev_info.chan[0] = 0x19;
        smart_zigbee_dev_info.power_level[0] = 0x03;

        //panid 0x5754
        *((unsigned short *)smart_zigbee_dev_info.pan_id) = 0x5457; //大小端问题0x5754
        *((unsigned short *)smart_zigbee_dev_info.my_addr) = GetSmartAddr();

        smart_zigbee_dev_info.send_mode[0] = ZIGBEE_SEND_MODE_BROADCAST;
        wrt_modify_zigbee_local_config(smart_zigbee_dev_info);
        return -1;
    }

    printf("zigbee local config ok!\n");

    return 0;
}

//======================================================
//** 函数名称: wrt_zigbee_reset
//** 功能描述: ZIGBEE复位
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static int wrt_zigbee_reset()
{
    if (sizeof(ZIGBEE_DEV_INFO) != 65)
    {
        printf("buf_len:%d != 65", sizeof(ZIGBEE_DEV_INFO));
        return -1;
    }

    unsigned char para[9];
    memset(para, 0, sizeof(para));
    para[0] = ZIGBEE_HEAD_AB;
    para[1] = ZIGBEE_HEAD_BC;
    para[2] = ZIGBEE_HEAD_CD;
    para[3] = ZIGBEE_RESET;

    //para[4] = 0x20;   //local zigbee network address
    //para[5] = 0x01;   //local zigbee network address
    *(unsigned short *)(&(para[4])) = zigbee_addr;
    para[6] = 0x00;   	//zigbee dev type 0 or 1
    para[7] = smart_zigbee_dev_info.dev_mode[0];

    for (int index = 0; index < (sizeof(para) - 1); index++)
    {
        para[8] += para[index];
    }

    wrt_zigbee_send((char *)para, sizeof(para));

    return 0;
}


/****************************************************************/
//串口接收指令部分
//======================================================
//** 函数名称: wrt_zigbee_get_local_config
//** 功能描述: ZIGBEE获取配置
//** 输　入: buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static int wrt_zigbee_get_local_config(const void *buf, const int buf_len)
{
    if (74 != buf_len)
    {
        printf("buf_len:%d != 74\n", buf_len);
        return -1;
    }

    const ZIGBEE_DEV_INFO zigbee_info = *((ZIGBEE_DEV_INFO *)(((int8_t *)buf) + 4));
    memcpy(&smart_zigbee_dev_info, &zigbee_info, sizeof(ZIGBEE_DEV_INFO));

    //修改zigbee配置需要读出来的my_addr地址
    zigbee_addr = *(unsigned short *)(smart_zigbee_dev_info.my_addr);

	char *data = (char *)buf;
	zigbee_ver = data[72] << 8 | data[73];

    return 0;
}

//======================================================
//** 函数名称: wrt_zigbee_modify_local_config
//** 功能描述: ZIGBEE修改配置
//** 输　入: buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static int wrt_zigbee_modify_local_config(const void *buf, const int buf_len)
{
    if (7 != buf_len)
    {
        printf("buf_len:%d != 7\n", buf_len);
        return -1;
    }

    return 0;
}

//======================================================
//** 函数名称: wrt_zigbee_default_setting
//** 功能描述: ZIGBEE默认设置
//** 输　入: buf buf_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static int wrt_zigbee_default_setting(const void *buf, const int buf_len)
{
    if (9 != buf_len)
    {
        printf("buf_len:%d != 9\n", buf_len);
        return -1;
    }

    return 0;
}

//======================================================
//** 函数名称: wrt_zigbee_cmd_proc
//** 功能描述: ZIGBEE指令处理
//** 输　入: data nread
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
int wrt_zigbee_cmd_proc(const void *data, const int nread)
{
    //zigbee网关配置指令发送后将从串口收到
    const unsigned char *arr = (const unsigned char *)data;
    if ((ZIGBEE_HEAD_AB == arr[0]) &&
            (ZIGBEE_HEAD_BC == arr[1]) &&
            (ZIGBEE_HEAD_CD == arr[2]))
    {
        const unsigned char cmd = arr[3];
        switch (cmd)
        {
            case ZIGBEE_GET_LOCAL_CONFIG:
                wrt_zigbee_get_local_config(arr, nread);
                break;

            case ZIGBEE_MODIFY_LOCAL_CONFIG:
                wrt_zigbee_modify_local_config(arr, nread);
                break;

            case ZIGBEE_DEFAULT_SETTING:
                wrt_zigbee_default_setting(arr, nread);
                break;
            case ZIGBEE_RESET:
                break;
            default:
                printf("zigbee setting unknown cmd:0x%02x\n", cmd);
                break;
        }
    }
    else
    {
        printf("not zigbee setting cmd!\n");
        return -1;
    }

    return 0;
}

//======================================================
//** 函数名称: wrt_zigbee_init
//** 功能描述: ZIGBEE初始化
//** 输　入: wrt_zigbee_init
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
int wrt_zigbee_init()
{
    ZIGBEE_DEV_INFO devinfo = {0};
    int count = 6;

    DPSleep(1000);
    while (--count)
    {
        wrt_zigbee_get_local_dev_info(&devinfo);
        DPSleep(1000);
        if (wrt_zigbee_modify_config_ornot())
        {
            DPSleep(1000);
            wrt_zigbee_reset();
            DPSleep(1000);
        }
        else
        {
            break;
        }
    }

    if (0 != count)
    {
        printf("zigbee init ok!\n");
        return 0;
    }
    else
    {
        printf("zigbee init error!\n");
        return -1;
    }
}

//======================================================
//** 函数名称: wrt_zigbee_send
//** 功能描述: ZIGBEE数据发送
//** 输　入: send_buff send_len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void wrt_zigbee_send(char *send_buff, const int send_len)
{
    SendZigbeeCmd(send_buff, send_len);
}

//======================================================
//** 函数名称: SmartZigbeeProc
//** 功能描述: ZIGBEE指令处理
//** 输　入: data len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SmartZigbeeProc(const void *data, const int len)
{
    wrt_zigbee_cmd_proc(data, len);
}

//======================================================
//** 函数名称: InitSmartZigbee
//** 功能描述: 初始化ZIGBEE
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void InitSmartZigbee()
{
    wrt_zigbee_init();
}
