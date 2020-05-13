#include "dpwifi.h"
#include "roomlib.h"

static wifi_info g_info;
static BOOL g_bWifiConnect;

void StartWifiServer()
{
	g_info.count = 0;
	g_info.pitem = (wifi_scan_item *)malloc(sizeof(wifi_scan_item) * MAX_SSID_NUMBER);
}

void StopWifiServer()
{

}

DWORD CALLBACK ScanThread(LPVOID pParam)
{
	Sleep(2000);
	g_info.count = 10;
	for(int i = 0; i < 10; i++)
	{
		sprintf(g_info.pitem[i].ssid, "ssid_%d", i + 1);
		switch(i % 3)
		{
		case 0:
			strcpy(g_info.pitem[i].flags, "[ESS]");
			break;
		case 1:
			strcpy(g_info.pitem[i].flags, "[PSK]");
			break;
		case 2:
			strcpy(g_info.pitem[i].flags, "[WEP]");
			break;
		}
	}
	DPPostMessage(MSG_PRIVATE, CLOUD_WIFI_APPID, WIFI_SCAN, (DWORD)&g_info);
	return 0;
}

DWORD CALLBACK ConnectThread(LPVOID pParam)
{
	Sleep(3000);
	static BOOL flag = TRUE;
	DPPostMessage(MSG_PRIVATE, CLOUD_WIFI_APPID, WIFI_CONNECT, flag);
	flag = !flag;
	return 0;
}

void ReqWifi(int cmd, char* ssid, int wpa_key, char* key)
{
	if(cmd == WIFI_SCAN)
		CloseHandle(CreateThread(NULL, 0, ScanThread, NULL, 0, NULL));
	if(cmd == WIFI_CONNECT)
		CloseHandle(CreateThread(NULL, 0, ConnectThread, NULL, 0, NULL));
}

bool GetWifiStatus(char* ssid)
{
	return g_bWifiConnect;
}