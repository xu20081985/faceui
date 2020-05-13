#include "roomlib.h"

#define PCMSG_PORT				0x8866

#define	PCMSG_UPDATE_APP0		0x60000000
const char APP0_FILE[] =		"/Windows/app0";

typedef struct
{
    WORD devtype[32];
    DWORD type;
    DWORD length;
} PCMSG_DATA;

static BOOL g_bPCStart;
static HANDLE g_hPCThread;
static StaticLock g_CS;

static void HandleUpdateApp0(SOCKET sockfd, PCMSG_DATA *pMsg)
{
    int ret = FALSE;
    char *pdata = NULL;
    do
    {
        pdata = (char *)malloc(pMsg->length + 1);
        if(NULL == pdata)
            break;

        if(pMsg->length != TcpRecvData(sockfd, pdata, pMsg->length, 50000))
        {
            DBGMSG(DPINFO, "HandleUpdateApp0 Recv fail:%d\r\n", DPGetLastError());
            break;
        }

        DWORD dwSize = 0;
        FILE *pFile = fopen(APP0_FILE, "wb");
        if(pFile)
        {
            dwSize = fwrite(pdata, 1, pMsg->length, pFile);
            fclose(pFile);
            DBGMSG(DPINFO, "HandleUpdateApp0 Success\r\n");
        }

        if(dwSize != pMsg->length)
            break;

#ifdef DPLINUX
        chmod("/Windows/app0", 777);
#endif

        DPPostMessage(MSG_SYSTEM, WATCHDOG_CHANGE, FALSE, 0);
        ret = TRUE;
    }
    while(0);

    TcpSendData(sockfd, (char *)&ret, 4, 1000);
    if(pdata)
    {
        free(pdata);
    }
}

static DWORD PCThread(HANDLE pParam)
{
    DBGMSG(DPINFO, "PCThread start\r\n");

    SOCKET listenSock = TcpListen(NULL, PCMSG_PORT);
    if(INVALID_SOCKET == listenSock)
    {
        DBGMSG(DPERROR, "PCThread Socket TcpListen fail:%d\r\n", DPGetLastError());
        return 0;
    }

    PCMSG_DATA msg;

    while(g_bPCStart)
    {
        SOCKET sockfd = TcpAccpet(listenSock, 1000, NULL);
        if(INVALID_SOCKET == sockfd)
            continue;

        SocketUnblock(sockfd);
        int ret = TcpRecvData(sockfd, (char *)&msg, sizeof(PCMSG_DATA), 1000);
        if (ret != sizeof(PCMSG_DATA))
        {
            DBGMSG(DPWARNING, "PCThread recv length fail expect:%d ret:%d\r\n", sizeof(PCMSG_DATA), ret);
            SocketClose(sockfd);
            continue;
        }

        DBGMSG(DPINFO, "Recv PC Msg %x %x\r\n", msg.type, msg.length);
        switch(msg.type)
        {
            case PCMSG_UPDATE_APP0:
                /************************************************************************/
                /*    µ˜ ‘”√                                                            */
                /************************************************************************/
                HandleUpdateApp0(sockfd, &msg);
                break;
        }

        SocketClose(sockfd);
    }

    return 0;
}

void StartPCServer(void)
{
#ifdef DPCE
    return;
#endif

    g_CS.lockon();
    DBGMSG(DPINFO, "StartPCServer start\r\n");

    if(!g_bPCStart)
    {
        g_bPCStart = TRUE;
        g_hPCThread = DPThreadCreate(0x4000, PCThread, NULL, TRUE, 5);
    }
    g_CS.lockoff();
}

void StopPCServer(void)
{
    g_CS.lockon();
    DBGMSG(DPINFO, "StopPCServer start\r\n");

    if(g_bPCStart)
        g_bPCStart = FALSE;

    if(g_hPCThread)
    {
        DPThreadJoin(g_hPCThread);
        g_hPCThread = NULL;
    }
    g_CS.lockoff();
}