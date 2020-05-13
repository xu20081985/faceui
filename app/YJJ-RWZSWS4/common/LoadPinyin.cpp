#include "roomlib.h"

typedef struct
{
    char pinyin[8];
    char *poffset;
} HeadObj;

static HeadObj m_head[512];
static char *m_string = NULL;
static DWORD m_headtotal = 0;

BOOL LoadPinyin(char *pPath)
{
    char wszLine[256];
    FILE   *fin;
    int flen;
    char *pstr;
    char *pcur;

    if(m_string != NULL)
    {
        free(m_string);
        m_string = NULL;
    }
    m_headtotal = 0;

    fin = fopen(pPath, "rb");

    if(fin == NULL)
        return FALSE;

    fseek(fin, 0, SEEK_END);
    flen = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    m_string = (char *)malloc(flen);
    if(NULL == m_string)
    {
        fclose(fin);
        return FALSE;
    }
    pcur = m_string;

    fgets(wszLine, 4, fin);
    while (fgets(wszLine, 256, fin))
    {
        pstr = strchr(wszLine, ':');
        if(pstr == NULL)
            continue;
        *pstr = 0;
        pstr++;
        strcpy(m_head[m_headtotal].pinyin, wszLine);
        m_head[m_headtotal].poffset = pcur;
        strcpy(pcur, pstr);
        pstr = strchr(pcur, '\n');
        if(pstr)
        {
            if(pstr[-1] == '\r')
                pstr[-1] = '\0';
            pstr[0] = '\0';
        }
        pcur += strlen(pcur) + 1;
        m_headtotal++;
    }
    fclose(fin);
    return TRUE;
}

char *GetStringByPinyin(char *pinyin)
{
    DWORD i;
    for(i = 0; i < m_headtotal; i++)
    {
        if(strcmp(pinyin, m_head[i].pinyin) == 0)
            break;
    }
    if(i == m_headtotal)
        return NULL;
    return m_head[i].poffset;
}

