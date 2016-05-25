#include "cserialport.h"
#include "protocol/tmsxx.h"

//计算读写时间
#define TIMEOUT_SEC(buflen, baud)       (buflen * 20 / baud + 2)
#define TIMEOUT_USEC    0
#define BAUD_DEFAUT 1

/*
   **************************************************************************************
 *  函数名称：serial_read_thread
 *  函数描述：读取串口
 *                ：
 *  入口参数：调用者的指针
 *  返回参数：无
 *  作者       ：wen
 *  日期       ：
 *  修改日期：2016-05-25
 *  修改内容：while循环中增加sleep, 否则，cpu占有率太高
 *                ：
 **************************************************************************************
*/
//读到0d 0a 认为是 结束
#define COM_BUF_SIZE 256
void * serial_read_thread(void * args)
{
    CSerialPort *pSerialPort;
    pSerialPort = (CSerialPort *)args;
    char buf[COM_BUF_SIZE];
    int wait_time, count;
    bool result;
    fd_set rcv_fd;
    int actual_size;
    struct timeval tv;
    tv.tv_sec=30;
    tv.tv_usec=0;
    wait_time = 2000; //2ms
    //2016-05-25 输出线程号，与htop中的线程号对应
    printf("%s : Line : %d  thread id %ld \n",  __FILE__, __LINE__,(long int)syscall(224));
    while(true&&pSerialPort->stopped != 1)
    {
        FD_ZERO(&rcv_fd);
        FD_SET(pSerialPort->fd,&rcv_fd);
        if (select(1+ pSerialPort->fd, &rcv_fd, NULL, NULL, &tv)>0)
        {
            if (FD_ISSET(pSerialPort->fd, &rcv_fd))
            {
                bzero(buf, sizeof(buf));
                result = false;
                count = 0;
                actual_size = read(pSerialPort->fd, buf, COM_BUF_SIZE);
                while (true)
                {
                    //printf("COM READ: size %d  count %d %s \n", actual_size,count, buf);
                    //PrintfMemory((uint8_t *)buf, actual_size);
                    if((buf[actual_size - 2] == 0xd && buf[actual_size - 1] == 0xa)||\
                            (buf[actual_size - 2] == 0x3e && buf[actual_size - 1] == 0x20))
                    {
                        result = true;
                        break;
                    }
                    else if(count >= 5)
                    {
                        break;
                    }
                    else
                    {
                        usleep(wait_time);
                        count++;
                    }
                    actual_size += read(pSerialPort->fd, buf + actual_size, COM_BUF_SIZE);

                }
                if(result)
                    pSerialPort->pTskSMS->proc_com_data(buf, actual_size);
            }
        }
        //2016-05-25 usleep 1ms
        usleep(1000);
    }
    pthread_exit(NULL);
}
CSerialPort::CSerialPort(void *parent)
{
    fd = NULL;
    thread_rcv = NULL;
    pTskSMS = (tsk_SMS_Send *)parent;
    pthread_mutex_init (&objMutex, NULL);
    port_baud = BAUD_DEFAUT;
}
/*
   **************************************************************************************
 *  函数名称：open_port
 *  函数描述：打开串口
 *  入口参数：串口路径，波特率
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int CSerialPort:: open_port(char *str_port, int baud)
{
    int res_code;
    res_code = 0;
    fd = open(str_port, O_RDWR|O_NOCTTY);
    if(fd<=0)
    {
        printf("open port error fd = %d\n", fd);
        res_code = -1;
        fd = NULL;
        goto usr_exit;
    }
    //perror("open");
    res_code = set_opt(fd, baud, 8, 'N', 1);
    if(res_code != 0)
    {
        close(fd);
        fd = NULL;
        goto usr_exit;
    }
    res_code = pthread_create(&thread_rcv, NULL, serial_read_thread, (void *)this);
    if(res_code != 0)
    {
        printf("cread thread error %d", res_code);
        close(fd);
        fd = NULL;
        goto usr_exit;
    }
    port_baud = baud;
usr_exit:
    return res_code;
}
/*
   **************************************************************************************
 *  函数名称：
 *  函数描述：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int CSerialPort::stop_port()
{
     stopped = 1;
    //关闭串口
    if(fd != NULL)
        close(fd);
    //释放线程
    usleep(1000000);
    if(thread_rcv != NULL)
    {
        pthread_cancel(thread_rcv);
        pthread_join(thread_rcv, NULL);
    }

    return 0;
}
/*
   **************************************************************************************
 *  函数名称：write_port
 *  函数描述：写端口
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int CSerialPort::write_port(char buf[], int write_bytes)
{
    int actual_bytes, len, retval;
    int error_cout;
    fd_set fs_write;
    struct timeval tv_timeout;
    actual_bytes = -1;
    retval = 0;
    if(fd == NULL)
    {
        printf("write port fd = %d \n", fd);
        goto usr_exit;
    }
    //设置等待时间
    FD_ZERO (&fs_write);
    FD_SET (fd, &fs_write);
    tv_timeout.tv_sec = TIMEOUT_SEC (write_bytes, port_baud);
    tv_timeout.tv_usec = TIMEOUT_USEC;
    pthread_mutex_lock(&objMutex);
    actual_bytes = 0;
    error_cout = 0;
    //开始写
    while (actual_bytes < write_bytes)
    {
        retval = select (fd + 1, NULL, &fs_write, NULL, &tv_timeout);
        if(retval)
        {
            error_cout = 0;
            len = write(fd, buf + actual_bytes, write_bytes - actual_bytes);
            actual_bytes += len;
        }
        else
        {
            //错误连续10次，跳出
            error_cout++;
            if(error_cout > 100)
                break;
        }
    }
    pthread_mutex_unlock(&objMutex);
usr_exit:
    return actual_bytes;
}

/*
   **************************************************************************************
 *  函数名称：set_opt
 *  函数描述：设置串口参数
 *  入口参数：波特率，停止位
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
int CSerialPort::set_opt(int fd, int nspeed, int nbits, char nevent, int nstop)
{
    struct termios newtio, oldtio;
    int res_code;
    res_code = 0;
    if(tcgetattr(fd, &oldtio)!=0)
    {
        printf("setup serial 1");
        res_code = -1;
        goto usr_exit;
    }
    bzero(&newtio, sizeof(newtio));

    newtio.c_cflag |= CLOCAL|CREAD;
    newtio.c_cflag &=~CSIZE;
    switch(nbits){
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    default:
        printf("data bits error %d", nbits);
        res_code = -1;
        goto usr_exit;
    }
    switch(nevent){
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK|ISTRIP);
        break;
    case 'E':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &=~PARODD;
        newtio.c_iflag |=(INPCK|ISTRIP);
        break;
    case 'N':
        newtio.c_cflag &=~PARENB;
        break;
    default:
        printf("nevent error %d", nevent);
        res_code = -1;
        goto usr_exit;

    }
    switch(nspeed){
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    default:
        printf("speed error %d", nspeed);
        res_code = -1;
        goto usr_exit;

    }

    if(nstop == 1)
        newtio.c_cflag &=~CSTOPB;
    else if(nstop == 2)
        newtio.c_cflag |=CSTOPB;
    else
    {
        printf("stop bit error %d", nstop);
        res_code = -1;
        goto usr_exit;
    }

    newtio.c_cc[VTIME]= 0;
    newtio.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);
    if(tcsetattr(fd, TCSANOW, &newtio) != 0)
    {
        printf("set com error tcsetattr");
        res_code = -1;
        goto usr_exit;
    }

    printf("set done!\n");
usr_exit:
    return res_code;
}
