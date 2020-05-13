
DEFINE = -D DPLINUX

OBJS = YJJ-SWS3.o \
		dpbase.o dpcom.o dpdevice.o dpfile.o dptime.o dplib.o dptcpip.o\
		dpgraphic.o dptext.o jpegdec.o pngimage.o \
		DBConfig.o DBSet.o DBSmartDev.o dpdebug.o inputkb.o inputtp.o LoadPinyin.o loadstring.o roomlib.o ServiceFile.o systemmsg.o ServiceSmart.o SmartConfig.o ServicePC.o \
		CCtrlBase.o CAppBase.o\
		CCtrlButton.o CCtrlClock.o CCtrlEditBox.o CCtrlEmpty.o CCtrlKeyboard.o CCtrlListView.o \
		CCtrlMButton.o CCtrlProgress.o CCtrlStatic.o CCtrlTable.o CCtrlText.o CCtrlTime.o CAppshow.o CAppPrjSetTemp.o \
		CCtrlMKeyboard.o CCtrlEnKeyboard.o CCtrlImeInput.o CCtrlMTable.o \
		CAppMain.o CAppPwdInput.o CAppProject.o CAppClock.o CAppMechine.o CAppBK.o CAppTip.o \
		CAppPrjCalendar.o CAppPrjInfo.o CAppPrjLanguage.o CAppPrjReport.o CAppPrjReset.o CAppPrjSetDate.o CAppPrjSetDelay.o CAppPrjSetPwd.o CAppPrjSetTimer.o CAppPrjSetUI.o CAppPrjText.o CAppPrjUpgrade.o \
		CAppTimerAction.o CAppTimerDevice.o CAppTimerObject.o CAppTimerSelect.o CAppTimerTime.o CAppTimerWeek.o CAppTimerEdit.o \
		CAppLight.o CAppCurtain.o CAppDimmer.o CAppScene.o CAppAirC.o CAppHeat.o CAppWindow.o CAppOutlet.o CAppTV.o CAppMusic.o CAppirair.o CAppNewWind.o
		
		
EXE = app0
CC = ../../arm_toolchain/external-toolchain/bin/arm-none-linux-gnueabi-g++
INC = -I ./include -I ./dpgraphic -I ./ui/cctrl -I .
CFLAGS = -c -Wall -O2 -rdynamic $(DEFINE) $(INC)
COM = ./common/
DPBASE = ./dpbaseLinux/
GRAPHIC = ./dpgraphic/
CCTRL = ./ui/cctrl/
APP = ./ui/
LIB = -lpthread -lrt -L ./dpbaseLinux/lib -lfreetype -ljpeg -lpng16 -lz

$(EXE) : $(OBJS)
	$(CC) $(OBJS) $(LIB) -o $(EXE)	
	
#APP
CAppMain.o : $(APP)CAppMain.cpp
	$(CC) $(CFLAGS) $(APP)CAppMain.cpp
CAppClock.o : $(APP)CAppClock.cpp
	$(CC) $(CFLAGS) $(APP)CAppClock.cpp
CAppPwdInput.o : $(APP)CAppPwdInput.cpp
	$(CC) $(CFLAGS) $(APP)CAppPwdInput.cpp
CAppProject.o : $(APP)CAppProject.cpp
	$(CC) $(CFLAGS) $(APP)CAppProject.cpp
CAppMechine.o : $(APP)CAppMechine.cpp
	$(CC) $(CFLAGS) $(APP)CAppMechine.cpp
CAppBK.o : $(APP)CAppBK.cpp
	$(CC) $(CFLAGS) $(APP)CAppBK.cpp
CAppTip.o : $(APP)CAppTip.cpp
	$(CC) $(CFLAGS) $(APP)CAppTip.cpp
	
#Timer
CAppTimerAction.o : $(APP)CAppTimerAction.cpp
	$(CC) $(CFLAGS) $(APP)CAppTimerAction.cpp
CAppTimerDevice.o : $(APP)CAppTimerDevice.cpp
	$(CC) $(CFLAGS) $(APP)CAppTimerDevice.cpp
CAppTimerObject.o : $(APP)CAppTimerObject.cpp
	$(CC) $(CFLAGS) $(APP)CAppTimerObject.cpp
CAppTimerSelect.o : $(APP)CAppTimerSelect.cpp
	$(CC) $(CFLAGS) $(APP)CAppTimerSelect.cpp
CAppTimerTime.o : $(APP)CAppTimerTime.cpp
	$(CC) $(CFLAGS) $(APP)CAppTimerTime.cpp
CAppTimerWeek.o : $(APP)CAppTimerWeek.cpp
	$(CC) $(CFLAGS) $(APP)CAppTimerWeek.cpp
CAppTimerEdit.o : $(APP)CAppTimerEdit.cpp
	$(CC) $(CFLAGS) $(APP)CAppTimerEdit.cpp	
	
#Project
CAppPrjCalendar.o : $(APP)CAppPrjCalendar.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjCalendar.cpp
CAppPrjInfo.o : $(APP)CAppPrjInfo.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjInfo.cpp
CAppPrjLanguage.o : $(APP)CAppPrjLanguage.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjLanguage.cpp
CAppPrjReport.o : $(APP)CAppPrjReport.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjReport.cpp
CAppPrjReset.o : $(APP)CAppPrjReset.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjReset.cpp
CAppPrjSetDate.o : $(APP)CAppPrjSetDate.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjSetDate.cpp
CAppPrjSetDelay.o : $(APP)CAppPrjSetDelay.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjSetDelay.cpp
CAppPrjSetPwd.o : $(APP)CAppPrjSetPwd.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjSetPwd.cpp
CAppPrjSetTimer.o : $(APP)CAppPrjSetTimer.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjSetTimer.cpp
CAppPrjSetUI.o : $(APP)CAppPrjSetUI.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjSetUI.cpp
CAppPrjText.o : $(APP)CAppPrjText.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjText.cpp
CAppPrjUpgrade.o : $(APP)CAppPrjUpgrade.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjUpgrade.cpp
CAppshow.o : $(APP)CAppshow.cpp
	$(CC) $(CFLAGS) $(APP)CAppshow.cpp
CAppPrjSetTemp.o : $(APP)CAppPrjSetTemp.cpp
	$(CC) $(CFLAGS) $(APP)CAppPrjSetTemp.cpp
	
#SmartDev
CAppLight.o : $(APP)CAppLight.cpp
	$(CC) $(CFLAGS) $(APP)CAppLight.cpp
CAppCurtain.o : $(APP)CAppCurtain.cpp
	$(CC) $(CFLAGS) $(APP)CAppCurtain.cpp
CAppDimmer.o : $(APP)CAppDimmer.cpp
	$(CC) $(CFLAGS) $(APP)CAppDimmer.cpp
CAppScene.o : $(APP)CAppScene.cpp
	$(CC) $(CFLAGS) $(APP)CAppScene.cpp
CAppAirC.o : $(APP)CAppAirC.cpp
	$(CC) $(CFLAGS) $(APP)CAppAirC.cpp
CAppHeat.o : $(APP)CAppHeat.cpp
	$(CC) $(CFLAGS) $(APP)CAppHeat.cpp
CAppWindow.o : $(APP)CAppWindow.cpp
	$(CC) $(CFLAGS) $(APP)CAppWindow.cpp
CAppOutlet.o : $(APP)CAppOutlet.cpp
	$(CC) $(CFLAGS) $(APP)CAppOutlet.cpp
CAppTV.o : $(APP)CAppTV.cpp
	$(CC) $(CFLAGS) $(APP)CAppTV.cpp
CAppMusic.o : $(APP)CAppMusic.cpp
	$(CC) $(CFLAGS) $(APP)CAppMusic.cpp
CAppirair.o : $(APP)CAppirair.cpp
	$(CC) $(CFLAGS) $(APP)CAppirair.cpp
CAppNewWind.o : $(APP)CAppNewWind.cpp
	$(CC) $(CFLAGS) $(APP)CAppNewWind.cpp	
#common
DBConfig.o : $(COM)DBConfig.cpp
	$(CC) $(CFLAGS) $(COM)DBConfig.cpp
DBSet.o : $(COM)DBSet.cpp
	$(CC) $(CFLAGS) $(COM)DBSet.cpp
DBSmartDev.o : $(COM)DBSmartDev.cpp
	$(CC) $(CFLAGS) $(COM)DBSmartDev.cpp
dpdebug.o : $(COM)dpdebug.cpp
	$(CC) $(CFLAGS) $(COM)dpdebug.cpp
inputkb.o : $(COM)inputkb.cpp
	$(CC) $(CFLAGS) $(COM)inputkb.cpp
inputtp.o : $(COM)inputtp.cpp
	$(CC) $(CFLAGS) $(COM)inputtp.cpp
LoadPinyin.o : $(COM)LoadPinyin.cpp
	$(CC) $(CFLAGS) $(COM)LoadPinyin.cpp
loadstring.o : $(COM)loadstring.cpp
	$(CC) $(CFLAGS) $(COM)loadstring.cpp
roomlib.o : $(COM)roomlib.cpp
	$(CC) $(CFLAGS) $(COM)roomlib.cpp
ServiceFile.o : $(COM)ServiceFile.cpp
	$(CC) $(CFLAGS) $(COM)ServiceFile.cpp
ServicePC.o : $(COM)ServicePC.cpp
	$(CC) $(CFLAGS) $(COM)ServicePC.cpp
systemmsg.o : $(COM)systemmsg.cpp
	$(CC) $(CFLAGS) $(COM)systemmsg.cpp
ServiceSmart.o : $(COM)ServiceSmart.cpp
	$(CC) $(CFLAGS) $(COM)ServiceSmart.cpp
SmartConfig.o : $(COM)SmartConfig.cpp
	$(CC) $(CFLAGS) $(COM)SmartConfig.cpp
	
#dpbase
dpbase.o : $(DPBASE)dpbase.cpp
	$(CC) $(CFLAGS) $(DPBASE)dpbase.cpp
dpcom.o : $(DPBASE)dpcom.cpp
	$(CC) $(CFLAGS) $(DPBASE)dpcom.cpp
dpdevice.o : $(DPBASE)dpdevice.cpp
	$(CC) $(CFLAGS) $(DPBASE)dpdevice.cpp
dpfile.o : $(DPBASE)dpfile.cpp
	$(CC) $(CFLAGS) $(DPBASE)dpfile.cpp
dptime.o : $(DPBASE)dptime.cpp
	$(CC) $(CFLAGS) $(DPBASE)dptime.cpp
dplib.o : $(DPBASE)dplib.cpp
	$(CC) $(CFLAGS) $(DPBASE)dplib.cpp
dptcpip.o : $(DPBASE)dptcpip.cpp
	$(CC) $(CFLAGS) $(DPBASE)dptcpip.cpp
	
#dpgraphic
pngimage.o : $(GRAPHIC)pngimage.cpp
	$(CC) $(CFLAGS) $(GRAPHIC)pngimage.cpp
jpegdec.o : $(GRAPHIC)jpegdec.cpp
	$(CC) $(CFLAGS) $(GRAPHIC)jpegdec.cpp
dptext.o : $(GRAPHIC)dptext.cpp
	$(CC) $(CFLAGS) $(GRAPHIC)dptext.cpp
dpgraphic.o : $(GRAPHIC)dpgraphic.cpp
	$(CC) $(CFLAGS) $(GRAPHIC)dpgraphic.cpp

#Ctrl Modules
CCtrlMTable.o : $(CCTRL)CCtrlMTable.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlMTable.cpp
	
CCtrlMKeyboard.o : $(CCTRL)CCtrlMKeyboard.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlMKeyboard.cpp
	
CCtrlEnKeyboard.o : $(CCTRL)CCtrlEnKeyboard.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlEnKeyboard.cpp
	
CCtrlImeInput.o : $(CCTRL)CCtrlImeInput.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlImeInput.cpp
	
CCtrlTime.o : $(CCTRL)CCtrlTime.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlTime.cpp
	
CCtrlText.o : $(CCTRL)CCtrlText.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlText.cpp
	
CCtrlTable.o : $(CCTRL)CCtrlTable.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlTable.cpp
	
CCtrlStatic.o : $(CCTRL)CCtrlStatic.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlStatic.cpp
	
CCtrlProgress.o : $(CCTRL)CCtrlProgress.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlProgress.cpp
	
CCtrlMButton.o : $(CCTRL)CCtrlMButton.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlMButton.cpp
	
CCtrlListView.o : $(CCTRL)CCtrlListView.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlListView.cpp
	
CCtrlKeyboard.o : $(CCTRL)CCtrlKeyboard.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlKeyboard.cpp
	
CCtrlEditBox.o : $(CCTRL)CCtrlEditBox.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlEditBox.cpp	
	
CCtrlEmpty.o : $(CCTRL)CCtrlEmpty.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlEmpty.cpp
	
CCtrlClock.o : $(CCTRL)CCtrlClock.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlClock.cpp
	
CCtrlButton.o : $(CCTRL)CCtrlButton.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlButton.cpp
	
CCtrlBase.o : $(CCTRL)CCtrlBase.cpp
	$(CC) $(CFLAGS) $(CCTRL)CCtrlBase.cpp
	
CAppBase.o : $(CCTRL)CAppBase.cpp
	$(CC) $(CFLAGS) $(CCTRL)CAppBase.cpp
	
#main
YJJ-SWS3.o : YJJ-SWS3.cpp
	$(CC) $(CFLAGS) YJJ-SWS3.cpp
	
clean:
	rm -f $(OBJS)

