#ifndef __WRT_GPIO_H__
#define __WRT_GPIO_H__

typedef unsigned short 			UINT16;
typedef unsigned long long		UINT64;
typedef unsigned char			UINT8;
typedef unsigned long			DWORD;
typedef unsigned short			WORD;
typedef unsigned char			BYTE;
typedef unsigned char			BOOL;
typedef unsigned int			HANDLE;
typedef unsigned long			COLORREF;
typedef int						LONG;
typedef int						SOCKET;
typedef unsigned int    		UINT;
typedef unsigned char 			bool_t;
typedef BYTE * 					PBYTE;


#define	GPIO_A0							0
#define	GPIO_A1							1
#define	GPIO_A2							2
#define	GPIO_A3							3
#define	GPIO_A4							4
#define	GPIO_A5							5
#define	GPIO_A6							6
#define	GPIO_A7							7
#define	GPIO_A8							8
#define	GPIO_A9							9
#define	GPIO_A10						10
#define	GPIO_A11						11
#define	GPIO_A12						12
#define	GPIO_A13						13
#define	GPIO_A14						14
#define	GPIO_A15						15
#define	GPIO_A16						16
#define	GPIO_A17						17
#define	GPIO_A18						18
#define	GPIO_A19						19
#define	GPIO_A20						20
#define	GPIO_A21						21
#define	GPIO_A22						22
#define	GPIO_A23						23
#define	GPIO_A24						24
#define	GPIO_A25						25
#define	GPIO_A26						26
#define	GPIO_A27						27
#define	GPIO_A28						28
#define	GPIO_A29						29
#define	GPIO_A30						30
#define	GPIO_A31						31

#define	GPIO_B0							32
#define	GPIO_B1							33
#define	GPIO_B2							34
#define	GPIO_B3							35
#define	GPIO_B4							36
#define	GPIO_B5							37
#define	GPIO_B6							38
#define	GPIO_B7							39
#define	GPIO_B8							40
#define	GPIO_B9							41
#define	GPIO_B10						42
#define	GPIO_B11						43
#define	GPIO_B12						44
#define	GPIO_B13						45
#define	GPIO_B14						46
#define	GPIO_B15						47
#define	GPIO_B16						48
#define	GPIO_B17						49
#define	GPIO_B18						50
#define	GPIO_B19						51
#define	GPIO_B20						52
#define	GPIO_B21						53
#define	GPIO_B22						54
#define	GPIO_B23						55
#define	GPIO_B24						56
#define	GPIO_B25						57
#define	GPIO_B26						58
#define	GPIO_B27						59
#define	GPIO_B28						60
#define	GPIO_B29						61
#define	GPIO_B30						62
#define	GPIO_B31						63

#define	GPIO_C0							64
#define	GPIO_C1							65
#define	GPIO_C2							66
#define	GPIO_C3							67
#define	GPIO_C4							68
#define	GPIO_C5							69
#define	GPIO_C6							70
#define	GPIO_C7							71
#define	GPIO_C8							72
#define	GPIO_C9							73
#define	GPIO_C10						74
#define	GPIO_C11						75
#define	GPIO_C12						76
#define	GPIO_C13						77
#define	GPIO_C14						78
#define	GPIO_C15						79
#define	GPIO_C16						80
#define	GPIO_C17						81
#define	GPIO_C18						82
#define	GPIO_C19						83
#define	GPIO_C20						84
#define	GPIO_C21						85
#define	GPIO_C22						86
#define	GPIO_C23						87
#define	GPIO_C24						88
#define	GPIO_C25						89
#define	GPIO_C26						90
#define	GPIO_C27						91
#define	GPIO_C28						92
#define	GPIO_C29						93
#define	GPIO_C30						94
#define	GPIO_C31						95


#define	GPIO_D0							96
#define	GPIO_D1							97
#define	GPIO_D2							98
#define	GPIO_D3							99
#define	GPIO_D4							100
#define	GPIO_D5							101
#define	GPIO_D6							102
#define	GPIO_D7							103
#define	GPIO_D8							104
#define	GPIO_D9							105
#define	GPIO_D10						106
#define	GPIO_D11						107
#define	GPIO_D12						108
#define	GPIO_D13						109
#define	GPIO_D14						110
#define	GPIO_D15						111
#define	GPIO_D16						112
#define	GPIO_D17						113
#define	GPIO_D18						114
#define	GPIO_D19						115
#define	GPIO_D20						116
#define	GPIO_D21						117
#define	GPIO_D22						118
#define	GPIO_D23						119
#define	GPIO_D24						120
#define	GPIO_D25						121
#define	GPIO_D26						122
#define	GPIO_D27						123
#define	GPIO_D28						124
#define	GPIO_D29						125
#define	GPIO_D30						126
#define	GPIO_D31						127

#define	GPIO_E0							128
#define	GPIO_E1							129
#define	GPIO_E2							130
#define	GPIO_E3							131
#define	GPIO_E4							132
#define	GPIO_E5							133
#define	GPIO_E6							134
#define	GPIO_E7							135
#define	GPIO_E8							136
#define	GPIO_E9							137
#define	GPIO_E10						138
#define	GPIO_E11						139
#define	GPIO_E12						140
#define	GPIO_E13						141
#define	GPIO_E14						142
#define	GPIO_E15						143
#define	GPIO_E16						144
#define	GPIO_E17						145
#define	GPIO_E18						146
#define	GPIO_E19						147
#define	GPIO_E20						148
#define	GPIO_E21						149
#define	GPIO_E22						150
#define	GPIO_E23						151
#define	GPIO_E24						152
#define	GPIO_E25						153
#define	GPIO_E26						154
#define	GPIO_E27						155
#define	GPIO_E28						156
#define	GPIO_E29						157
#define	GPIO_E30						158
#define	GPIO_E31						159

#define	GPIO_F0							160
#define	GPIO_F1							161
#define	GPIO_F2							162
#define	GPIO_F3							163
#define	GPIO_F4							164
#define	GPIO_F5							165
#define	GPIO_F6							166
#define	GPIO_F7							167
#define	GPIO_F8							168
#define	GPIO_F9							169
#define	GPIO_F10						170
#define	GPIO_F11						171
#define	GPIO_F12						172
#define	GPIO_F13						173
#define	GPIO_F14						174
#define	GPIO_F15						175
#define	GPIO_F16						176
#define	GPIO_F17						177
#define	GPIO_F18						178
#define	GPIO_F19						179
#define	GPIO_F20						180
#define	GPIO_F21						181
#define	GPIO_F22						182
#define	GPIO_F23						183
#define	GPIO_F24						184
#define	GPIO_F25						185
#define	GPIO_F26						186
#define	GPIO_F27						187
#define	GPIO_F28						188
#define	GPIO_F29						189
#define	GPIO_F30						190
#define	GPIO_F31						191

#define	GPIO_G0							192 
#define	GPIO_G1							193 
#define	GPIO_G2							194 
#define	GPIO_G3							195  
#define	GPIO_G4							196 
#define	GPIO_G5							197 
#define	GPIO_G6							198 
#define	GPIO_G7							199 
#define	GPIO_G8							200 
#define	GPIO_G9							201
#define	GPIO_G10						202
#define	GPIO_G11						203 
#define	GPIO_G12						204
#define	GPIO_G13						205 
#define	GPIO_G14						206 
#define	GPIO_G15						207
#define	GPIO_G16						208
#define	GPIO_G17						209
#define	GPIO_G18						210
#define	GPIO_G19						211
#define	GPIO_G20						212 
#define	GPIO_G21						213
#define	GPIO_G22						214 
#define	GPIO_G23						215 
#define	GPIO_G24						216
#define	GPIO_G25						217 
#define	GPIO_G26						218 
#define	GPIO_G27						219
#define	GPIO_G28						220 
#define	GPIO_G29						221 
#define	GPIO_G30						222 
#define	GPIO_G31						223  


#define	GPIO_H0							224
#define	GPIO_H1							225  
#define	GPIO_H2							226  
#define	GPIO_H3							227  
#define	GPIO_H4							228 
#define	GPIO_H5							229  
#define	GPIO_H6							230  
#define	GPIO_H7							231  
#define	GPIO_H8							232  
#define	GPIO_H9							233  
#define	GPIO_H10						234  
#define	GPIO_H11						235  
#define	GPIO_H12						236  
#define	GPIO_H13						237  
#define	GPIO_H14						238  
#define	GPIO_H15						239  
#define	GPIO_H16						240  
#define	GPIO_H17						241  
#define	GPIO_H18						242  
#define	GPIO_H19						243  
#define	GPIO_H20						244  
#define	GPIO_H21						245  
#define	GPIO_H22						246  
#define	GPIO_H23						247  
#define	GPIO_H24						248  
#define	GPIO_H25						249  
#define	GPIO_H26						250  
#define	GPIO_H27						251  
#define	GPIO_H28						252  
#define	GPIO_H29						253  
#define	GPIO_H30						254  
#define	GPIO_H31						255  


#define	GPIO_I0							256  
#define	GPIO_I1							257  
#define	GPIO_I2							258  
#define	GPIO_I3							259  
#define	GPIO_I4							260 
#define	GPIO_I5							261  
#define	GPIO_I6							262  
#define	GPIO_I7							263  
#define	GPIO_I8							264 
#define	GPIO_I9							265  
#define	GPIO_I10						266  
#define	GPIO_I11						267  
#define	GPIO_I12						268  
#define	GPIO_I13						269  
#define	GPIO_I14						270  
#define	GPIO_I15						271  
#define	GPIO_I16						272  
#define	GPIO_I17						273  
#define	GPIO_I18						274  
#define	GPIO_I19						273  
#define	GPIO_I20						276  
#define	GPIO_I21						277  
#define	GPIO_I22						278  
#define	GPIO_I23						279  
#define	GPIO_I24						280  
#define	GPIO_I25						281  
#define	GPIO_I26						282  
#define	GPIO_I27						283  
#define	GPIO_I28						284  
#define	GPIO_I29						285  
#define	GPIO_I30						286  
#define	GPIO_I31						287 

#define INPUT							0
#define OUTPUT							1

#define	PULL_DISABLE					0
#define	PULL_UP							1 //light off
#define	PULL_DOWN						0 //light on

#define IOCTRL_GPIO_INIT 				0x4c434b06
#define IOCTRL_SET_GPIO_VALUE			0x4c434b07
#define IOCTRL_GET_GPIO_VALUE			0x4c434b08

#define IOCTL_START_WATCHDOG			0x12345678
#define IOCTL_REFRESH_WATCHDOG			0x12345679
#define IOCTL_STOP_WATCHDOG				0x1234567a

#define IOCTL_READ_RTC      			0x1234567b 
#define IOCTL_SET_RTC      				0x1234567c

/* Custom define */
#define LED_GPIO	  					GPIO_C4
#define WATCHDOG_GPIO					GPIO_C8
#define BUZZER_GPIO   					GPIO_C14
#define SET_KEY_GPIO  					GPIO_C15

typedef struct
{
	WORD gpio_num;
	UINT8 gpio_cfg;		//0 input 1 output (0~7)
	UINT8 gpio_pull;	//PULL_DISABLE,PULL_UP,PULL_DOWN	
	UINT8 gpio_driver;	//0~3
} GPIO_CFG;

#ifdef __cplusplus
extern "C" {
#endif

int InitGpio(int*fd,GPIO_CFG gpio_cfg);
int SetGpioVal(int fd,unsigned char data);
int GetGpioVal(int fd,unsigned char*data);
int StartWatchDog();

#ifdef __cplusplus
}
#endif



#endif




