#include "SmartConfig.h"
#include "roomlib.h"

//======================================================
//** 函数名称: SmartTimerHandle
//** 功能描述: 定时数据处理
//** 输　入: pItem
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void SmartTimerHandle(PSmartTimer pItem)
{
    if (pItem->way == SCMD_SCENE)
    {
        SendSmartCmd(&pItem->device, pItem->way,  pItem->param);
        SetStatusByScene(pItem->param);
    }
    else
    {
        //SendSmartCmd(&pItem->device, pItem->way, pItem->param);
        SetStatusBySync(&pItem->device, pItem->way, pItem->param);
        if (pItem->device.id == GetDevID()
                && pItem->device.type == GetDevType())
        {
            if (pItem->way == SCMD_OPEN)
                SetLightGpioVal(pItem->device.channel, FALSE);
            else
                SetLightGpioVal(pItem->device.channel, TRUE);
            SendStatusCmd(&pItem->device, pItem->way, 0);
        }
        else
        {
            SendSmartCmd(&pItem->device, pItem->way, 0);
        }		
    }
    DPPostMessage(MSG_BROADCAST, SMART_STATUS_SYNC, 0, 0);
}

//======================================================
//** 函数名称: SmartTimerThread
//** 功能描述: 定时线程
//** 输　入: pParam
//** 输　出: 无
//**
//** 作　者: HJ
//** 日　期: 2018年11月19日
//======================================================
void *SmartTimerThread(void *pParam)
{
    SYSTEMTIME tm;
    PSmartTimer m_pItem;
    int hour = 0;
    int minute = 0;

    while (1)
    {
        DPGetLocalTime(&tm);
        if (hour == tm.wHour && minute == tm.wMinute)
        {
            DPSleep(1000);
            continue;
        }
        m_pItem = GetTimerHead();
        while (m_pItem)
        {
            if (m_pItem->onoff == TRUE
                    && tm.wHour == m_pItem->hour
                    && tm.wMinute == m_pItem->minute)
            {
                int curWeek = (tm.wDayOfWeek + 6) % 7;
                if ((m_pItem->week & 0x7f) == 0)
                {
                    m_pItem->onoff = FALSE;
                    UpdatSmartTimerSet();
                    SmartTimerHandle(m_pItem);
                }
                else if (m_pItem->week & (1 << curWeek))
                {
                    SmartTimerHandle(m_pItem);
                }
            }
            m_pItem = GetTimerNext(&(m_pItem->lpObj));
        }
        hour = tm.wHour;
        minute = tm.wMinute;

    }
    return NULL;
}
