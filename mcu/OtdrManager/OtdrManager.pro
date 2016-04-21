#-------------------------------------------------
#
# Project created by QtCreator 2015-01-19T11:14:45
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app
#--------------------------------------------------
#
#平台相关选项配置-----start
#
#--------------------------------------------------

#x86平台
ARCH = x86
#ARM平台
#ARCH=armv7
#GPIO驱动版本
DEFINES *= GPIO_EDITION_12
#定时重启功能
DEFINES *= PRIOD_REBOOT
#调试选项
DEFINES *= TMS_DEBUG

contains(ARCH,x86)\
{
TARGET = OtdrManager
#编译x86平台 Start
LIBS += -lreadline -lhistory -ltermcap -ltms_gpio -L/usr/local/install/lib -lsqlite3 -L/usr/lib
QMAKE_INCDIR += /usr/local/include  /usr/include
message("build x86")
}
#armv7平台
contains(ARCH,armv7)\
{
CONFIG += debug_and_release
 CONFIG(debug,debug|release) {
     TARGET = targetExe-d
 } else {
     TARGET = targetExe-r
 }
DEFINES *= ARM_BOARD
QMAKE_INCDIR += /usr/4412/install/include
LIBS += -lreadline -lhistory -ltermcap -ltms_gpio -L/usr/4412/install/lib -lsqlite3 -L/usr/lib
message("build armv7 ")
}

QMAKE_CXXFLAGS += -std=c99
QMAKE_CFLAGS += -std=c99
#需要用gdb调试时打开
QMAKE_CXXFLAGS += -g
QMAKE_CFLAGS += -g
#定义wchar 为两个字节，屏蔽
#QMAKE_CXXFLAGS += -fshort-wchar
#QMAKE_CFLAGS += -fshort-wchar
#--------------------------------------------------
#
#平台相关选项配置-----end
#
#--------------------------------------------------


SOURCES += main.cpp\
        mainwindow.cpp \
    dlgotdr.cpp \
    global.cpp \
    dlgmcu.cpp \
    qcurv.cpp \
    qmanagelinkedbuf.cpp \
    qmanageringbuf.cpp \
    tsk_otdrmanage.cpp \
    tsk_datadispatch.cpp \
    dlgopm.cpp \
    dlgolp.cpp \
    dlggsm.cpp \
    dlgosw.cpp \
    src/tms_app.c \
    src/ep_app.c \
    shell/cmd/cmd_tmsxx.c \
    shell/cmd/cmd_server.c \
    osnet/ossocket.c \
    osnet/epollserver.c \
    osnet/bipbuffer.c \
    protocol/tmsxx.c \
    protocol/glink.c \
    shell/minishell_core.c \
    src/tmsxxdb.c \
    tsk_host_commu.cpp \
    tsk_waitotdrcurv.cpp \
    tsk_sockretrysend.cpp \
    tsk_sms_send.cpp \
    cserialport.cpp \
    sms_protocol.cpp \
    program_run_log.c

HEADERS  += mainwindow.h \
    constant.h \
    dlgotdr.h \
    struct.h \
    globalExt.h \
    dlgmcu.h \
    qcurv.h \
    qmanagelinkedbuf.h \
    qmanageringbuf.h \
    tsk_otdrmanage.h \
    tsk_datadispatch.h \
    dlgopm.h \
    dlgolp.h \
    dlggsm.h \
    dlgosw.h \
    src/tmsxxdb.h \
    tsk_host_commu.h \
    tsk_waitotdrcurv.h \
    protocol/tmsxx.h \
    tsk_sockretrysend.h \
    tsk_sms_send.h \
    cserialport.h \
    sms_protocol.h \
    src/tms_gpio.h \
    src/libgpio.h \
    src/autoconfig.h \
    program_run_log.h

FORMS    += mainwindow.ui \
    dlgotdr.ui \
    dlgmcu.ui \
    dlgopm.ui \
    dlgolp.ui \
    dlggsm.ui \
    dlgosw.ui

RESOURCES += \
    res.qrc
QMAKE_INCDIR += ./include \
            ./osnet \
            ./shell \
            ./protocol \
            ./src  \
            ../debug \
            /usr/local/include





