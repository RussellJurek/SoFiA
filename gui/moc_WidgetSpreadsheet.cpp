/****************************************************************************
** Meta object code from reading C++ file 'WidgetSpreadsheet.h'
**
** Created: Tue Nov 26 04:36:47 2013
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "WidgetSpreadsheet.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WidgetSpreadsheet.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_WidgetSpreadsheet[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      41,   34,   18,   18, 0x08,
      56,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_WidgetSpreadsheet[] = {
    "WidgetSpreadsheet\0\0widgetClosed()\0"
    "column\0sortTable(int)\0reloadCatalog()\0"
};

const QMetaObject WidgetSpreadsheet::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_WidgetSpreadsheet,
      qt_meta_data_WidgetSpreadsheet, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &WidgetSpreadsheet::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *WidgetSpreadsheet::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *WidgetSpreadsheet::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WidgetSpreadsheet))
        return static_cast<void*>(const_cast< WidgetSpreadsheet*>(this));
    return QWidget::qt_metacast(_clname);
}

int WidgetSpreadsheet::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: widgetClosed(); break;
        case 1: sortTable((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: reloadCatalog(); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void WidgetSpreadsheet::widgetClosed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
