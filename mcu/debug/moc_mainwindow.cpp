/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created: Thu Mar 31 11:48:18 2016
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../OtdrManager/mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      25,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   12,   11,   11, 0x05,
      47,   12,   11,   11, 0x05,
      77,   11,   73,   11, 0x05,
      89,   11,   73,   11, 0x05,
     106,   11,   73,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
     122,   11,   11,   11, 0x0a,
     137,   11,   73,   11, 0x0a,
     152,   11,   73,   11, 0x0a,
     172,   11,   73,   11, 0x0a,
     192,   11,   11,   11, 0x08,
     219,   11,   11,   11, 0x08,
     246,   11,   11,   11, 0x08,
     273,   11,   11,   11, 0x08,
     300,   11,   11,   11, 0x08,
     327,   11,   11,   11, 0x08,
     354,   11,   11,   11, 0x08,
     381,   11,   11,   11, 0x08,
     408,   11,   11,   11, 0x08,
     435,   11,   11,   11, 0x08,
     463,   11,   11,   11, 0x08,
     491,   11,   11,   11, 0x08,
     519,   11,   11,   11, 0x08,
     547,   11,   11,   11, 0x08,
     575,   11,   11,   11, 0x08,
     603,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0,,\0dataToDlg(int,int,_tagBufList*)\0"
    "sendMsgToDlg(int,int,int)\0int\0findAlarm()\0"
    "checkCycleTime()\0updateDevType()\0"
    "countTimeout()\0inspectFiber()\0"
    "insepectCycleTime()\0s_update_dev_type()\0"
    "on_pushBtnCard_1_clicked()\0"
    "on_pushBtnCard_2_clicked()\0"
    "on_pushBtnCard_3_clicked()\0"
    "on_pushBtnCard_4_clicked()\0"
    "on_pushBtnCard_5_clicked()\0"
    "on_pushBtnCard_6_clicked()\0"
    "on_pushBtnCard_7_clicked()\0"
    "on_pushBtnCard_8_clicked()\0"
    "on_pushBtnCard_9_clicked()\0"
    "on_pushBtnCard_10_clicked()\0"
    "on_pushBtnCard_13_clicked()\0"
    "on_pushBtnCard_14_clicked()\0"
    "on_pushBtnCard_15_clicked()\0"
    "on_pushBtnCard_16_clicked()\0"
    "on_pushBtnCard_11_clicked()\0"
    "on_pushBtnCard_12_clicked()\0"
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: dataToDlg((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< _tagBufList*(*)>(_a[3]))); break;
        case 1: sendMsgToDlg((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: { int _r = findAlarm();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 3: { int _r = checkCycleTime();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 4: { int _r = updateDevType();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 5: countTimeout(); break;
        case 6: { int _r = inspectFiber();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 7: { int _r = insepectCycleTime();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 8: { int _r = s_update_dev_type();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 9: on_pushBtnCard_1_clicked(); break;
        case 10: on_pushBtnCard_2_clicked(); break;
        case 11: on_pushBtnCard_3_clicked(); break;
        case 12: on_pushBtnCard_4_clicked(); break;
        case 13: on_pushBtnCard_5_clicked(); break;
        case 14: on_pushBtnCard_6_clicked(); break;
        case 15: on_pushBtnCard_7_clicked(); break;
        case 16: on_pushBtnCard_8_clicked(); break;
        case 17: on_pushBtnCard_9_clicked(); break;
        case 18: on_pushBtnCard_10_clicked(); break;
        case 19: on_pushBtnCard_13_clicked(); break;
        case 20: on_pushBtnCard_14_clicked(); break;
        case 21: on_pushBtnCard_15_clicked(); break;
        case 22: on_pushBtnCard_16_clicked(); break;
        case 23: on_pushBtnCard_11_clicked(); break;
        case 24: on_pushBtnCard_12_clicked(); break;
        default: ;
        }
        _id -= 25;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::dataToDlg(int _t1, int _t2, _tagBufList * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MainWindow::sendMsgToDlg(int _t1, int _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
int MainWindow::findAlarm()
{
    int _t0;
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(&_t0)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
    return _t0;
}

// SIGNAL 3
int MainWindow::checkCycleTime()
{
    int _t0;
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(&_t0)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
    return _t0;
}

// SIGNAL 4
int MainWindow::updateDevType()
{
    int _t0;
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(&_t0)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
    return _t0;
}
QT_END_MOC_NAMESPACE
