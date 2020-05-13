#include "CCtrlMKeyboard.h"

BOOL CMKeyboard::DoInit(ContentManage *pCm)
{
    char *pcontent;
    DWORD i;

    if((pcontent = pCm->FindContentByName("row")) == NULL)
        return FALSE;
    m_row = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("col")) == NULL)
        return FALSE;
    m_col = strtol(pcontent, NULL, 10);

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

    m_hFrameBak = m_pSpr->ReqTempBlk(m_width, m_height);
    m_pSpr->BitBlt(m_hFrameBak, 0, 0, m_width, m_height, m_pLayer->m_frame, m_left, m_top);

    if((pcontent = pCm->FindContentByName("keymap")) == NULL)
        return FALSE;
    strcpy(m_keymap, pcontent);

    if((pcontent = pCm->FindContentByName("keyout")) == NULL)
        return FALSE;
    strcpy(m_keyout, pcontent);

    if((pcontent = pCm->FindContentByName("bakpng")) != NULL)
    {
        strcpy(m_bakpng[STATUS_NORMAL], pcontent);

        if((pcontent = pCm->FindContentByName("bakpngpress")) != NULL)
            strcpy(m_bakpng[STATUS_PRESSED], pcontent);
        else
            strcpy(m_bakpng[STATUS_PRESSED], m_bakpng[STATUS_NORMAL]);

        if((pcontent = pCm->FindContentByName("bakpngfocus")) != NULL)
            strcpy(m_bakpng[STATUS_FOCUS], pcontent);
        else
            strcpy(m_bakpng[STATUS_FOCUS], m_bakpng[STATUS_PRESSED]);

        if((pcontent = pCm->FindContentByName("bakpngunack")) != NULL)
            strcpy(m_bakpng[STATUS_UNACK], pcontent);
        else
            strcpy(m_bakpng[STATUS_UNACK], m_bakpng[STATUS_PRESSED]);
    }

    if((pcontent = pCm->FindContentByName("textsize")) != NULL)
    {
        m_textsize = strtol(pcontent, NULL, 10);
    }

    if((pcontent = pCm->FindContentByName("textcolor")) != NULL)
    {
        m_textcolor = hexconvert(pcontent);
    }

    if((pcontent = pCm->FindContentByName("buttonleft")) != NULL)
        m_buttommap[BUTTON_LEFT] = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("buttonup")) != NULL)
        m_buttommap[BUTTON_UP] = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("buttonrignt")) != NULL)
        m_buttommap[BUTTON_RIGHT] = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("buttonup")) != NULL)
        m_buttommap[BUTTON_DOWN] = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("buttonprev")) != NULL)
        m_buttommap[BUTTON_PREV] = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("buttonnext")) != NULL)
        m_buttommap[BUTTON_NEXT] = strtol(pcontent, NULL, 10);

    if((pcontent = pCm->FindContentByName("buttonenter")) != NULL)
        m_buttommap[BUTTON_ENTER] = strtol(pcontent, NULL, 10);

    m_kwidth = m_width / m_col;
    m_kheight = m_height / m_row;
    m_maxkey = m_col * m_row;

    m_pLayer->RegisterEvent(m_left, m_top, m_width, m_height, this);
    m_pLayer->RegisterCtrl(CTRL_MKEYBOARD, (char *)"mkeybd", this, m_msgid);

    for(i = 0; i < BUTTON_MAX; i++)
    {
        if(m_buttommap[i] != 0)
            m_pLayer->RegisterKeyEvent(m_buttommap[i], this);
    }
    for(i = 0; i < m_col * m_row; i++)
        Show(i, STATUS_NORMAL);
    return TRUE;
}

void CMKeyboard::Show(DWORD index, DWORD status)
{
    DWORD col, row;
    HANDLE hbak = NULL;
    HANDLE himage = NULL;
    HANDLE hstr = NULL;
    DWORD wide;
    SIZE size;
    char keyname[32];

    row = index / m_col;
    col = index % m_col;
    hbak = m_pSpr->ReqTempBlk(m_kwidth, m_kheight);
    m_pSpr->BitBlt(hbak, 0, 0, m_kwidth, m_kheight, m_hFrameBak, col * m_kwidth, row * m_kheight);

    himage = m_pSpr->LoadImage(m_bakpng[status], &size);
    sprintf(keyname, "%c", m_keymap[index]);
    hstr = m_pSpr->LoadStr(keyname, m_textsize, m_textcolor, &wide, NULL);

    HANDLE himage2 = m_pSpr->DupBlock(himage);
    m_pSpr->CloseBlk(himage);
    m_pSpr->AlphaBlend(himage2, (size.cx - wide) / 2, (size.cy - m_textsize) / 2, wide, m_textsize, hstr, 0, 0);
    m_pSpr->CloseBlk(hstr);
    m_pSpr->AlphaBlend(hbak, (m_kwidth - size.cx) / 2, (m_kheight - size.cy) / 2, size.cx, size.cy, himage2, 0, 0);
    m_pSpr->CloseBlk(himage2);

    m_pSpr->BitBlt(m_pLayer->m_frame, m_left + col * m_kwidth, m_top + row * m_kheight, m_kwidth, m_kheight, hbak, 0, 0);
    m_pSpr->CloseBlk(hbak);
}

BOOL CMKeyboard::DoResponse(DWORD x, DWORD y, DWORD statue)
{
    DWORD col, row;
    col = x / m_kwidth;
    row = y / m_kheight;

    switch(statue)
    {
        case TOUCH_DOWN:
            m_isPressed = TRUE;
            m_dwPressed = row * m_col + col;
            Show(m_dwPressed, STATUS_PRESSED);
            break;
        case TOUCH_VALID:
            break;
        case TOUCH_MOVEOUT:
            if(m_isPressed)
            {
                Show(m_dwPressed, STATUS_NORMAL);
                m_isPressed = FALSE;
            }
            break;
        case TOUCH_UP:
            if(m_isPressed)
            {
                DPPostMessage(KBD_MESSAGE, KBD_CTRL, m_keyout[m_dwPressed], 0);
                Show(m_dwPressed, STATUS_NORMAL);
                m_isPressed = FALSE;
            }
            break;
    }
    return TRUE;
}
