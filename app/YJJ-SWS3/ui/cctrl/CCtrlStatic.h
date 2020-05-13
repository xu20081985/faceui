#pragma once
#include "CCtrlBase.h"

class CDPStatic:public CCtrlBase
{
public:
	CDPStatic(CLayOut* pLayer):CCtrlBase(pLayer)
	{
		m_type = ITEM_TYPE_TEXT;
		m_textcolor = 0xFFFFFFFF;
		m_src[0] = 0;
		m_swidth = 0;
		m_sheight = 0;
		m_align = 0;
		m_sleft = 0;
		m_stop = 0;
		m_textsize = GetTextSize(TS_STATIC);
	}
	~CDPStatic()
	{
		if(m_hFrameBak != NULL)
		{
			m_pSpr->CloseBlk(m_hFrameBak);
			m_hFrameBak = NULL;
		}
	}

	BOOL DoInit(ContentManage*);

	void SetSrc(char* fpng)
	{
		strcpy(m_src, fpng);
	}

	void SetSrc(DWORD fpng)
	{
		sprintf(m_src, "%u", fpng);
	}

	BOOL SrcPngCompare(char* png)
	{
		if(strcmp(m_src, png) == 0)
			return TRUE;
		else
			return FALSE;
	}

	void SetTextColor(DWORD color)
	{
		m_textcolor = color;
	}
	
	void SetTextSize(DWORD size)
	{
		m_textsize = size;
	}
	void SetAlign(DWORD align)
	{
		m_align = align;
	}
	void SetType(DWORD type)
	{
		m_type = type;
	}
	void SetStart(DWORD x, DWORD y)
	{
		m_left = x;
		m_top = y;
	}
	DWORD GetWidth()
	{
		return m_swidth;
	}
	DWORD GetTop()
	{
		return m_top;
	}
	void Show(BOOL isShow);
	void RefreshBak();	// ˢ�±���
private:
	DWORD m_textsize;	// ���ֵİ���
	DWORD m_textcolor;	// ���ֵ���ɫ
	char m_src[256];	// ����Ҫ��ʾ�����ֵ��ַ�����Ҫ��ʾ��ͼƬ�ļ���
	DWORD m_type;		// ��ǰ��ʾ�������ֻ���ͼƬ
	DWORD m_align;		// ���뷽ʽ��������ֱ��ˮƽ����Ķ��뷽ʽ
	DWORD m_sleft;		// ���ĸ�����������ǵ�ǰ��ʾ��λ�úʹ�С���������ݱ�����m_hFrameBak��
	DWORD m_stop;
	DWORD m_swidth;
	DWORD m_sheight;
};


