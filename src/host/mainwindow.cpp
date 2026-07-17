/*
    mainwindow.cpp - implementation of mainwindow.h
*/

#include "mainwindow.h"

#include "serial_bridge.h"
#include "tool_manager.h"
#include "cad_parser.h"
#include "visualizer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>

MainWindow::MainWindow(ToolManager* toolManager,
                       SerialBridge* serialBridge,
                       CadParser* cadParser,
                       Visualizer* visualizer,
                       QWidget* parent)
    : QMainWindow(parent)
    , m_toolManager(toolManager)
    , m_serialBridge(serialBridge)
    , m_cadParser(cadParser)
    , m_visualizer(visualizer)
{
    setWindowTitle("CNC Control System - MKS DLC32");
    resize(1000, 640);

    auto* central = new QWidget(this);
    auto* root = new QHBoxLayout(central);

    // ---- Left control column ----
    auto* left = new QVBoxLayout;

    m_connectBtn = new QPushButton("Connect");
    m_homeBtn = new QPushButton("Home ($H)");
    m_estopBtn = new QPushButton("E-STOP");
    m_loadBtn = new QPushButton("Load Drawing");

    auto* serialBox = new QGroupBox("Serial");
    auto* serialLayout = new QVBoxLayout(serialBox);
    serialLayout->addWidget(m_connectBtn);
    serialLayout->addWidget(m_homeBtn);
    serialLayout->addWidget(m_estopBtn);

    m_gcodeEdit = new QLineEdit("G0 X10 Y10 Z5");
    m_sendBtn = new QPushButton("Send G-code");
    auto* gcodeBox = new QGroupBox("Manual G-code");
    auto* gcodeLayout = new QVBoxLayout(gcodeBox);
    gcodeLayout->addWidget(m_gcodeEdit);
    gcodeLayout->addWidget(m_sendBtn);

    auto* toolBox = new QGroupBox("Tools");
    auto* toolLayout = new QVBoxLayout(toolBox);
    m_toolList = new QListWidget;
    toolLayout->addWidget(m_toolList);

    left->addWidget(serialBox);
    left->addWidget(gcodeBox);
    left->addWidget(toolBox);
    left->addWidget(m_loadBtn);

    auto* right = new QVBoxLayout;
    m_statusLabel = new QLabel("Disconnected");
    m_posLabel = new QLabel("X: 0.00  Y: 0.00  Z: 0.00");
    if (m_visualizer)
    {
        m_visualizer->setParent(this);
        m_visualizer->setMinimumSize(600, 480);
        m_visualizer->setMachineBounds(MachineBounds{QVector3D(0, 0, 0), QVector3D(300, 200, 60)});
    }
    right->addWidget(m_statusLabel);
    right->addWidget(m_posLabel);
    if (m_visualizer)
    {
        right->addWidget(m_visualizer);
    }

    root->addLayout(left, 0);
    root->addLayout(right, 1);
    setCentralWidget(central);

    connect(m_connectBtn, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_homeBtn, &QPushButton::clicked, this, &MainWindow::onHomeClicked);
    connect(m_estopBtn, &QPushButton::clicked, this, &MainWindow::onEStopClicked);
    connect(m_loadBtn, &QPushButton::clicked, this, &MainWindow::onLoadDrawingClicked);
    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSendGCodeClicked);
    connect(m_toolList, &QListWidget::currentRowChanged, this, &MainWindow::onToolSelected);

    if (m_serialBridge)
    {
        connect(m_serialBridge, &SerialBridge::positionReceived,
                this, &MainWindow::onPositionReceived);
        connect(m_serialBridge, &SerialBridge::statusChanged,
                this, &MainWindow::onStatusChanged);
        connect(m_serialBridge, &SerialBridge::connectionError,
                this, [](const QString& e) { qWarning() << "Serial error:" << e; });
    }

    refreshToolList();
}

void MainWindow::onConnectClicked()
{
    if (!m_serialBridge)
    {
        return;
    }
    if (m_serialBridge->isConnected())
    {
        m_serialBridge->disconnectFromPort();
        m_statusLabel->setText("Disconnected");
    }
    else
    {
        m_serialBridge->connectToPort(m_serialBridge->currentPort().isEmpty()
                                          ? "/dev/ttyACM0"
                                          : m_serialBridge->currentPort());
        m_statusLabel->setText("Connecting...");
        m_serialBridge->queryStatus();
    }
}

void MainWindow::onSendGCodeClicked()
{
    if (m_serialBridge)
    {
        m_serialBridge->queueGCode(m_gcodeEdit->text());
    }
}

void MainWindow::onHomeClicked()
{
    if (m_serialBridge)
    {
        m_serialBridge->home();
    }
}

void MainWindow::onEStopClicked()
{
    if (m_serialBridge)
    {
        m_serialBridge->emergencyStop();
    }
}

void MainWindow::onLoadDrawingClicked()
{
    if (!m_cadParser || !m_toolManager)
    {
        return;
    }
    const QString path = QFileDialog::getOpenFileName(
        this, "Open drawing", QString(), m_cadParser->getSupportedFilters());
    if (path.isEmpty())
    {
        return;
    }

    ParsingConfig cfg;
    const ParseResult res = m_cadParser->parseFile(QFileInfo(path), m_toolManager->getActiveTool(), cfg);
    if (!res.isSuccess)
    {
        QMessageBox::warning(this, "Parse error", res.error);
        return;
    }

    if (m_visualizer)
    {
        m_visualizer->loadToolPath(res.toolpath);
    }

    for (const PathSegment& seg : res.toolpath)
    {
        if (m_serialBridge)
        {
            const QString line = QString("G1 X%1 Y%2 Z%3")
                .arg(seg.end.x(), 0, 'f', 3)
                .arg(seg.end.y(), 0, 'f', 3)
                .arg(seg.end.z(), 0, 'f', 3);
            m_serialBridge->queueGCode(line);
        }
    }
}

void MainWindow::onToolSelected(int row)
{
    Q_UNUSED(row);
    if (!m_toolManager)
    {
        return;
    }
    const QList<ToolProfile> tools = m_toolManager->getAllTools();
    if (row >= 0 && row < tools.size())
    {
        m_toolManager->setActiveTool(tools[row].id);
    }
}

void MainWindow::onPositionReceived(double x, double y, double z)
{
    m_posLabel->setText(QString("X: %1  Y: %2  Z: %3").arg(x, 0, 'f', 2).arg(y, 0, 'f', 2).arg(z, 0, 'f', 2));
    if (m_visualizer)
    {
        m_visualizer->updateToolPosition(QVector3D(x, y, z));
    }
}

void MainWindow::onStatusChanged(const MachineState& state)
{
    m_statusLabel->setText("State: " + state.statusMessage);
    if (m_visualizer)
    {
        m_visualizer->updateSpindleState(state.spindleRPM > 0.0);
    }
}

void MainWindow::onToolAdded(const ToolProfile& tool)
{
    Q_UNUSED(tool);
    refreshToolList();
}

void MainWindow::refreshToolList()
{
    if (!m_toolList || !m_toolManager)
    {
        return;
    }
    m_toolList->clear();
    for (const ToolProfile& tool : m_toolManager->getAllTools())
    {
        m_toolList->addItem(QString("%1: %2 (dia %.2fmm)").arg(tool.id).arg(tool.name).arg(tool.diameter));
    }
}
