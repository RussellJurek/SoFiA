/****************************************************************************
** Meta object code from reading C++ file 'SoFiA.h'
**
** Created: Tue Nov 26 04:36:48 2013
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "SoFiA.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SoFiA.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SoFiA[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      23,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       7,    6,    6,    6, 0x08,
      29,    6,    6,    6, 0x08,
      44,    6,    6,    6, 0x08,
      59,    6,    6,    6, 0x08,
      76,    6,    6,    6, 0x08,
      88,    6,    6,    6, 0x08,
      99,    6,    6,    6, 0x08,
     121,  116,    6,    6, 0x08,
     143,    6,    6,    6, 0x28,
     158,    6,    6,    6, 0x08,
     171,    6,    6,    6, 0x08,
     196,    6,    6,    6, 0x08,
     218,    6,    6,    6, 0x08,
     235,    6,    6,    6, 0x08,
     252,    6,    6,    6, 0x08,
     267,    6,    6,    6, 0x08,
     281,    6,    6,    6, 0x08,
     306,    6,    6,    6, 0x08,
     331,    6,    6,    6, 0x08,
     376,  356,    6,    6, 0x08,
     426,    6,    6,    6, 0x08,
     456,  450,    6,    6, 0x08,
     501,    6,    6,    6, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SoFiA[] = {
    "SoFiA\0\0selectInputDataFile()\0"
    "loadSettings()\0saveSettings()\0"
    "saveSettingsAs()\0saveLogAs()\0clearLog()\0"
    "resetToDefault()\0page\0showHandbook(QString)\0"
    "showHandbook()\0aboutSoFiA()\0"
    "selectInputWeightsFile()\0selectInputMaskFile()\0"
    "displayPrevTab()\0displayNextTab()\0"
    "updateFields()\0runPipeline()\0"
    "pipelineProcessReadStd()\0"
    "pipelineProcessReadErr()\0"
    "pipelineProcessStarted()\0exitCode,exitStatus\0"
    "pipelineProcessFinished(int,QProcess::ExitStatus)\0"
    "pipelineProcessCancel()\0error\0"
    "pipelineProcessError(QProcess::ProcessError)\0"
    "showCatalogue()\0"
};

const QMetaObject SoFiA::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_SoFiA,
      qt_meta_data_SoFiA, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SoFiA::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SoFiA::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SoFiA::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SoFiA))
        return static_cast<void*>(const_cast< SoFiA*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int SoFiA::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: selectInputDataFile(); break;
        case 1: loadSettings(); break;
        case 2: saveSettings(); break;
        case 3: saveSettingsAs(); break;
        case 4: saveLogAs(); break;
        case 5: clearLog(); break;
        case 6: resetToDefault(); break;
        case 7: showHandbook((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: showHandbook(); break;
        case 9: aboutSoFiA(); break;
        case 10: selectInputWeightsFile(); break;
        case 11: selectInputMaskFile(); break;
        case 12: displayPrevTab(); break;
        case 13: displayNextTab(); break;
        case 14: updateFields(); break;
        case 15: runPipeline(); break;
        case 16: pipelineProcessReadStd(); break;
        case 17: pipelineProcessReadErr(); break;
        case 18: pipelineProcessStarted(); break;
        case 19: pipelineProcessFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 20: pipelineProcessCancel(); break;
        case 21: pipelineProcessError((*reinterpret_cast< QProcess::ProcessError(*)>(_a[1]))); break;
        case 22: showCatalogue(); break;
        default: ;
        }
        _id -= 23;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
