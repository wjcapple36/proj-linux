/****************************************************************************
** Meta object code from reading C++ file 'dlgotdr.h'
**
** Created: Thu Mar 31 11:48:21 2016
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../OtdrManager/dlgotdr.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dlgotdr.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QDlgOtdr[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      22,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      32,   10,    9,    9, 0x08,
      94,   72,    9,    9, 0x08,
     120,    9,    9,    9, 0x08,
     135,    9,    9,    9, 0x08,
     154,    9,    9,    9, 0x08,
     179,    9,    9,    9, 0x08,
     204,    9,    9,    9, 0x08,
     230,    9,    9,    9, 0x08,
     255,    9,    9,    9, 0x08,
     281,    9,    9,    9, 0x08,
     309,    9,    9,    9, 0x08,
     335,    9,    9,    9, 0x08,
     361,    9,    9,    9, 0x08,
     394,  388,    9,    9, 0x08,
     433,  428,    9,    9, 0x08,
     479,  428,    9,    9, 0x08,
     524,  428,    9,    9, 0x08,
     568,    9,    9,    9, 0x08,
     597,    9,    9,    9, 0x08,
     626,    9,    9,    9, 0x08,
     659,    9,    9,    9, 0x08,
     688,  428,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QDlgOtdr[] = {
    "QDlgOtdr\0\0cmd,dataLen,pListHead\0"
    "rcvDataFromParent(int,int,_tagBufList*)\0"
    "cmd,reservd1,reservd2\0rcvParentMsg(int,int,int)\0"
    "usr_time_out()\0set_measur_lamda()\0"
    "on_pushBtnExit_clicked()\0"
    "on_pushBtnHZin_clicked()\0"
    "on_pushBtnHZout_clicked()\0"
    "on_pushBtnVZin_clicked()\0"
    "on_pushBtnVZout_clicked()\0"
    "on_pushBtnResotre_clicked()\0"
    "on_pushBtnLockA_clicked()\0"
    "on_pushBtnLockB_clicked()\0"
    "on_pushBtnMeasur_clicked()\0index\0"
    "on_listEvent_pressed(QModelIndex)\0"
    "arg1\0on_cobBoxFrameNo_currentIndexChanged(QString)\0"
    "on_cobBoxSlotNo_currentIndexChanged(QString)\0"
    "on_cobBoxRange_currentIndexChanged(QString)\0"
    "on_pushBtnGetSInfo_clicked()\0"
    "on_pushBtnGetHInfo_clicked()\0"
    "on_pushBtnGetModulPara_clicked()\0"
    "on_pushBtnGetConct_clicked()\0"
    "on_cobBoxPortNo_currentIndexChanged(QString)\0"
};

const QMetaObject QDlgOtdr::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_QDlgOtdr,
      qt_meta_data_QDlgOtdr, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QDlgOtdr::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QDlgOtdr::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QDlgOtdr::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QDlgOtdr))
        return static_cast<void*>(const_cast< QDlgOtdr*>(this));
    return QDialog::qt_metacast(_clname);
}

int QDlgOtdr::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: rcvDataFromParent((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< _tagBufList*(*)>(_a[3]))); break;
        case 1: rcvParentMsg((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: usr_time_out(); break;
        case 3: set_measur_lamda(); break;
        case 4: on_pushBtnExit_clicked(); break;
        case 5: on_pushBtnHZin_clicked(); break;
        case 6: on_pushBtnHZout_clicked(); break;
        case 7: on_pushBtnVZin_clicked(); break;
        case 8: on_pushBtnVZout_clicked(); break;
        case 9: on_pushBtnResotre_clicked(); break;
        case 10: on_pushBtnLockA_clicked(); break;
        case 11: on_pushBtnLockB_clicked(); break;
        case 12: on_pushBtnMeasur_clicked(); break;
        case 13: on_listEvent_pressed((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 14: on_cobBoxFrameNo_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 15: on_cobBoxSlotNo_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 16: on_cobBoxRange_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 17: on_pushBtnGetSInfo_clicked(); break;
        case 18: on_pushBtnGetHInfo_clicked(); break;
        case 19: on_pushBtnGetModulPara_clicked(); break;
        case 20: on_pushBtnGetConct_clicked(); break;
        case 21: on_cobBoxPortNo_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 22;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
