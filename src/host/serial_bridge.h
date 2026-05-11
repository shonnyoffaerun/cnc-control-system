#pragma once 

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QQueue>
#include <QTimer>
#include <QMutex>
#include <QSerialPort>
#include <qqueue.h>
#include <qstringview.h>

/*
    struct MachineState - struct for machine state;
    Attributes: double posX - current X position;
                double posY - current Y position;
                double posZ - current Z position;
                double feedRate - current feed rate;
                double spindleRPM - current spindle RPM;
                bool isRunning - is machine running;
                bool isHomed - is machine homed;
                QString statusMessage - status message;
                int activeTool - current active tool;
*/
struct MachineState
{
    double  posX{0.0};
    double  posY{0.0};
    double  posZ{0.0};
    double  feedRate{0.0};
    double  spindleRPM{0.0};
    bool    isRunning{false};
    bool    isHomed{false};
    QString statusMessage;
    int     activeTool{0};
};

/*
    enum class CommandType - enum for command type;
    Attributes: GCode - G-code command;
                ToolConfig - tool configuration command;
                QueryPosition - query position command;
                QueryStatus - query status command;
                EmergencyStop - emergency stop command;
                Home - go to home position command;
                Reset - reset machine command;
*/
enum class CommandType 
{
    GCode,
    ToolConfig,
    QueryPosition,
    QueryStatus,
    EmergencyStop,
    Home,
    Reset
};

/*
    struct Command - command element for next calls;
    Attributes: CommandType command - type of command;
                QString data - command data;
                int id - command id;
                bool isHighPriority - is command high priority;
*/
struct Command
{
    CommandType command;
    QString     data;
    int         id;
    bool        isHighPriority{false};
};

/*
    class SerialBridge - class for serial communication with CNC;
    Attributes: QSerialPort *m_serial_port - pointer to serial port;
                QTimer *m_connection_timer - pointer to connection timer;
                QTimer *m_command_timer - pointer to command timer;
                QTimer *m_queue_processor - pointer to queue processor;
                QQueue<Command> m_command_queue - command queue;
                QQueue<Command> m_priority_queue - priority queue;
                mutable QMutex m_queue_mutex - mutex for queue;
                mutable QMutex m_state_mutex - mutex for state;
                MachineState m_current_state - current machine state;
                QString m_port_name - port name;
                int m_baud_rate - baud rate;
                int m_command_timeout - command timeout;
                int m_max_queue_size - max queue size;
                bool m_auto_reconnect - auto reconnect;
                bool m_is_connected - is connected;
                bool m_waiting_for_responce - is waiting for responce;
                int m_pending_command - pending commands;
                int m_total_commands_sent - total commands;
                Command m_pending_command - pending command;
                QByteArray m_receive_buffer - receive buffer;
                static constexpr int DefaultBaudRate - default baud rate;
                static constexpr int DefaultTimeout - default command timeout;
                static constexpr int DefaultMaxQueueSize - default max queue size;
                static constexpr int QueueProcessInterval - queue process interval;
    Methods:    explicit SerialBridge(QObject *parent = nullptr) - class constructor;
                SerialBridge(const QString &portName, int baudRate = 115200, int dataBits = 8, int parity = 1, int stopBits = 1, QObject *parent = nullptr) - default class constructor;
                ~SerialBridge() override - class destructor;
                bool connectToPort(const QString &portName, int baudRate = 115200) - connect to serial port;
                void disconnectFromPort() - disconnect from serial port;
                bool isConnected() const - check if connected to serial port;
                QString currentPort() const - get current port name;
                int currentPortName() const - get current port name;
                void queueGCode(const QString &gcode) - queue G-code command;
                void sendToolConfig(int toolID, double length, double diameter, int material) - send tool configuration command;
                void queryPosition() - query position command;
                void queryStatus() - query status command;
                void emergencyStop() - emergency stop command;
                void home() - go to home position command;
                void reset() - reset machine command;
                void setAutoReconnect(bool enabled) - set auto reconnect;
                void setCommandTimeout(int ms) - set command timeout;
                void setMaxQuerySize(int size) - set max query size;
                int pendingCommands() const - get pending commands;
                int totalCommands() const - get total commands;
                MachineState currentState() const - get current machine state;

*/
class SerialBridge : public QObject
{
    Q_OBJECT

public:

    explicit             SerialBridge(QObject *parent = nullptr);
                         SerialBridge(const QString &portName, int baudRate = 115200, int dataBits = 8, int parity = 1, int stopBits = 1, QObject *parent = nullptr);
                         ~SerialBridge() override;

    bool                 connectToPort(const QString &portName, int baudRate = 115200);
    void                 disconnectFromPort();
    bool                 isConnected() const;
    QString              currentPort() const;
    int                  currentPortName() const;

    void                 queueGCode(const QString &gcode);
    void                 sendToolConfig(int toolID, double length, double diameter, int material);
    void                 queryPosition();
    void                 queryStatus();
    void                 emergencyStop();
    void                 home();
    void                 reset();

    void                 setAutoReconnect(bool enabled);
    void                 setCommandTimeout(int ms);
    void                 setMaxQuerySize(int size);

    int                  pendingCommands() const;
    int                  totalCommands() const;
    MachineState         currentState() const;

signals: 

    void                 connected(const QString &port);
    void                 disconnected();
    void                 connectionError(const QString &error);

    void                 positionReceived(double x, double y, double z);
    void                 statusChanged(const MachineState &state);
    void                 spindleSpeedChanged(double rpm);
    void                 feedRateChanged(double rate);

    void                 commandSent(int id, const QString &data);
    void                 commandReceived(int id, const QString &data);
    void                 commandError(int id, const QString &error);
    void                 queueProgress(int current, int data);

private slots:

    void                 onSignalDataReceived(const QByteArray &data);
    void                 onSerialPortError(QSerialPort::SerialPortError error);
    void                 onConnectionTimeout();
    void                 onCommandTimeout();
    void                 processCommandQueue();

private:

    bool                 openSerialPort(const QString &portName, int baudRate, int dataBits, int parity, int stopBits);
    void                 closeSerialPort();
    void                 sendCommand(const QString &data);
    void                 sendRawData(const QByteArray &data);
    void                 parseResponse(const QByteArray &data);
    void                 parsePositionResponce(const QString &data);
    void                 parseStatusResponce(const QString &data);
    void                 parseErrorResponce(const QString &data);
    void                 addToQueue(const Command &command);
    void                 clearQueue();
    QString              generateResponseId(const QString &data);

    QSerialPort          *m_serial_port;
    QTimer               *m_connection_timer;
    QTimer               *m_command_timer;
    QTimer               *m_queue_processor;

    QQueue<Command>      m_command_queue;
    QQueue<Command>      m_priority_queue;

    mutable QMutex       m_queue_mutex;
    mutable QMutex       m_state_mutex;

    MachineState         m_current_state;
    QString              m_port_name;
    int                  m_baud_rate;
    int                  m_command_timeout;
    int                  m_max_queue_size;
    bool                 m_auto_reconnect;
    bool                 m_is_connected;
    bool                 m_waiting_for_responce;

    int                  m_total_commands_sent;
    int                  m_current_command_id;
    Command              m_pending_command;

    QByteArray           m_receive_buffer;

    static constexpr int DefaultBaudRate = 115200;
    static constexpr int DefaultTimeout = 5000;
    static constexpr int DefaultMaxQueueSize = 100;
    static constexpr int QueueProcessInterval = 10;

};
