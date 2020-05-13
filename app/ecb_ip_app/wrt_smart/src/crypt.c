#include <stdio.h>
#include <stdlib.h>
#include "ortp/port.h"
#include "ortp/b64.h"
#include "openssl/rc4.h"
#include "wrt_crypt.h"
#include "openssl/md5.h"
#include "uuid.h"


/**
函数名称：WRTRC4Encrypt
函数功能：使用RC4算法，加密数据块,
输入参数：
	  pwd, 16位密码。需要带结束符。
	  buf, 需要加密的数据块的地址。
	  buflen,需要加密的数据块的长度。
	  outbuf:编码后的数据。长度与buflen一样。
返回：
	　0:返回成功。-1返回失败。
	  
*/
int  WRTRC4Encrypt(const char* pwd, unsigned char* buf, int buflen, unsigned char* outbuf)
{
	
	RC4_KEY rc4key;  
	if(pwd == NULL || buf == NULL || buflen == 0 || outbuf == NULL)
		return -1;
	RC4_set_key(&rc4key, strlen(pwd),  (const unsigned char*)pwd);
	RC4(&rc4key,buflen,buf,outbuf);
	return 0;	
}


/**
函数名称：WRTRC4Decrypt
函数功能：使用RC4算法，解密数据块
输入参数：
	  pwd: 16位密码。需要带结束符。
	  buf: 需要解密的数据块的地址。
	  buflen:需要解密的数据块的长度。
	  outbuf:解码后的数据。长度与buflen一样。
返回：
	　0:返回成功。-1返回失败。
	  
*/
int  WRTRC4Decrypt(const char* pwd, unsigned char* buf, int buflen, unsigned char* outbuf)
{
		
	RC4_KEY rc4key;  
	if(pwd == NULL || buf == NULL || buflen == 0 || outbuf == NULL)
		return -1;
	RC4_set_key(&rc4key, strlen(pwd),  (const unsigned char*)pwd);
	RC4(&rc4key,buflen,buf,outbuf);
	return 0;
}

/**
函数名称：WRTMD5
函数功能：使用MD5算法，计算MD5校验值。
输入参数：
	  buf: 需要计算的数据块的地址。
	  buflen:需要计算的数据块的长度。
	  outbuf:计算后的校验码,经过base64编码后的16个字节。
返回：
	　0:返回成功。-1返回失败。
*/
	
int  WRTMD5(unsigned char* buf, int buflen, unsigned char* outbuf, int outbuflen)
{
	unsigned char binout[18];
	char out[30];
	MD5_CTX state;
	if(outbuf == NULL || outbuflen < 16 || buf == NULL)
		return -1;
	memset(binout, 0, 18);
	MD5_Init(&state);
	MD5_Update(&state, buf, buflen);
	MD5_Final( binout,&state);
	memset(out,0,30);
	b64_encode((void const*)binout,16,out,30);
	memcpy(outbuf,out,outbuflen);
	return 0;
	
}



/**
函数名称：WRTMD5
函数功能：使用MD5算法，计算MD5校验值。
输入参数：
	  buf: 需要计算的数据块的地址。
	  buflen:需要计算的数据块的长度。
	  outbuf:计算后的校验码,32字节。
返回：
	　0:返回成功。-1返回失败。
*/
	
int  WRTMD5_16(unsigned char* buf, int buflen, char* outbuf, int outbuflen)
{
	unsigned char binout[16];
	char out[30];
	int i =0;
	MD5_CTX state;
	if(outbuf == NULL || outbuflen < 16 || buf == NULL)
		return -1;
	
	MD5_Init(&state);
	MD5_Update(&state, buf, buflen);
	MD5_Final( binout,&state);
	//memset(out,0,30);
	//b64_encode((void const*)binout,16,out,30);
	//memcpy(outbuf,binout,outbuflen);
	for(i = 0; i<16; i++){
		sprintf(outbuf+i*2,"%02x",(unsigned char)binout[i]);
	}
	return 0;
	
}




/**
函数名称：WRTGetRandPwd
函数功能：使用MD5算法，获取随机密码
输入参数：
	  outrandpwd: 随机密码缓冲。
	  randpwdlen 随机密码长度16个字节
返回：
	　0:返回成功。-1返回失败。
*/
	
int  WRTGetRandPwd( char* outrandpwd,int randpwdlen)
{
	char _out[30];
	unsigned char tmp[36];	
	unsigned char tmp2[36];
	if(outrandpwd == NULL)
		return -1;
	memset(_out,0,30);
	memset(tmp,0,36);
	memset(tmp2,0,36);
	eXosip_generate_random(tmp2,16);
	eXosip_generate_random(tmp,16);
	strcat(tmp,tmp2);
	b64_encode((const char*)tmp, (size_t)16, _out, 30);
	memcpy(outrandpwd,_out,randpwdlen);
	return 0;
}