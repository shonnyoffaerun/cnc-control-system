/*
    main.h - main header for host application
*/

#pragma once 

#include <QApplication>
#include <QMainWindow>
#include <QScopedPointer>

/*
    SerialBridge - class for serial communication with CNC;
*/
class SerialBridge;

/*
    ToolManager - class for tools management;
*/
class ToolManager;

/*
    CadParser - class for parsing G-code;
*/
class CadParser;

/*
    Visualizer - class for 3D visualization;
*/
class Visualizer;

/*
    MainWindow - QMainWindow application class;
*/
class MainWindow;

/*
    AppConfig - struct for application configuration;

    Attributes: QString portName - name of current port;
                int baudRate - baud rate;
                int dataBits - data bits;
                int parity - parity;
                int stopBits - stop bits;
                QString serialPort - path to serial port;
                bool useGui - use GUI;
*/
struct AppConfig 
{
    QString portName;
    int     baudRate;
    int     dataBits;
    int     parity;
    int     stopBits;
    QString serialPort;
    bool    useGui;
};

/*
    CNCApplication - main application class;

    Attributes: QScopedPointer<MainWindow> m_mainWindow - pointer to main window;
                QScopedPointer<SerialBridge> m_serialBridge - pointer to serial bridge;
                QScopedPointer<ToolManager> m_toolManager - pointer to tool manager;
                QScopedPointer<CadParser> m_cadParser - pointer to G-code parser;
                QScopedPointer<Visualizer> m_visualizer - pointer to 3D visualizer;
    Methods:    CNCApplication(int argc, char *argv[]) - constructor;
                ~CNCApplication() - destructor;
                int exec() - main application loop;
                int m_argc - number of command line arguments;
                char **m_argv - command line arguments;
                AppConfig m_config - application configuration;
                void parseCommandLine(int argc, char *argv[]) - parse command line arguments;
                void loadConfigFile() - load application configuration from file;
                void initComponents() - initialize application components;
                void setupConnections() - setup application connections;
                void cleanup() - cleanup application resources;
*/
class CNCApplication 
{
public:

    explicit                     CNCApplication(int argc, char *argv[]);
                                 ~CNCApplication();

    int                          exec();

private:

    void                         parseCommandLine(int argc, char *argv[]);
    void                         loadConfigFile();
    void                         initComponents();
    void                         setupConnections();
    void                         cleanup();

    int                          m_argc;
    char                         **m_argv;
    AppConfig                    m_config;

    QScopedPointer<MainWindow>   m_mainWindow;
    QScopedPointer<SerialBridge> m_serialBridge;
    QScopedPointer<ToolManager>  m_toolManager;
    QScopedPointer<CadParser>    m_cadParser;
    QScopedPointer<Visualizer>   m_visualizer;

};
