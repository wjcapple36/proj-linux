#include "dlgotdr.h"


#include <QDebug>
#include <QTableWidget>
#include <QTextCodec>
#include <QMessageBox>
#include <iostream>
#include <QMouseEvent>
#include <QTableWidgetItem>


#include "constant.h"
#include "ui_dlgotdr.h"
#include "protocol/tmsxx.h"
#include "src/tmsxxdb.h"


QDlgOtdr::QDlgOtdr(QWidget *parent,int subrackNo, int cardNo, int devType) :
    QDialog(parent),
    ui(new Ui::QDlgOtdr)
{
    QString str;
    int ret;
    ui->setupUi(this);
    setWindowFlags(windowFlags()|Qt::FramelessWindowHint|Qt::WindowTitleHint);
    m_pParent = (MainWindow *)parent;
    m_pParent->m_currenShowDlg++;    

    dlgAttribute.cardNo = cardNo;
    dlgAttribute.frameNo = subrackNo;
    dlgAttribute.devType = devType;
    ret = m_pParent->check_dev_type(subrackNo, cardNo,devType);
    if(ret != RET_SUCCESS)
        m_pParent->dealAbnormalResCod(ret);


    //关联信号，队列形式
    connect(m_pParent, SIGNAL(dataToDlg(int, int, _tagBufList *)), this, \
            SLOT(rcvDataFromParent(int, int, _tagBufList *)),Qt::QueuedConnection);
    connect(m_pParent, SIGNAL(sendMsgToDlg(int,int,int)),this,\
            SLOT(rcvParentMsg(int , int, int)),Qt::QueuedConnection);
    //设在背景色
    QPalette paletteBack = this->palette();
    paletteBack.setColor(QPalette::Background,QColor (255,255,255) );
    ui->widgetDraw->setPalette(paletteBack);

    LayoutOnDlg();
    CreatEventListTable();
    CreatConctTable();
    CreatMeasurResultList();
    if(dlgAttribute.devType == OTDR)
        CreatModulParaTable();
    memcpy(&m_pParent->curShowDlgAttr, &dlgAttribute, sizeof(dlgAttribute));

    memset(&m_drawCtrl, 0,sizeof(m_drawCtrl));
    referOtdrPara.initial = 0;

    initial_attribute();
    initial_module_conect_list();
    get_frame_card_info();
    initial_otdr_test_para();
    usr_timer_initial();

}
QDlgOtdr::~QDlgOtdr()
{
    m_pParent->m_currenShowDlg--;
    memset(&m_pParent->curShowDlgAttr, 0, sizeof(m_pParent->curShowDlgAttr));
    disconnect(m_pParent);
    delete ui;
    qDebug("CDlgOtdr delete ui ");
}
/*
   **************************************************************************************
 *  函数名称：LayoutWidgetsOnMainPage
 *  函数描述：在页面上布局部件
 *  入口参数：
 *  返回参数：
 *  作者       ：wen
 *  日期       ：2015-03-23
 **************************************************************************************
*/
void QDlgOtdr:: LayoutOnDlg()
{
    //    QVBoxLayout * pVLout;
    //    pVLout = ui->verticalLayout;
    //    ui->tabEdtFiberSec->setLayout(pVLout);

    ui->gridLayout->addWidget(ui->widgetDraw, 0, 0, 5, 15);
    ui->gridLayout->addWidget(ui->gBoxCurvOprate, 5, 0, 1, 15);
    ui->gridLayout->addWidget(ui->tabWidgetList, 6, 0, 9, 15);

    ui->gridLayout->addWidget(ui->tabWidgetPara, 0, 15, 6,5);
    ui->gridLayout->addWidget(ui->tabWidgetTestResult, 6, 15, 9, 5);
    if(/*dlgAttribute.devType != OTDR*/1)
    {
        ui->tabWidgetList->removeTab(2);
    }
    ui->tabWidgetPara->removeTab(2);
    ui->tabWidgetTestResult->removeTab(1);

    setLayout(ui->gridLayout);
}

/*
   **************************************************************************************
 *  函数名称：Initial_osw_combox
 *  函数描述：根据最新的机框号，板卡组成初始化，只有光开关才能进行OTDR测量
 *                    此函数初始化系统内全部的光开关的机框号，槽位号，端口号
 *  入口参数：
 *  返回参数：
 *  日期       ：2015-03-21
 *  作者      ：wen
 **************************************************************************************
*/
void QDlgOtdr::Initial_osw_combox()
{
    int i, j, k, state_1,state_2;
    int frame_num, frame_no;
    bool is_ok;
    QString str;
    frame_num = 0;
    //先清空之前的组合框
    ui->cobBoxFrameNo->clear();
    ui->cobBoxSlotNo->clear();
    ui->cobBoxPortNo->clear();

    for(i = 0; i < NUM_SUBRACK; i++)
    {
        //从机号存在
        is_ok = false;
        state_1 = m_subrackInfo.onState;
        if(1 == ((state_1 >> i)&1))
        {
            for(j = 0; j < NUM_CARD;j++)
            {
                //只要有一个板卡随光开关，就添加该从机号
                state_2 = m_subrackCard[i].onState;
                if((1 == ((state_2 >>j)&1)) && (OSW == m_subrackCard[i].type[j]))
                {
                    is_ok = true;
                    frame_num++;
                    break;
                }
            }
        }
        if(is_ok)
        {
            str.setNum(i + 1);
            ui->cobBoxFrameNo->addItem(str,i);
        }
    }
    //光开关板卡存在
    if(frame_num > 0)
    {
        ui->cobBoxFrameNo->setCurrentIndex(0);
        str = ui->cobBoxFrameNo->currentText();
        frame_no = str.toInt() -1;
        k = initial_card_combBox(frame_no);
        if(k >= 0)
            initial_port_comBox(frame_no,k);
    }

}
void QDlgOtdr::on_pushBtnExit_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::information(this, tr("提示"), tr("是否退出？"),
                                     QMessageBox::Yes, QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
        close();
    }
}
//横向放大
void QDlgOtdr::on_pushBtnHZin_clicked()
{
    if(1 == m_drawCtrl.IsDraw && m_drawCtrl.zoomTimesH >=0)
    {
        m_drawCtrl.zoomTimesH++;
        m_qCurv.HoriZoom(m_drawCtrl.zoomTimesH, m_drawCtrl.lineA,m_drawCtrl.sampleNum, \
                         m_drawCtrl.xStart, m_drawCtrl.xEnd);
        update();
    }
}
//横向缩小
void QDlgOtdr::on_pushBtnHZout_clicked()
{
    if(1 == m_drawCtrl.IsDraw && m_drawCtrl.zoomTimesH >0)
    {
        m_drawCtrl.zoomTimesH--;
        m_qCurv.HoriZoom(m_drawCtrl.zoomTimesH, m_drawCtrl.lineA,m_drawCtrl.sampleNum, \
                         m_drawCtrl.xStart, m_drawCtrl.xEnd);
        update();
    }
}
//纵向放大
void QDlgOtdr::on_pushBtnVZin_clicked()
{
    float fdB;
    if(1 == m_drawCtrl.IsDraw && m_drawCtrl.zoomTimesV >=0)
    {
        m_drawCtrl.zoomTimesV++;
        fdB = m_db_1000[m_drawCtrl.lineA]/1000.0;
        m_qCurv.VertiZoom(m_drawCtrl.zoomTimesV,fdB,m_drawCtrl.yStart,m_drawCtrl.yEnd);

        update();
    }
}
//纵向缩小
void QDlgOtdr::on_pushBtnVZout_clicked()
{
    float fdB;
    if(1 == m_drawCtrl.IsDraw && m_drawCtrl.zoomTimesV >0)
    {
        m_drawCtrl.zoomTimesV--;
        fdB = m_db_1000[m_drawCtrl.lineA]/1000.0;

        m_qCurv.VertiZoom(m_drawCtrl.zoomTimesV,fdB,m_drawCtrl.yStart,m_drawCtrl.yEnd);
        update();
    }
}
//全迹线显示
void QDlgOtdr::on_pushBtnResotre_clicked()
{
    float fdB;
    m_drawCtrl.zoomTimesH = 0;
    m_drawCtrl.zoomTimesV = 0;
    m_qCurv.HoriZoom(m_drawCtrl.zoomTimesH, m_drawCtrl.lineA,m_drawCtrl.sampleNum, \
                     m_drawCtrl.xStart, m_drawCtrl.xEnd);
    fdB = m_db_1000[m_drawCtrl.lineA]/1000.0;
    m_qCurv.VertiZoom(m_drawCtrl.zoomTimesV,fdB,m_drawCtrl.yStart,m_drawCtrl.yEnd);

    update();
}
//锁定标杆
void QDlgOtdr::on_pushBtnLockA_clicked()
{
    m_drawCtrl.moveLineA = 0;
    update();
}
//移动标杆，原先设计位锁定
void QDlgOtdr::on_pushBtnLockB_clicked()
{
    m_drawCtrl.moveLineA = 1;
    update();
}
/*
   **************************************************************************************
 *  函数名称：paintEvent
 *  函数描述：绘图函数
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::paintEvent(QPaintEvent *event)
{
    //Get window rect
    int i, xStart_m;
    int xEnd_m;
    float xDistace;
    QString str;
    QPainter painter(this);

    //设置背景色
    painter.fillRect(m_rectFigure, QColor(255, 255, 255));
    m_qCurv.DrawGridLine(&painter,m_rectFigure,10,10);
    //如果没有数据，那么直接返回
    if(0 == m_drawCtrl.IsDraw)
        return;
    //绘制坐标轴
    xStart_m = m_qCurv.Get_xCoord(m_drawCtrl.xStart,m_measurPara.samplHz,\
                                  m_measurPara.n);
    xEnd_m = m_qCurv.Get_xCoord(m_drawCtrl.xEnd,m_measurPara.samplHz,\
                                m_measurPara.n);

    m_qCurv.DrawCoordinate(&painter, m_rectFigure, xStart_m,xEnd_m , \
                           m_drawCtrl.yStart,m_drawCtrl.yEnd);

    m_qCurv.GetPixel(m_rectFigure, m_ptBuf,m_db_1000,m_drawCtrl.yStart,m_drawCtrl.yEnd,
                     m_drawCtrl.xStart, m_drawCtrl.xEnd);
    m_qCurv.DrawCurve(&painter, m_ptBuf,m_drawCtrl.xStart,m_drawCtrl.xEnd,QColor(255,0,0));

    //标杆
    xDistace = m_qCurv.Get_xCoord(m_drawCtrl.lineA,m_measurPara.samplHz,\
                                  m_measurPara.n);
    str.setNum(xDistace,'f',3);
    str = str + "km";
    if(1 == m_drawCtrl.moveLineA)
        m_qCurv.DrawMarkLine(&painter,m_ptBuf[m_drawCtrl.lineA].x(),\
                             m_rectFigure,str,QColor(255,0,0),2);
    else
        m_qCurv.DrawMarkLine(&painter,m_ptBuf[m_drawCtrl.lineA].x(),\
                             m_rectFigure,str,QColor(0,0,0),2);
    //绘制事件点标志
    for(i = 0; i < m_eventNum;i++)
    {
        m_qCurv.DrawSignsOnXLocation(&painter,m_rectFigure, m_eventBuf[i].type,\
                                     m_ptBuf[m_eventBuf[i].pos]);
    }
}
//测试代码
void QDlgOtdr :: LoadData()
{
#if 0
    FILE *fp;
    float ftemp;
    int i, itemp;
    fp = fopen("/home/targetnfs/232-11-24.otdr","rb");
    if(NULL == fp)
        fp = fopen("/tmp/232-11-24.otdr","rb");
    qDebug()<<"fp"<<fp;
    //测量参数
    fread(&m_referCurv.measurPara.measureTime_s,sizeof(int),1,fp);
    fread(&m_referCurv.measurPara.n,sizeof(float),1,fp);
    fread(&m_referCurv.measurPara.range_m,sizeof(int),1,fp);
    fread(&m_referCurv.measurPara.pulseWidth_ns,sizeof(int),1,fp);
    fread(&m_referCurv.measurPara.lamda_ns,sizeof(int),1,fp);
    fread(&m_referCurv.measurPara.NonRelectThreshold,sizeof(float),1,fp);
    fread(&m_referCurv.measurPara.endThreshold,sizeof(float),1,fp);

    //测量结果

    fread(&m_referCurv.measurResult.fiberLoss,sizeof(int),1,fp);
    fread(&m_referCurv.measurResult.fiberLen,sizeof(int),1,fp);
    fread(&m_referCurv.measurResult.fiberAtten,sizeof(float),1,fp);
    //反向散射系数
    fread(&ftemp, sizeof(float),1,fp);
    //测量时间
    fread(m_referCurv.measurTime,20*sizeof(char),1,fp);
    //采样点
    fread(&itemp, sizeof(int),1,fp);
    if(itemp < 0 || itemp > MAX_PT_NUM)
        return ;
    m_referCurv.ptNum = itemp;
    memset(m_referCurv.dataPt, 0, sizeof(m_referCurv.dataPt));

    for(i = 0; i < m_referCurv.ptNum;i++)
    {
        fread(&ftemp,sizeof(float),1,fp);
        m_db_1000[i] = ftemp * 1000;
    }

    //事件点
    fread(&itemp, sizeof(int),1,fp);
    m_referCurv.eventNum = itemp;
    for(i = 0; i < itemp; i++)
    {
        fread(&m_referCurv.eventBuf[i].pos, sizeof(int), 1,fp);
        fread(&m_referCurv.eventBuf[i].type, sizeof(int), 1,fp);
        fread(&m_referCurv.eventBuf[i].attenCoef, sizeof(float), 1,fp);
        fread(&m_referCurv.eventBuf[i].insertLoss, sizeof(float), 1,fp);
        fread(&m_referCurv.eventBuf[i].reflectLoss, sizeof(float), 1,fp);
        fread(&m_referCurv.eventBuf[i].totalLoss, sizeof(float), 1,fp);
    }
    fclose(fp);

    //    memcpy(m_fdB, m_referCurv.dataPt, sizeof(m_fdB));
    memcpy(&m_measurPara, &m_referCurv.measurPara, sizeof(m_measurPara));
    memcpy(&m_measurResult, &m_referCurv.measurResult, sizeof(m_measurResult));
    m_eventNum = m_referCurv.eventNum;
    memcpy(m_eventBuf, m_referCurv.eventBuf, m_eventNum*sizeof(_tagEvent));
    qDebug()<<"eventNum = "<<m_eventNum;
    m_measurPara.samplHz =12500000;
    m_measurPara.n = 1.4685;
    m_drawCtrl.sampleNum = m_referCurv.ptNum;
    m_drawCtrl.xStart = 0;
    m_drawCtrl.xEnd = m_drawCtrl.sampleNum;
    m_drawCtrl.yStart = 0;
    m_drawCtrl.yEnd = MAX_F_DB ;
    m_drawCtrl.lineA = m_drawCtrl.sampleNum / 3;
    m_drawCtrl.lineB = 2 * m_drawCtrl.lineA;
    m_drawCtrl.IsDraw = 1;
    m_drawCtrl.xOrigPt = m_rectFigure.left();
#else
    _tagOtdrCurv *potdr_curv;
    _tagMeasurResult *pmeasur_result;
    _tagEvent *pevent;
    char *pbuf;
    char *pkey_id;
    int offset;
    m_pParent->OtdrCurv.mutexObj.lock();
    //采样点有可能可变，所以，要根据采集到的数据进行调整
    potdr_curv  = ( _tagOtdrCurv *)& (m_pParent->OtdrCurv.curv);
    memcpy(&m_measurPara, &potdr_curv->measurPara, sizeof(m_measurPara));
    m_sampl_num = potdr_curv->ptNum;
    pbuf = potdr_curv->dpid;
    qDebug("sample num %d dpid %s", m_sampl_num, pbuf);

    if(m_sampl_num < 100 || m_sampl_num > MAX_PT_NUM)
    {
        qDebug("sample num error %d", m_sampl_num);
        goto usr_exit;
    }
    //拷贝数据点
    memcpy(m_db_1000, potdr_curv->dataPt, sizeof(unsigned short)*m_sampl_num);

    //事件点数目
    m_eventNum = potdr_curv->eventNum;
    if(m_eventNum < 0 && m_eventNum > MAX_EVENT_NUM)
    {
        qDebug("event num error %d",m_eventNum);
        goto usr_exit;
    }

    memcpy(m_eventBuf,  potdr_curv->eventBuf, m_eventNum*sizeof(_tagEvent));

    memcpy(&m_measurResult, &potdr_curv->measurResult, sizeof(m_measurResult));
    m_measurPara.samplHz = potdr_curv->measurPara.samplHz;
    m_measurPara.n = potdr_curv->measurPara.n;
    m_drawCtrl.sampleNum = m_sampl_num;
    m_drawCtrl.xStart = 0;
    m_drawCtrl.xEnd = m_drawCtrl.sampleNum;
    m_drawCtrl.yStart = 0;
    m_drawCtrl.yEnd = MAX_F_DB ;
    m_drawCtrl.lineA = m_drawCtrl.sampleNum / 3;
    m_drawCtrl.lineB = 2 * m_drawCtrl.lineA;
    m_drawCtrl.IsDraw = 1;
    m_drawCtrl.xOrigPt = m_rectFigure.left();
    //刷新列表
    InitialEventList();
    InitialResultList();
    update();
    m_pParent->OtdrCurv.mutexObj.unlock();
usr_exit:   return;
#endif

}
//开始测量
void QDlgOtdr::on_pushBtnMeasur_clicked()
{
    _tagDevComm *ptest_port;
    _tagDevComm otdrDev;
    _tagMeasurPara *pmeasur_para;
    _tagCtrlAppointTest appoint_test;
    tsk_OtdrManage *pOtdrTsk;
    QString str;
    char msg[SHORT_MSG_LEN];
    int res_code;
    int result, i;
    int wait_curv_time, return_val;
    int wait_res_time;
    char buf[128];
    pOtdrTsk = NULL;
    memset(msg,0, sizeof(msg));
    m_drawCtrl.IsDraw = 0;
    memset(m_db_1000, 0, sizeof(m_db_1000));
    bzero(&appoint_test, sizeof(appoint_test));
    //初始化命令码，源地址，目的地址，流水号
    pmeasur_para = &appoint_test.para;
    ptest_port = &appoint_test.test_port;
    appoint_test.cmd = ID_GET_OTDR_TEST;
    appoint_test.opt.src = ADDR_MCU;
    appoint_test.opt.dst = ADDR_CARD;
    appoint_test.opt.pkid = m_pParent->creat_pkid();
    //本对话框需要回应消息
    appoint_test.ack_to = ACK_TOT_DLG;
    memset(buf, 0, sizeof(buf));
    //获取测量参数
    str = ui->cobBoxFrameNo->currentText();
    ptest_port->frame_no = str.toInt() - 1;
    //槽位号
    str = ui->cobBoxSlotNo->currentText();
    ptest_port->card_no = str.toInt() - 1;
    //端口号
    str = ui->cobBoxPortNo->currentText();
    ptest_port->port = str.toInt() - 1;
    ptest_port->type = dlgAttribute.devType;


    //检查从机号和端口号
    res_code = m_pParent->check_dev_type(ptest_port->frame_no, ptest_port->card_no,ptest_port->type);
    if(res_code !=  RET_SUCCESS)
    {
        sprintf(msg,"osw type error frame %d card %d", \
                ptest_port->frame_no, ptest_port->card_no);
        goto usr_exit;
    }
    res_code = m_pParent->check_dev_port_no(ptest_port->frame_no, ptest_port->card_no,ptest_port->port);
    if(res_code != RET_SUCCESS)
    {
        sprintf(msg,"osw port error frame %d card %d port %d ", \
                ptest_port->frame_no, ptest_port->card_no,ptest_port->port);
        goto usr_exit;
    }
    else
    {
        //测量参数，量程
        str = ui->cobBoxRange->currentText();
        pmeasur_para->range_m = str.toInt() * 1000;
        //波长
        str = ui->cobBoxLamda->currentText();
        pmeasur_para->lamda_ns = str.toInt() ;
        //测量时间
        str = ui->cobBoxTestTime_s->currentText();
        pmeasur_para->measureTime_s = str.toInt() ;

        //脉宽
        str = ui->cobBoxPW->currentText();
        pmeasur_para->pulseWidth_ns = str.toInt() ;

        //结束门限，非反射门限，折射率
        pmeasur_para->endThreshold = ui->dblSpBoxEndTh->value();
        pmeasur_para->n = ui->dblSpBoxN->value();
        pmeasur_para->NonRelectThreshold = ui->dblSpBoxNorEndTh->value();
        pmeasur_para->src_ip = 0;
        pmeasur_para->test_reason = 0;

        qDebug("从机%d 槽位%d端口 %d ",\
               ptest_port->frame_no,ptest_port->card_no, ptest_port->port);
        qDebug("波长%d 结束门限%.2f测量时间 %d 折射率%.4f 非反射门限%.2f 脉宽%d 量程%d ",\
               pmeasur_para->lamda_ns, pmeasur_para->endThreshold, pmeasur_para->measureTime_s,
               pmeasur_para->n,pmeasur_para->NonRelectThreshold, pmeasur_para->pulseWidth_ns,pmeasur_para->range_m);
        result = -1;
        //最坏的情况，切换olp,osw,然后才是otdr
        wait_curv_time = pmeasur_para->measureTime_s * 3*1000;

        //系统一经运行，本模块的路由已经读取了所以，不用再次初始化了
        if(rout_buf.record_num > 0)
        {
            i = rout_buf.record_num - 1;
            memcpy(&otdrDev, &rout_buf.buf[i].DevA, sizeof(_tagDevComm));
        }
        else if(dlgAttribute.devType == OTDR) //测试设备本身就是otdr
        {
            memcpy(&otdrDev, ptest_port, sizeof(_tagDevComm));
        }
        else
        {
            goto usr_exit;
        }
        result = m_pParent->dispatch_test_port((void *)(&appoint_test), OTDR_MOD_APPOINT);

        //发送成功
        if( result  >=0)
        {
            res_code = RET_SUCCESS;
            wait_res_time =  (result + 3) * 1000;
            wait_curv_time = pmeasur_para->measureTime_s * 3*1000;
            str.clear();
            str.setNum(wait_res_time/1000);
            str = tr("otdr正在被使用，使用最长时间")  + str;
            if(result > 0)
                dealAbnormal(str);
            //等待回应，3秒钟
            m_pParent->objSyncSem[SEM_OTDR_DLG].resCmd = ID_GET_OTDR_TEST;
            m_pParent->objSyncSem[SEM_OTDR_DLG].resCode = -1;
            return_val = showOtdrProgressbar(wait_res_time,ID_GET_OTDR_TEST,false);
            if(0 == return_val)
            {
                m_pParent->objSyncSem[SEM_OTDR_DLG].resCmd = ID_RET_OTDR_TEST;
                m_pParent->objSyncSem[SEM_OTDR_DLG].resCode = -1;
                str = tr("测量参数发送成功，测量正在进行！");
                dealAbnormal(str);
                return_val = showOtdrProgressbar(wait_curv_time, ID_RET_OTDR_TEST,false);
                //如果返回其他错误码，进度条处理函数会处理的
                if(RET_SUCCESS == return_val)
                    LoadData();
            }
        }
        else
        {
            res_code = abs(result);
            m_pParent->dealAbnormalResCod(res_code);
        }
    }

usr_exit:
    //恢复成默认状态
    m_pParent->objSyncSem[SEM_OTDR_DLG].resCmd = -1;
    m_pParent->objSyncSem[SEM_OTDR_DLG].resCode = -1;
    return;
}
/*
   **************************************************************************************
 *  函数名称：mousePressEvent
 *  函数描述：鼠标按下，如果满足条件那么移动标杆
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr :: mousePressEvent(QMouseEvent *event)
{
    int clickPos;
    clickPos = event->pos().x();
    if(clickPos > m_rectFigure.left() && clickPos < m_rectFigure.right()&&
            (1 == m_drawCtrl.moveLineA))
    {
        m_drawCtrl.lineA = m_qCurv.GetSampPt(m_rectFigure, clickPos,\
                                             m_drawCtrl.xStart, m_drawCtrl.xEnd);
        update();
    }
}
/*
   **************************************************************************************
 *  函数名称：CreatEventListTable
 *  函数描述：创建事件列表
 *                    设置为只能单行选中
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::CreatEventListTable()
{
    QStringList header;
    header<<tr("类型")<<tr("位置")<<tr("回波损耗")<<tr("插损")<<tr("衰减系数")\
         <<tr("累计损耗");
    ui->listEvent->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //    ui->listEvent->verticalHeader()->setFixedWidth(80);
    ui->listEvent->setRowCount(7);
    ui->listEvent->setColumnCount(6);
    //    ui->listEvent->setColumnWidth(4,185);
    ui->listEvent->setSelectionBehavior(QAbstractItemView::SelectRows );
    ui->listEvent->setSelectionMode ( QAbstractItemView::SingleSelection); //设置选择模式，选择单行

    ui->listEvent->setHorizontalHeaderLabels(header);
    ui->listEvent->horizontalHeader()->setStretchLastSection(true);//关键
}
/*
   **************************************************************************************
 *  函数名称：CreatConctTable
 *  函数描述：创建关联关系表
 *                    设置为只能单行选中
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::CreatConctTable()
{
    QStringList header;
    header<<tr("源地址")<<tr("目的地址")<<tr("机框")<<tr("槽位")<<tr("端口")\
         <<tr("类型");
    ui->listConect->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //    ui->listEvent->verticalHeader()->setFixedWidth(80);
    ui->listConect->setRowCount(7);
    ui->listConect->setColumnCount(6);
    ui->listConect->setColumnWidth(0,100);
    ui->listConect->setColumnWidth(1,100);
    ui->listConect->setSelectionBehavior(QAbstractItemView::SelectRows );
    ui->listConect->setSelectionMode ( QAbstractItemView::SingleSelection); //设置选择模式，选择单行

    ui->listConect->setHorizontalHeaderLabels(header);
    ui->listConect->horizontalHeader()->setStretchLastSection(true);//关键
}
/*
   **************************************************************************************
 *  函数名称：CreatModulParaTable
 *  函数描述：创建模块参数表
 *                    设置为只能单行选中
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::CreatModulParaTable()
{
    QStringList header;
    header<<tr("波长")<<tr("动态")<<tr("备注");
    ui->listModulPara->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //    ui->listEvent->verticalHeader()->setFixedWidth(80);
    ui->listModulPara->setRowCount(7);
    ui->listModulPara->setColumnCount(3);
    //    ui->listEvent->setColumnWidth(4,185);
    ui->listModulPara->setSelectionBehavior(QAbstractItemView::SelectRows );
    ui->listModulPara->setSelectionMode ( QAbstractItemView::SingleSelection); //设置选择模式，选择单行

    ui->listModulPara->setHorizontalHeaderLabels(header);
    ui->listModulPara->horizontalHeader()->setStretchLastSection(true);//关键
}

/*
   **************************************************************************************
 *  函数名称：InitialEventList
 *  函数描述：初始化事件列表
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::InitialEventList()
{
    int i;
    float distace_km ;
    int row, column;
    QString str;
    //先释放之前段资源
    ui->listEvent->clearContents();
    //    ui->listEvent->removeRow(1);
    ui->listEvent->setRowCount(m_eventNum);
    row = 0;
    column = 0;
    //轮询，初始化机框信息
    for(i = 0; i < m_eventNum;i++)
    {
        column = 0;
        //事件类型
        str = getEventType(m_eventBuf[i].type);
        ui->listEvent->setItem(i,column++, new QTableWidgetItem(str));
        distace_km = m_qCurv.Get_xCoord(m_eventBuf[i].pos, \
                                        m_measurPara.samplHz, m_measurPara.n);
        //事件点位置，km
        str.setNum(distace_km, 'f', 3);
        ui->listEvent->setItem(i,column++, new QTableWidgetItem(str));
        //回波损耗
        if((EVENT_TYPE_REFLEC == m_eventBuf[i].type) && INVALID_VALUE != m_eventBuf[i].reflectLoss)
            str.setNum(m_eventBuf[i].reflectLoss, 'f', 3);
        else
            str = "-.---";
        ui->listEvent->setItem(i,column++, new QTableWidgetItem(str));
        //插损
        if((EVENT_TYPE_END != m_eventBuf[i].type) && \
                (EVENT_TYPE_START != m_eventBuf[i].type)&& INVALID_VALUE != m_eventBuf[i].insertLoss)
            str.setNum(m_eventBuf[i].insertLoss, 'f', 3);
        else
            str = "-.---";
        ui->listEvent->setItem(i,column++, new QTableWidgetItem(str));
        //衰减系数
        if((EVENT_TYPE_END != m_eventBuf[i].type) && INVALID_VALUE != m_eventBuf[i].attenCoef)
            str.setNum(m_eventBuf[i].attenCoef, 'f', 3);
        else
            str = "-.---";
        ui->listEvent->setItem(i,column++, new QTableWidgetItem(str));
        //累计损耗
        if((EVENT_TYPE_START != m_eventBuf[i].type) && INVALID_VALUE != m_eventBuf[i].totalLoss)
            str.setNum(m_eventBuf[i].totalLoss, 'f', 3);
        else
            str = "-.---";
        ui->listEvent->setItem(i,column++, new QTableWidgetItem(str));
    }

}
/*
   **************************************************************************************
 *  函数名称：getEventType
 *  函数描述：获取事件类型
 *  入口参数：事件类型
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
QString QDlgOtdr::getEventType(int eventType)
{
    QString str;
    switch(eventType)
    {
    case EVENT_TYPE_START:
    {
        str = (tr("开始事件"));
        break;
    }
    case EVENT_TYPE_REFLEC:
    {
        str = (tr("反射事件"));
        break;
    }
    case EVENT_TYPE_NO_REFLEC:
    {
        str = (tr("非反射事件"));
        break;
    }
    case EVENT_TYPE_END:
    {
        str = (tr("结束事件"));
        break;
    }
    default:
        break;
    }
    return str;
}
/*
   **************************************************************************************
 *  CreatMeasurResultList
 *  函数描述：创建事件列表
 *                    设置为只能单行选中
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::CreatMeasurResultList()
{

    ui->listMeasurResult->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //    ui->listEvent->verticalHeader()->setFixedWidth(80);
    ui->listMeasurResult->setRowCount(8);
    ui->listMeasurResult->setColumnCount(2);
    ui->listMeasurResult->setSelectionBehavior(QAbstractItemView::SelectRows );
    ui->listMeasurResult->setSelectionMode ( QAbstractItemView::SingleSelection); //设置选择模式，选择单行
    ui->listMeasurResult->verticalHeader()->setHidden(true);
    ui->listMeasurResult->horizontalHeader()->setHidden(true);
    //    ui->listMeasurResult->resizeColumnToContents(0);
    ui->listMeasurResult->setColumnWidth(0,130);
    ui->listMeasurResult->horizontalHeader()->setStretchLastSection(true);//关键

}
/*
   **************************************************************************************
 *  函数名称：InitialResult
 *  函数描述：初始化测量结果列表
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::InitialResultList()
{
    int row;
    QString str;
    //先释放之前段资源
    ui->listMeasurResult->clearContents();
    row = 0;
    //量程
    str = tr("量程(km)");
    ui->listMeasurResult->setItem(row,0, new QTableWidgetItem(str));
    str.setNum(m_measurPara.range_m /1000);
    ui->listMeasurResult->setItem(row++,1, new QTableWidgetItem(str));
    //折射率
    str = tr("折射率");
    ui->listMeasurResult->setItem(row,0, new QTableWidgetItem(str));
    str.setNum(m_measurPara.n, 'f',4);
    ui->listMeasurResult->setItem(row++,1, new QTableWidgetItem(str));
    //脉宽
    str = tr("脉宽(ns)");
    ui->listMeasurResult->setItem(row,0, new QTableWidgetItem(str));
    str.setNum(m_measurPara.pulseWidth_ns);
    ui->listMeasurResult->setItem(row++,1, new QTableWidgetItem(str));

    //测量时间
    str = tr("测量时间(s)");
    ui->listMeasurResult->setItem(row,0, new QTableWidgetItem(str));
    str.setNum(m_measurPara.measureTime_s);
    ui->listMeasurResult->setItem(row++,1, new QTableWidgetItem(str));
    //链长
    str = tr("链长(km)");
    ui->listMeasurResult->setItem(row,0, new QTableWidgetItem(str));
    str.setNum(m_measurResult.fiberLen /1000.0);
    ui->listMeasurResult->setItem(row++, 1,  new QTableWidgetItem(str));
    //链损耗
    str = tr("链损耗(dB)");
    ui->listMeasurResult->setItem(row, 0,  new QTableWidgetItem(str));
    str.setNum(m_measurResult.fiberLoss);
    ui->listMeasurResult->setItem(row++, 1, new QTableWidgetItem(str));
    //链衰减
    str = tr("链衰减(dB/km)");
    ui->listMeasurResult->setItem(row, 0, new QTableWidgetItem(str));
    str.setNum(m_measurResult.fiberAtten);
    ui->listMeasurResult->setItem(row++, 1 , new QTableWidgetItem(str));
}

//事件列表中按下
void QDlgOtdr::on_listEvent_pressed(const QModelIndex &index)
{
    QString str;
    int  i;
    QTableWidgetItem *item = ui->listEvent->currentItem();
    //判断用户点击地方有无设置item
    if(NULL == item)
    {
        return;
    }
    //获取机框编号，每行的第0列
    i = index.row();
    str = ui->listEvent->item(i,0)->text();
    m_drawCtrl.lineA = m_eventBuf[i].pos;
    update();
}
/*
   **************************************************************************************
 *  函数名称：rcvDataFromMainwindow
 *  函数描述：从mainwindow接收数据
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::rcvDataFromParent(int cmd, int dataLen, _tagBufList * pBufHead)
{
    switch(cmd)
    {
    //处理消息的回应码
    case ID_CMD_ACK:
    {
        m_pParent->LinkedBufManager.freeBuf(pBufHead);
        break;
    }
    case 1:
    {
        m_pParent->LinkedBufManager.freeBuf(pBufHead);
        break;
    }
    }
    return;
}
//处理异常提示
void QDlgOtdr::dealAbnormal(QString str)
{
    QMessageBox msg;
    msg.setText(str);
    msg.move(m_rectFigure.width()/2, 2*m_rectFigure.height()/3);
    msg.setWindowTitle(tr("提示"));
    msg.exec();
    update();

}
/*
   **************************************************************************************
 *  函数名称：rcvParentMsg
 *  函数描述：收到父窗口的消息
 *  入口参数：cmd，标志，cmdCode命令的errorCod
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::rcvParentMsg(int cmd, int reservd1, int reservd2)
{

    switch(cmd)
    {
    //收到OTDR曲线
    case ID_RET_OTDR_TEST:
    {
        LoadData();
        break;
    }
        //更新从机，槽位配置信息
    case CMD_REFRES_FRAME_CARD:
    {
        get_frame_card_info();
        break;
    }
    default:
        break;
    }
}
/*
   **************************************************************************************
 *  函数名称：get_frame_card_info
 *  函数描述：如果板卡组成信息发生变化，需要及时更新
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::get_frame_card_info()
{
    memcpy(m_subrackCard, m_pParent->m_subrackCard, NUM_SUBRACK*sizeof(_tagSubrackCard));
    memcpy(&m_subrackInfo, &m_pParent->m_subrackInfo, sizeof(m_subrackInfo));    
    //Initial_osw_combox();

}
//机框号发生变化
void QDlgOtdr::on_cobBoxFrameNo_currentIndexChanged(const QString &arg1)
{
    return;
    int frame_no;
    frame_no = arg1.toInt() - 1;
    if(frame_no > -1)
        initial_card_combBox(frame_no);
}
//槽位号发生变化
void QDlgOtdr::on_cobBoxSlotNo_currentIndexChanged(const QString &arg1)
{
    return;
    QString str;
    int frame_no, slot_no;
    str = ui->cobBoxFrameNo->currentText();
    frame_no = str.toInt() - 1;
    slot_no = arg1.toInt() - 1;
    if(frame_no > -1 && slot_no > - 1)
        initial_port_comBox(frame_no, slot_no);
}
//初始化指定机框槽位号
int QDlgOtdr::initial_card_combBox(int frame_no)
{
    int i, state;
    int sel_slot;
    QString str;
    sel_slot = -1;
    if(frame_no >= 0&& frame_no < NUM_SUBRACK)
    {
        ui->cobBoxSlotNo->clear();
        state = m_subrackCard[frame_no].onState;
        for(i = 0; i < NUM_CARD;i++)
        {
            if(1 == ((state >> i)&1) && OSW == m_subrackCard[frame_no].type[i])
            {
                sel_slot++;
                str.setNum(i + 1);
                ui->cobBoxSlotNo->addItem(str, i);
            }
        }
        if(sel_slot > -1)
            ui->cobBoxSlotNo->setCurrentIndex(0);
        str = ui->cobBoxSlotNo->currentText();
        sel_slot = str.toInt() - 1;
    }
    return sel_slot;
}
//初始化端口号
int QDlgOtdr::initial_port_comBox(int frame_no, int slot_no)
{
    int i, port_num;
    int sel_port;
    QString str;
    sel_port = -1;
    if(frame_no >= 0 && frame_no < NUM_SUBRACK && slot_no >= 0 && slot_no < NUM_CARD)
    {
        ui->cobBoxPortNo->clear();
        port_num = m_subrackCard[frame_no].ports[slot_no];
        for(i = 0; i < port_num;i++)
        {
            str.setNum(i + 1);
            ui->cobBoxPortNo->addItem(str,i);
        }
        if(port_num > 0)
        {
            ui->cobBoxPortNo->setCurrentIndex(0);
            str = ui->cobBoxSlotNo->currentText();
            sel_port = str.toInt() - 1;
        }
    }
    return sel_port;
}
/*
   **************************************************************************************
 *  函数名称：initial_local_device_test_port
 *  函数描述：初始化本设备可测量端口
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::initial_local_device_test_port()
{
    int i, port_num;
    int sel_port;
    int frame_no, slot_no;
    QString str;
    sel_port = -1;
    frame_no = dlgAttribute.frameNo;
    slot_no = dlgAttribute.cardNo;


    if(frame_no >= 0 && frame_no < NUM_SUBRACK && slot_no >= 0 && slot_no < NUM_CARD)
    {
        //插入机框号
        ui->cobBoxFrameNo->clear();
        str.setNum(frame_no + 1);
        ui->cobBoxFrameNo->addItem(str,0);
        //插入槽位号
        ui->cobBoxSlotNo->clear();
        str.setNum(slot_no + 1);
        ui->cobBoxSlotNo->addItem(str,0);
        //如果本模块不是otdr，并且末端没有接otdr直接返回
         i = rout_buf.record_num - 1;
         if(i < 0 && dlgAttribute.devType != OTDR) //osw没有关联otdr
             goto usr_exit;
        else if(rout_buf.buf[i].DevA.type != OTDR&&dlgAttribute.devType != OTDR )
            goto usr_exit;

        ui->cobBoxPortNo->clear();
        port_num = m_pParent->m_subrackCard[frame_no].ports[slot_no];
        sel_port = -1;
        for(i = 0; i < port_num;i++)
        {
            //不存在级联关系不能测，级联端口不能测;级联关系起始模块都能测
            if(rout_buf.record_num > 0&&(i == rout_buf.buf[0].DevA.port)&&rout_buf.buf[0].DevA.type != NONE)
                continue;
            str.setNum(i + 1);
            sel_port++;
            ui->cobBoxPortNo->addItem(str,i);
        }
        if(sel_port > 0)
        {
            ui->cobBoxPortNo->setCurrentIndex(0);
            str = ui->cobBoxSlotNo->currentText();
        }
    }
usr_exit:
    return ;
}

/*
   **************************************************************************************
 *  initial_rang_pulse_strlist
 *  函数描述：量程对应脉宽用QStringlist表示，每个量程对应一个脉宽list
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr:: initial_rang_pulse_strlist()
{
    int i;
    //量程
    for(i = 0; i < RANGE_NUM;i++)
        strlist_pulse[i].clear();
    strlist_range<<"1"<<"5"<<"10"<<"30"<<"60"<<"100"<<"180";
    i = 0;
    //1km
    strlist_pulse[i++] <<"5"<<"10"<<"20"<<"40";
    //5km
    strlist_pulse[i++] <<"5"<<"10"<<"20"<<"40"<<"80"<<"160";
    //10km
    strlist_pulse[i++] <<"5"<<"10"<<"20"<<"40"<<"80"<<"160"<<"320";
    //30km
    strlist_pulse[i++] <<"5"<<"10"<<"20"<<"40"<<"80"<<"160"<<"320"<<"640";
    //60km
    strlist_pulse[i++] <<"80"<<"160"<<"320"<<"640"<<"1280";
    //100km
    strlist_pulse[i++] <<"160"<<"320"<<"640"<<"1280"<<"2560"<<"5000"<<"10000"<<"20000";
    //180km
    strlist_pulse[i++] <<"160"<<"320"<<"640"<<"1280"<<"2560"<<"5000"<<"10000"<<"20000";
    return;
}
void QDlgOtdr::initial_otdr_test_para()
{
    QStringList strlist_temp;       
    initial_rang_pulse_strlist();
    set_measur_lamda();
    ui->cobBoxRange->addItems(strlist_range);
    ui->cobBoxRange->setCurrentIndex(4);
    //不初始化，ui->cobBoxRange->setCurrentIndex(4)回调用相关函数
    //ui->cobBoxPW->addItems(strlist_pulse[4]);
    strlist_temp.clear();
    strlist_temp<<"8"<<"15"<<"30"<<"60"<<"120"<<"180";
    ui->cobBoxTestTime_s->addItems(strlist_temp);
    //设置折射率spinBox
    ui->dblSpBoxN->setDecimals(4);
    ui->dblSpBoxN->setSingleStep(0.0005);
    ui->dblSpBoxN->setRange(1.0,2.0);
    ui->dblSpBoxN->setValue(1.4685);
    //设置非反射门限spinBox
    ui->dblSpBoxNorEndTh->setDecimals(2);
    ui->dblSpBoxNorEndTh->setSingleStep(0.01);
    ui->dblSpBoxNorEndTh->setRange(0, 2.0);
    ui->dblSpBoxNorEndTh->setValue(0);
    //设置反射门限spinBox
    ui->dblSpBoxEndTh->setDecimals(2);
    ui->dblSpBoxEndTh->setSingleStep(0.01);
    ui->dblSpBoxEndTh->setRange(5, 10);
    ui->dblSpBoxEndTh->setValue(9);
}
//根据选择的量程选择对应的脉宽
void QDlgOtdr::on_cobBoxRange_currentIndexChanged(const QString &arg1)
{
    int range, index;
    index = -1;
    range = arg1.toInt();
    switch(range)
    {
    case 1: //1km
    {
        index = 0;
        break;
    }
    case 5://5km
    {
        index = 1;
        break;
    }
    case 10://10km
    {
        index = 2;
        break;
    }
    case 30://30km
    {
        index = 3;
        break;
    }
    case 60://60km
    {
        index = 4;
        break;
    }
    case 100://100km
    {
        index = 5;
        break;
    }
    case 180://180km
    {
        index = 6;
        break;
    }
    default:
        break;
    }
    if(index > -1)
    {
        ui->cobBoxPW->clear();
        ui->cobBoxPW->addItems(strlist_pulse[index]);
    }
    return;
}
//窗口发生变化事件，设置画图窗口大小，设置测试进度条
void QDlgOtdr::resizeEvent(QResizeEvent *)
{
    QRect rectProgessBar;
    QPoint pt_1,pt_2;
    m_rectFrame = ui->widgetDraw->geometry();
    m_rectFigure = ui->widgetDraw->geometry();
    m_rectFigure.adjust(m_rectFigure.width()/15,m_rectFigure.height()/15,\
                        -m_rectFigure.width()/20, -m_rectFigure.height()/30);

    pt_1.setX(m_rectFigure.left());
    pt_1.setY(m_rectFigure.bottom());
    pt_2.setX(m_rectFigure.right());
    pt_2.setY(m_rectFigure.bottom() + 20);
    rectProgessBar.setTopLeft(pt_1);
    rectProgessBar.setBottomRight(pt_2);
    ui->progressBarTest->setGeometry(rectProgessBar);
    ui->progressBarTest->setHidden(true);
}
//设在显示进度条，每100ms更新一次
int QDlgOtdr:: showOtdrProgressbar(int msec, unsigned int  cmd, int sucess_notify)
{
    bool is_ok;
    int count, wait_num;
    int return_val, wait_space;
    QString str;
    is_ok = false;
    wait_space = 100;
    wait_num = msec/wait_space;
    count = 0;
    update();
    ui->progressBarTest->setRange(0, msec);
    ui->progressBarTest->setHidden(false);
    while(count < wait_num)
    {
        is_ok = m_pParent->objSyncSem[SEM_OTDR_DLG].objSem.tryAcquire(1,wait_space);
        if((is_ok && -1!= m_pParent->objSyncSem[SEM_OTDR_DLG].resCode) || count >= wait_num)
        {
            break;
        }
        else
        {
            count++;
            ui->progressBarTest->setValue(count * 100);
            update();
        }
    }
    ui->progressBarTest->setHidden(true);
    if(is_ok)
    {
        return_val = m_pParent->objSyncSem[SEM_OTDR_DLG].resCode;
        //返回为0时，是否提示
        if( RET_SUCCESS != return_val || sucess_notify)
            m_pParent->dealAbnormalResCod(m_pParent->objSyncSem[SEM_OTDR_DLG].resCode);

    }
    else
    {
        qDebug("hello sb  %d", m_pParent->objSyncSem[SEM_OTDR_DLG].resCode);
        str = tr("等待回应超时，请重试！");
        dealAbnormal(str);
        return_val = -1;
    }
    qDebug("进度条返回 %d",return_val);
    update();
    return return_val;
}

//点击获取软件信息
void QDlgOtdr::on_pushBtnGetSInfo_clicked()
{
    _tagDlgAttribute dev_type;
    //得到的设备软件版本号
    _tagCardVersion *pCardVersion;
    tms_devbase devbase;
    _tagBufList *pBufHead;
    QString str;
    int res_code;
    bool wait_sem_result;
    wait_sem_result = false;
    memcpy(&dev_type, &dlgAttribute, sizeof(dev_type));
    devbase.fd = 0;
    devbase.frame = dev_type.frameNo;
    devbase.slot = dev_type.cardNo;
    pBufHead = NULL;
    res_code = dlg_send((char *)&devbase, (char *)&dev_type,ID_GET_VERSION,\
                        ID_RET_VERSION,&pBufHead,ACK_TOT_DLG);
    if(res_code == RET_SUCCESS)
    {
        if(pBufHead != NULL)
        {
            //机框号
            pCardVersion = (_tagCardVersion *)(pBufHead->buf + 8);
            str.setNum(pCardVersion->frameNo + 1);
            ui->lineEdtFrame->setText(str);
            //板卡号
            str.setNum(pCardVersion->cardNo + 1);
            ui->lineEdtCard->setText(str);
            //设备类型
            str = m_pParent->m_strlstAllEqType.at(pCardVersion->devType);
            ui->lineEdtType->setText(str);
            ui->lineEdtType_2->setText(str);
            //软件版本号
            str = QString(QLatin1String(pCardVersion->softV));
            ui->lineEdtSoftV->setText(str);

            m_pParent->LinkedBufManager.freeBuf(pBufHead);
        }
        else
        {
            str = tr("pBufHead 为空，没有收到数据！");
            m_pParent->dealAbnormal(str);
            m_pParent->dealAbnormalResCod(res_code);
        }
    }
    return;
}
//初始化属性对话框
void QDlgOtdr::initial_attribute()
{
    int frame, card;
    QString str;
    //机框编号
    frame = dlgAttribute.frameNo;
    str.setNum(frame + 1);
    ui->lineEdtFrame->setText(str);
    //槽位号
    card = dlgAttribute.cardNo;
    str.setNum(card + 1);
    ui->lineEdtCard->setText(str);
    //类型
    str = m_pParent->m_strlstAllEqType.at(dlgAttribute.devType);
    ui->lineEdtType->setText(str);
     ui->lineEdtType_2->setText(str);
    //端口数目
    str.setNum(m_pParent->m_subrackCard[frame].ports[card]);
    ui->lineEdtPortNum->setText(str);
    if(dlgAttribute.devType == OTDR)
    {
        //标定是串行，还是并行
        if(m_pParent->m_subrackCard[frame].opt[card][1] == OTDR_MODE_PARAL)
        {
            str = tr("并行otdr");
        }
        else
        {
            str = tr("串行otdr");
        }
        ui->lineEditOther->setText(str);
    }
    return;
}

//获取硬件信息
void QDlgOtdr::on_pushBtnGetHInfo_clicked()
{
    _tagDlgAttribute dev_type;
    //得到的设备软件版本号
    _tagDlgAttribute *pCardHead;
    tms_devbase devbase;
    _tagBufList *pBufHead;
    QString str;
    int res_code, offset;
    bool wait_sem_result;
    char dataBuf[64];//产品序列号64位，最大的数组

    bzero(dataBuf, sizeof(dataBuf));
    wait_sem_result = false;
    memcpy(&dev_type, &dlgAttribute, sizeof(dev_type));
    devbase.fd = 0;
    devbase.frame = dev_type.frameNo;
    devbase.slot = dev_type.cardNo;
    pBufHead = NULL;
    res_code = dlg_send((char *)&devbase, (char *)&dev_type,ID_GET_DEV_PRODUCE,\
                        ID_RET_DEV_PRODUCE,&pBufHead,ACK_TOT_DLG);
    if(res_code == RET_SUCCESS)
    {
        if(pBufHead != NULL)
        {
            //_tagBufList 一个buf 256 字节，超过256字节，要从另外一个buf里面拷贝
            offset = sizeof(_tagDataHead);
            pCardHead = (_tagDlgAttribute *)(pBufHead->buf + offset);
            offset += sizeof(_tagDlgAttribute);
            //硬件版本
            memcpy(dataBuf,pBufHead->buf + offset, 20);
            offset += 20;
            str = QString(QLatin1String(dataBuf)); //char *转QString
            ui->lineEdtHardV->setText(str);

            //出厂日期
            memcpy(dataBuf,pBufHead->buf + offset, 20);
            offset += 20;
            str = QString(QLatin1String(dataBuf)); //char *转QString
            ui->lineEdtProDate->setText(str);
            //产品序列号
            memcpy(dataBuf,pBufHead->buf + offset, 64);
            str = QString(QLatin1String(dataBuf)); //char *转QString
            ui->lineEdtSerri->setText(str);
            m_pParent->LinkedBufManager.freeBuf(pBufHead);
        }
        else
        {
            str = tr("pBufHead 为空，没有收到数据！");
            m_pParent->dealAbnormal(str);
            m_pParent->dealAbnormalResCod(res_code);
        }
    }
    return;
}
//获取板卡参数
void QDlgOtdr::on_pushBtnGetModulPara_clicked()
{
    _tagDlgAttribute dev_type;
    //得到的设备软件版本号
    _tagDlgAttribute *pCardHead;
    tms_devbase devbase;
    _tagBufList *pBufHead;
    QString str;
    int res_code, offset;
    bool wait_sem_result;
    char dataBuf[64];//产品序列号64位，最大的数组

    bzero(dataBuf, sizeof(dataBuf));
    wait_sem_result = false;
    memcpy(&dev_type, &dlgAttribute, sizeof(dev_type));
    devbase.fd = 0;
    devbase.frame = dev_type.frameNo;
    devbase.slot = dev_type.cardNo;
    pBufHead = NULL;
    res_code = -1;
//    res_code = dlg_send((char *)&devbase, (char *)&dev_type,ID_GET_OTDR_PARAM,\
//                        ID_RET_OTDR_PARAM,&pBufHead,ACK_TOT_DLG);
    if(res_code == RET_SUCCESS)
    {
        if(pBufHead != NULL)
        {
            //_tagBufList 一个buf 256 字节，超过256字节，要从另外一个buf里面拷贝
            offset = sizeof(_tagDataHead);
            pCardHead = (_tagDlgAttribute *)(pBufHead->buf + offset);
            offset += sizeof(_tagDlgAttribute);
            //硬件版本
            memcpy(dataBuf,pBufHead->buf + offset, 20);
            str = QString(QLatin1String(dataBuf)); //char *转QString
            ui->lineEdtProDate->setText(str);
            //出厂日期
            memcpy(dataBuf,pBufHead->buf + offset, 20);
            str = QString(QLatin1String(dataBuf)); //char *转QString
            ui->lineEdtHardV->setText(str);
            //产品序列号
            memcpy(dataBuf,pBufHead->buf + offset, 64);
            str = QString(QLatin1String(dataBuf)); //char *转QString
            ui->lineEdtSerri->setText(str);
            m_pParent->LinkedBufManager.freeBuf(pBufHead);
        }
        else
        {
            str = tr("pBufHead 为空，没有收到数据！");
            m_pParent->dealAbnormal(str);
            m_pParent->dealAbnormalResCod(res_code);
        }
    }
    return;
}
/*
   **************************************************************************************
 *  函数名称：initial_module_conect_list
 *  函数描述：初始化关联模块关联表。0000---otdr.x
 *                     00000000-------osw1.com
 *                     osw1.com-------osw2.x
 *                     osw2.com-------osw2.com
 *                     osw2.com-------otdr.x
 *                     otdr.port(x)-------------00000
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::initial_module_conect_list()
{
    int ret_db,  i;
    int row, column;
    QString str;
    char *ptr_ip;
    struct in_addr ip_addr;
    tdb_route_t record;
    tdb_route_t *record_buf;
    pReferOtdrTsk = NULL;

    bzero(&rout_buf.buf, sizeof(_tagRoutRecord)*ROUT_BUF_LEN);
    bzero(&record, sizeof(tdb_route_t));
    record.frame_b = dlgAttribute.frameNo;
    record.slot_b = dlgAttribute.cardNo;
    record.type_b = dlgAttribute.devType;
    record.port_b = COMN_COM;
    record_buf = (tdb_route_t *)(&rout_buf.buf);
    rout_buf.record_num = ROUT_BUF_LEN;
    ret_db = tmsdb_Select_oneline(&record,record_buf,&rout_buf.record_num);
    if(ret_db >= 0&&rout_buf.record_num > 0)
    {
        ui->listConect->clearContents();
        for(i = 0; i < rout_buf.record_num;i++)
        {
            row = i;
            column = 0;
            memcpy(&ip_addr, &record_buf[i].ip_src,sizeof(record_buf[i].ip_src));
            ptr_ip = inet_ntoa(ip_addr);
            str = QString(QLatin1String(ptr_ip));
            //str.setNum(record_buf[i].ip_src);//源地址
            m_pParent->insert_tablewidget_item(str,ui->listConect, row,column++);

            memcpy(&ip_addr, &record_buf[i].ip_dst,sizeof(record_buf[i].ip_dst));//目的地址
            ptr_ip = inet_ntoa(ip_addr);
            //str.setNum(record_buf[i].ip_dst);//目的地址
            str = QString(QLatin1String(ptr_ip));
            m_pParent->insert_tablewidget_item(str,ui->listConect, row,column++);

            str.setNum(record_buf[i].frame_a);//机框号
            m_pParent->insert_tablewidget_item(str,ui->listConect, row,column++);

            str.setNum(record_buf[i].slot_a);//槽位号
            m_pParent->insert_tablewidget_item(str,ui->listConect, row,column++);

            if(record_buf[i].port_a != COMN_COM)
                str.setNum(record_buf[i].port_a);//端口号
            else
                str = "COM";
            m_pParent->insert_tablewidget_item(str,ui->listConect, row,column++);

            str = m_pParent->m_strlstAllEqType.at(record_buf[i].type_a);//设备类型
            m_pParent->insert_tablewidget_item(str,ui->listConect, row,column++);
        }
        //释放数据库分配的内存资源
        //delete []record_buf;
    }
    if(dlgAttribute.devType == OTDR)//本对话框为otdr
    {
        referOtdr.frame_no = dlgAttribute.frameNo;
        referOtdr.card_no = dlgAttribute.cardNo;
        referOtdr.type = OTDR;
        referOtdr.port = 0;
    }
    //本对话框osw，初始化为关联otdr对话框
    else if(rout_buf.record_num > 0 && rout_buf.buf[rout_buf.record_num - 1].DevA.type == OTDR)
    {
        memcpy(&referOtdr, &rout_buf.buf[rout_buf.record_num - 1].DevA, sizeof(_tagDevComm));
    }
    else //初始化为非法值
    {
        referOtdr.frame_no = -1;
        referOtdr.card_no = -1;
    }
    //获取关联otdr的管理任务
    pReferOtdrTsk = (tsk_OtdrManage *)m_pParent->get_otdr_tsk(referOtdr);
    /*
    qDebug("frame/slot/type/port %d %d %d %d", referOtdr.frame_no, referOtdr.card_no, referOtdr.type, referOtdr.port);
    qDebug("pReferOtdrTsk %x ", pReferOtdrTsk);
    */
    initial_local_device_test_port();
}
//获取模块关联路由从本地数据库中读取
void QDlgOtdr::on_pushBtnGetConct_clicked()
{
    initial_module_conect_list();
}
/*
   **************************************************************************************
 *  函数名称：
 *  函数描述：如果otdr端口发生变化，需要修改关联的波长
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr::on_cobBoxPortNo_currentIndexChanged(const QString &arg1)
{
    int port, res_code;   
    res_code = -1;
    port = arg1.toInt() - 1;
    set_measur_lamda();
    return;
}
//设置用户定时器
void QDlgOtdr::usr_timer_initial()
{
    usr_timer.ptr_timer= new QTimer;

    connect(usr_timer.ptr_timer,SIGNAL(timeout()),this,SLOT(usr_time_out()));

    usr_timer.ptr_timer->start(500);
}
//如果没有获取到关联模块的波长，那么定时去获取
void QDlgOtdr::usr_time_out()
{
    QString str;
    if(pReferOtdrTsk != NULL && referOtdrPara.initial != 1\
            && pReferOtdrTsk->modulPara.initial == 1)
    {
        referOtdrPara.initial = pReferOtdrTsk->modulPara.initial;
        referOtdrPara.lamdaList = pReferOtdrTsk->modulPara.lamdaList;
        set_measur_lamda();
        str = referOtdrPara.lamdaList.at(0);
        //qDebug()<<"lmda"<<str.simplified();
    }
    /*
    qDebug("pReferOtdrTsk 0x%x local initial %d tsk initial %d",\
           pReferOtdrTsk, referOtdrPara.initial,pReferOtdrTsk->modulPara.initial);
    */
}
/*
   **************************************************************************************
 *  函数名称：set_measur_lamda
 *  函数描述：设置波长
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgOtdr:: set_measur_lamda()
{
    int port;
    QString str;
    if(dlgAttribute.devType == OTDR)
    {
        str = ui->cobBoxPortNo->currentText();
        port = str.toInt() - 1;
    }
    else
    {
        port = referOtdr.port;
    }
    if(pReferOtdrTsk != NULL && referOtdrPara.initial == 1)
    {
        ui->cobBoxLamda->clear();
        str = referOtdrPara.lamdaList.at(port);
        ui->cobBoxLamda->insertItem(0, str);
        ui->pushBtnMeasur->setEnabled(true);
    }
    else
    {
        ui->pushBtnMeasur->setEnabled(false);
    }
    update();
    return;
}
