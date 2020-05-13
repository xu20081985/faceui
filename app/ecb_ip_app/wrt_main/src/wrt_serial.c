#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <syslog.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <termios.h>


static pthread_mutex_t uart_mutex;


int uartset(int fd, int nSpeed, int nBits, unsigned char nEvent, int nStop)
{

	struct termios opt;
	if (tcgetattr(fd, &opt) != 0) 
	{ 
		return -1;
	}
  
	opt.c_cflag |= CLOCAL | CREAD; 
	opt.c_cflag &= ~CSIZE;
	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	opt.c_oflag &= ~OPOST;
	opt.c_oflag &= ~(ONLCR | OCRNL);   

	opt.c_iflag &= ~(ICRNL | INLCR);
	opt.c_iflag &= ~(IXON | IXOFF | IXANY);  

	switch (nBits)
	{
	case 7:
		opt.c_cflag |= CS7;
	break;
	case 8:
		opt.c_cflag |= CS8;
	break;
	}

	switch (nEvent)
	{
	case 'O':
		opt.c_cflag |= PARENB;
		opt.c_cflag |= PARODD;
		opt.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E': 
		opt.c_iflag |= (INPCK | ISTRIP);
		opt.c_cflag |= PARENB;
		opt.c_cflag &= ~PARODD;
		break;
	case 'N': 
		opt.c_cflag &= ~PARENB;
		break;
	}

	switch (nSpeed)
	{
	case 2400:
		cfsetispeed(&opt, B2400);
		cfsetospeed(&opt, B2400);
		break;
	case 4800:
		cfsetispeed(&opt, B4800);
		cfsetospeed(&opt, B4800);
		break;
	case 9600:
		cfsetispeed(&opt, B9600);
		cfsetospeed(&opt, B9600);
		break;
	case 115200:
		cfsetispeed(&opt, B115200);
		cfsetospeed(&opt, B115200);
		break;
	default:
		cfsetispeed(&opt, B115200);
		cfsetospeed(&opt, B115200);
		break;
	}
	if (nStop == 1)
	{
		opt.c_cflag &= ~CSTOPB;
	}
	else if (nStop == 2)
	{
		opt.c_cflag |= CSTOPB;
	}
	
	opt.c_cc[VTIME] = 1;
	opt.c_cc[VMIN] = 50;
	tcflush(fd, TCIOFLUSH);
	if((tcsetattr(fd, TCSANOW, &opt))!=0)
	{
		return -1;
	}
	
	if(fcntl(fd, F_SETFL, 0) < 0)   
	{	 
		printf("set block mode error!\n");    
	}

	return 0;
}

int uartopen(int port)
{
	char *dev[]={"/dev/ttyS0", "/dev/ttyS1", "/dev/ttyS2"};
	int vdisable;
	int fd;
	if (port == 1)
	{ 
		fd = open("/dev/ttyS0", O_RDWR|O_NOCTTY|O_NDELAY);
		if (-1 == fd)
		return -1;
	}
	else if(port == 2)
	{ 
		fd = open("/dev/ttyS1", O_RDWR|O_NOCTTY|O_NDELAY);
		if (-1 == fd)
		return -1;
	}
	else if (port == 3)
	{
		fd = open("/dev/ttyS2", O_RDWR|O_NOCTTY|O_NDELAY);
		if (-1 == fd)
		return -1;
	}

	pthread_mutex_init(&uart_mutex, NULL);

	return fd;
}


int uartclose(int fd)
{
	pthread_mutex_destroy(&uart_mutex);
	close(fd);
	return 0;
}


int uartwrite(int fd, unsigned char *str, int size)
{
	if (NULL == str)
		return -1;
	
	int ret;
	pthread_mutex_lock(&uart_mutex);
	ret = write(fd, str, size);
	pthread_mutex_unlock(&uart_mutex);	
	return ret;
}

int uartread(int fd, unsigned char *str, int size)
{
	if (NULL == str)
		return -1;
	
	int ret;
	ret = read(fd, str, size);
	return ret;
}


