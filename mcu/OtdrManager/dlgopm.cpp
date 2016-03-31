#include <QMessageBox>
#include "dlgopm.h"
#include "ui_dlgopm.h"
#include "protocol/tmsxx.h"

dlgOpm::dlgOpm(QWidget *parent ,int frameNo, int cardNo,\
               int devType) :
    QDialog(parent),
    ui(new Ui::dlgOpm)
{
    QString str;
    ui->setupUi(this);
    m_pParent = (MainWindow *)parent;
    dlgAttribute.cardNo = cardNo;
    dlgAttribute.frameNo = frameNo;
    dlgAttribute.devType = OPM;
    if(devType != dlgAttribute.devType)
    {
        str = tr("设备类型与操作界面不符，建议进入界面后退出！");
        m_pParent->dealAbnormal(str);
    }
    m_pParent->m_currenShowDlg++;
    m_pParent->m_ptrCurentDlg = this;

    setWindowFlags(windowFlags()|Qt::FramelessWindowHint|Qt::WindowTitleHint);
    m_iPortNum = m_pParent->m_subrackCard[frameNo].ports[cardNo];
    m_ptrLable = NULL;
    m_ptrLcdNum = NULL;
    //m_iPortNum = 32;
    if(m_iPortNum < 0 || m_iPortNum > 32)
    {
        str = tr("端口数目出错！");
        m_pParent->dealAbnormal(str);
    }
    else
    {
        creat_widget();
    }

    LayoutOnDlg();
    memcpy(&m_pParent->curShowDlgAttr, &dlgAttribute, sizeof(dlgAttribute));
    initial_context();
    m_iIsGetPw = 0;
    usr_timer_initial();
}

dlgOpm::~dlgOpm()
{
    m_pParent->m_currenShowDlg--;
    m_pParent->m_ptrCurentDlg = NULL;
    memset(&m_pParent->curShowDlgAttr, 0, sizeof(m_pParent->curShowDlgAttr));
    disconnect(m_pParent);
    if(m_ptrLable != NULL)
        delete []m_ptrLable;
    if(m_ptrLcdNum != NULL)
        delete []m_ptrLcdNum;
    if(usr_timer.ptr_timer != NULL)
    {
        delete usr_timer.ptr_timer;
    }
    delete ui;
}
//创建控件
void dlgOpm::creat_widget()
{
    int i;
    QString str;
    m_ptrLable = new QLabel[m_iPortNum];
    if(m_ptrLable ==  NULL)
    {
        goto usr_exit;
    }
    m_ptrLcdNum = new QLineEdit [m_iPortNum];
    if(m_ptrLable ==  NULL)
    {
        delete []m_ptrLable;
        m_ptrLable = NULL;
        goto usr_exit;
    }
    //给每个端口赋值    
    for(i = 0;i < m_iPortNum;i++)
    {
        str = tr("端口") + str.setNum(i + 1);
        m_ptrLable[i].setText(str);
        str = "FFFFF";
        m_ptrLcdNum[i].setReadOnly(true);
        m_ptrLcdNum[i].setText(str);
    }
usr_exit:
    return;

}
/*
   **************************************************************************************
 *  函数名称：LayoutOnDlg
 *  函数描述：在tab上布局
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void dlgOpm:: LayoutOnDlg()
{
    int i, row, col1, col2;
    int row_pw;
    int height, width;
    int row_num, col_num;

    //将tab加入到主布局中
    ui->gridLayoutMain->addWidget(ui->tabWdgtCard, 0, 0, 1, 1);
    setLayout(ui->gridLayoutMain);
    //设置tab的布局
    row_num = 48;
    col_num = 12;
    row = 0;
    ui->tabPower->setLayout(ui->gridLayoutPower);
    //布局分三部分，1，设备信息区，2，显示功率区，3操作按钮区,后面为其所占行列
    ui->gridLayoutPower->addWidget(ui->groupBoxTitle, 0,0,8 ,11);
    ui->gridLayoutPower->addWidget(ui->groupBoxPw, 8,0,33 ,11);
    ui->gridLayoutPower->addWidget(ui->groupBoxOperate, 41,0,6 ,11);
    //设备信息所占行列
     ui->widgetDeviceInfo->setLayout(ui->VLayoutDeviceInfo);
     ui->gridLayoutPower->addWidget(ui->widgetDeviceInfo,0,0,7,6);
    //三个操作按钮所占区
    ui->widgetOperate->setLayout(ui->VHLayoutOperateBtn);
    ui->gridLayoutPower->addWidget(ui->widgetOperate,41,0,6,11);
    if(m_ptrLable != NULL&&m_ptrLcdNum != NULL)
    {
        row_pw = 9;
        //分48行，12列：前12行显示设备信息，中间48行显示功率，最后4行显示操作按钮
        if(m_iPortNum <= 16) //16端口
        {
            height = 2;
            width = 2;
            row = row_pw;
            col1 = col_num / 3 - 1;
            col2 = 2*col_num / 3 - 1;

            for (i = 0;i < m_iPortNum;i ++)
            {
                ui->gridLayoutPower->addWidget(( QWidget * )(&m_ptrLable[i]),row, col1,height, width );
                ui->gridLayoutPower->addWidget(( QWidget * )(&m_ptrLcdNum[i]),row, col2,height, width );
                row += height;
            }
        }
        else //32端口
        {            
            height = 2;
            width = 2;
            col1 = col_num / 5 - 1;
            col2 = width + col1;
            row = row_pw;
            for (i = 0;i < 16;i ++)
            {
                ui->gridLayoutPower->addWidget(( QWidget * )(&m_ptrLable[i]),row, col1,height, width );
                ui->gridLayoutPower->addWidget(( QWidget * )(&m_ptrLcdNum[i]),row, col2,height, width );
                row += height;
            }
            row = row_pw;
            col1 = col2 + width + 1;
            col2 = width + col1;
            for (;i < m_iPortNum;i ++)
            {
                ui->gridLayoutPower->addWidget(( QWidget * )(&m_ptrLable[i]),row, col1,height, width );
                ui->gridLayoutPower->addWidget(( QWidget * )(&m_ptrLcdNum[i]),row, col2,height, width );
                row += height;
            }
        }
    }

    //ui->tabWdgtCard->setLayout(ui->gridLayoutPower);
}
void dlgOpm::on_pushBtnGetCardVerson_clicked()
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
            ui->lineEdtDev->setText(str);
            //软件版本号
            str = QString(QLatin1String(pCardVersion->softV));
            ui->lineEdtVerson->setText(str);

            m_pParent->LinkedBufManager.freeBuf(pBufHead);
        }
        else
        {
            str = tr("pBufHead 为空，没有收到数据！");
            m_pParent->dealAbnormal(str);
        }
    }
    return;
}

void dlgOpm::on_pushBtnExit_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::information(this, tr("提示"), tr("是否退出？"),
                                     QMessageBox::Yes, QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
        close();
    }
}
void dlgOpm::initial_context()
{
    QString str;
    int port_num;
    //机框号
    str.setNum(dlgAttribute.frameNo + 1);
    ui->lineEdtFrame_1->setText(str);
    ui->lineEdtFrame->setText(str);
    //槽位号
    str.setNum(dlgAttribute.cardNo + 1);
    ui->lineEdtCard_1->setText(str);
    ui->lineEdtCard->setText(str);
    //设备类型
    str = m_pParent->m_strlstAllEqType.at(dlgAttribute.devType);
    ui->lineEdtType_1->setText(str);
    ui->lineEdtDev->setText(str);
    //端口数目
    port_num = m_pParent->m_subrackCard[dlgAttribute.frameNo].ports[dlgAttribute.cardNo];
    str.setNum(port_num);
    ui->lineEdtPort_1->setText(str);
}
//设置用户定时器
void dlgOpm::usr_timer_initial()
{
    usr_timer.ptr_timer = new QTimer;

    connect(usr_timer.ptr_timer,SIGNAL(timeout()),this,SLOT(usr_time_out()));

    usr_timer.ptr_timer->start(1000);
}
//发送获取光功率
void dlgOpm::usr_time_out()
{
    tms_devbase devbase;
    _tagDlgAttribute temp;
    memcpy(&temp, &dlgAttribute, sizeof(dlgAttribute));
    if(m_iIsGetPw == 1)
    {
        tms_GetDevBaseByLocation(dlgAttribute.frameNo, dlgAttribute.cardNo, &devbase);
        send_cmd(devbase.fd,(char *)(&temp),ID_GET_OPM_OP);
    }
}
//开始获取光功率
void dlgOpm::on_pushBbtnGetPw_clicked()
{
    QString str;
    str = tr("mcu与板卡联系中断！");
    if(m_pParent->DevCommuState[dlgAttribute.frameNo][dlgAttribute.cardNo].cur_type == NONE)
        m_pParent->dealAbnormal(str,0);
    m_iIsGetPw = 1;
}
//停止获取光功率
void dlgOpm::on_pushBtnStopGet_clicked()
{
    m_iIsGetPw = 0;
}
//初始化光功率值
void dlgOpm::initial_cur_pw(char buf[])
{
    QString str;
    _tagDevComm *pDev;
    int count,offset,i;
    int  port,pw;
    float fpw;
    fpw = 0;
    offset = sizeof(_tagDataHead);
    pDev = (_tagDevComm *)(buf + offset);
    offset += sizeof(_tagDevComm);
    count = pDev->port;
    qDebug("count %d", count);
    for(i = 0; i < count;i++)
    {
        memcpy(&port, buf + offset, sizeof(port));
        offset += sizeof(int);
        memcpy(&pw, buf + offset, sizeof(pw));
        offset += sizeof(int);
        if(port > m_iPortNum)
            continue;
        if(pw != 0xAFFFFFFF)
        {
            fpw = pw/10.0;            
            str.setNum(fpw,'f',1);
        }
        else
        {
            str = "FFFFF";
        }
        qDebug("opm %.1f pw %d port %d",fpw, pw, port);
        m_ptrLcdNum[port].setText(str);       
    }
}
