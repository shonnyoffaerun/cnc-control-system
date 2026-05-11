/*
    main.cpp - implementation of main.h QApplication entry point
*/

#include "main.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <iostream>

CNCApplication::CNCApplication(int argc, char *argv[]) : m_argc(argc), m_argv(argv)  
{
    m_config.baudRate = 115200;
    m_config.serialPort = "/dev/ttyACM0";
    m_config.useGui = true;
}

CNCApplication::~CNCApplication()
{
    cleanup();
}

int CNCApplication::exec()
{
    parseCommandLine(m_argc, m_argv);
    loadConfigFile();
    initComponents();
    setupConnections();
    if (m_config.useGui)
    {
        QApplication app(m_argc, m_argv);
        
        m_mainWindow.reset(new MainWindow(m_toolManager.data(), m_serialBridge.data(), m_cadParser.data(), m_visualizer.data()));
        m_mainWindow->show();
        return app.exec();
    }
    else 
    {
        std::cout << "CNC Control System [CLI mode]\n"; 
        std::cout << "Connected to: " << m_config.serialPort.toStdString() << "\n";
        while (true)
        {
            std::cout << ">";
            std::string command;
            std::getline(std::cin, command);
            if (command == "exit")
            {
                break;
            }
        }
        return 0;
    }
}

void CNCApplication::parseCommandLine(int argc, char *argv[])
{
    QCommandLineParser parser;
    parser.setApplicationDescription("CNC Control System for MKS DLC32");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption(QStringList() << "p" << "port", "serial port", "port", "ttyACM0");
    parser.addOption(portOption);

    QCommandLineOption baudOption(QStringList() << "b" << "baud", "baud rate", "baud", "115200");
    parser.addOption(baudOption);

    QCommandLineOption dataOption(QStringList() << "d" << "data", "data bits", "data", "8");
    parser.addOption(dataOption);

    QCommandLineOption parityOption(QStringList() << "P" << "parity", "parity", "parity", "none");
    parser.addOption(parityOption);

    QCommandLineOption stopOption(QStringList() << "s" << "stop", "stop bits", "stop", "1");
    parser.addOption(stopOption);

    parser.process(QCoreApplication::arguments());
    m_config.portName = parser.value(portOption);
    m_config.baudRate = parser.value(baudOption).toInt();
    m_config.dataBits = parser.value(dataOption).toInt();
    m_config.parity = parser.value(parityOption).toInt();
    m_config.stopBits = parser.value(stopOption).toInt();
    m_config.serialPort = parser.value(portOption);
}

void CNCApplication::loadConfigFile()
{
    QFile file("config.json");
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        QJsonDocument document = QJsonDocument::fromJson(data);
        QJsonObject object = document.object();
        m_config.portName = object.value("portName").toString();
        m_config.baudRate = object.value("baudRate").toInt();
        m_config.dataBits = object.value("dataBits").toInt();
        m_config.parity = object.value("parity").toInt();
        m_config.stopBits = object.value("stopBits").toInt();
        m_config.serialPort = object.value("serialPort").toString();
        m_config.useGui = object.value("useGui").toBool();
    }
}

void CNCApplication::initComponents()
{
    m_serialBridge.reset(new SerialBridge(m_config.portName, m_config.baudRate, m_config.dataBits, m_config.parity, m_config.stopBits));
    m_toolManager.reset(new ToolManager(m_serialBridge.data()));
    m_cadParser.reset(new CadParser(m_serialBridge.data()));
    m_visualizer.reset(new Visualizer(m_serialBridge.data()));
}

void CNCApplication::setupConnections()
{
    QObject::connect(m_serialBridge.data(), &SerialBridge::dataReceived, m_toolManager.data(), &ToolManager::dataReceived);
    QObject::connect(m_serialBridge.data(), &SerialBridge::dataReceived, m_cadParser.data(), &CadParser::dataReceived);
    QObject::connect(m_serialBridge.data(), &SerialBridge::dataReceived, m_visualizer.data(), &Visualizer::dataReceived);
}

void CNCApplication::cleanup()
{
    m_serialBridge.reset();
    m_toolManager.reset();
    m_cadParser.reset();
    m_visualizer.reset();
}

int main(int argc, char *argv[])
{
    #ifdef Q_OS_WIN
        SetConsoleOutputCP(CP_UTF8);
    #endif

    CNCApplication app(argc, argv);
    return app.exec();
}
