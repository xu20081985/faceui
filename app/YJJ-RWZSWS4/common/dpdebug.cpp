//*************************************************************************
//**************** ������� ***********************************************
//*************************************************************************
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
#include "roomlib.h"

static DWORD printlevel = 0xE0000000;
static StaticLock g_DebugCS;

//======================================================
//** ��������: InitDebugLeven
//** ��������: ��ʼ��debug����
//** �䡡��: level
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void InitDebugLeven(DWORD level)
{
    printlevel |= level;
}

//======================================================
//** ��������: DBGMSG
//** ��������: DEBUG��Ϣ
//** �䡡��: level
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
//** ��������: fatal_signal_handler
//** ��������: ϵͳ�����źŴ���
//** �䡡��: signum
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
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
        for (char *p = trace[i]; *p; ++p)   // �����˷�����Ϣ�ĸ�ʽ
        {
            if (*p == '(')   // ������
            {
                begin_name = p;
            }
            else if (*p == '+' && begin_name)   // ��ַƫ�Ʒ���
            {
                begin_offset = p;
            }
            else if (*p == ')' && begin_offset)   // ������
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
//** ��������: DPBacktrace
//** ��������: �������
//** �䡡��: ��
//** �䡡��: ��
//**
//** ������: HJ
//** �ա���: 2018��11��19��
//======================================================
void DPBacktrace()
{
    signal(SIGSEGV, fatal_signal_handler);
    signal(SIGBUS,  fatal_signal_handler);
    signal(SIGILL,  fatal_signal_handler);
}
