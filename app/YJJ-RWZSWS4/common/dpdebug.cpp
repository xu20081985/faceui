//*************************************************************************
//**************** 错误回溯 ***********************************************
//*************************************************************************
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
#include "roomlib.h"

static DWORD printlevel = 0xE0000000;
static StaticLock g_DebugCS;

//======================================================
//** 函数名称: InitDebugLeven
//** 功能描述: 初始化debug级别
//** 输　入: level
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void InitDebugLeven(DWORD level)
{
    printlevel |= level;
}

//======================================================
//** 函数名称: DBGMSG
//** 功能描述: DEBUG消息
//** 输　入: level
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
int DBGMSG(DWORD level, const char *format, ...)
{
    char tszInfo[512];
    //if(((1 << level) & printlevel) == 0)
    //	return 0;
    va_list va;
    g_DebugCS.lockon();
    va_start(va, format);
    vsprintf(&tszInfo[11], format, va);
    va_end(va);

    SYSTEMTIME cursystem;
    DPGetLocalTime(&cursystem);
    sprintf(tszInfo, "[%02d:%02d:%02d]", cursystem.wHour, cursystem.wMinute, cursystem.wSecond);
    tszInfo[10] = 0x20;
    printf("%s", tszInfo);
    g_DebugCS.lockoff();
    return 0;
}

//======================================================
//** 函数名称: fatal_signal_handler
//** 功能描述: 系统错误信号处理
//** 输　入: signum
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
static void fatal_signal_handler(int signum)
{
    void *buffer[100] = { NULL };
    char **trace = NULL;
    int size = backtrace(buffer, 100);
    trace = backtrace_symbols(buffer, size);
    if (NULL == trace)
    {
        return;
    }

    size_t name_size = 100;
    char *name = (char *)malloc(name_size);
    for (int i = 0; i < size; ++i)
    {
        char *begin_name = 0;
        char *begin_offset = 0;
        char *end_offset = 0;
        for (char *p = trace[i]; *p; ++p)   // 利用了符号信息的格式
        {
            if (*p == '(')   // 左括号
            {
                begin_name = p;
            }
            else if (*p == '+' && begin_name)   // 地址偏移符号
            {
                begin_offset = p;
            }
            else if (*p == ')' && begin_offset)   // 右括号
            {
                end_offset = p;
                break;
            }
        }
        if (begin_name && begin_offset && end_offset )
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';
            int status = -4; // 0 -1 -2 -3
            char *ret = abi::__cxa_demangle(begin_name, name, &name_size, &status);
            if (0 == status)
            {
                name = ret;
                printf("%s:%s+%s\n", trace[i], name, begin_offset);
            }
            else
            {
                printf("%s:%s()+%s\n", trace[i], begin_name, begin_offset);
            }
        }
        else
        {
            printf("%s\n", trace[i]);
        }
    }
    free(name);
    free(trace);
    exit(1);
}

//======================================================
//** 函数名称: DPBacktrace
//** 功能描述: 错误回溯
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void DPBacktrace()
{
    signal(SIGSEGV, fatal_signal_handler);
    signal(SIGBUS,  fatal_signal_handler);
    signal(SIGILL,  fatal_signal_handler);
}
