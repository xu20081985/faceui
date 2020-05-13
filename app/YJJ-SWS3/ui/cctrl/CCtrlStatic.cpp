#include "CCtrlStatic.h"

BOOL CDPStatic::DoInit(ContentManage* pCm)
{
	char* pcontent;
	int text;

	// ��ȡҪ��ʾ�����ͣ�ȱʡΪ����
	if((pcontent = pCm->FindContentByName("type")) != NULL)
	{
		if(strcmp(pcontent, "text") == 0)
			m_type = ITEM_TYPE_TEXT;
		else if(strcmp(pcontent, "png") == 0)
			m_type = ITEM_TYPE_PNG;
	}

	// ��ȡ���뷽ʽ,ȱʡΪ���϶���
	if((pcontent = pCm->FindContentByName("align")) != NULL)
	{
		if(strstr(pcontent, "center") != NULL)
			m_align |= ALIGN_CENTER;
		else if(strstr(pcontent, "right") != NULL)
			m_align |= ALIGN_RIGHT;
		else
			m_align |= ALIGN_LEFT;

		if(strstr(pcontent, "middle") != NULL)
			m_align |= ALIGN_MIDDLE;
		else if(strstr(pcontent, "bottom") != NULL)
			m_align |= ALIGN_BOTTOM;
		else
			m_align |= ALIGN_TOP;
	}
	else
		m_align = ALIGN_LEFT|ALIGN_TOP;

	// ��ȡҪ��ʾ��λ��	
	if((pcontent = pCm->FindContentByName("left")) == NULL)
		return FALSE;
	m_left = strtol(pcontent, NULL, 10); 

	if((pcontent = pCm->FindContentByName("top")) == NULL)
		return FALSE;
	m_top = strtol(pcontent, NULL, 10);

	if(m_type == ITEM_TYPE_PNG)
	{
		if((pcontent = pCm->FindContentByName("srcpng")) != NULL)
			strcpy(m_src, pcontent);
	}
	else if(m_type == ITEM_TYPE_TEXT)
	{
		if((pcontent = pCm->FindContentByName("textsize")) != NULL)
			m_textsize = strtol(pcontent, NULL, 10);
		
		if((pcontent = pCm->FindContentByName("textcolor")) != NULL)
			m_textcolor = hexconvert(pcontent);
		
		if((pcontent = pCm->FindContentByName("text")) != NULL)
		{
			text = strtol(pcontent, NULL, 10);
			strcpy(m_src, GetStringByID(text));
		}
	}

	if((pcontent = pCm->FindContentByName("ishide")) == NULL)
		Show(TRUE);

	if((pcontent = pCm->FindContentByName("name")) == NULL)
		return FALSE;
	m_pLayer->RegisterCtrl(CTRL_STATIC, pcontent, this, m_msgid);
	return TRUE;
}

void CDPStatic::Show(BOOL isShow)
{
	DWORD left, top, width, height;
 	DWORD align;
	HANDLE hpng;
 	SIZE size;

	if(isShow)
	{
		if(strlen(m_src) == 0)	// ����Դ��������ʾ
			return;
		if(m_type == ITEM_TYPE_PNG)
		{
			hpng = m_pSpr->LoadImage(m_src, &size);
			width = size.cx;
			height = size.cy;
		}
		else
		{
			hpng = m_pSpr->LoadStr(m_src, m_textsize, m_textcolor, &width);
			height = m_textsize;
		}
		
		align = m_align & (ALIGN_RIGHT | ALIGN_CENTER | ALIGN_LEFT);
		switch(align)
		{
			case ALIGN_RIGHT:
				left = m_left  - width;
				break;
			case ALIGN_CENTER:
				left = m_left - width/2;
				break;
			default:
				left = m_left;
				break;
		}

		align = m_align & (ALIGN_BOTTOM | ALIGN_MIDDLE | ALIGN_TOP);
		switch(align)
		{
			case ALIGN_BOTTOM:
				top = m_top - height;
				break;
			case ALIGN_MIDDLE:
				top = m_top - height/2;
				break;
			default:
				top = m_top;
				break;
		}

		// �����ǰ������ʾ״̬���������ָ�
		// �ָ����������
		if(m_hFrameBak != NULL)
		{
			DWORD newleft, newtop, newwidth, newheight;
			HANDLE htemp;
			// �������������ʾ�;���ʾ����λ�úʹ�С
			if(m_sleft < left)
				newleft = m_sleft;
			else
				newleft = left;
			if(m_stop < top)
				newtop = m_stop;
			else
				newtop = top;
			if((m_sleft + m_swidth) > (left + width))
				newwidth = m_sleft + m_swidth;
			else
				newwidth = left + width;
			if((m_stop + m_sheight) > (top + height))
				newheight = m_stop + m_sheight;
			else
				newheight = top + height;
			newwidth -= newleft;
			newheight -= newtop;
			// �����µ�framebk
			htemp = m_pSpr->ReqTempBlk(newwidth, newheight);
			m_pSpr->BitBlt(htemp, 0, 0, newwidth, newheight, m_pLayer->m_frame, newleft, newtop);
			m_pSpr->BitBlt(htemp, m_sleft - newleft, m_stop - newtop, m_swidth, m_sheight, m_hFrameBak, 0, 0);
			m_pSpr->CloseBlk(m_hFrameBak);
			m_hFrameBak = m_pSpr->DupBlock(htemp);
			m_sleft = newleft;
			m_stop = newtop;
			m_swidth = newwidth;
			m_sheight = newheight;
			// ���µ���ʾ������ʾ����
			m_pSpr->AlphaBlend(htemp, left - newleft, top - newtop, width, height, hpng, 0, 0);
			m_pSpr->CloseBlk(hpng);
			m_pSpr->BitBlt(m_pLayer->m_frame, m_sleft, m_stop, m_swidth, m_sheight, htemp, 0, 0);
			m_pSpr->CloseBlk(htemp);
		}
		else
		{
			// ��Ҫ��ʾ��ͼƬ����
			m_hFrameBak = m_pSpr->ReqTempBlk(width, height);
			m_pSpr->BitBlt(m_hFrameBak, 0, 0, width, height, m_pLayer->m_frame, left, top);

			m_pSpr->AlphaBlend(m_pLayer->m_frame, left, top, width, height, hpng, 0, 0);
			m_pSpr->CloseBlk(hpng);
			m_sleft = left;
			m_stop = top;
			m_swidth = width;
			m_sheight = height;
		}
 	}
	else
	{
		if(m_hFrameBak == NULL)	// ��δ��ʾ��ֱ�ӷ���
			return;
		m_pSpr->BitBlt(m_pLayer->m_frame, m_sleft, m_stop, m_swidth, m_sheight, m_hFrameBak, 0, 0);
		m_pSpr->CloseBlk(m_hFrameBak);
		m_hFrameBak = NULL;
	}
}

void CDPStatic::RefreshBak()
{
	m_pSpr->CloseBlk(m_hFrameBak);
	m_hFrameBak = NULL;
}