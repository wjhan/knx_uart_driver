#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <linux/serial.h>

#define BAUDRATE B19200
#define MODEMDEVICE "/dev/ttymxc4"
#define MAX_RX_SIZE 1024

unsigned char _write_count_value = 0;
unsigned char _read_count_value = 0;
int _fd = -1;

struct Rx_Frame_Type {
	int length;
	int sourceaddress;
	int value;
};

int _write_count = 0;
int _read_count = 0;
int _error_count = 0;

struct Rx_Frame_Type rx_frame; 

int command_data = 0;

unsigned char rx_buf[MAX_RX_SIZE]={0};
int rx_w_index = 0;
int rx_r_index = 0;


void dump_data(unsigned char * b, int count) {
	printf("%i bytes: ", count);
	int i;
	for (i=0; i < count; i++) {
		printf("%02x ", b[i]);
	}

	printf("\n");
}

void rx_data_handler(unsigned char * b, int count)
{
    int i;
    for(i=0;i<count;i++) {
        rx_buf[rx_w_index] = b[i];
        rx_w_index++;
        if(rx_w_index>=MAX_RX_SIZE) rx_w_index -= MAX_RX_SIZE;
    }
}

void dump_rx_buf(int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		printf("rec: %02x\n", rx_buf[rx_r_index++]);
		if(rx_r_index>=MAX_RX_SIZE) rx_r_index -= MAX_RX_SIZE;
	} 

}

void process_rx_data(void)
{
	int i;
    int len = rx_w_index - rx_r_index;
    if(len < 0) len += MAX_RX_SIZE;

    while(len>8)
    {
       // printf("receive data len %d\n",len);
	//dump_rx_buf(len);
	i = rx_r_index;
	if(rx_buf[i]==0xBC)
	{
		dump_rx_buf(9);	
		rx_frame.length=9;
		rx_frame.sourceaddress = rx_buf[i+3]<<8 | rx_buf[i+4];
		rx_frame.value = rx_buf[i+7] & 0x0F;
		printf("The value of %02x is %d\n",rx_frame.sourceaddress,rx_frame.value);

		break;
	}

	else {
		rx_r_index ++;
		if(rx_r_index>=MAX_RX_SIZE) rx_r_index -= MAX_RX_SIZE;
		len = rx_w_index - rx_r_index;
   		if(len < 0) len += MAX_RX_SIZE;
	}			
        //rx_r_index += len;
        //if(rx_r_index>=MAX_RX_SIZE) rx_r_index -= MAX_RX_SIZE;
    }
}

void dump_serial_port_stats()
{
	struct serial_icounter_struct icount = {};

	printf("%s: count for this session: rx=%i, tx=%i, rx err=%i\n", MODEMDEVICE, _read_count, _write_count, _error_count);

	int ret = ioctl(_fd, TIOCGICOUNT, &icount);
	if (ret != -1) {
		printf("%s: TIOCGICOUNT: ret=%i, rx=%i, tx=%i, frame = %i, overrun = %i, parity = %i, brk = %i, buf_overrun = %i\n",
				MODEMDEVICE, ret, icount.rx, icount.tx, icount.frame, icount.overrun, icount.parity, icount.brk,
				icount.buf_overrun);
	}
}

void setup_serial_port(void)
{
	struct termios newtio;

	_fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (_fd < 0) {
		printf("Error opening serial port \n");
		free(MODEMDEVICE);
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));

	newtio.c_cflag = BAUDRATE | CS8;
    newtio.c_cflag |= PARENB;
    newtio.c_cflag &= ~PARODD;
    newtio.c_cflag |=INPCK;
    newtio.c_cflag &= ~CSTOPB;
    newtio.c_iflag = 0;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;

	newtio.c_cc[VINTR] = 0; /* Ctrl-c */

    newtio.c_cc[VQUIT] = 0; /* Ctrl-\ */

    newtio.c_cc[VERASE] = 0; /* del */

    newtio.c_cc[VKILL] = 0; /* @ */

    newtio.c_cc[VEOF] = 4; /* Ctrl-d */

    newtio.c_cc[VTIME] = 5; /* inter-character timer, timeout VTIME*0.1 */

    newtio.c_cc[VMIN] = 0; /* blocking read until VMIN character arrives */

    newtio.c_cc[VSWTC] = 0; /* '\0' */

    newtio.c_cc[VSTART] = 0; /* Ctrl-q */

    newtio.c_cc[VSTOP] = 0; /* Ctrl-s */

    newtio.c_cc[VSUSP] = 0; /* Ctrl-z */

    newtio.c_cc[VEOL] = 0; /* '\0' */

    newtio.c_cc[VREPRINT] = 0; /* Ctrl-r */

    newtio.c_cc[VDISCARD] = 0; /* Ctrl-u */

    newtio.c_cc[VWERASE] = 0; /* Ctrl-w */

    newtio.c_cc[VLNEXT] = 0; /* Ctrl-v */

    newtio.c_cc[VEOL2] = 0; /* '\0' */

    newtio.c_cc[VMIN]=0;
    newtio.c_cc[VTIME]=0;
    tcflush(_fd, TCIFLUSH);
    tcsetattr(_fd,TCSANOW,&newtio);
}

int main(void)
{
	unsigned char rb[1024];

	unsigned char tb[10] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};
	int _cl_stats =1;



	printf("Linux serial test app\n");

	setup_serial_port();

	//struct pollfd serial_poll;
	//serial_poll.fd = _fd;

	//serial_poll.events |= POLLIN;
	//serial_poll.events |= POLLOUT;

	struct timespec last_stat;

	clock_gettime(CLOCK_MONOTONIC, &last_stat);

	while (1) {

		/** Group Write boolen 
		groupWriteBool(_fd,2,1,227,0);
		**/

		/** Group Read Boolen status
		groupReadBoolReq(_fd,2,1,227,0);

		usleep(50000);



		int c = read(_fd, &rb, 1024);

		if(c>8) printf("knx write success\n");

		usleep(50000);
		c = read(_fd, &rb, 1024);
		if(c>0) rx_data_handler(rb,c);
		process_rx_data();
		***/


		/*** Grour write 1 Byte data ***/

		groupWriteByte(_fd,0,1,1,0xF0);
		

		sleep(3);

/**
		if(c>0) dump_data(rb, c);
		if (retval == -1) {
			perror("poll()");
		} else if (retval) {
			if (serial_poll.revents & POLLIN) {
				//process_read_data();
				int c = read(_fd, &rb, sizeof(rb));
				if(c>0) dump_data(rb, c);
			}

			if (serial_poll.revents & POLLOUT) {
				//process_write_data();
			}
		} else {
			printf("No data within ten seconds.\n");
		}
***/
		if (_cl_stats) {
			struct timespec current;
			clock_gettime(CLOCK_MONOTONIC, &current);
			if (current.tv_sec - last_stat.tv_sec > 5) {
				dump_serial_port_stats();
				last_stat = current;
				groupWriteBool(_fd,2,1,227,1);
				//groupReadBoolReq(_fd,2,1,227,0);
			}
		}

	}
	free(MODEMDEVICE);
}


