#include "SmartConfig.h"
#include "roomlib.h"
#include "list.h"

static CMyList 			m_msglist;					 // 消息列表
static CMyList 			m_sendlist;					 // 发送列表
static StaticLock		g_CS;
static StaticLock		g_CSSend;


typedef struct
{
    LISTOBJ lpObj;
    DEVICE device;
    WORD cmd;
    BYTE tick;
    BYTE wait;
    BYTE count;
    BYTE len;
    char msg[64];
} SmartCom, *PSmartCom;

WORD smartComCmd[] =
{
    0x0103,
    0x0205,
    0x0384,
    0x0394,
    0x0330,
};

//======================================================
//** 函数名称: SmartComSend
//** 功能描述: 数据指令发送
//** 输　入: buf len
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
BOOL SmartComSend(char *buf, int len)
{
    ECB_DATA *pData = (ECB_DATA *)buf;
    WORD cmd = pData->type << 8 | pData->cmd;
    BOOL flag = FALSE;

    for (int i = 0; i < sizeof(smartComCmd) / sizeof(WORD); i++)
    {
        if (cmd == smartComCmd[i])
            flag = TRUE;
    }

    if (flag == FALSE)
        return FALSE;

    g_CS.lockon();

    PSmartCom pItem = (PSmartCom)m_msglist.Head();
    PSmartCom pobj = new SmartCom;
    PSmartCom pItemNext;

    if (cmd == 0x0103)
    {
        pobj->device.channel = 1;
        memcpy(&pobj->device.id, pData->data, 6);
    }
    else
    {
        memcpy(&pobj->device.channel, pData->data, 7);
    }

    pobj->cmd = cmd + 1;
    pobj->count = 3;
    pobj->tick = 0;
    pobj->wait = 20;
    pobj->len = len;
    memcpy(pobj->msg, buf, len);

    while (pItem)
    {
        pItemNext = (PSmartCom) m_msglist.Next(&(pItem->lpObj));
        if (pItem->cmd == cmd && !memcmp(&pItem->device, &pobj->device, 7))
        {
            m_msglist.Disconnect(&(pItem->lpObj));
            delete pItem;
        }
        pItem = pItemNext;
    }

    m_msglist.AddTail(&pobj->lpObj);

    g_CS.lockoff();

    return TRUE;
}

//======================================================
//** 函数名称: SmartComRecv
//** 功能描述: 数据指令接收
//** 输　入: pECB
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SmartComRecv(ECB_DATA *pECB)
{
    WORD cmd = pECB->type << 8 | pECB->cmd;

    BOOL flag = FALSE;

    for (int i = 0; i < sizeof(smartComCmd) / sizeof(WORD); i++)
    {
        if (cmd == smartComCmd[i] + 1)
            flag = TRUE;
    }

    if (flag == FALSE)
        return;

    g_CS.lockon();
    PSmartCom pItem = (PSmartCom)m_msglist.Head();
    PSmartCom pItemNext;

    while (pItem)
    {
        pItemNext = (PSmartCom) m_msglist.Next(&(pItem->lpObj));
        if (pItem->cmd == cmd)
        {
            if (cmd == 0x0104)
            {
                if (!memcmp(&pItem->device.id, pECB->data, 6))
                {
                    m_msglist.Disconnect(&(pItem->lpObj));
                    delete pItem;
                }
            }
            else
            {
                if (!memcmp(&pItem->device, pECB->data, 7))
                {
                    m_msglist.Disconnect(&(pItem->lpObj));
                    delete pItem;
                }
            }
        }
        pItem = pItemNext;
    }
    g_CS.lockoff();
}

//======================================================
//** 函数名称: SmartComThread
//** 功能描述: 重复处理线程
//** 输　入: pParam
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void *SmartComThread(void *pParam)
{
    while (1)
    {
        DPSleep(100);
        PSmartCom pItem = (PSmartCom)m_msglist.Head();
        PSmartCom pItemNext;
        while (pItem)
        {
            pItemNext = (PSmartCom) m_msglist.Next(&(pItem->lpObj));
            if (pItem->tick)
            {
                pItem->tick--;
            }
            else
            {
                pItem->tick = pItem->wait;
                if (pItem->count)
                    pItem->count--;
                else
                {
                    g_CS.lockon();
                    m_msglist.Disconnect(&(pItem->lpObj));
                    delete pItem;
                    pItem = pItemNext;
                    g_CS.lockoff();
                    continue;
                }

                g_CSSend.lockon();
                PSmartCom pobj = new SmartCom;
                memcpy(pobj->msg, pItem->msg, pItem->len);
                pobj->len = pItem->len;
                m_sendlist.AddTail(&pobj->lpObj);
                g_CSSend.lockoff();
            }
            pItem = pItemNext;
        }
    }
    return NULL;
}

//======================================================
//** 函数名称: SmartSendThread
//** 功能描述: 数据发送线程
//** 输　入: pParam
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void *SmartSendThread(void *pParam)
{
    while (1)
    {
        DPSleep(100);
        PSmartCom pItem = (PSmartCom)m_sendlist.Head();
        PSmartCom pItemNext;
        while (pItem)
        {
            pItemNext = (PSmartCom) m_sendlist.Next(&(pItem->lpObj));

            SmartSendCmd(pItem->msg, pItem->len);
            g_CSSend.lockon();
            m_sendlist.Disconnect(&(pItem->lpObj));
            delete pItem;
            pItem = pItemNext;
            g_CSSend.lockoff();
        }
    }
    return NULL;
}

//======================================================
//** 函数名称: DPCreateSmartCom
//** 功能描述: 创建重复处理线程
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void DPCreateSmartCom()
{
    pthread_t pid0;
    pthread_create(&pid0, NULL, SmartComThread, NULL);
}

//======================================================
//** 函数名称: DPCreateSmartSend
//** 功能描述: 创建数据发送线程
//** 输　入: 无
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void DPCreateSmartSend()
{
    pthread_t pid0;
    pthread_create(&pid0, NULL, SmartSendThread, NULL);
}
