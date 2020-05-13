#ifdef __cplusplus
extern "C" {
#endif	


/**
函数名称：WRTRC4Encrypt
函数功能：使用RC4算法，加密数据块,
输入参数：
	  pwd, 16位密码。需要带结束符。
	  buf, 需要加密的数据块的地址。
	  buflen,需要加密的数据块的长度。
输出参数：	  
	  outbuf:编码后的数据。长度与buflen一样。
返回：
	　0:返回成功。-1返回失败。
	  
*/
int  WRTRC4Encrypt(const char* pwd, unsigned char* buf, int buflen, unsigned char* outbuf);


/**
函数名称：WRTRC4Decrypt
函数功能：使用RC4算法，解密数据块
输入参数：
	  pwd: 16位密码。需要带结束符。
	  buf: 需要解密的数据块的地址。
	  buflen:需要解密的数据块的长度。
输出参数：	  
	  outbuf:解码后的数据。长度与buflen一样。
返回：
	　0:返回成功。-1返回失败。
*/	
int  WRTRC4Decrypt(const char* pwd, unsigned char* buf, int buflen, unsigned char* outbuf);

/**
函数名称：WRTMD5
函数功能：使用MD5算法，计算MD5校验值。
输入参数：
	  buf: 需要计算的数据块的地址。
	  buflen:需要计算的数据块的长度。
输出参数：	  
	  outbuf:计算后的校验码,经过base64编码后的16个字节。
	  outbuflen:outbuf 缓冲大小
返回：
	　0:返回成功。-1返回失败。
*/
	
int  WRTMD5(unsigned char* buf, int buflen, unsigned char* outbuf,int outbuflen);

//16进制字符输出。outbuflen 为32个字节
int  WRTMD5_16(unsigned char* buf, int buflen, char* outbuf, int outbuflen);

/**
函数名称：WRTGetRandPwd
函数功能：使用MD5算法，获取随机密码
输入参数：
	  outrandpwd: 随机密码缓冲。
	  randpwdlen 随机密码长度
返回：
	　0:返回成功。-1返回失败。
*/
	
int  WRTGetRandPwd( char* outrandpwd,int randpwdlen);




#ifdef __cplusplus
}
#endif