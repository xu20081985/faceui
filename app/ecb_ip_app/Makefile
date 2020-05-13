#============================================
#Makefile for wrtApp
#============================================


TARGET_TOOLCHAIN = ../../arm_toolchain/external-toolchain/bin/arm-none-linux-gnueabi-

CC = $(TARGET_TOOLCHAIN)gcc
AR = $(TARGET_TOOLCHAIN)ar
STRIP = $(TARGET_TOOLCHAIN)strip
CXX = $(TARGET_TOOLCHAIN)g++
LD = $(TARGET_TOOLCHAIN)ld


CFLAGS = -lpthread -ldl

INCLUDE = -I./wrt_lib/include \
	-I./wrt_cloud/include \
	-I./wrt_smart/include \
	-I./wrt_main/include

BIN_TARGET= wrt_room			 
			 
BIN_CXXOBJS= \
	./wrt_main/src/wrt_main.o \
	./wrt_main/src/wrt_MsgQueue.o \
	./wrt_main/src/wrt_broadcast.o \
	./wrt_main/src/wrt_common.o \
	./wrt_main/src/threadPool.o \
	./wrt_main/src/sock.o \
	./wrt_main/src/wrt_network.o \
	./wrt_main/src/wrt_msgHandler.o \
	./wrt_main/src/wrt_devHandler.o \
	./wrt_main/src/wrt_cfg.o \
	./wrt_main/src/wrt_log.o
	
BIN_COBJS= \
	./wrt_main/src/sqlite3.o \
	./wrt_main/src/queue.o \
	./wrt_main/src/wrt_serial.o \
	./wrt_main/src/wrt_gpio.o \
	./wrt_main/src/wrt_ntp.o \
	
SMART_OBJS= ./wrt_smart/src/auth.o \
	./wrt_smart/src/callbacks.o \
	./wrt_smart/src/crypt.o \
	./wrt_smart/src/md5.o \
	./wrt_smart/src/message.o \
	./wrt_smart/src/port.o \
	./wrt_smart/src/sal.o \
	./wrt_smart/src/sal_eXosip2.o \
	./wrt_smart/src/sal_eXosip2_presence.o \
	./wrt_smart/src/sal_eXosip2_sdp.o \
	./wrt_smart/src/smartUac.o \
	./wrt_smart/src/subpub.o \
	./wrt_smart/src/uuid.o
	
CLOUD_OBJS= ./wrt_cloud/src/base64.o \
	./wrt_cloud/src/dbid.o \
	./wrt_cloud/src/dstring.o \
	./wrt_cloud/src/hash_table.o \
	./wrt_cloud/src/msg_queue.o \
	./wrt_cloud/src/ptr_vector.o \
	./wrt_cloud/src/ref_cntr.o \
	./wrt_cloud/src/serialize.o \
	./wrt_cloud/src/sstr.o \
	./wrt_cloud/src/vector.o \
	./wrt_cloud/src/wrt_cloud.o
	
	
DEP_LIBS = \
	-L./wrt_lib/lib/ -lgoahead \
	-L./wrt_lib/lib/ -ljson-c \
	-L./wrt_lib/lib/ -leXosip2 \
	-L./wrt_lib/lib/ -lssl \
	-L./wrt_lib/lib/ -lcrypto \
	-L./wrt_lib/lib/ -losipparser2 \
	-L./wrt_lib/lib/ -losip2 \
	-L./wrt_lib/lib/ -lcurl \
	-L./wrt_lib/lib/ -lortp

all:$(BIN_TARGET)

$(BIN_CXXOBJS):%.o:%.cpp
	$(CXX)  $(CFLAGS) $(INCLUDE) -lstdc++ -c $< -o $@ 

$(BIN_COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(SMART_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(CLOUD_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
$(BIN_TARGET): $(BIN_CXXOBJS) $(BIN_COBJS) $(SMART_OBJS) $(CLOUD_OBJS)
	$(CXX) -o $(BIN_TARGET)  $(SMART_OBJS) $(CLOUD_OBJS) $(BIN_CXXOBJS) $(BIN_COBJS) $(DEP_LIBS)
	$(STRIP) $(BIN_TARGET)

clean:
	rm -f *.o $(BIN_COBJS) $(BIN_CXXOBJS) $(SMART_OBJS) $(CLOUD_OBJS) $(BIN_TARGET)

