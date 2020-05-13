#ifndef __WRT_CFG_H__
#define __WRT_CFG_H__

/******************************** Description *********************************/
 
/*
 *  Using the previous structure, synchronize WEB SERVER
 */
 
/********************************* Defines ************************************/

/*
 *   Boot configuration
 */
typedef struct {
	char SN[32];
	unsigned char Version[12];    //wrt_room app version
	unsigned char SysVersion[12]; //image version
	unsigned char MAC[8];
	unsigned char version[32];
	unsigned long program_position;
	unsigned long program_size;
	unsigned long reserved[64];
}T_BOOTINFO_GW;

/*
 *   Gateway network configuration
 */
typedef struct {
	unsigned long DHCP_flag;
	unsigned long DHCP_serviceIP;
	unsigned long LocalIP;
	unsigned long SubMaskIP;
	unsigned long GateWayIP;
	unsigned long DNS_IP;
	unsigned long reserved[64];
}T_PRIVATE_GW;

/*
 *   Gateway info configuration
 */
typedef struct {
	char 		  variableKey[64];
	char 		  gateWayDeviceID[64];
	unsigned long reserved[64];
}T_MYSYSINFO_GW;

/*
 *   Gateway system configuration
 */
typedef struct {
	T_BOOTINFO_GW	BootInfo;
	T_PRIVATE_GW	LocalSetting;
	T_MYSYSINFO_GW	DoorSysInfo;
}T_SYSTEMINFO;


void set_default_cfg();
T_SYSTEMINFO* get_system_info();
int write_system_info();
int read_system_info();
int init_system_info();


#endif




