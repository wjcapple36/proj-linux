/****************************************************************************
** Meta object code from reading C++ file 'dlgopm.h'
**
** Created: Thu Mar 31 11:48:30 2016
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../OtdrManager/dlgopm.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dlgopm.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_dlgOpm[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x0a,
      23,    7,    7,    7, 0x08,
      57,    7,    7,    7, 0x08,
      82,    7,    7,    7, 0x08,
     109,    7,    7,    7, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_dlgOpm[] = {
    "dlgOpm\0\0usr_time_out()\0"
    "on_pushBtnGetCardVerson_clicked()\0"
    "on_pushBtnExit_clicked()\0"
    "on_pushBbtnGetPw_clicked()\0"
    "on_pushBtnStopGet_clicked()\0"
};

const QMetaObject dlgOpm::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_dlgOpm,
      qt_meta_data_dlgOpm, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &dlgOpm::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *dlgOpm::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *dlgOpm::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_dlgOpm))
        return static_cast<void*>(const_cast< dlgOpm*>(this));
    return QDialog::qt_metacast(_clname);
}

int dlgOpm::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: usr_time_out(); break;
        case 1: on_pushBtnGetCardVerson_clicked(); break;
        case 2: on_pushBtnExit_clicked(); break;
        case 3: on_pushBbtnGetPw_clicked(); break;
        case 4: on_pushBtnStopGet_clicked(); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
