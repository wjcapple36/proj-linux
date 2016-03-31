#include <QMessageBox>
#include <QDebug>
#include <QTableWidgetItem>
#include <QByteArray>
#include "dlgmcu.h"
#include "ui_dlgmcu.h"
#include "constant.h"
#include "time.h"
#include "protocol/tmsxx.h"


QDlgMcu::QDlgMcu(QWidget *parent, int frameNo, \
                 int cardNo, int deviceType) :
    QDialog(parent),
    ui(new Ui::QDlgMcu)
{
    QString str;
    ui->setupUi(this);
    m_pParent = (MainWindow *)parentWidget();
    setWindowFlags(windowFlags()|Qt::FramelessWindowHint|Qt::WindowTitleHint);
    //当前显示段对话框数目++
    m_pParent->m_currenShowDlg++;
    dlgAttribute.cardNo = cardNo;
    dlgAttribute.frameNo = frameNo;
    dlgAttribute.devType = MCU;
    if(deviceType != dlgAttribute.devType)
    {
        str = tr("设备类型与操作界面不符，建议进入界面后退出！");
    }
    memcpy(&m_pParent->curShowDlgAttr, &dlgAttribute, sizeof(dlgAttribute));
    is_config_modfy = false;
    //初始化槽位号，板卡号
    InitialCardTypeCombox();
    //布局
    LayoutOnDlg();
    InitialSubrackAndCadInfo();
    InitialSubrackCombox();
    //创建机框信息表
    CreatSubrackTable();
    //显示机框信息
    InitialSubrackList();
    str = tr("软件版本号: ") + m_pParent->strSoftVerson;
    ui->labelSoftV->setText(str);

}

QDlgMcu::~QDlgMcu()
{
    m_pParent->m_currenShowDlg--;
    memset(&m_pParent->curShowDlgAttr, 0, sizeof(m_pParent->curShowDlgAttr));
    disconnect(m_pParent);
    delete ui;
}
/*
   **************************************************************************************
 *  函数名称：LayoutOnDlg
 *  函数描述：在dlg界面上排列部件
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgMcu::LayoutOnDlg()
{
    //屏幕比例4*3，横纵各分20*15
    ui->gridLayoutMain->addWidget(ui->wdgtDisplay, 0, 0, 3, 15);
    ui->gridLayoutMain->addWidget(ui->wdgtModifySlot, 0, 16, 15, 3);
    ui->gridLayoutMain->addWidget(ui->wdgtAddFrame, 3, 0, 12, 15);
    setLayout(ui->gridLayoutMain);
}
/*
   **************************************************************************************
 *  函数名称：on_pushBtnExit_clicked
 *  函数描述：退出对话框
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/

void QDlgMcu::on_pushBtnExit_clicked()
{    
    QMessageBox::StandardButton reply;    
    reply = QMessageBox::information(this, tr("提示"), tr("是否退出？"),
                                     QMessageBox::Yes, QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
        if(is_config_modfy)
            save_config();
        m_pParent->refresh_card_slot_config();
        close();

    }
}
/*
   **************************************************************************************
 *  函数名称：CreatSubrackTable
 *  函数描述：创建机框信息表：编号，插槽书目，已使用数目，修改日期
 *                    设置为只能单行选中
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgMcu::CreatSubrackTable()
{
    QStringList header;
    header<<tr("机框编号")<<tr("机框类型")<<tr("已用槽位")<<tr("槽位总数")<<tr("修改日期");
    ui->listFrame->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //    ui->listFrame->verticalHeader()->setFixedWidth(80);
    ui->listFrame->setRowCount(NUM_SUBRACK);
    ui->listFrame->setColumnCount(5);
    ui->listFrame->setColumnWidth(4,185);
    ui->listFrame->setSelectionBehavior(QAbstractItemView::SelectRows );
    ui->listFrame->setSelectionMode ( QAbstractItemView::SingleSelection); //设置选择模式，选择单行

    ui->listFrame->setHorizontalHeaderLabels(header);
    ui->listFrame->horizontalHeader()->setStretchLastSection(true);//关键
}
/*
   **************************************************************************************
 *  函数名称：on_listFrame_clicked
 *  函数描述：在列表上点击，返回选中段列
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgMcu::on_listFrame_clicked(const QModelIndex &index)
{
    QString str;
    int subrackNo, i;
    QTableWidgetItem *item = ui->listFrame->currentItem();
    //判断用户点击地方有无设置item
    if(NULL == item)
    {
        return;
    }
    //获取机框编号，每行的第0列
    i = index.row();
    str = ui->listFrame->item(i,0)->text();
    //    qDebug()<<str.simplified();
    //    qDebug()<<"select row"<<index.row()<<"select colmn"<<index.column();
    subrackNo = str.toInt() - 1;

    qDebug()<<"subrackNo"<<subrackNo;
    //初始化设备类型
    InitialSelectSubrackCardType( subrackNo);

}
/*
   **************************************************************************************
 *  函数名称：InitialSubrackList
 *  函数描述：根据配置信息初始化
 *  入口参数：sel_frame,要处于显示段机框号 -1代表默认情况，显示最后一行
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgMcu:: InitialSubrackList(int sel_frame)
{
    int i, state,subrackNo ;
    int row, column;
    int sel_row;
    QString str;
    //先释放之前段资源
    ui->listFrame->clearContents();
    row = 0;
    column = 0;
    //轮询，初始化机框信息
    sel_row = -1;
    for(i = 0; i < NUM_SUBRACK;i++)
    {
        state = m_subrackInfo.onState;
        if(1 ==  ( 1&(state >> i))
                )
        {           
            column = 0;
            //机框编号
            str.setNum(i + 1);
            ui->listFrame->setItem(row,column++, new QTableWidgetItem(str));
            if(0 == i)
            {
                str = tr("主机");
            }
            else
            {
                str = tr("从机");
            }
            //确定处于选中状态的行号
            if(i == sel_frame)
                sel_row = row;
            ui->listFrame->setItem(row,column++, new QTableWidgetItem(str));
            str.setNum(m_subrackCard[i].numInUse);
            ui->listFrame->setItem(row,column++, new QTableWidgetItem(str));
            str.setNum(m_subrackCard[i].numTotal);
            ui->listFrame->setItem(row,column++, new QTableWidgetItem(str));
            str = QString(QLatin1String(m_subrackInfo.oprateTime[i]));
            ui->listFrame->setItem(row++,column++, new QTableWidgetItem(str));            

        }
    }
    if(sel_row > -1 && sel_row  < row )
    {
        i = sel_row;
        ui->listFrame->selectRow(sel_row);
    }
    else if(row > 0)
    {
        i = row - 1;
        ui->listFrame->selectRow(row - 1);
    }
    else
        goto usr_exit;

    str = ui->listFrame->item(i,0)->text();
    subrackNo = str.toInt() - 1;
    //  qDebug()<<str.simplified()<<subrackNo;
    InitialSelectSubrackCardType(subrackNo);
    //设置当前显示段机框编号
    str.setNum(m_subrackInfo.curShowNo + 1);
    str = tr("当前显示机框号:") + str;
    ui->labelShowFrame->setText(str);
usr_exit:    return;
}
/*
   **************************************************************************************
 *  函数名称：InitialSubrackAndCadInfo
 *  函数描述：初始化级联以及插卡信息
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgMcu::InitialSubrackAndCadInfo()
{    
    memcpy(m_subrackCard, m_pParent->m_subrackCard, sizeof(_tagSubrackCard)*NUM_SUBRACK);
    memcpy(&m_subrackInfo, &m_pParent->m_subrackInfo, sizeof(m_subrackInfo));    
}
/*
   **************************************************************************************
 *  函数名称：InitialSubrackCombox
 *  函数描述：初始化当前显示机框号和增加从机机框号
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgMcu::InitialSubrackCombox()
{
    int i, k_1, k_2, state;
    QString str;
    k_1 = 0;
    k_2 = 0;
    ui->combBoxShowFram->clear();
    ui->combBoxAddFrame->clear();
    for(i = 0; i < NUM_SUBRACK; i++)
    {
        state = m_subrackInfo.onState;
        if(0 ==  ( 1&(state >> i))
                )
        {
            //设置增加从机框
            str.setNum(i + 1);
            ui->combBoxAddFrame->addItem(str, k_2++);
        }
        else if(i != m_subrackInfo.curShowNo)
        {
            //设置当前显示框combox
            //            qDebug()<<"i"<<i<<"m_subrackInfo.curShowNo"<<m_subrackInfo.curShowNo;
            str.setNum(i + 1);
            ui->combBoxShowFram->addItem(str, k_1++);
        }
    }
    //    str.setNum(m_subrackInfo.curShowNo);
    //    k_1 = ui->combBoxShowFram->findText(str);
    ui->combBoxShowFram->setCurrentIndex(0);
}
/*
   **************************************************************************************
 *  函数名称：InitialCardTypeCombox
 *  函数描述：将槽位设备类型段combox初始化，10MCU，11，12电源
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgMcu::InitialCardTypeCombox()
{
    int i;
    //初始化槽位端口号
    i = 0;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_1;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_2;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_3;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_4;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_5;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_6;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_7;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_8;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_9;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_10;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_11;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_12;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_13;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_14;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_15;
    m_pComBoxSlotPort[i++] = ui->comBoxSlotPort_16;
    /*
    ui->comBoxSlotPort_10->insertItem(0,"0");
    ui->comBoxSlotPort_11->insertItem(0,"0");
    ui->comBoxSlotPort_12->insertItem(0,"0");
    */
    //初始化槽位设备类型
    i = 0;
    m_pComBoxSlot[i++] =ui->comboxSlot_1;
    m_pComBoxSlot[i++] =ui->comboxSlot_2;
    m_pComBoxSlot[i++] =ui->comboxSlot_3;
    m_pComBoxSlot[i++] =ui->comboxSlot_4;
    m_pComBoxSlot[i++] =ui->comboxSlot_5;
    m_pComBoxSlot[i++] =ui->comboxSlot_6;
    m_pComBoxSlot[i++] =ui->comboxSlot_7;
    m_pComBoxSlot[i++] =ui->comboxSlot_8;
    m_pComBoxSlot[i++] =ui->comboxSlot_9;
    m_pComBoxSlot[i++] =ui->comboxSlot_10;
    m_pComBoxSlot[i++] =ui->comboxSlot_11;
    m_pComBoxSlot[i++] =ui->comboxSlot_12;
    m_pComBoxSlot[i++] =ui->comboxSlot_13;
    m_pComBoxSlot[i++] =ui->comboxSlot_14;
    m_pComBoxSlot[i++] =ui->comboxSlot_15;
    m_pComBoxSlot[i++] =ui->comboxSlot_16;
    //前9个为普通段槽位
    for(i = 0;i < MCU_CARD;i++)
    {
        m_pComBoxSlot[i]->addItems(m_pParent->m_strlstEqType);
    }
    m_pComBoxSlot[MCU_CARD]->insertItem(0,tr("MCU"));
    m_pComBoxSlot[MCU_CARD]->insertItem(0,tr("TU"));
    for(i = MCU_CARD + 1; i < NUM_CARD;i++)
    {
        m_pComBoxSlot[i]->insertItem(0,tr("NONE"));
        m_pComBoxSlot[i]->insertItem(1,tr("PWU"));
    }
    /*
    //10槽位，MCU在11，12槽位加入PWU
    ui->comboxSlot_10->insertItem(0,tr("MCU"));
    ui->comboxSlot_11->insertItem(0,tr("NONE"));
    ui->comboxSlot_11->insertItem(1,tr("PWU"));
    ui->comboxSlot_12->insertItem(0,tr("NONE"));
    ui->comboxSlot_12->insertItem(1,tr("PWU"));
    */
}
/*
   **************************************************************************************
 *  函数名称：InitialSelectSubrackCardType
 *  函数描述：机框列表中选中某一行，右边初始化该机框的设备类型
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgMcu::InitialSelectSubrackCardType(int subrackNo)
{
    bool check;
    check = subrackNo >=0 && subrackNo < NUM_SUBRACK;
    Q_ASSERT_X(check, "subrackNo", "subrackNo 非法");
    int i, state, index,port_num;
    int dev_type;
    QString str;
    str.setNum(subrackNo + 1);
    ui->lineEdtSubrack->setText(str);
    for(i = 0; i < NUM_CARD;i++)
    {
        state = m_subrackCard[subrackNo].onState;
        if(1 ==  ( 1&(state >> i))
                )
        {
            dev_type = m_subrackCard[subrackNo].type[i];
            //qDebug()<<"state"<<state;
            str = m_pParent->m_strlstAllEqType.at(dev_type);
            index = m_pComBoxSlot[i]->findText(str);
            if(-1 != index)
            {
                m_pComBoxSlot[i]->setCurrentIndex(index);
                port_num = m_subrackCard[subrackNo].ports[i];
                initial_dev_port_num(i,dev_type,port_num);
            }
            else
                m_pComBoxSlot[i]->setCurrentIndex(0);
            //qDebug()<<"name = "<<str.simplified()<<"index"<<index<<i;
        }
        else
        {
            m_pComBoxSlot[i]->setCurrentIndex(0);

        }
    }
}

/*
   **************************************************************************************
 *  函数名称：on_pushBtnAddFrame_clicked
 *  函数描述：增加从机，更新显示机框编号复合框，机框信息列表
 *  函数描述：默认双电源+MCU
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgMcu::on_pushBtnAddFrame_clicked()
{
    int subrackNo;
    QString str;
    str = ui->combBoxAddFrame->currentText();
    //如果机框全部使用，那么该机框为空
    if(str.isEmpty())
        return;
    subrackNo = str.toInt() - 1;
    //    qDebug()<<"subrankNo"<<subrackNo<<"onState"<<m_subrackInfo.onState;

    m_subrackInfo.numInUse++;
    m_subrackInfo.onState = m_subrackInfo.onState | (1 << subrackNo);
    //    qDebug()<<"onState2"<<m_subrackInfo.onState;
    m_pParent->GetCurrentTime(m_subrackInfo.oprateTime[subrackNo]);
    memset(&m_subrackCard[subrackNo], 0 ,sizeof(m_subrackCard[subrackNo]));
    //默认启用的是双电源，mcu
    m_subrackCard[subrackNo].onState = 1 << MCU_CARD;
    m_subrackCard[subrackNo].onState =\
            m_subrackCard[subrackNo].onState | 1 << (MCU_CARD + 1) ;
    m_subrackCard[subrackNo].onState =\
            m_subrackCard[subrackNo].onState | 1 << (MCU_CARD + 2) ;
    m_subrackCard[subrackNo].numInUse = 3;
    m_subrackCard[subrackNo].type[MCU_CARD] = MCU;
    m_subrackCard[subrackNo].type[MCU_CARD + 1] = PWU;
    m_subrackCard[subrackNo].type[MCU_CARD + 2] = PWU;
    m_subrackCard[subrackNo].ports[MCU_CARD] = 0;
    m_subrackCard[subrackNo].ports[MCU_CARD + 1] = 0;
    m_subrackCard[subrackNo].ports[MCU_CARD + 2] = 0;
    m_subrackCard[subrackNo].numTotal = NUM_CARD;
    InitialSubrackList(subrackNo);
    InitialSubrackCombox();
    str = tr("请稍候添加槽位号！");
    m_pParent->dealAbnormal(str);
    return;
}
//删除从机
void QDlgMcu::on_pushBtnDelFrame_clicked()
{
    QString str;
    int row, subrackNo;
    row = ui->listFrame->currentRow();
    str = ui->listFrame->item(row,0)->text();
    subrackNo = str.toInt() - 1;
    if(0 == subrackNo)
        return;
    else if(m_subrackInfo.curShowNo == subrackNo)
        m_subrackInfo.curShowNo = 0;
    str = tr("是否删除本从机及其关联设备？");
    if(m_pParent->dealAbnormal(str,1) > 0)
    {
        if(m_subrackCard[subrackNo].numInUse > 0)
            is_config_modfy = true;
        //清零该位
        m_subrackInfo.onState =  m_subrackInfo.onState & (~(1 << subrackNo));
        memset(&m_subrackCard[subrackNo], 0 ,sizeof(m_subrackCard[subrackNo]));
        //更新机框列表，显示机框编号
        InitialSubrackList();
        InitialSubrackCombox();
    }
}
//切换当前显示机框编号
void QDlgMcu::on_pushBtnShowFram_clicked()
{
    int subrackNo;
    QString str;
    str = ui->combBoxShowFram->currentText();
    subrackNo = str.toInt() - 1;
    if(subrackNo > -1 && subrackNo < NUM_SUBRACK)
    {
        m_subrackInfo.curShowNo = subrackNo;
        ui->labelShowFrame->clear();
        //    str.setNum(subrackNo);
        str = tr("当前显示机框号:") + str;
        ui->labelShowFrame->setText(str);
        m_pParent->m_subrackInfo.curShowNo = subrackNo;
        InitialSubrackCombox();
    }
    return;
}

//处理异常提示
void QDlgMcu::dealAbnormal(QString str)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::information(this, tr("提示"), str,
                                     QMessageBox::Yes);

}
/*
   **************************************************************************************
 *  函数名称：on_pushBtnModifyFram_clicked
 *  函数描述：修改当前从机的槽位配置
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgMcu::on_pushBtnModifyFram_clicked()
{
    int cur_frame_no;
    int dev_type[NUM_CARD];
    int dev_port[NUM_CARD];
    int card_used, on_state;
    int i;
    bool is_modify;
    is_modify = false;
    QString str;
    str = tr("是否确定修改本槽位设备类型？");
    if(m_pParent->dealAbnormal(str,1) < 1)
        goto usr_exit;
    card_used = 0;
    on_state = 0;
    str = ui->lineEdtSubrack->text();
    qDebug()<<str.simplified();
    cur_frame_no = str.toInt() - 1;
    memset(dev_type, 0, sizeof(dev_type));
    memset(dev_port, 0, sizeof(dev_port));
    if(cur_frame_no > -1 && cur_frame_no < NUM_SUBRACK)
    {
        for(i = 0; i < NUM_CARD; i++)
        {
            //获取设备类型
            str = m_pComBoxSlot[i]->currentText();
            dev_type[i] = m_pParent->m_strlstAllEqType.indexOf(str);
            //获取端口数目
            str= m_pComBoxSlotPort[i]->currentText();
            dev_port[i] = str.toInt();

            if(dev_type[i] < m_pParent->m_ctrlStat.minDevType || dev_type[i] > m_pParent->m_ctrlStat.maxDevType\
                    || dev_port[i] < 0)
                break;
            else if(dev_type[i] != NONE)
            {
                card_used++;
                on_state = (on_state | (1 <<i));
            }
        }
        if(i == NUM_CARD)
        {
             is_modify = false;
            //check的时候会给出提示信息
            if(check_dev_type(cur_frame_no, dev_type,dev_port) > -1)
            {
                //如果检查成功，那么使用设备获取到的端口号
                is_modify = true;
                //如果能板卡通信，则使用动态获取的端口数目
                for(i = 0; i < NUM_CARD;i++)
                {
                    if(m_subrackCard[cur_frame_no].type[i] != PWU &&\
                            m_subrackCard[cur_frame_no].type[i] != MCU &&m_subrackCard[cur_frame_no].type[i] != NONE)
                        dev_port[i] = m_pParent->DevCommuState[cur_frame_no][i].cur_port;
                }
                //memcpy(dev_port, m_pParent->dynamicDevtype[cur_frame_no].port_num,sizeof(dev_port));
            }
            else
            {
                str = tr("您所选择段设备类型与系统探测到的设备类型不同，\n是否使用您所选的配置？");
                if(m_pParent->dealAbnormal(str,1) >= 1)
                     is_modify = true;
            }
            //如果修改
            if(is_modify)
            {
                memcpy(m_subrackCard[cur_frame_no].type, dev_type, sizeof(dev_type));
                memcpy(m_subrackCard[cur_frame_no].ports, dev_port, sizeof(dev_port));
                //设置mcu pwu pwu的端口号
                m_subrackCard[cur_frame_no].ports[MCU_CARD] = 0;
                m_subrackCard[cur_frame_no].ports[MCU_CARD + 1] = 0;
                m_subrackCard[cur_frame_no].ports[MCU_CARD + 2] = 0;
                m_subrackInfo.onState = m_subrackInfo.onState | (1 << cur_frame_no);
                m_subrackCard[cur_frame_no].onState = on_state;
                m_subrackCard[cur_frame_no].numInUse = card_used;
                m_subrackCard[cur_frame_no].numTotal = NUM_CARD;
                m_pParent->GetCurrentTime(m_subrackCard[cur_frame_no].oprateTime);
                memcpy(m_subrackInfo.oprateTime[cur_frame_no], m_subrackCard[cur_frame_no].oprateTime,sizeof(char)*20);
                is_config_modfy = true;
                InitialSubrackList(cur_frame_no);
            }
        }
        else
        {
            str.setNum(i);
            str = str + tr("槽位设备类型错误");
            m_pParent->dealAbnormal(str);
        }
    }
usr_exit:    return;
}
/*
  **************************************************************************************
*  函数名称：on_pushBtnSaveFram_clicked
*  函数描述：保存所有已修改的配置
*  入口参数：
*  返回参数：
*  作者       ：
*  日期       ：
**************************************************************************************
*/
void QDlgMcu::on_pushBtnSaveFram_clicked()
{
    save_config();
    return;
}
/*
   **************************************************************************************
 *  函数名称：check_dev_type
 *  函数描述：检查MCU，PWU是否存在。OLS不会与MCU通信，其他要与设备
 *  函数描述：发上来的信息做比对，如果满足条件方可判别为正确
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改       ：MCU槽位是9，不是10，从0开始计算 2015-10-22
 **************************************************************************************
*/
int QDlgMcu ::check_dev_type(int frame_no, int dev_type[], int port[])
{
    int i, return_val;
    QString str1, str2;
    return_val = 0;
    // 10,11,12槽位设备类型固定
    if( (dev_type[MCU_CARD ] == MCU ||  dev_type[MCU_CARD ] == TU )&& \
            (dev_type[MCU_CARD + 1]  == PWU || dev_type[MCU_CARD + 2] == PWU))
    {
        //m_pParent->update_dev_type(frame_no);
        for(i = 0; i < NUM_COMM_CARD;i++)
        {
            //判断设备类型与从板卡收到的设备类型作比较
            if(dev_type[i] != m_pParent->DevCommuState[frame_no][i].cur_type)
            {
                //2015-10-22 OLS 不再特殊处理
                if(/*dev_type[i] != OLS*/true)
                {
                    return_val = -1;
                    break;
                }
            }
            else
            {
                port[i] = m_pParent->DevCommuState[frame_no][i].cur_port;
            }
        }
        if(return_val == -1)
        {
            str1.setNum(i + 1);
            str2 = m_pParent->m_strlstAllEqType.at(m_pParent->DevCommuState[frame_no][i].cur_type);
            str1 = tr("请注意：槽位")  + str1 + tr("设备类型为") + str2 + tr("你配置的为");
            str2 = m_pParent->m_strlstAllEqType.at(dev_type[i]);
            str1 = str1 + str2 ;
            m_pParent->dealAbnormal(str1);
        }
    }
    else
    {
        str2.setNum(MCU_CARD + 2);
        str1.setNum(MCU_CARD + 1);
        str1 = str1 + tr("槽位固定是MCU或者TU") + ","  + str2 +  ",";
        str2.setNum(MCU_CARD + 3);
        str1 = str1 + str2 + tr("最少有一个是电源，请确认！");
        m_pParent->dealAbnormal(str1);
        return_val = -1;
    }
    return return_val;
}
//保存配置
void QDlgMcu::save_config()
{
    QString str;
    int res_code;
    if(is_config_modfy)
    {
        str = tr("配置已修改，是否保存 ？");
        if(m_pParent->dealAbnormal(str,1) > 0)
        {
            //首先清空数据库
            res_code = m_pParent->check_card_comp(m_subrackCard);
            if(res_code == RET_SUCCESS)
            {
                //m_pParent->find_changed_dev(&m_subrackInfo, m_subrackCard);
                m_pParent->objOperateFrameCard.lock();
                memcpy(m_pParent->m_subrackCard, m_subrackCard, sizeof(_tagSubrackCard)*NUM_SUBRACK);
                memcpy(&m_pParent->m_subrackInfo,&m_subrackInfo, sizeof(m_subrackInfo));
                m_pParent->wrtie_db_card_comp();
                m_pParent->objOperateFrameCard.unlock();
            }
            is_config_modfy = false;

        }
    }
    else
    {
        str = tr("配置没有修改！");
        m_pParent->dealAbnormal(str);
    }
    return;
}


/*
   **************************************************************************************
 *  函数名称：on_pushBtnViewCurCfg_clicked
 *  函数描述：查看当前的实时配置。OLS特殊处理，由用户配置
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 *  修改       ：OLS PWU 全部从动态获取中配置 2015-10-22
 **************************************************************************************
*/
//查看当前实时配置
void QDlgMcu::on_pushBtnViewCurCfg_clicked()
{
    int i, frame_no, index;
    int port_num, dev_type;
    bool is_empty;
    QString str, str1;
    str = ui->lineEdtSubrack->text();
    frame_no = str.toInt() - 1;
    if(frame_no >= 0 && frame_no < NUM_SUBRACK)
    {
        m_pParent->update_dev_type(frame_no);
        for(i = 0; i < NUM_CARD;i++)
        {
            dev_type = m_pParent->DevCommuState[frame_no][i].cur_type; //获取设备类型
            str = m_pParent->m_strlstAllEqType.at(dev_type);//回去类型的字符串
            str1 = m_pComBoxSlot[i]->currentText();//找到当前显示的设备类型
            index = m_pComBoxSlot[i]->findText(str);
            if(-1 != index)
            {
                //2015-10-22 OLS通过实时状态信息配置
                /*
                if(str != "NONE")
                {
                    m_pComBoxSlot[i]->setCurrentIndex(index);
                    port_num = m_pParent->dynamicDevtype[frame_no].port_num[i];
                    initial_dev_port_num(i, dev_type,port_num);
                }
                //str == NONE，str1 != OLS 说明该槽位没有获得设备类型
                else if(str1 != "OLS")
                    m_pComBoxSlot[i]->setCurrentIndex(index);
                */
                m_pComBoxSlot[i]->setCurrentIndex(index);
                port_num = m_pParent->DevCommuState[frame_no][i].cur_port;
                initial_dev_port_num(i, dev_type,port_num);
            }
        }
    }    

}
/*
   **************************************************************************************
 *  函数名称：initial_comm_port_num
 *  函数描述：选中设备类型之后，对应段的端口数目需要更新，opm,ols,osw
 *  函数描述：端口数目8 16 32 64 128，otdr, olp，分别为 1 ，3个端口
 *  入口参数：
 *  返回参数：
 *  作者       ：
 *  日期       ：
 **************************************************************************************
*/
void QDlgMcu::initial_dev_port_num(int slot_index, int dev_type, int port_num)
{
    QString str;
    int index, count;
    //首先检查参数是否合法
    if(slot_index < 0 || slot_index > NUM_CARD)
        goto usr_exit;
    if(dev_type < m_pParent->m_ctrlStat.minDevType || dev_type > m_pParent->m_ctrlStat.maxDevType)
        goto usr_exit;
    str.setNum(0);
    m_pComBoxSlotPort[slot_index]->clear();
    switch(dev_type)
    {
    case OPM:
    case OLS:
    case OSW:
    {
        {
            m_pComBoxSlotPort[slot_index]->addItems(m_pParent->strlstDevPortNum);
            break;
        }
    }
    case OTDR:
    {
        m_pComBoxSlotPort[slot_index]->addItems(m_pParent->strlstOtdrPortNum);
        break;
    }
    case OLP:
    {
        m_pComBoxSlotPort[slot_index]->addItems(m_pParent->strlstOlpPortNum);
        break;
    }
    default:
    {
        m_pComBoxSlotPort[slot_index]->addItem(str);
        break;
    }
    }
    //根据端口选择当前index,如果端口不存在，那么就插入到列表中并选中
    if(port_num != -1)
    {
        str.setNum(port_num);
        index = m_pComBoxSlotPort[slot_index]->findText(str);
        if(index != -1)
        m_pComBoxSlotPort[slot_index]->setCurrentIndex(index);
        else
        {
            count = m_pComBoxSlotPort[slot_index]->count();
            m_pComBoxSlotPort[slot_index]->insertItem(count,str);
            m_pComBoxSlotPort[slot_index]->setCurrentIndex(count);
        }
    }

usr_exit:return;
}

void QDlgMcu::on_comboxSlot_1_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 0;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_2_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 1;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_3_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 2;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_4_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 3;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_5_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 4;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_6_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 5;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_7_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 6;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_8_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 7;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_9_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 8;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_10_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 9;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_11_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 10;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_12_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 11;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_13_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 12;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_14_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 13;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_15_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 14;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}

void QDlgMcu::on_comboxSlot_16_currentIndexChanged(const QString &arg1)
{
    int slot_index, dev_type;
    slot_index = 15;
    dev_type = m_pParent->get_dev_type(arg1);
    initial_dev_port_num(slot_index, dev_type);
    return;
}
