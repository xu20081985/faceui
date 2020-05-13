#pragma once

#define MAX_SSID_NUMBER		64

typedef struct
{
	char bssid[32];
	char freq[6];
	char level[6];
	char flags[64];
	char ssid[32];
}wifi_scan_item;

typedef struct
{
	int count;
	wifi_scan_item *pitem;
}wifi_info;

enum
{
	WIFI_SCAN = 0x1001, 
	WIFI_CONNECT,
	WIFI_DISCONNECT
};

void StartWifiServer();
void StopWifiServer();
void ReqWifi(int cmd, char* ssid, int wpa_key, char* key);
bool GetWifiStatus(char* ssid);