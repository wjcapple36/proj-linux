#include <QDebug>
#include "tsk_host_commu.h"
#include "protocol/tmsxx.h"
#include "sys/ioctl.h"
#include <linux/watchdog.h>

tsk_host_commu::tsk_host_commu(QObject *parent) :
    QThread(parent)
{
    pMainWidow = (MainWindow *)parent;
    stopped = false;

    up_cyc_curv = false;
    timer_space = 250;
    usr_timer_initial();
    led_run_state = 0;
    bzero(mcu_stat, sizeof(mcu_stat));
    bzero(mcu_stat_last, sizeof(mcu_stat_last));
    gpioCtrl.alarm_output = ALARM_OUTPUT_NORMAL;
    gpioCtrl.run_state = LED_RUN_FLICKER;
    gpioCtrl.alarm_led_val = ALARM_OUTPUT_NORMAL;
    fdWatchdog = -1;
    //2016-05-16 停止喂狗
    stop_dog = false;
#ifdef ARM_BOARD
    fdWatchdog =  open("/dev/watchdog", O_WRONLY);
    printf("open watchdog fd %d \n",fdWatchdog);
    if(fdWatchdog <= 0)
        printf("open watchdog error \n");
    else
        ioctl(fdWatchdog, WDIOC_SETTIMEOUT, 12);
    feed_watch_dog();
#endif

}
/*
   **************************************************************************************
 *  函数名称：run()
 *  函数描述：一上电，获取模块参数；如果与网管相连，那么上报周期性测量
 *                ：曲线。
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改       ：2015-10-29 短信的内容不再在此处理，有专门的任务管理短
 *                ：信模块，不再查询总的光功告警，
 *   修改       ：2016-05-25 输出线程号
 **************************************************************************************
*/
void tsk_host_commu::run()
{
    int res_code;
    unsigned long  wait_time;
    int alarm_curv_id;
    wait_time = 3000000;
    alarm_curv_id = 0;
    //2016-05-25 输出线程号，与htop中的线程号对应
    printf("%s : Line : %d  thread id %ld \n",  __FILE__, __LINE__,(long int)syscall(224));
    while (!stopped)
    {
        usleep(2000000);
        if(pMainWidow->isGetOtdrPara == 1)//首先获取到otdr模块参数
        {
            pMainWidow->get_otdr_modul_para();
        }
        if(pMainWidow->m_ctrlStat.NMstat == NM_EXIST)//网络状态存在
        {
            usleep(wait_time);
            /*
             *2016-01-28 主动发送olp切换记录
             *增加了判断条件，如果没有收到曲线回应就不再发送
            */
            if(pMainWidow->m_ctrlUpData.IsPermitRet80 == PERMIT_SEND)
            {
                //olp切换记录
                pMainWidow->send_olp_action_record_to_host(SEND_TYPE_INITIATIVE);
                pMainWidow->m_ctrlUpData.IsPermitRet80 = UNPERMIT_SEND;
            }
            else if(pMainWidow->m_ctrlUpData.IsPermitRet77 == PERMIT_SEND)
            {
                //总的光功告警
               pMainWidow->m_ctrlUpData.IsPermitRet77 = UNPERMIT_SEND;
               pMainWidow->send_totoal_opm_alarm(SEND_TYPE_INITIATIVE);
            }
            else if(pMainWidow->m_ctrlUpData.IsPermitRet31 == PERMIT_SEND)
            {
                //总的硬件告警
                pMainWidow->m_ctrlUpData.IsPermitRet31 = UNPERMIT_SEND;
                pMainWidow->send_total_hw_alarm(SEND_TYPE_INITIATIVE);
            }
            else if(up_cyc_curv&&pMainWidow->m_ctrlUpData.IsPermitRet32 == RET_SUCCESS)
            {
                res_code = pMainWidow->deal_db_alarm_curv(alarm_curv_id);
                if(res_code == DB_RET_EMPTY || res_code == DB_RET_OTHSER_ERROR )
                    up_cyc_curv = false; //数据库表为空，或者返回-1 说明数据库没有周期性测量曲线
                else
                    alarm_curv_id = res_code;
            }
        }

    }

    stopped = false;
}

void tsk_host_commu::stop()
{
    stopped = true;
}
//设置用户定时器
void tsk_host_commu::usr_timer_initial()
{
    bzero(&usr_counter, sizeof(usr_counter));
    usr_timer.ptr_timer = new QTimer;

    connect(usr_timer.ptr_timer,SIGNAL(timeout()),this,SLOT(usr_time_out()));

    usr_timer.ptr_timer->start(timer_space);
    usr_counter.commCounter = 41;
}
/*
   **************************************************************************************
 *  函数名称：usr_time_out
 *  函数描述：计时器超时函数。如果电源，板卡插拔状态有变化，需要及时处理
 *                ：300毫秒进入该函数一次
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-10-30
 *  修改日期：
 *  修改内容：
 **************************************************************************************
*/
void tsk_host_commu::usr_time_out()
{
    //喂狗
    if((usr_counter.commCounter % 10 == 0) &&(!stop_dog) )
    {
#ifdef ARM_BOARD
        feed_watch_dog();
#endif
    }
    //4.5秒检查一次
    if((usr_counter.commCounter % 17) == 0) //每6s检查一次告警状态
    {
        check_commu_state();
        check_alarm_stat();
    }
    //3.25秒检查一次状态
    else if((usr_counter.commCounter % 13 )== 0)
        refresh_nm_state();
    //750ms检查一次
    else if(usr_counter.commCounter % 3 == 0)
    {
        if(pMainWidow->m_ctrlStat.io_fd > 0)
            read_all_io();
    }
    //250ms 控一次灯
    else /*if(usr_counter.commCounter % 2 == 0)*/
    {
        if(pMainWidow->m_ctrlStat.io_fd > 0)
            usr_ctrl_io();
    }

    usr_counter.commCounter++;

    return;
}
/*
   **************************************************************************************
 *  函数名称：read_all_io
 *  函数描述：读取io口的值，调用mainwindow的读取IO口的函数，如果IO状态
 *  函数描述：发生变化，检查板卡的插拔状态
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-01
 *  修改日期：2015-11-18
 *  修改内容：增加去抖动
 **************************************************************************************
*/
void tsk_host_commu::read_all_io()
{    
    int ret;
    bzero(mcu_stat, sizeof(mcu_stat));
    //读取io 口状态
    ret = pMainWidow->read_all_io_val(mcu_stat);
    if(ret == RET_SUCCESS)
    {
        /*
        *0----20去抖动，21---26是运行告警灯，告警输出，短信模块复位
        *不能去抖动
        */
        ret = memcmp(mcu_stat,mcu_stat_last,21*sizeof(int));
        memcpy(mcu_stat_last, mcu_stat,sizeof(mcu_stat));
        //不一致，跳出
        if(ret != 0)
        {
            goto usr_exit;
        }
        //连续两个周期内检测到低电平，消音
        if(mcu_stat[PIN_ALARM_SILENCE] == 0)
        {
            char cur_time[TIME_STR_LEN];
            pMainWidow->GetCurrentTime(cur_time);
            strcpy(pMainWidow->devState.buzzing_time, cur_time);
            pMainWidow->save_dev_state();
            printf("usr put silence key down ,cur time %s !\n", pMainWidow->devState.buzzing_time);
        }
        //比较1----19个int，电源--板卡槽位
        ret = memcmp(&mcu_stat[1],(void *)(&pMainWidow->mcu_stat[1]),19*sizeof(int));

        //如果有变化，更新面板
        if(ret != 0)
        {
            for(int i = 0; i < GPIO_PIN_NUM;i++)
                if(pMainWidow->mcu_stat[i]!=mcu_stat[i])
                    printf("index %d  last %d  cur  %d \n",i,pMainWidow->mcu_stat[i],mcu_stat[i]);

            memcpy(pMainWidow->mcu_stat, mcu_stat, sizeof(mcu_stat));
            pMainWidow->refresh_mcu_card_state();

        }
        else
        {
            memcpy(pMainWidow->mcu_stat, mcu_stat, sizeof(mcu_stat));
        }
        //其他处理

    }
usr_exit:
    return ;
}
/*
   **************************************************************************************
 *  函数名称：check_nm_state
 *  函数描述：检查网管状态
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改日期：
 *  修改内容：
 **************************************************************************************
*/
void tsk_host_commu::refresh_nm_state()
{
    int fd;
    unsigned int  addr;
    addr = ADDR_HOST_VIP;
    fd = tms_SelectFdByAddr(&addr);
    //如果fd有效，无论当前网管处于什么状态，都必须马上为有效
    if(fd > 0)
    {
        //如果网管已经处于断开状态，那么要取消掉周期性测试,同时允许上报周期性测量曲线
        if(pMainWidow->m_ctrlStat.NMstat == NM_LOST)
        {
            pMainWidow->cancelCycleTest();
            pMainWidow->gsm_send_alarm_NM_state(ALARM_NONE);
            memset(&pMainWidow->m_ctrlUpData, 0, sizeof(_tagCtrActivUpData));
        }
        // 重新设置网管状态
        pMainWidow->m_ctrlStat.NMstat = NM_EXIST;
        up_cyc_curv = true;

    }
    //如果当前网管存在，那么马上设置成“预断开“状态
    else if(pMainWidow->m_ctrlStat.NMstat == NM_EXIST)
    {
        pMainWidow->m_ctrlStat.NMstat = NM_PRE_LOST;
    }
    return ;
}
/*
   **************************************************************************************
 *  函数名称：check_commu_state
 *  函数描述：检查板卡与MCU之间的通信，如果有一块通信不正常，即红灯
 *                ：运行灯将回闪
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-10
 *  修改日期：2015-11-19
 *  修改内容：通信状态通告警判断。run灯一直闪烁
 *                ：
 **************************************************************************************
*/
void tsk_host_commu::check_commu_state()
{
    /*
    int i, j, card_num;
    int fd;
    bool findError;
    tms_devbase devbase;
    findError = false;
    card_num = 0;
    for(i = 0; i < NUM_SUBRACK;i++)
    {
        for(j = 0;j < NUM_COMM_CARD;j++)
        {
            if( i == MCU_FRAME && j == MCU_CARD)
                continue;
            if(pMainWidow->m_subrackCard[i].type[j] != PWU && \
                    pMainWidow->m_subrackCard[i].type[j] != NONE)
            {
                card_num++;
                fd = tms_GetDevBaseByLocation(i,j,&devbase);
                if(fd <= 0)
                {
                    findError = true;
                    goto usr_exit;
                }
            }
        }
    }
usr_exit:
    if(findError)
    {
        gpioCtrl.run_state = LED_RUN_RED; //找到错误，红灯
    }
    else if(card_num == 0)
    {
        gpioCtrl.run_state = LED_RUN_GREEN; //没有插板卡，绿灯
    }
    else
    {
        gpioCtrl.run_state = LED_RUN_FLICKER; //正常，闪烁
    }
    */
    gpioCtrl.run_state = LED_RUN_FLICKER;
    return ;
}

/*
   **************************************************************************************
 *  函数名称：check_alarm_stat
 *  函数描述：检查是否存在告警
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-10
 *  修改日期：2016-03-11
 *  修改内容：按下消音按钮后，如果发现新的告警，告警声音输出
 *                ：如果发现有告警，告警灯即红
 **************************************************************************************
*/
void tsk_host_commu::check_alarm_stat()
{
    int i,j,card_num;
    bool findAlarm,findNewAlarm;
    findAlarm = false;//是否有告警
    findNewAlarm = false;//是否有新的告警
    card_num = 0;
    char *pAlarmTime ,*pBuzzTime;
    pBuzzTime = pMainWidow->devState.buzzing_time;
    //检查硬件告警
    for(i = 0; i < NUM_SUBRACK;i++)
    {
        for(j = 0;j < NUM_CARD;j++)
        {
            if(pMainWidow->m_subrackCard[i].type[j] != NONE)
            {
                card_num++;
                //不等于板卡恢复，不等于电源正常，不等于新增板卡
                if(pMainWidow->DevCommuState[i][j].cur_alarm != CARD_ALARM_RECOVER &&\
                        pMainWidow->DevCommuState[i][j].cur_alarm != CARD_ALARM_POWER_NORMAL &&\
                        pMainWidow->DevCommuState[i][j].cur_alarm != CARD_ALARM_NEW)

                {
                    /*
                     *2016-01-20 如果告警时间比按下消音键的时间更新，那么就蜂鸣
                     *2016-03-11 如果发现告警，即告警灯变红
                    */
                    findAlarm = true;
                    pAlarmTime = pMainWidow->DevCommuState[i][j].alarm_time;
                    if(strcmp(pAlarmTime, pBuzzTime) > 0)
                    {
                        findNewAlarm = true;
                        goto usr_exit;
                    }
                }
            }
        }
    }
    //没有插板卡，直接退出，点绿灯
    if(card_num <= 0)
    {
        findAlarm = false;
        goto usr_exit;
    }
    //检查光功告警
    int frame, card,type;
    //此处须上锁，如果访问的时候在其他地方被清空了，访问到被释放的空间会崩溃的
    pMainWidow->total_opm_alarm.mutexBoj.lock();
    for(i = 0; i < pMainWidow->total_opm_alarm.OpmList.size();i++)
    {
        if(pMainWidow->total_opm_alarm.OpmList[i].cur_alarm_num > 0)
        {
            frame = pMainWidow->total_opm_alarm.OpmList[i].frame;
            card   = pMainWidow->total_opm_alarm.OpmList[i].card;
            if(pMainWidow->check_frame_card_range(frame, card) == RET_SUCCESS)
            {
                type = pMainWidow->m_subrackCard[frame].type[card];
                if(type == OPM || type == OLP)
                {
                    for(j = 0; j < pMainWidow->total_opm_alarm.OpmList[i].alarmArray.size();j++)
                    {
                        if(pMainWidow->total_opm_alarm.OpmList[i].alarmArray[j].lev == ALARM_NONE)
                            continue;
                        /*
                         *2016-01-20 如果告警时间比按下消音键的时间更新，那么就蜂鸣
                         *2016-03-11 如果发现告警，即告警灯变红
                        */
                        pAlarmTime = pMainWidow->total_opm_alarm.OpmList[i].alarmArray[j].come_time;
                        findAlarm = true;
                        if(strcmp(pAlarmTime, pBuzzTime) > 0)
                        {
                            findNewAlarm = true;
                            goto end_check_opm_alarm;
                        }

                    }
                }
            }

        }
    }
end_check_opm_alarm:
    pMainWidow->total_opm_alarm.mutexBoj.unlock();
usr_exit:    
    if(findNewAlarm)
    {
        gpioCtrl.alarm_output = ALARM_OUTPUT_ALARM;
    }
    else
    {
        gpioCtrl.alarm_output = ALARM_OUTPUT_NORMAL;
    }
    if(findAlarm)
        gpioCtrl.alarm_led_val = ALARM_OUTPUT_ALARM;
    else
        gpioCtrl.alarm_led_val = ALARM_OUTPUT_NORMAL;
    return;
}
/*
   **************************************************************************************
 *  函数名称：usr_ctrl_io
 *  函数描述：根据各个io口的状态，设置io口的值
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-10
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
void tsk_host_commu::usr_ctrl_io()
{
    //run灯
    switch(gpioCtrl.run_state)
    {
    case LED_RUN_FLICKER:        
        pMainWidow->set_io_val(PIN_LED_RUN,(led_run_state++)%2);
        break;
    case LED_RUN_GREEN:        
        pMainWidow->set_io_val(PIN_LED_RUN,1);
        break;
        /*
    case LED_RUN_RED:
        pMainWidow->set_io_val(PIN_LED_ALARM_COMM,1);
        pMainWidow->set_io_val(PIN_LED_RUN,0);
        break;
        */
    default:
        pMainWidow->set_io_val(PIN_LED_RUN,(led_run_state++)%2);
        break;;
    }

    //告警灯    
   pMainWidow->set_io_val(PIN_LED_ALARM_TOTAL, gpioCtrl.alarm_led_val);
   //是否输出告警声音
   if(pMainWidow->devState.gpioAlarm == GPIO_ALARM_OPEN)
   {
     pMainWidow->set_io_val(PIN_ALARM_BUZZER, gpioCtrl.alarm_output);
   }
   else
   {
       pMainWidow->set_io_val(PIN_ALARM_BUZZER, ALARM_OUTPUT_NORMAL);
   }   
    return;
}
/*
   **************************************************************************************
 *  函数名称：feed_watch_dog
 *  函数描述：喂狗的程序
 *                ：
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：2015-11-12
 *  修改日期：
 *  修改内容：
 *                ：
 **************************************************************************************
*/
void tsk_host_commu::feed_watch_dog()
{
    /*
    int bytes;
    char chrTmp;
    chrTmp = 1;
    if(fdWatchdog > 0)
        bytes = write(fdWatchdog, &chrTmp, 1);
    printf("feed dog %d \n", bytes);
    return ;
*/
    if(fdWatchdog > 0)
    {
        ioctl(fdWatchdog, WDIOC_KEEPALIVE, 0);
        //printf("feed dog !\n");
    }
    return ;

}
/*
   **************************************************************************************
 *  函数名称：stop_feed_dog
 *  函数描述：停止喂看门狗
 *                ：
 *  入口参数：无
 *  返回参数：无
 *  作者       ：
 *  日期       ：
 *  修改日期：2016-05-15
 *  修改内容：
 *                ：
 **************************************************************************************
*/
void tsk_host_commu::stop_feed_dog()
{
    stop_dog = true;
    if(fdWatchdog > 0)
    {
        close(fdWatchdog);
        fdWatchdog = -1;
    }

    return ;
}
