/****************************************************************************
** Meta object code from reading C++ file 'dlgmcu.h'
**
** Created: Thu Dec 17 10:21:57 2015
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../OtdrManager/dlgmcu.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dlgmcu.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QDlgMcu[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x08,
      40,   34,    8,    8, 0x08,
      74,    8,    8,    8, 0x08,
     103,    8,    8,    8, 0x08,
     132,    8,    8,    8, 0x08,
     161,    8,    8,    8, 0x08,
     192,    8,    8,    8, 0x08,
     221,    8,    8,    8, 0x08,
     257,  252,    8,    8, 0x08,
     302,  252,    8,    8, 0x08,
     347,  252,    8,    8, 0x08,
     392,  252,    8,    8, 0x08,
     437,  252,    8,    8, 0x08,
     482,  252,    8,    8, 0x08,
     527,  252,    8,    8, 0x08,
     572,  252,    8,    8, 0x08,
     617,  252,    8,    8, 0x08,
     662,  252,    8,    8, 0x08,
     708,  252,    8,    8, 0x08,
     754,  252,    8,    8, 0x08,
     800,  252,    8,    8, 0x08,
     846,  252,    8,    8, 0x08,
     892,  252,    8,    8, 0x08,
     938,  252,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QDlgMcu[] = {
    "QDlgMcu\0\0on_pushBtnExit_clicked()\0"
    "index\0on_listFrame_clicked(QModelIndex)\0"
    "on_pushBtnAddFrame_clicked()\0"
    "on_pushBtnDelFrame_clicked()\0"
    "on_pushBtnShowFram_clicked()\0"
    "on_pushBtnModifyFram_clicked()\0"
    "on_pushBtnSaveFram_clicked()\0"
    "on_pushBtnViewCurCfg_clicked()\0arg1\0"
    "on_comboxSlot_1_currentIndexChanged(QString)\0"
    "on_comboxSlot_2_currentIndexChanged(QString)\0"
    "on_comboxSlot_3_currentIndexChanged(QString)\0"
    "on_comboxSlot_4_currentIndexChanged(QString)\0"
    "on_comboxSlot_5_currentIndexChanged(QString)\0"
    "on_comboxSlot_6_currentIndexChanged(QString)\0"
    "on_comboxSlot_7_currentIndexChanged(QString)\0"
    "on_comboxSlot_8_currentIndexChanged(QString)\0"
    "on_comboxSlot_9_currentIndexChanged(QString)\0"
    "on_comboxSlot_10_currentIndexChanged(QString)\0"
    "on_comboxSlot_11_currentIndexChanged(QString)\0"
    "on_comboxSlot_12_currentIndexChanged(QString)\0"
    "on_comboxSlot_13_currentIndexChanged(QString)\0"
    "on_comboxSlot_14_currentIndexChanged(QString)\0"
    "on_comboxSlot_15_currentIndexChanged(QString)\0"
    "on_comboxSlot_16_currentIndexChanged(QString)\0"
};

const QMetaObject QDlgMcu::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_QDlgMcu,
      qt_meta_data_QDlgMcu, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QDlgMcu::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QDlgMcu::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QDlgMcu::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QDlgMcu))
        return static_cast<void*>(const_cast< QDlgMcu*>(this));
    return QDialog::qt_metacast(_clname);
}

int QDlgMcu::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_pushBtnExit_clicked(); break;
        case 1: on_listFrame_clicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 2: on_pushBtnAddFrame_clicked(); break;
        case 3: on_pushBtnDelFrame_clicked(); break;
        case 4: on_pushBtnShowFram_clicked(); break;
        case 5: on_pushBtnModifyFram_clicked(); break;
        case 6: on_pushBtnSaveFram_clicked(); break;
        case 7: on_pushBtnViewCurCfg_clicked(); break;
        case 8: on_comboxSlot_1_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: on_comboxSlot_2_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: on_comboxSlot_3_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: on_comboxSlot_4_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: on_comboxSlot_5_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: on_comboxSlot_6_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: on_comboxSlot_7_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 15: on_comboxSlot_8_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 16: on_comboxSlot_9_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 17: on_comboxSlot_10_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 18: on_comboxSlot_11_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 19: on_comboxSlot_12_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 20: on_comboxSlot_13_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 21: on_comboxSlot_14_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 22: on_comboxSlot_15_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 23: on_comboxSlot_16_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 24;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
