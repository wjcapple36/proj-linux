#include <QMessageBox>

#include "dlggsm.h"
#include "ui_dlggsm.h"
#include "protocol/tmsxx.h"

dlgGsm::dlgGsm(QWidget *parent ,int frameNo, int cardNo,\
               int devType) :
    QDialog(parent),
    ui(new Ui::dlgGsm)
{
    QString str;
    ui->setupUi(this);
    m_pParent = (MainWindow *)parent;
    dlgAttribute.cardNo = cardNo;
    dlgAttribute.frameNo = frameNo;
    dlgAttribute.devType = GSM;
    if(devType != dlgAttribute.devType)
    {
        str = tr("设备类型与操作界面不符，建议进入界面后退出！");
        m_pParent->dealAbnormal(str);
    }
    m_pParent->m_currenShowDlg++;
    memcpy(&m_pParent->curShowDlgAttr, &dlgAttribute, sizeof(dlgAttribute));
    setWindowFlags(windowFlags()|Qt::FramelessWindowHint|Qt::WindowTitleHint);
    LayoutOnDlg();
}

dlgGsm::~dlgGsm()
{
    m_pParent->m_currenShowDlg--;
    memset(&m_pParent->curShowDlgAttr, 0, sizeof(m_pParent->curShowDlgAttr));
    disconnect(m_pParent);
    delete ui;
}
void dlgGsm:: LayoutOnDlg()
{
    ui->gridLayoutMain->addWidget(ui->tabWdgtCard, 0, 0, 1, 1);
    ui->tabWdgtCard->removeTab(1);

    setLayout(ui->gridLayoutMain);
}

//获取软件版本号
void dlgGsm::on_pushBtnGetCardVerson_clicked()
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

void dlgGsm::on_pushBtnExit_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::information(this, tr("提示"), tr("是否退出？"),
                                     QMessageBox::Yes, QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
        close();
    }
}
