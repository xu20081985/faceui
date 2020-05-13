#include <roomlib.h>
#include "CCtrlEditBox.h"

BOOL CEditBox::DoInit(ContentManage *pCm)
{
    char *pcontent;

    // ��ȡ������λ��
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

    // ���ݵ�ǰ�ı���
    m_hFrameBak = m_pSpr->ReqTempBlk(m_width, m_height);
    m_pSpr->BitBlt(m_hFrameBak, 0, 0, m_width, m_height, m_pLayer->m_frame, m_left, m_top);
    m_hShow = m_pSpr->ReqTempBlk(m_width, m_height);

    if((pcontent = pCm->FindContentByName("noresp")) != NULL)
        m_bResp = TRUE;

    // ��ȡ��������
    if((pcontent = pCm->FindContentByName("textsize")) != NULL)
        m_textsize = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("textcolor")) != NULL)
        m_textcolor = hexconvert(pcontent);

    if((pcontent = pCm->FindContentByName("flickcolor")) != NULL)
        m_flickColor = hexconvert(pcontent);

    // ���ɱ���
    if((pcontent = pCm->FindContentByName("pngbak")) == NULL)
        m_hBak = m_pSpr->DupBlock(m_hFrameBak);
    else
        m_hBak = SetBackGround(pcontent);

    if((pcontent = pCm->FindContentByName("pngbakfocus")) == NULL)
        m_hBakFocus = m_pSpr->DupBlock(m_hBak);
    else
        m_hBakFocus = SetBackGround(pcontent);

    m_maxlen = (m_width / m_textsize - 1) * 2;

    m_pLayer->RegisterEvent(m_left, m_top, m_width, m_height, this);
    m_msgid = m_pLayer->RequestMsgId();
    if((pcontent = pCm->FindContentByName("name")) == NULL)
        m_pLayer->RegisterCtrl(CTRL_EDITBOX, (char *)"editbox", this, m_msgid);
    else
        m_pLayer->RegisterCtrl(CTRL_EDITBOX, pcontent, this, m_msgid);
    return TRUE;
}

HANDLE CEditBox::SetBackGround(char *pngname)
{
    HANDLE hpng;
    SIZE size;
    HANDLE hbak;

    hpng = m_pSpr->LoadImage(pngname, &size);
    if(hpng == NULL)
        return NULL;
    hbak = m_pSpr->ReqTempBlk(m_width, m_height);
    m_pSpr->BitBlt(hbak, 0, 0, m_width, m_height, m_hFrameBak, 0, 0);
    m_pSpr->AlphaBlend(hbak, 0, 0, m_width, m_height, hpng, (size.cx - m_width) / 2, (size.cy - m_height) / 2);
    m_pSpr->CloseBlk(hpng);
    return hbak;
}

BOOL CEditBox::SetString(char *str)
{
    HANDLE hpng;
    DWORD wide;

    if ((str == NULL)
            || (str[0] == '\0'))
    {
        TotalLen = 0;
        CurPos = 0;
        wString[0] = 0;
        memset(szString, 0, sizeof(szString));
        if (m_bIsFocus)
            m_pSpr->BitBlt(m_hShow, 0, 0, m_width, m_height, m_hBakFocus, 0, 0);
        else
            m_pSpr->BitBlt(m_hShow, 0, 0, m_width, m_height, m_hBak, 0, 0);
        FlickPos = CTL_INPUT_ALIGN;
        m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height, m_hShow, 0, 0);
    }
    else
    {
        TotalLen = utf8len(str);
        if (TotalLen > m_maxlen)
            TotalLen = (WORD)m_maxlen;

        strncpy(szString, str, sizeof(szString) - 1);
        utf82unicode((WORD *)wString, (BYTE *)szString);
        wString[TotalLen] = 0;

        CurPos = TotalLen;
        if (m_bIsPwd)
        {
            char startstr[32];
            DWORD i = 0;
            for (i = 0; i < TotalLen; i++)
                startstr[i] = '*';
            startstr[i] = 0;
            hpng = m_pSpr->LoadStr(startstr, m_textsize, m_textcolor, &wide, wStringPos);
        }
        else
        {
            hpng = m_pSpr->LoadStr(szString, m_textsize, m_textcolor, &wide, wStringPos);
        }

        if (m_bIsFocus)
            m_pSpr->BitBlt(m_hShow, 0, 0, m_width, m_height, m_hBakFocus, 0, 0);
        else
            m_pSpr->BitBlt(m_hShow, 0, 0, m_width, m_height, m_hBak, 0, 0);
        m_pSpr->AlphaBlend(m_hShow, CTL_INPUT_ALIGN, (m_height - m_textsize) / 2, wide, m_textsize, hpng, 0, 0);
        m_pSpr->CloseBlk(hpng);
        m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height, m_hShow, 0, 0);
        FlickPos = CTL_INPUT_ALIGN + wide;
    }
    return TRUE;
}

void CEditBox::Show(BOOL isFlick)
{
    RECT rect;
    m_bFlick = isFlick;
    m_pSpr->AlphaBlend(m_pLayer->m_frame, m_left, m_top, m_width, m_height, m_hShow, 0, 0);
    if (m_bFlick)
    {
        rect.left = m_left + FlickPos;
        rect.top = m_top + (m_height - m_textsize) / 2;
        rect.right = rect.left + 2;
        rect.bottom = rect.top + m_textsize;
        m_pSpr->FillSolidRect(m_pLayer->m_frame, &rect, m_textcolor);
    }
}

void CEditBox::ShowPwd()
{
    HANDLE hpng;
    DWORD wide;
    char startstr[32];
    DWORD i = 0;

    m_pSpr->BitBlt(m_hShow, 0, 0, m_width, m_height, m_hBakFocus, 0, 0);

    if (m_bIsPwd)
    {
        for (i = 0; i < TotalLen; i++)
            startstr[i] = '*';
        startstr[i] = 0;
        hpng = m_pSpr->LoadStr(startstr, m_textsize, m_textcolor, &wide, wStringPos);
    }
    else
    {
        hpng = m_pSpr->LoadStr(szString, m_textsize, m_textcolor, &wide, wStringPos);
    }

    m_pSpr->AlphaBlend(m_hShow, CTL_INPUT_ALIGN, (m_height - m_textsize) / 2, wide, m_textsize, hpng, 0, 0);
    m_pSpr->CloseBlk(hpng);
    FlickPos = wStringPos[CurPos] + CTL_INPUT_ALIGN;
    Show(TRUE);
}

void CEditBox::ShowFlick(BOOL isFlick)
{
    RECT rect;
    m_bFlick = isFlick;
    m_pSpr->AlphaBlend(m_pLayer->m_frame, m_left, m_top, m_width, m_height, m_hShow, 0, 0);
    if (m_bFlick)
    {
        rect.left = m_left + FlickPos;
        rect.top  = m_top + (m_height + m_textsize) / 2;
        rect.right = rect.left + 20;
        rect.bottom = rect.top + 2;
        m_pSpr->FillSolidRect(m_pLayer->m_frame, &rect, m_textcolor);
    }
}

void CEditBox::FlickSet()
{
    RECT rect;

    if (m_bFlick)
    {
        m_pSpr->AlphaBlend(m_pLayer->m_frame, m_left, m_top, m_width, m_height, m_hShow, 0, 0);
        m_bFlick = FALSE;
    }
    else
    {
        m_bFlick = TRUE;
        rect.left = m_left + FlickPos;
        rect.top = m_top + (m_height + m_textsize) / 2;
        rect.right = rect.left + 20;
        rect.bottom = rect.top + 2;
        m_pSpr->FillSolidRect(m_pLayer->m_frame, &rect, m_textcolor);
    }
}

void CEditBox::Flick()
{
    RECT rect;

    if (m_bFlick)
    {
        m_pSpr->AlphaBlend(m_pLayer->m_frame, m_left, m_top, m_width, m_height, m_hShow, 0, 0);
        m_bFlick = FALSE;
    }
    else
    {
        m_bFlick = TRUE;
        rect.left = m_left + FlickPos;
        rect.top = m_top + (m_height - m_textsize) / 2;
        rect.right = rect.left + 2;
        rect.bottom = rect.top + m_textsize;
        m_pSpr->FillSolidRect(m_pLayer->m_frame, &rect, m_textcolor);
    }
}

BOOL CEditBox::Input(WORD winput)
{
    HANDLE hpng;
    DWORD wide;

    if (TotalLen >= m_maxlen)
        return FALSE;

    wString[TotalLen] = winput;
    TotalLen++;
    wString[TotalLen] = 0;
    unicode2utf8((BYTE *)szString, (wchar_t *)wString);

    CurPos++;
    m_pSpr->BitBlt(m_hShow, 0, 0, m_width, m_height, m_hBakFocus, 0, 0);
    if (m_bIsPwd)
    {
        char startstr[32];
        DWORD i = 0;
        for (i = 0; i < TotalLen; i++)
            startstr[i] = '*';
        startstr[i] = 0;
        hpng = m_pSpr->LoadStr(startstr, m_textsize, m_textcolor, &wide, wStringPos);
    }
    else
    {
        char startstr[32];
        WORD i = 0;
        for (i = 0; i < TotalLen - 1; i++)
            startstr[i] = '*';
        startstr[i] = szString[i];
        startstr[TotalLen] = 0;

        hpng = m_pSpr->LoadStr(startstr, m_textsize, m_textcolor, &wide, wStringPos);

        if (wide > m_width - CTL_INPUT_ALIGN * 2)
        {
            TotalLen--;
            CurPos--;
            wString[TotalLen] = 0;
            m_pSpr->CloseBlk(hpng);
            return FALSE;
        }
    }
    m_pSpr->AlphaBlend(m_hShow, CTL_INPUT_ALIGN, (m_height - m_textsize) / 2, wide, m_textsize, hpng, 0, 0);
    m_pSpr->CloseBlk(hpng);
    FlickPos = wStringPos[CurPos] + CTL_INPUT_ALIGN;
    Show(TRUE);
    return TRUE;
}

BOOL CEditBox::Input_show(char *str, WORD pos)
{
    HANDLE hpng;
    DWORD wide;

    CurPos = pos;
    hpng = m_pSpr->LoadStr(str, m_textsize, m_textcolor, &wide, wStringPos);
    if(wide > m_width - CTL_INPUT_ALIGN * 2)
    {
        TotalLen--;
        CurPos--;
        wString[TotalLen] = 0;
        m_pSpr->CloseBlk(hpng);
        return FALSE;
    }
    m_pSpr->BitBlt(m_hShow, 0, 0, m_width, m_height, m_hBak, 0, 0);
    m_pSpr->AlphaBlend(m_hShow, CTL_INPUT_ALIGN, (m_height - m_textsize) / 2, wide, m_textsize, hpng, 0, 0);
    m_pSpr->CloseBlk(hpng);
    m_pSpr->BitBlt(m_pLayer->m_frame, m_left, m_top, m_width, m_height, m_hShow, 0, 0);
    FlickPos = wStringPos[CurPos] + CTL_INPUT_ALIGN;
    return TRUE;
}

void CEditBox::set_curpos(WORD pos)
{
    CurPos = pos;
    FlickPos = wStringPos[CurPos] + CTL_INPUT_ALIGN;
}

void CEditBox::Delete(void)
{
    HANDLE hpng;
    DWORD wide;

    if (CurPos == 0)
        return;
    if (TotalLen != CurPos)
        memmove(&wString[CurPos - 1], &wString[CurPos], (TotalLen - CurPos) * sizeof(wString[0]));
    TotalLen--;
    CurPos--;
    wString[TotalLen] = 0;
    unicode2utf8((BYTE *)szString, (wchar_t *)wString);

    m_pSpr->BitBlt(m_hShow, 0, 0, m_width, m_height, m_hBakFocus, 0, 0);
    if (m_bIsPwd)
    {
        char startstr[32];
        DWORD i;
        for (i = 0; i < TotalLen; i++)
            startstr[i] = '*';
        startstr[i] = 0;
        hpng = m_pSpr->LoadStr(startstr, m_textsize, m_textcolor, &wide, wStringPos);
    }
    else
    {
        hpng = m_pSpr->LoadStr(szString, m_textsize, m_textcolor, &wide, wStringPos);
    }
    if (hpng != NULL)
    {
        m_pSpr->AlphaBlend(m_hShow, CTL_INPUT_ALIGN, (m_height - m_textsize) / 2, wide, m_textsize, hpng, 0, 0);
        m_pSpr->CloseBlk(hpng);
    }
    FlickPos = wStringPos[CurPos] + CTL_INPUT_ALIGN;     //��ǰ��λ������
    Show(TRUE);                                        //��ʾ��ǰλ��
    return;
}

BOOL CEditBox::DoResponse(DWORD xoff, DWORD yoff, DWORD statue)
{
    if (statue == TOUCH_UP)
    {
        DPPostMessage(TOUCH_MESSAGE, m_msgid, 0, 0);
    }
    return TRUE;
}

void CEditBox::SetFocus(BOOL IsFocus)
{
    HANDLE hpng;
    DWORD wide;

    m_bIsFocus = IsFocus;
    if(m_bIsFocus)
        m_pSpr->BitBlt(m_hShow, 0, 0, m_width, m_height, m_hBakFocus, 0, 0);
    else
        m_pSpr->BitBlt(m_hShow, 0, 0, m_width, m_height, m_hBak, 0, 0);
    if(szString[0] != '\0')
    {
        if(m_bIsPwd)
        {
            char startstr[32];
            DWORD i;
            for(i = 0; i < TotalLen; i++)
                startstr[i] = '*';
            startstr[i] = 0;
            hpng = m_pSpr->LoadStr(startstr, m_textsize, m_textcolor, &wide, wStringPos);
        }
        else
            hpng = m_pSpr->LoadStr(szString, m_textsize, m_textcolor, &wide, wStringPos);
        m_pSpr->AlphaBlend(m_hShow, CTL_INPUT_ALIGN, (m_height - m_textsize) / 2, wide, m_textsize, hpng, 0, 0);
        m_pSpr->CloseBlk(hpng);
    }
    Show(FALSE);
}
