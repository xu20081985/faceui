#include "CCtrlProgress.h"

#define	PROGRESS_EDGE		0

BOOL CDPProgress::DoInit(ContentManage *pCm)
{
    char *pcontent;

    if((pcontent = pCm->FindContentByName("width")) == NULL)
        return FALSE;
    m_width = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("height")) == NULL)
        return FALSE;
    m_height = strtol(pcontent, NULL, 10);

    m_left = (m_pLayer->m_width - m_width) / 2;
    m_top = (m_pLayer->m_height - m_height) / 2;

    if((pcontent = pCm->FindContentByName("left")) != NULL)
        m_left = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("top")) != NULL)
        m_top = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("colorbak")) != NULL)
        m_ColorBak = hexconvert(pcontent);

    if((pcontent = pCm->FindContentByName("colorprogress")) != NULL)
        m_ColorSrc = hexconvert(pcontent);

    if((pcontent = pCm->FindContentByName("total")) != NULL)
        m_ProgressTotal = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("touchable")) != NULL)
        m_bTouch = TRUE;

    if((pcontent = pCm->FindContentByName("hidevalue")) != NULL)
        m_bShowValue = FALSE;

    if((pcontent = pCm->FindContentByName("srcpng")) != NULL)
        strcpy(m_src, pcontent);

    if((pcontent = pCm->FindContentByName("active_height")) != NULL)
        m_active_height = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("active_width")) != NULL)
        m_active_width = strtol(pcontent, NULL, 10);

    if(m_active_height == 0)
        m_pLayer->RegisterEvent(m_left, m_top, m_width, m_height, this);
    else
        m_pLayer->RegisterEvent(m_left - (m_active_width - m_width) / 2,
                                m_top - (m_active_height - m_height) / 2,
                                m_active_width, m_active_height, this);
    m_hFrameBak = m_pSpr->ReqTempBlk(m_width, m_height);
    m_msgid = m_pLayer->RequestMsgId();
    m_pSpr->BitBlt(m_hFrameBak, 0, 0, m_width, m_height, m_pLayer->m_frame, m_left, m_top);
    if((pcontent = pCm->FindContentByName("name")) == NULL)
        m_pLayer->RegisterCtrl(CTRL_PROGRESS, "progress", this, m_msgid);
    else
        m_pLayer->RegisterCtrl(CTRL_PROGRESS, pcontent, this, m_msgid);
    return TRUE;
}

BOOL CDPProgress::DoResponse(DWORD xoff, DWORD yoff, DWORD statue)
{
    yoff = yoff;

    if(!m_bTouch)
        return FALSE;
    switch(statue)
    {
        case TOUCH_DOWN:
            break;
        case TOUCH_VALID:
            if(xoff < PROGRESS_EDGE)
                xoff = PROGRESS_EDGE;
            xoff -= PROGRESS_EDGE;
            if(xoff >= (m_width - 2 * PROGRESS_EDGE))
                xoff = (m_width - 2 * PROGRESS_EDGE);
            m_ProgressCur = (xoff) * m_ProgressTotal / (m_width - 2 * PROGRESS_EDGE);
            DPPostMessage(TOUCH_MESSAGE, m_msgid, m_ProgressCur, 1);
            Show();
            break;
        case TOUCH_UP:
        case TOUCH_SLIDE:
        case TOUCH_MOVEOUT:
            if(xoff < PROGRESS_EDGE)
                xoff = PROGRESS_EDGE;
            xoff -= PROGRESS_EDGE;
            if(xoff >= (m_width - 2 * PROGRESS_EDGE))
                xoff = (m_width - 2 * PROGRESS_EDGE);
            m_ProgressCur = (xoff) * m_ProgressTotal / (m_width - 2 * PROGRESS_EDGE);
            DPPostMessage(TOUCH_MESSAGE, m_msgid, m_ProgressCur, 0);
            Show();
            break;
        default:
            break;
    }
    return TRUE;
}

void CDPProgress::Show(void)
{
    HANDLE hcur;
    RECT rect;
    char str[8];
    HANDLE hstr;
    DWORD wide;
    DWORD width, height;
    SIZE size;
    HANDLE hpng;
    HANDLE hbak;

    hcur = m_pSpr->ReqTempBlk(m_width, m_height);
    m_pSpr->SetBK(hcur, m_ColorBak);

    rect.left = PROGRESS_EDGE;
    rect.top = PROGRESS_EDGE;
    rect.right = rect.left + (m_width - PROGRESS_EDGE * 2) * m_ProgressCur / m_ProgressTotal;
    rect.bottom = m_height - PROGRESS_EDGE;
    m_pSpr->FillSolidRect(hcur, &rect, m_ColorSrc);

    if(m_bShowValue)
    {
        sprintf(str, "%u", m_ProgressCur);
        hstr = m_pSpr->LoadStr(str, m_textsize, m_textcolor, &wide);
        m_pSpr->AlphaBlend(hcur, (m_width - wide) / 2, (m_height - m_textsize) / 2, wide, m_textsize, hstr, 0, 0);
        m_pSpr->CloseBlk(hstr);
    }

    if(strlen(m_src) != 0)
    {
        hpng = m_pSpr->LoadImage(m_src, &size);
        width = size.cx;
        height = size.cy;

        hbak = m_pSpr->ReqTempBlk(m_width, height);
        m_pSpr->BitBlt(hbak, 0, 0, m_width, height, m_pLayer->m_frame, m_left, m_top - (height - m_height) / 2);
        m_pSpr->AlphaBlend(hbak, 0, (height - m_height) / 2, m_width, m_height, hcur, 0, 0);
        m_pSpr->AlphaBlend(hbak, rect.right - width / 2, 0, width, height, hpng, 0, 0);
        m_pSpr->CloseBlk(hpng);

        m_pSpr->BitBlt(m_pLayer->m_frame,
                       m_left,
                       m_top - (height - m_height) / 2, m_width, height, hbak, 0, 0);
        m_pSpr->CloseBlk(hbak);
    }
    else
    {
        m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height, hcur, 0, 0);
        m_pSpr->CloseBlk(hcur);
    }

}

void CDPProgress::Hide(void)
{
    m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height, m_hFrameBak, 0, 0);
}
