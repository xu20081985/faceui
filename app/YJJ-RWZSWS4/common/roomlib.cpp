#include "roomlib.h"

#define	SECOND_FROM_1601_TO_1970	11644473600

StaticLock::StaticLock()
{
    DPInitCriticalSection(&m_cs);
}

StaticLock::~StaticLock()
{
    DPDeleteCriticalSection(&m_cs);
}

void StaticLock::lockon()
{
    DPEnterCriticalSection(&m_cs);
}

void StaticLock::lockoff()
{
    DPLeaveCriticalSection(&m_cs);
}

DWORD hexconvert(char *data)
{
    char buf[16];
    DWORD len;
    len = strlen(data);
    if(len < 7)
        return strtol(data, NULL, 16);
    else
    {
        DWORD height, low;
        strcpy(buf, data);
        buf[len - 6] = 0;
        height = strtol(buf, NULL, 16);
        low = strtol(data + len - 6, NULL, 16);
        return ((height << 24) | low);
    }
}

char *CFindContent(char *start, char *match, char *ret, BOOL remove)
{
    char begin[32];
    char end[32];
    int leftlen;

    sprintf(begin, "<%s>", match);
    sprintf(end, "</%s>", match);
    start = strstr(start, begin);
    if(start == NULL)
        return NULL;
    match = strstr(start, end);		//-V778
    if(match == NULL)
        return NULL;
    leftlen = match - start;
    leftlen -= strlen(begin);
    memcpy(ret, start + strlen(begin), leftlen);
    ret[leftlen] = 0;
    match += strlen(end);
    leftlen = strlen(match);
    memmove(start, match, leftlen);
    start[leftlen] = 0;
    return start;
}

int BReadFile(char *filename, char **buf)
{
    FILE *tf;
    int flen;
    char *pdata;
    *buf = NULL;

    tf = fopen(filename, "rb");
    if(tf == NULL)
    {
        //printf("open file %s error\r\n", filename);
        return 0;
    }

    fseek(tf, 0, SEEK_END);
    flen = ftell(tf);
    fseek(tf, 0, SEEK_SET);

    pdata = (char *)malloc(flen + 1);
    if(pdata == NULL)
    {
        //printf("malloc buf %d error\r\n", flen);
        fclose(tf);
        return 0;
    }

    if(flen != (int)fread(pdata, 1, flen, tf))
    {
        //printf("read file %d error\r\n", flen);
        fclose(tf);
        free(pdata);
        return 0;
    }
    fclose(tf);
    pdata[flen] = 0;
    *buf = pdata;
    return flen;
}


static WORD *uni_table = NULL;
static const char GB2UN_FILE[] = "/FlashDev/FONTS/gb2un.dat";
void InitGb2Unicode(void)
{
    FILE *fd;
    int filelen;

    fd = fopen(GB2UN_FILE, "rb");
    if(fd == NULL)
    {
        DBGMSG(DPERROR, "InitGb2Unicode fopen %s fail\r\n", GB2UN_FILE);
        return;
    }
    fseek(fd, 0, SEEK_END);
    filelen = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    uni_table = (WORD *)malloc(filelen);
    fread(uni_table, 1, filelen, fd);
    fclose(fd);
}

void GbConvert(WORD *dst, BYTE *src)
{
    while(*src != 0)
    {
        if(*src < 0x80)
            *dst++ = *src++;
        else
        {
            if(*src > 0x80)
            {
                *dst++ = uni_table[(src[0] << 8) + src[1]];
                src += 2;
            }
            else
            {
                *dst++ = 0x20;
                src++;
            }
        }
    }
    *dst = 0;
}

int utf82unicode(WORD *dst, BYTE *utf8)
{
    WORD *pStart = (WORD *)dst;
    while(utf8[0] != 0)
    {
        if((utf8[0] & 0xc0) == 0xc0)
        {
            if((utf8[0] & 0x20) == 0x20)
            {
                if((utf8[0] & 0x10) == 0x10)
                {
                    if((utf8[0] & 0x08) == 0x08)
                    {
                        if((utf8[0] & 0x04) == 0x04)
                        {
                            utf8 += 5;
                        }
                        else
                            utf8 += 4;
                    }
                    else
                        utf8 += 3;
                }
                else
                {
                    *dst = ((utf8[0] & 0xf) << 12) | ((utf8[1] & 0x3f) << 6) | (utf8[2] & 0x3f);
                    dst++;
                    utf8 += 3;
                }
            }
            else
            {
                *dst = ((utf8[0] & 0x1f) << 6) | (utf8[1] & 0x3f);
                dst++;
                utf8 += 2;
            }
        }
        else
        {
            *dst++ = *utf8++;
        }
    }
    *dst = 0;
    return dst - pStart;
}

void unicode2utf8(BYTE *dst, wchar_t *unicode)
{
    WORD *ptr = NULL;
    ptr = (WORD *)unicode;
    while(*ptr != 0)
    {
        if(*ptr >= 0x0800)
        {
            *dst = 0xe0 | ((*ptr >> 12) & 0x0f);
            dst ++;
            *dst = 0x80 | ((*ptr >> 6) & 0x3f);
            dst ++;
            *dst = 0x80 | ((*ptr) & 0x3f);
            dst ++;
            ptr ++;

        }
        else if(*ptr >= 0x80 && *ptr <= 0x7ff)
        {
            *dst = 0xc0 | ((*ptr >> 6) & 0x1f);
            dst ++;
            *dst = 0x80 | (*ptr & 0x3f);
            dst ++;
            ptr ++;
        }
        else
        {
            *dst = (BYTE)(*ptr);
            dst ++;
            ptr ++;
        }
    }
    *dst = 0;
}

void unicode2wchar(wchar_t *dst, WORD *unicode)
{
    WORD *ptr = unicode;
    while(*ptr)
    {
        *dst++ = *ptr++;
    }
    *dst = 0;
}

int utf8len(char *str)
{
    if(str == NULL)
        return 0;

    int len = 0;
    BYTE *ptr = (BYTE *)str;

    while(ptr[0])
    {
        if((ptr[0] >> 4) == 0xF)		// 1111 0XXX 4Byte
            ptr += 4;
        else if((ptr[0] >> 4) == 0xE)	// 1110 XXXX 3Byte
            ptr += 3;
        else if((ptr[0] >> 5) == 0x7)	// 110X XXXX 2Byte
            ptr += 2;
        else
            ptr += 1;				// 0XXX XXXX 1Byte
        len++;
    }

    return len;
}

FILETIME timeToFileTime(const time_t *ptime)
{
    UINT64 iTime = (UINT64) * ptime * 10000000 + SECOND_FROM_1601_TO_1970;
    FILETIME ftime;
    ftime.dwHighDateTime = (DWORD)((iTime >> 32) & 0x00000000FFFFFFFF);
    ftime.dwLowDateTime = (DWORD)(iTime & 0x00000000FFFFFFFF);
    return ftime;
}

void FindFileFromDirectory(char *dir, void (*func)(char *directory, char *fileName, void *param), void *param)
{
    char fileName[MAX_PATH];
    HANDLE hFind = DPFindFirstFile(dir, fileName);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            func(dir, fileName, param);
        }
        while(DPFindNextFile(hFind, fileName));
        DPFindClose(hFind);
    }
}

BOOL DPCopyFileEx(const char *dstfile, const char *srcfile)
{
    DWORD tick = DPGetTickCount();
    BOOL ret = FALSE;
    FILE *pSrcFile = NULL;
    FILE *pDstFile = NULL;
    char *pbuf = NULL;
    int len = 0;

    do
    {
        if(dstfile == NULL || srcfile == NULL)
            break;

        pSrcFile = fopen(srcfile, "rb");
        if(pSrcFile == NULL)
            break;

        fseek(pSrcFile, 0, SEEK_END);
        len = ftell(pSrcFile);
        fseek(pSrcFile, 0, SEEK_SET);

        pbuf = (char *)malloc(len);
        if(pbuf == NULL)
            break;

        if(len != fread(pbuf, 1, len, pSrcFile))
            break;

        pDstFile = fopen(dstfile, "wb");
        if(NULL == pDstFile)
            break;

        if(len != fwrite(pbuf, 1, len, pDstFile))
            break;

        ret = TRUE;
    }
    while(0);

    if(pSrcFile)
        fclose(pSrcFile);
    if(pDstFile)
        fclose(pDstFile);
    if(pbuf)
        free(pbuf);

    DBGMSG(DPINFO, "DPCopyFileEx %s to %s len:%d, ret:%d, cost %dms\r\n", srcfile, dstfile, len, ret, DPGetTickCount() - tick);
    return ret;
}

BOOL dp_inet_addr(char *string, int *ip)
{
    char *pStart = string;
    char *pEnd = NULL;
    BOOL ret = FALSE;

    do
    {
        if(NULL == pStart)
            break;

        if(ip != NULL)
            *ip = 0;

        DWORD num;
        int i;
        for(i = 0; i < 4; i++)
        {
            num = strtol(pStart, &pEnd, 10);
            if(pStart == pEnd)
                return FALSE;

            if(num > 255)
                break;

            if(ip)
                *ip |= num << (i * 8);

            if(*pEnd != '.')
                break;

            pStart = pEnd + 1;
        }

        if(i != 3)
            break;

        if(*pEnd != 0)
            break;

        ret = TRUE;
    }
    while(0);

    return ret;
}

BOOL GetJpgSize(char *filename, int *width, int *height)
{
    FILE *pFile = fopen(filename, "rb");
    if(pFile)
    {
        WORD head, end;
        fread(&head, 1, 2, pFile);

        fseek(pFile, -2, SEEK_END);
        fread(&end, 1, 2, pFile);

        if(head != 0xD8FF || end != 0xD9FF)
        {
            fclose(pFile);
            return FALSE;
        }

        fseek(pFile, 2, SEEK_SET);

        int type = -1;
        do
        {
            int ff = -1;
            int pos = 0;

            do
            {
                ff = fgetc(pFile);
            }
            while(ff != 0xFF);

            do
            {
                type = fgetc(pFile);
            }
            while(type == 0xFF);

            switch (type)
            {
                case 0x00:
                case 0x01:
                case 0xD0:
                case 0xD1:
                case 0xD2:
                case 0xD3:
                case 0xD4:
                case 0xD5:
                case 0xD6:
                case 0xD7:
                    break;
                case 0xC0:
                case 0xC2:
                {
                    fseek(pFile, 3, SEEK_CUR);
                    int h = fgetc(pFile) * 256;
                    h += fgetc(pFile);
                    int w = fgetc(pFile) * 256;
                    w += fgetc(pFile);
                    *height = h;
                    *width = w;
                    fclose(pFile);
                    return TRUE;
                }
                break;
                default:
                    pos = fgetc(pFile) * 256 + fgetc(pFile);
                    fseek(pFile, pos - 3, SEEK_CUR);
                    break;
            }
        }
        while(type != 0xDA);

        fclose(pFile);
    }

    return FALSE;
}