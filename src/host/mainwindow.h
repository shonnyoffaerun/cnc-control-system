/*
    mainwindow.h - Qt GUI for the CNC control system.
*/

#pragma once

#include <QMainWindow>
#include <QPointer>

#include "serial_bridge.h"
#include "tool_manager.h"

class CadParser;
class Visualizer;

class QListWidget;
class QPushButton;
class QLabel;
class QLineEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ToolManager* toolManager,
                        SerialBridge* serialBridge,
                        CadParser* cadParser,
                        Visualizer* visualizer,
                        QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onConnectClicked();
    void onLoadDrawingClicked();
    void onSendGCodeClicked();
    void onHomeClicked();
    void onEStopClicked();
    void onToolSelected(int row);
    void onPositionReceived(double x, double y, double z);
    void onStatusChanged(const MachineState& state);
    void onToolAdded(const ToolProfile& tool);

private:
    void refreshToolList();

    QPointer<ToolManager>  m_toolManager;
    QPointer<SerialBridge> m_serialBridge;
    QPointer<CadParser>    m_cadParser;
    QPointer<Visualizer>   m_visualizer;

    QListWidget* m_toolList{nullptr};
    QPushButton* m_connectBtn{nullptr};
    QPushButton* m_loadBtn{nullptr};
    QPushButton* m_sendBtn{nullptr};
    QPushButton* m_homeBtn{nullptr};
    QPushButton* m_estopBtn{nullptr};
    QLineEdit*   m_gcodeEdit{nullptr};
    QLabel*      m_statusLabel{nullptr};
    QLabel*      m_posLabel{nullptr};
};
