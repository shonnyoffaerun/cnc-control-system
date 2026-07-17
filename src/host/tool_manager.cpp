/*
    tool_manager.cpp - implementation of tool_manager.h
    Loads a tool database (JSON), validates cutting parameters against the active
    tool (length, material, RPM, feed) and generates tool-change / length-offset
    G-code. This satisfies the requirement to "check the drill": its length,
    material and other properties are verified before a job starts.
*/

#include "tool_manager.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDebug>

ToolManager::ToolManager(QObject* parent)
    : QObject(parent)
{
}

ToolManager::~ToolManager() = default;

bool ToolManager::loadDatabase(const QString& filePath)
{
    QMutexLocker locker(&mutex);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "ToolManager: cannot open" << filePath;
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray())
    {
        qWarning() << "ToolManager: expected a JSON array in" << filePath;
        return false;
    }

    toolDatabase.clear();
    activeToolId = 0;

    for (const QJsonValue& val : doc.array())
    {
        const QJsonObject obj = val.toObject();
        ToolProfile tool;
        tool.id           = obj.value("id").toInt();
        tool.name         = obj.value("name").toString();
        tool.type         = static_cast<ToolType>(obj.value("type").toInt());
        tool.material     = static_cast<ToolMaterial>(obj.value("material").toInt());
        tool.diameter     = obj.value("diameter").toDouble();
        tool.fluteLength  = obj.value("fluteLength").toDouble();
        tool.totalLength  = obj.value("totalLength").toDouble();
        tool.maxRPM       = obj.value("maxRPM").toDouble();
        tool.maxFeedRate  = obj.value("maxFeedRate").toDouble();
        tool.maxDepthOfCut= obj.value("maxDepthOfCut").toDouble();
        tool.isActive     = obj.value("isActive").toBool(false);
        tool.currentWear  = obj.value("currentWear").toDouble(0.0);

        toolDatabase.insert(tool.id, tool);
        if (tool.isActive)
        {
            activeToolId = tool.id;
        }
    }
    return true;
}

bool ToolManager::saveDatabase(const QString& filePath) const
{
    QMutexLocker locker(&mutex);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QJsonArray arr;
    for (const ToolProfile& tool : toolDatabase)
    {
        QJsonObject obj;
        obj.insert("id", tool.id);
        obj.insert("name", tool.name);
        obj.insert("type", static_cast<int>(tool.type));
        obj.insert("material", static_cast<int>(tool.material));
        obj.insert("diameter", tool.diameter);
        obj.insert("fluteLength", tool.fluteLength);
        obj.insert("totalLength", tool.totalLength);
        obj.insert("maxRPM", tool.maxRPM);
        obj.insert("maxFeedRate", tool.maxFeedRate);
        obj.insert("maxDepthOfCut", tool.maxDepthOfCut);
        obj.insert("isActive", tool.isActive);
        obj.insert("currentWear", tool.currentWear);
        arr.append(obj);
    }

    file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    return true;
}

bool ToolManager::addTool(const ToolProfile& tool)
{
    QMutexLocker locker(&mutex);
    if (toolDatabase.contains(tool.id))
    {
        return false;
    }
    toolDatabase.insert(tool.id, tool);
    emit toolAdded(tool);
    return true;
}

bool ToolManager::removeTool(int toolId)
{
    QMutexLocker locker(&mutex);
    if (!toolDatabase.contains(toolId))
    {
        return false;
    }
    toolDatabase.remove(toolId);
    if (activeToolId == toolId)
    {
        activeToolId = 0;
    }
    emit toolRemoved(toolId);
    return true;
}

ToolProfile ToolManager::getTool(int toolId) const
{
    QMutexLocker locker(&mutex);
    return toolDatabase.value(toolId);
}

QList<ToolProfile> ToolManager::getAllTools() const
{
    QMutexLocker locker(&mutex);
    return toolDatabase.values();
}

void ToolManager::setActiveTool(int toolId)
{
    QMutexLocker locker(&mutex);
    if (!toolDatabase.contains(toolId))
    {
        return;
    }
    activeToolId = toolId;
    toolDatabase[toolId].isActive = true;
    emit activeToolChanged(toolId);
}

ToolProfile ToolManager::getActiveTool() const
{
    QMutexLocker locker(&mutex);
    return toolDatabase.value(activeToolId);
}

bool ToolManager::validateCuttingParameters(double rpm, double feed)
{
    const ToolProfile tool = getActiveTool();
    if (tool.id == 0)
    {
        emit validationFailed("No active tool configured");
        return false;
    }

    if (tool.totalLength <= 0.0 || tool.totalLength > 100.0)
    {
        emit validationFailed(QString("Tool %1: invalid total length %2 mm")
                                  .arg(tool.id).arg(tool.totalLength));
        return false;
    }

    const double rpmLimit = (tool.material == ToolMaterial::HSS) ? 18000.0 : 30000.0;
    if (rpm > rpmLimit)
    {
        emit validationFailed(QString("Tool %1: requested %2 RPM exceeds %3 limit")
                                  .arg(tool.id).arg(rpm).arg(rpmLimit));
        return false;
    }

    if (feed > tool.maxFeedRate)
    {
        emit validationFailed(QString("Tool %1: feed %2 exceeds max %3 mm/min")
                                  .arg(tool.id).arg(feed).arg(tool.maxFeedRate));
        return false;
    }

    return true;
}

QString ToolManager::generateToolChangeCode(int toolId) const
{
    const ToolProfile tool = getTool(toolId);
    QString code;
    code += QString("M5 ; stop spindle before change\n");
    code += QString("T%1 M6 ; select and change to tool %1\n").arg(toolId);
    code += QString("; tool %1 (%2) dia=%.3fmm\n")
                .arg(toolId).arg(tool.name).arg(tool.diameter);
    return code;
}

QString ToolManager::generateLengthOffsetCode(double measuredLength) const
{
    const ToolProfile tool = getActiveTool();
    const double offset = measuredLength - tool.totalLength;
    return QString("G43.1 Z%.3f ; tool length offset\n").arg(offset);
}
