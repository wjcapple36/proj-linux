#include <QApplication>
#include <QTextCodec>

#include "mainwindow.h"
#include "src/tms_app.h"
//通信模块
struct ep_t ep;
extern MainWindow *pmain_window;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    /*
     *2016-03-11再此设置一次就ok
    */
    if(NULL == setlocale (LC_ALL, "zh_CN.UTF-8"))
    {
        qDebug("setlocale zh_CN.UTF-8 fail");
    }
    //首先运行serverAndShell
    ThreadRunServerAndShell(&ep);
    //设置回调函数
    tms_SetCB((void *)process_data_from_sock);
    //qDebug("mainwindow handle %d", pmain_window);
    sleep(3);
    MainWindow w;
    w.show();
    //w.showFullScreen();
    w.setWindowState(Qt::WindowMaximized);

    return a.exec();
}
