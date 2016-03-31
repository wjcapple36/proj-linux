#ifndef DLGOPM_H
#define DLGOPM_H

#include <QDialog>
#include <QLabel>
#include <QLCDNumber>
#include "mainwindow.h"
#include "struct.h"
namespace Ui {
class dlgOpm;
}
class dlgOpm : public QDialog
{
    Q_OBJECT
    
public:
    explicit dlgOpm(QWidget *parent = 0,int frameNo = 0, int cardNo = 0,\
                    int devType = OPM);
    ~dlgOpm();
public:
    //指向父窗口的类
    MainWindow * m_pParent;
    //从机号,槽位号，设备类型
    _tagDlgAttribute dlgAttribute;
    QLabel *m_ptrLable;
    QLineEdit *m_ptrLcdNum;
    _tagUsrTimer usr_timer;
    int m_iPortNum;
    int m_iIsGetPw; //是否获取功率值
public:
    void LayoutOnDlg();
    void creat_widget();
    void initial_context();
    void usr_timer_initial();
    void initial_cur_pw(char buf[]);
public slots:
    void usr_time_out() ;
private slots:    
    
    void on_pushBtnGetCardVerson_clicked();

    void on_pushBtnExit_clicked();

    void on_pushBbtnGetPw_clicked();

    void on_pushBtnStopGet_clicked();

private:
    Ui::dlgOpm *ui;
};

#endif // DLGOPM_H
