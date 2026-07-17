/****************************************************************************
** Meta object code from reading C++ file 'serial_bridge.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/host/serial_bridge.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'serial_bridge.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN12SerialBridgeE_t {};
} // unnamed namespace

template <> constexpr inline auto SerialBridge::qt_create_metaobjectdata<qt_meta_tag_ZN12SerialBridgeE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SerialBridge",
        "connected",
        "",
        "port",
        "disconnected",
        "connectionError",
        "error",
        "positionReceived",
        "x",
        "y",
        "z",
        "statusChanged",
        "MachineState",
        "state",
        "spindleSpeedChanged",
        "rpm",
        "feedRateChanged",
        "rate",
        "commandSent",
        "id",
        "data",
        "commandReceived",
        "commandError",
        "queueProgress",
        "current",
        "onSignalDataReceived",
        "onSerialPortError",
        "QSerialPort::SerialPortError",
        "onConnectionTimeout",
        "onCommandTimeout",
        "processCommandQueue"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'connected'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'disconnected'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'connectionError'
        QtMocHelpers::SignalData<void(const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Signal 'positionReceived'
        QtMocHelpers::SignalData<void(double, double, double)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 8 }, { QMetaType::Double, 9 }, { QMetaType::Double, 10 },
        }}),
        // Signal 'statusChanged'
        QtMocHelpers::SignalData<void(const MachineState &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Signal 'spindleSpeedChanged'
        QtMocHelpers::SignalData<void(double)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 15 },
        }}),
        // Signal 'feedRateChanged'
        QtMocHelpers::SignalData<void(double)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 17 },
        }}),
        // Signal 'commandSent'
        QtMocHelpers::SignalData<void(int, const QString &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::QString, 20 },
        }}),
        // Signal 'commandReceived'
        QtMocHelpers::SignalData<void(int, const QString &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::QString, 20 },
        }}),
        // Signal 'commandError'
        QtMocHelpers::SignalData<void(int, const QString &)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::QString, 6 },
        }}),
        // Signal 'queueProgress'
        QtMocHelpers::SignalData<void(int, int)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 24 }, { QMetaType::Int, 20 },
        }}),
        // Slot 'onSignalDataReceived'
        QtMocHelpers::SlotData<void(const QByteArray &)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QByteArray, 20 },
        }}),
        // Slot 'onSerialPortError'
        QtMocHelpers::SlotData<void(QSerialPort::SerialPortError)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 27, 6 },
        }}),
        // Slot 'onConnectionTimeout'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCommandTimeout'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'processCommandQueue'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SerialBridge, qt_meta_tag_ZN12SerialBridgeE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SerialBridge::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12SerialBridgeE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12SerialBridgeE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12SerialBridgeE_t>.metaTypes,
    nullptr
} };

void SerialBridge::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SerialBridge *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->connected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->disconnected(); break;
        case 2: _t->connectionError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->positionReceived((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3]))); break;
        case 4: _t->statusChanged((*reinterpret_cast<std::add_pointer_t<MachineState>>(_a[1]))); break;
        case 5: _t->spindleSpeedChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 6: _t->feedRateChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 7: _t->commandSent((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 8: _t->commandReceived((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 9: _t->commandError((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 10: _t->queueProgress((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 11: _t->onSignalDataReceived((*reinterpret_cast<std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 12: _t->onSerialPortError((*reinterpret_cast<std::add_pointer_t<QSerialPort::SerialPortError>>(_a[1]))); break;
        case 13: _t->onConnectionTimeout(); break;
        case 14: _t->onCommandTimeout(); break;
        case 15: _t->processCommandQueue(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SerialBridge::*)(const QString & )>(_a, &SerialBridge::connected, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialBridge::*)()>(_a, &SerialBridge::disconnected, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialBridge::*)(const QString & )>(_a, &SerialBridge::connectionError, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialBridge::*)(double , double , double )>(_a, &SerialBridge::positionReceived, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialBridge::*)(const MachineState & )>(_a, &SerialBridge::statusChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialBridge::*)(double )>(_a, &SerialBridge::spindleSpeedChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialBridge::*)(double )>(_a, &SerialBridge::feedRateChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialBridge::*)(int , const QString & )>(_a, &SerialBridge::commandSent, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialBridge::*)(int , const QString & )>(_a, &SerialBridge::commandReceived, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialBridge::*)(int , const QString & )>(_a, &SerialBridge::commandError, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialBridge::*)(int , int )>(_a, &SerialBridge::queueProgress, 10))
            return;
    }
}

const QMetaObject *SerialBridge::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SerialBridge::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12SerialBridgeE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SerialBridge::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void SerialBridge::connected(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void SerialBridge::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SerialBridge::connectionError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void SerialBridge::positionReceived(double _t1, double _t2, double _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3);
}

// SIGNAL 4
void SerialBridge::statusChanged(const MachineState & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void SerialBridge::spindleSpeedChanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void SerialBridge::feedRateChanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void SerialBridge::commandSent(int _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2);
}

// SIGNAL 8
void SerialBridge::commandReceived(int _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1, _t2);
}

// SIGNAL 9
void SerialBridge::commandError(int _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1, _t2);
}

// SIGNAL 10
void SerialBridge::queueProgress(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1, _t2);
}
QT_WARNING_POP
