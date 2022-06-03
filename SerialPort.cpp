#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include "SerialPort.h"

SerialPort::SerialPort(const char *deviceName){
    if(open(deviceName) != 0)
        exit(1);
    if(configure() != 0)
        exit(2);
}

SerialPort::~SerialPort(){
    close();
}

const char *SerialPort::readString(){
    char buffer[1024];
    char *out;

    //read into buffer
    int numOfBytes = read(m_serial, &buffer, sizeof(buffer));
    if(!numOfBytes){
        fprintf(stderr,"Error from read(): Timed out.\n");
        return "";
    }

    //copy into smaller specifically sized buff
    out = (char *) malloc(numOfBytes);
    memcpy(out, buffer, numOfBytes);
    out[numOfBytes - 1] = '\0'; //replace \n with \0
    return out;
}

int SerialPort::readInt(){
    //read as a string
    char buffer[sizeof(int) + 1];
    int numOfBytes = read(m_serial, &buffer, sizeof(buffer));
    if(!numOfBytes){
        fprintf(stderr,"Error from read(): Timed out.\n");
        return -1;
    }

    //convert into an int
    return atoi(buffer);
}

int SerialPort::open(const char *deviceName){
    //open the serial port using a file descriptor
    m_serial = ::open(deviceName, O_RDWR);
	if(m_serial < 0){
		fprintf(stderr,"Error %i from open(): %s\n", errno, strerror(errno));
		return m_serial;
	}
    return 0;
}

int SerialPort::configure(){
    //read in i/o flags (see "termios" struct)
	struct termios tty;
	if(tcgetattr(m_serial, &tty)) {
		fprintf(stderr,"Error %i from tcgetattr(): %s\n", errno, strerror(errno));
		return 1;
	}	

	//communication configurations
	tty.c_cflag &= ~PARENB; //disable parity bit
	tty.c_cflag &= ~CSTOPB; //use only one stop bit
	tty.c_cflag &= ~CSIZE; //clear size bits (for statement below)
	tty.c_cflag |= CS8; //8 bits per byte
	tty.c_cflag &= ~CRTSCTS; //disable control flow
	tty.c_cflag |= CREAD | CLOCAL; //enable reading and disable control lines

	//local configurations
	tty.c_lflag |= ICANON; //enable canonical mode (input processed when receiving \n)
	tty.c_lflag &= ~ECHO; //disable echo back to serial port
	tty.c_lflag &= ~ECHOE; //disable erasure
	tty.c_lflag &= ~ECHONL; //disable new-line echo
	tty.c_lflag &= ~ISIG; //disable reading of INTR, QUIT, SUSP characters

	//input configurations
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); //disable software flow control
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP); //disable special handling

	//output configurations
	tty.c_oflag &= ~OPOST; //disable special interpretation of \n
	tty.c_oflag &= ~ONLCR; //disable conversion of \n to \r	

	//timing
	tty.c_cc[VTIME] = 50; //wait for up to n deciseconds
	tty.c_cc[VMIN] = 0;

	//set baud rate
	cfsetispeed(&tty, B9600);
	cfsetospeed(&tty, B9600);
	
	//save all of the configs stored in the struct to the operating system
	if (tcsetattr(m_serial, TCSANOW, &tty)) {
		fprintf(stderr,"Error %i from tcsetattr(): %s\n", errno, strerror(errno));
		return 1;
	}	
    return 0;
}

int SerialPort::close(){
    if(::close(m_serial)){
        fprintf(stderr, "Error from close()");
        return 1;
    }
    return 0;
}

int main(int argc, char **argv){
    SerialPort mySerial = "/dev/ttyACM0";

    const char *str = mySerial.readString();
    printf("%s\n",str);
    free((void *)str);

    printf("%d\n", mySerial.readInt());
    return 0;
}