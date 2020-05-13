#ifndef __WRTSERTIAL_H
#define __WRTSERTIAL_H

#ifdef __cplusplus
extern "C" {
#endif


int uartset(int fd, int nSpeed, int nBits, unsigned char nEvent, int nStop);

int uartopen(int port);

int uartclose(int fd);

int uartread(int fd, unsigned char *str, int size);
int uartwrite(int fd, unsigned char *str, int size);


#ifdef __cplusplus
}
#endif

#endif

