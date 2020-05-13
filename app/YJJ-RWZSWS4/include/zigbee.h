#ifndef __WRT_ZIGBEE_H__
#define __WRT_ZIGBEE_H__

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#define ZIGBEE_HEAD_AB						0xAB
#define ZIGBEE_HEAD_BC						0xBC
#define ZIGBEE_HEAD_CD						0xCD


#define ZIGBEE_DEV_NAME						"WRT Device"

//zigbee 工作模式，1路由模式，0终端模式
#define ZIGBEE_MODE_ROUTE					0x01
#define ZIGBEE_MODE_TERMINAL				0x00

//zigbee 发送模式，1广播模式，0单播模式
#define ZIGBEE_SEND_MODE_BROADCAST			0x01
#define ZIGBEE_SEND_MODE_SINGLE				0x00

//zigbee 配置命令
#define ZIGBEE_GET_LOCAL_CONFIG				0xD1		//获取本地配置信息
#define ZIGBEE_MODIFY_LOCAL_CONFIG			0xD6		//修改本地配置
#define ZIGBEE_RESET						0xD9		//复位	
#define ZIGBEE_DEFAULT_SETTING				0xDA     	//恢复出厂 

enum {
	ZIGBEE_OK 								= 0x00,
	ZIGBEE_LENGTH_FAUSE						= 0x01,
	ZIGBEE_ADDRESS_FAUSE 					= 0x02,
	ZIGBEE_CHECK_FAUSE 						= 0x03,
	ZIGBEE_WRITE_FAUSE 						= 0x04,
	ZIGBEE_OTHER_FAUSE 						= 0x05
};


#pragma pack(1)
typedef struct {
		unsigned char dev_name[16];      
		unsigned char dev_pwd[16];
		unsigned char dev_mode[1];
		unsigned char chan[1];
		unsigned char pan_id[2];
		unsigned char my_addr[2];    //本地网络地址
		unsigned char my_ieee[8];    //本地物理地址
		unsigned char dst_addr[2];
		unsigned char dst_ieee[8];
		unsigned char reserve[1];
		unsigned char power_level[1];
		unsigned char retry_num[1];		
		unsigned char tran_timeout[1];
		unsigned char serial_rate[1];
		unsigned char serial_dataB[1];
		unsigned char serial_stopB[1];
		unsigned char serial_parityB[1];
		unsigned char send_mode[1];	
}ZIGBEE_DEV_INFO;

#pragma pack()



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __WRT_ZIGBEE_H__ */


