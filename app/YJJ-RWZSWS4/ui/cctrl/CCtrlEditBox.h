#pragma once
#include "CCtrlBase.h"

#define	CTL_INPUT_ALIGN			6

class CEditBox: public CCtrlBase
{
public:
    CEditBox(CLayOut *pLayer): CCtrlBase(pLayer)
    {
        m_hShow = NULL;
        m_hBak = NULL;
        m_hBakFocus = NULL;
        TotalLen = 0;
        memset(wString, 0, sizeof(wString));
        memset(szString, 0, sizeof(szString));
        FlickPos = CTL_INPUT_ALIGN;
        CurPos = 0;
        m_bFlick = FALSE;
        m_textsize = GetTextSize(TS_EDITBOX);
        m_textcolor = 0;
        m_bIsFocus = FALSE;
        m_maxlen = 2;
        m_bIsPwd = FALSE;
        m_bResp = TRUE;
        m_flickColor = 0xffffffff;
    }

    ~CEditBox()
    {
        if(m_hBak != NULL)
        {
            m_pSpr->CloseBlk(m_hBak);
            m_hBak = NULL;
        }
        if(m_hBakFocus != NULL)
        {
            m_pSpr->CloseBlk(m_hBakFocus);
            m_hBakFocus = NULL;
        }
        if(m_hShow != NULL)
        {
            m_pSpr->CloseBlk(m_hShow);
            m_hShow = NULL;
        }
        if(m_hFrameBak != NULL)
        {
            m_pSpr->CloseBlk(m_hFrameBak);
            m_hFrameBak = NULL;
        }
    }

    BOOL DoInit(ContentManage *);
    BOOL SetString(char *str);
    void Show(BOOL isFlick);
    void ShowPwd();
    void ShowFlick(BOOL isFlick);
    void Flick();
    void FlickSet();
    BOOL Input(WORD winput);
    BOOL Input_show(char *str, WORD pos);
    void set_curpos(WORD pos);
    void Delete(void);
    BOOL DoResponse(DWORD xoff, DWORD yoff, DWORD statue);
    void SetMaxLen(DWORD len)
    {
        m_maxlen = len;
    }
    DWORD GetMaxLen(void)
    {
        return m_maxlen;
    }
    void SetIsPwd(BOOL ispwd)
    {
        m_bIsPwd = ispwd;
    }
    char *GetString(void)
    {
        return szString;
    }


    void SetFocus(BOOL IsFocus);
    DWORD GetCurCount(void)
    {
        return TotalLen;
    }
private:
    void SetPos(DWORD offset);
    HANDLE SetBackGround(char *pngname);
    char szString[512];
    WORD wString[128];
    WORD wStringPos[256];
    WORD TotalLen;
    WORD CurPos;
    DWORD FlickPos;

    HANDLE m_hBak;		// 正常背景
    HANDLE m_hBakFocus;	// 聚焦时背景
    HANDLE m_hShow;		// 当前显示的内容

    DWORD m_textcolor;
    DWORD m_textsize;

    BOOL m_bFlick;
    DWORD m_maxlen;
    BOOL m_bIsFocus;	// 当前是否处于聚焦状态
    BOOL m_bIsPwd;		// 当前是否处于密码状态
    BOOL m_bResp;		// 是否响应点击
    DWORD m_flickColor;
};

