#ifndef CSERIALPORT_H
#define CSERIALPORT_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
//#include "packet.h"
#include "tsk_sms_send.h"

#define SAVEFORMAT	"/var/save"
#define  DEVNAME2 "/dev/tts/2"
#define  DEVNAME1 "/dev/tts/1"
#define STOP '#'
class CSerialPort
{
public:
    CSerialPort(void * parent);
public:
    volatile int fd;
    volatile int savefd;
    volatile int stopped;
    pthread_t thread_rcv;
    struct tsk_SMS_Send *pTskSMS;
    int port_baud;   //波特率
public:
    int set_opt(int fd, int nspeed, int nbits, char nevent, int nstop);
    int open_port(char *str_port, int baud);
    int stop_port();
    int write_port(char buf[], int write_bytes);
private:
    pthread_mutex_t  objMutex;
};

#endif // CSERIALPORT_H
