#ifndef __WRT_NTP_H__
#define __WRT_NTP_H__


#define int8 			char
#define uint8 			unsigned char
#define uint32 			unsigned int
#define ulong32 		unsigned long
#define long32 			long
#define int32 			int
#define long64 			long long
#define u_int32 		unsigned long

#define NTP_PORT 				(123) 						/* NTP port number */
#define TIME_PORT 				(37) 						/* TIME/UDP port number */
#define NTP_PORT_STR   			"123" 						/* NTP dedicated port number string */
#define NTP_SERVER_IP   		"202.120.2.101" 			/* NTP service IP */
/* 3600s * 24h * (365days * 70years + 17days) */
#define JAN_1970 				(0x83aa7e80U) 				/* The number of seconds between 1900 and 1970 */
#define PKT_MODE(li_vn_mode) 	((u_char)((li_vn_mode) & 0x7))
#define PKT_VERSION(li_vn_mode) ((u_char)(((li_vn_mode) >> 3) & 0x7))
#define PKT_LEAP(li_vn_mode) 	((u_char)(((li_vn_mode) >> 6) & 0x3))

//#define debug 1 
struct ntp_packet
{
	uint8 li_vn_mode;
	uint8 stratum;
	uint8 poll;
	uint8 precision;
	ulong32 root_delay;
	ulong32 root_dispersion;
	// int8 ref_id[4];
	ulong32 ref_id;
	ulong32 reftimestamphigh;
	ulong32 reftimestamplow;
	ulong32 oritimestamphigh;
	ulong32 oritimestamplow;
	ulong32 recvtimestamphigh;
	ulong32 recvtimestamplow;
	ulong32 trantimestamphigh;
	ulong32 trantimestamplow;
};


#ifdef __cplusplus
extern "C" {
#endif

int sendpkt(int sockfd,struct addrinfo * res);

int getresponse(int sockfd,struct addrinfo * res,struct ntp_packet rpkt);

int setRtcTime();

int getRtcTime();

int StartNtpTime();

int setLocalTime();

#ifdef __cplusplus
}
#endif




#endif




