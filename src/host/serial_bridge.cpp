/*
    serial_bridge.cpp - implementation of serial_bridge.h
    Host-side UART bridge to the MKS DLC32 running grbl_esp32.
    Two layers of communication are supported:
      - textual grbl-style commands ("G0 X10", "?", "$H" ...)
      - binary ProtocolFrame packets (see protocol.h)
*/

#include "serial_bridge.h"

#include "protocol.h"

#include <QSerialPortInfo>
#include <QtEndian>
#include <QRegularExpression>
#include <QDebug>

SerialBridge::SerialBridge(QObject *parent)
    : QObject(parent)
    , m_serial_port(new QSerialPort(this))
    , m_connection_timer(new QTimer(this))
    , m_command_timer(new QTimer(this))
    , m_queue_processor(new QTimer(this))
    , m_baud_rate(DefaultBaudRate)
    , m_command_timeout(DefaultTimeout)
    , m_max_queue_size(DefaultMaxQueueSize)
    , m_auto_reconnect(false)
    , m_is_connected(false)
    , m_waiting_for_responce(false)
    , m_total_commands_sent(0)
    , m_current_command_id(0)
{
    setupTimers();
}

SerialBridge::SerialBridge(const QString &portName, int baudRate, int dataBits,
                           int parity, int stopBits, QObject *parent)
    : SerialBridge(parent)
{
    openSerialPort(portName, baudRate, dataBits, parity, stopBits);
}

SerialBridge::~SerialBridge()
{
    closeSerialPort();
}

void SerialBridge::setupTimers()
{
    m_connection_timer->setInterval(2000);
    m_command_timer->setSingleShot(true);
    m_queue_processor->setInterval(QueueProcessInterval);

    connect(m_connection_timer, &QTimer::timeout, this, &SerialBridge::onConnectionTimeout);
    connect(m_command_timer, &QTimer::timeout, this, &SerialBridge::onCommandTimeout);
    connect(m_queue_processor, &QTimer::timeout, this, &SerialBridge::processCommandQueue);

    connect(m_serial_port, &QSerialPort::readyRead, this, [this]() {
        onSignalDataReceived(m_serial_port->readAll());
    });
    connect(m_serial_port, &QSerialPort::errorOccurred, this, &SerialBridge::onSerialPortError);
}

bool SerialBridge::connectToPort(const QString &portName, int baudRate)
{
    return openSerialPort(portName, baudRate, 8, 0, 1);
}

void SerialBridge::disconnectFromPort()
{
    closeSerialPort();
}

bool SerialBridge::isConnected() const
{
    return m_is_connected;
}

QString SerialBridge::currentPort() const
{
    return m_port_name;
}

int SerialBridge::currentPortName() const
{
    return m_baud_rate;
}

void SerialBridge::queueGCode(const QString &gcode)
{
    Command cmd;
    cmd.command = CommandType::GCode;
    cmd.data = gcode.trimmed();
    cmd.id = ++m_current_command_id;
    cmd.isHighPriority = false;
    addToQueue(cmd);
}

void SerialBridge::sendToolConfig(int toolID, double length, double diameter, int material)
{
    ProtocolFrame frame{};
    frame.header.magic  = PROTOCOL_MAGIC;
    frame.header.type   = static_cast<uint8_t>(CmdType::TOOL_CONFIG);
    frame.header.seq_id = protocol_next_seq(m_pending_command.id);

    ToolConfigPayload payload{};
    payload.tool_id        = static_cast<uint8_t>(toolID);
    payload.length_offset  = static_cast<float>(length);
    payload.diameter       = static_cast<float>(diameter);
    payload.max_rpm        = 0;
    payload.max_feed_rate  = 0;
    payload.material_type  = static_cast<uint8_t>(material);

    std::memcpy(frame.payload, &payload, sizeof(payload));
    frame.header.length = static_cast<uint8_t>(sizeof(payload));

    uint8_t buffer[PROTOCOL_MAX_PACKET_SIZE];
    const size_t written = protocol_serialize(frame, buffer, sizeof(buffer));
    if (written > 0)
    {
        sendRawData(QByteArray(reinterpret_cast<const char*>(buffer), static_cast<int>(written)));
    }
}

void SerialBridge::queryPosition()
{
    Command cmd;
    cmd.command = CommandType::QueryPosition;
    cmd.data = "?";
    cmd.id = ++m_current_command_id;
    cmd.isHighPriority = true;
    addToQueue(cmd);
}

void SerialBridge::queryStatus()
{
    Command cmd;
    cmd.command = CommandType::QueryStatus;
    cmd.data = "?";
    cmd.id = ++m_current_command_id;
    cmd.isHighPriority = true;
    addToQueue(cmd);
}

void SerialBridge::emergencyStop()
{
    Command cmd;
    cmd.command = CommandType::EmergencyStop;
    cmd.data = "\x18";
    cmd.id = ++m_current_command_id;
    cmd.isHighPriority = true;
    addToQueue(cmd);
}

void SerialBridge::home()
{
    Command cmd;
    cmd.command = CommandType::Home;
    cmd.data = "$H";
    cmd.id = ++m_current_command_id;
    cmd.isHighPriority = true;
    addToQueue(cmd);
}

void SerialBridge::reset()
{
    Command cmd;
    cmd.command = CommandType::Reset;
    cmd.data = "\x18";
    cmd.id = ++m_current_command_id;
    cmd.isHighPriority = true;
    addToQueue(cmd);
}

void SerialBridge::setAutoReconnect(bool enabled)
{
    m_auto_reconnect = enabled;
    if (enabled)
    {
        if (!m_connection_timer->isActive())
        {
            m_connection_timer->start();
        }
    }
    else
    {
        m_connection_timer->stop();
    }
}

void SerialBridge::setCommandTimeout(int ms)
{
    m_command_timeout = ms;
}

void SerialBridge::setMaxQuerySize(int size)
{
    m_max_queue_size = size;
}

int SerialBridge::pendingCommands() const
{
    QMutexLocker locker(&m_queue_mutex);
    return m_command_queue.size() + m_priority_queue.size();
}

int SerialBridge::totalCommands() const
{
    return m_total_commands_sent;
}

MachineState SerialBridge::currentState() const
{
    QMutexLocker locker(&m_state_mutex);
    return m_current_state;
}

void SerialBridge::onSignalDataReceived(const QByteArray &data)
{
    m_receive_buffer.append(data);
    parseResponse(m_receive_buffer);
}

void SerialBridge::onSerialPortError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
    {
        return;
    }
    const QString msg = m_serial_port->errorString();
    emit connectionError(msg);
    if (m_auto_reconnect)
    {
        closeSerialPort();
    }
    else
    {
        closeSerialPort();
    }
}

void SerialBridge::onConnectionTimeout()
{
    if (!m_is_connected && !m_port_name.isEmpty())
    {
        openSerialPort(m_port_name, m_baud_rate, 8, 0, 1);
    }
}

void SerialBridge::onCommandTimeout()
{
    if (m_waiting_for_responce)
    {
        emit commandError(m_pending_command.id, "Command timeout");
        m_waiting_for_responce = false;
        processCommandQueue();
    }
}

void SerialBridge::processCommandQueue()
{
    if (m_waiting_for_responce || !m_is_connected)
    {
        return;
    }

    Command next;
    {
        QMutexLocker locker(&m_queue_mutex);
        if (!m_priority_queue.isEmpty())
        {
            next = m_priority_queue.dequeue();
        }
        else if (!m_command_queue.isEmpty())
        {
            next = m_command_queue.dequeue();
        }
        else
        {
            return;
        }
    }

    m_pending_command = next;
    m_waiting_for_responce = true;
    m_command_timer->start(m_command_timeout);
    sendCommand(next.data);
    emit commandSent(next.id, next.data);
}

bool SerialBridge::openSerialPort(const QString &portName, int baudRate,
                                  int dataBits, int parity, int stopBits)
{
    closeSerialPort();

    m_serial_port->setPortName(portName);
    m_serial_port->setBaudRate(baudRate);
    m_serial_port->setDataBits(static_cast<QSerialPort::DataBits>(dataBits));

    switch (parity)
    {
        case 0:  m_serial_port->setParity(QSerialPort::NoParity); break;
        case 1:  m_serial_port->setParity(QSerialPort::OddParity); break;
        case 2:  m_serial_port->setParity(QSerialPort::EvenParity); break;
        default: m_serial_port->setParity(QSerialPort::NoParity); break;
    }

    m_serial_port->setStopBits(stopBits == 2 ? QSerialPort::TwoStop : QSerialPort::OneStop);
    m_serial_port->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial_port->open(QIODevice::ReadWrite))
    {
        emit connectionError(m_serial_port->errorString());
        return false;
    }

    m_port_name = portName;
    m_baud_rate = baudRate;
    m_is_connected = true;
    m_queue_processor->start();
    emit connected(portName);
    return true;
}

void SerialBridge::closeSerialPort()
{
    m_queue_processor->stop();
    m_command_timer->stop();
    if (m_serial_port->isOpen())
    {
        m_serial_port->close();
    }
    m_is_connected = false;
    m_waiting_for_responce = false;
    emit disconnected();
}

void SerialBridge::sendCommand(const QString &data)
{
    if (!m_serial_port->isOpen())
    {
        return;
    }
    m_serial_port->write((data + "\n").toUtf8());
    ++m_total_commands_sent;
}

void SerialBridge::sendRawData(const QByteArray &data)
{
    if (!m_serial_port->isOpen())
    {
        return;
    }
    m_serial_port->write(data);
    ++m_total_commands_sent;
}

void SerialBridge::parseResponse(QByteArray &buffer)
{
    while (true)
    {
        const int nl = buffer.indexOf('\n');
        if (nl < 0)
        {
            break;
        }
        QByteArray line = buffer.left(nl).trimmed();
        buffer.remove(0, nl + 1);

        if (line.isEmpty())
        {
            continue;
        }

        const QByteArray view = line;
        if (view.startsWith('<') && view.endsWith('>'))
        {
            parseStatusResponce(QString::fromUtf8(view));
        }
        else if (view.startsWith("ok") || view.startsWith("error"))
        {
            m_waiting_for_responce = false;
            m_command_timer->stop();
            emit commandReceived(m_pending_command.id, QString::fromUtf8(view));
            processCommandQueue();
        }
    }
}

void SerialBridge::parsePositionResponce(const QString &data)
{
    Q_UNUSED(data);
}

void SerialBridge::parseStatusResponce(const QString &data)
{
    const QRegularExpression stateRe("([A-Za-z]+)");
    QRegularExpressionMatch match = stateRe.match(data);
    if (match.hasMatch())
    {
        const QString state = match.captured(1);
        {
            QMutexLocker locker(&m_state_mutex);
            m_current_state.statusMessage = state;
            m_current_state.isRunning = (state == "Run");
            m_current_state.isHomed   = (state == "Home" || state == "Idle");
        }
    }

    QRegularExpression coordRe("WPos:([-\\d.]+),([-\\d.]+),([-\\d.]+)");
    QRegularExpressionMatch cm = coordRe.match(data);
    if (cm.hasMatch())
    {
        const double x = cm.captured(1).toDouble();
        const double y = cm.captured(2).toDouble();
        const double z = cm.captured(3).toDouble();
        {
            QMutexLocker locker(&m_state_mutex);
            m_current_state.posX = x;
            m_current_state.posY = y;
            m_current_state.posZ = z;
        }
        emit positionReceived(x, y, z);
    }

    emit statusChanged(currentState());
}

void SerialBridge::parseErrorResponce(const QString &data)
{
    Q_UNUSED(data);
    m_waiting_for_responce = false;
    m_command_timer->stop();
    processCommandQueue();
}

void SerialBridge::addToQueue(const Command &command)
{
    QMutexLocker locker(&m_queue_mutex);
    if (command.isHighPriority)
    {
        if (m_priority_queue.size() < m_max_queue_size)
        {
            m_priority_queue.enqueue(command);
        }
    }
    else
    {
        if (m_command_queue.size() < m_max_queue_size)
        {
            m_command_queue.enqueue(command);
        }
    }
    emit queueProgress(m_priority_queue.size() + m_command_queue.size(),
                       m_max_queue_size);
}

void SerialBridge::clearQueue()
{
    QMutexLocker locker(&m_queue_mutex);
    m_command_queue.clear();
    m_priority_queue.clear();
}

QString SerialBridge::generateResponseId(const QString &data)
{
    return QString::number(qHash(data));
}
