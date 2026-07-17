/*
    cad_parser.cpp - implementation of cad_parser.h
    Converts CAD drawings (DXF / SVG / STEP) into G-code-style toolpaths and also
    reads existing G-code files. The output is a list of PathSegments consumed by
    the Visualizer and forwarded to the MKS DLC32 as ProtocolFrame GCODE commands.
*/

#include "cad_parser.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <cmath>

CadParser::CadParser(QObject* parent)
    : QObject(parent)
{
}

CadParser::~CadParser() = default;

bool CadParser::isSupportedFormat(const QString& fileInfo) const
{
    const QString lower = fileInfo.toLower();
    return lower.endsWith(".dxf") || lower.endsWith(".svg")
        || lower.endsWith(".step") || lower.endsWith(".stp")
        || lower.endsWith(".gcode") || lower.endsWith(".nc");
}

QString CadParser::getSupportedFilters() const
{
    return "CAD files (*.dxf *.svg *.step *.stp);;G-code (*.gcode *.nc);;All (*.*)";
}

ParseResult CadParser::parseFile(const QFileInfo& fileInfo,
                                 const ToolProfile& toolProfile,
                                 const ParsingConfig& config)
{
    emit parsingStarted(fileInfo.fileName());

    ParseResult result;
    const QString suffix = fileInfo.suffix().toLower();

    if (suffix == "dxf")
    {
        result = parseDXF(fileInfo, toolProfile, config);
    }
    else if (suffix == "svg")
    {
        result = parseSVG(fileInfo, toolProfile, config);
    }
    else if (suffix == "step" || suffix == "stp")
    {
        result.error = "STEP parsing requires an external kernel (e.g. OpenCascade); "
                       "not built into this host. Use DXF/SVG/G-code.";
        result.isSuccess = false;
    }
    else if (suffix == "gcode" || suffix == "nc")
    {
        result = parseGCodeFile(fileInfo.filePath());
    }
    else
    {
        result.error = "Unsupported file format: " + suffix;
    }

    emit parsingFinished(result);
    return result;
}

ParseResult CadParser::parseGCodeFile(const QString& filepath)
{
    ParseResult result;
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly))
    {
        result.error = "Cannot open G-code file: " + filepath;
        return result;
    }

    QTextStream in(&file);
    QVector3D pos(0, 0, 0);
    bool absolute = true;
    MoveType lastMove = MoveType::Rapid;

    while (!in.atEnd())
    {
        const QString raw = in.readLine().trimmed();
        if (raw.isEmpty() || raw.startsWith(';') || raw.startsWith('('))
        {
            continue;
        }

        float x = pos.x(), y = pos.y(), z = pos.z();
        bool hasX = false, hasY = false, hasZ = false;
        float val = 0;

        QRegularExpression re("([GXYZF])([-+]?[0-9]*\\.?[0-9]+)");
        QRegularExpressionMatchIterator it = re.globalMatch(raw);
        MoveType move = lastMove;
        while (it.hasNext())
        {
            const QRegularExpressionMatch m = it.next();
            const char axis = m.captured(1).at(0).toLatin1();
            val = m.captured(2).toFloat();
            switch (axis)
            {
                case 'G':
                    if (val == 0.0f) move = MoveType::Rapid;
                    else if (val == 1.0f) move = MoveType::Linear;
                    else if (val == 2.0f || val == 3.0f) move = MoveType::Arc;
                    else if (val == 90.0f) absolute = true;
                    else if (val == 91.0f) absolute = false;
                    break;
                case 'X': x = val; hasX = true; break;
                case 'Y': y = val; hasY = true; break;
                case 'Z': z = val; hasZ = true; break;
                default: break;
            }
        }

        const QVector3D next(
            hasX ? (absolute ? x : pos.x() + x) : pos.x(),
            hasY ? (absolute ? y : pos.y() + y) : pos.y(),
            hasZ ? (absolute ? z : pos.z() + z) : pos.z());

        if (next != pos)
        {
            PathSegment seg;
            seg.start = pos;
            seg.end = next;
            seg.type = move;
            result.toolpath.push_back(seg);
        }
        pos = next;
        lastMove = move;
    }

    result.totalPasses = 1;
    result.estimatedTime = calculateEstimatedTime(result.toolpath, 500.0);
    result.isSuccess = !result.toolpath.empty();
    if (!result.isSuccess)
    {
        result.error = "No motion found in G-code file.";
    }
    return result;
}

std::vector<QVector3D> CadParser::tesselateCurve(const QVector3D& start,
                                                 const QVector3D& end,
                                                 double segmentLength)
{
    std::vector<QVector3D> out;
    const double dx = end.x() - start.x();
    const double dy = end.y() - start.y();
    const double len = std::hypot(dx, dy);
    if (len < 1e-6)
    {
        out.push_back(start);
        return out;
    }
    const int n = std::max(1, static_cast<int>(std::ceil(len / segmentLength)));
    for (int i = 0; i <= n; ++i)
    {
        const double t = static_cast<double>(i) / n;
        out.push_back(QVector3D(start.x() + dx * t, start.y() + dy * t, start.z()));
    }
    return out;
}

std::vector<PathSegment> CadParser::generateZLevels(const std::vector<QVector3D>& geometry,
                                                    const ParsingConfig& config)
{
    std::vector<PathSegment> out;
    if (geometry.empty())
    {
        return out;
    }

    const int passes = std::max(1, static_cast<int>(std::ceil(config.totalDepth / config.stepDown)));
    for (int p = 1; p <= passes; ++p)
    {
        const double z = -config.stepDown * p;
        QVector3D prev = QVector3D(geometry.front().x(), geometry.front().y(), z);
        for (size_t i = 1; i < geometry.size(); ++i)
        {
            const QVector3D cur(geometry[i].x(), geometry[i].y(), z);
            PathSegment seg;
            seg.start = prev;
            seg.end = cur;
            seg.type = MoveType::Linear;
            out.push_back(seg);
            prev = cur;
        }
    }
    return out;
}

int CadParser::calculateEstimatedTime(const std::vector<PathSegment>& toolpath, double feedRate)
{
    double length = 0.0;
    for (const PathSegment& s : toolpath)
    {
        const QVector3D d = s.end - s.start;
        length += d.length();
    }
    if (feedRate <= 0.0)
    {
        return 0;
    }
    return static_cast<int>((length / feedRate) * 60.0);   // seconds
}

ParseResult CadParser::parseDXF(const QFileInfo& fileInfo,
                                const ToolProfile& toolProfile,
                                const ParsingConfig& config)
{
    ParseResult result;
    QFile file(fileInfo.filePath());
    if (!file.open(QIODevice::ReadOnly))
    {
        result.error = "Cannot open DXF: " + fileInfo.filePath();
        return result;
    }

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd())
    {
        lines.append(in.readLine().trimmed());
    }

    // Minimal DXF reader: collects LINE entities and LWPOLYLINE vertices.
    std::vector<QVector3D> polyline;
    bool inEntities = false;
    bool inLWPoly = false;
    int vertexCount = 0;

    for (int i = 0; i + 1 < lines.size(); ++i)
    {
        const QString code = lines[i];
        const QString value = lines[i + 1];

        if (code == "0" && value == "ENTITIES") { inEntities = true; continue; }
        if (code == "0" && value == "EOF") { inEntities = false; }

        if (!inEntities) continue;

        if (code == "0" && value == "LINE")
        {
            // Read the next group pairs for 10/20 (start) and 11/21 (end).
            double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
            for (int j = i + 2; j + 1 < lines.size() && lines[j] != "0"; j += 2)
            {
                if (lines[j] == "10") x1 = lines[j + 1].toDouble();
                else if (lines[j] == "20") y1 = lines[j + 1].toDouble();
                else if (lines[j] == "11") x2 = lines[j + 1].toDouble();
                else if (lines[j] == "21") y2 = lines[j + 1].toDouble();
            }
            polyline.emplace_back(x1, y1, 0);
            polyline.emplace_back(x2, y2, 0);
        }

        if (code == "0" && value == "LWPOLYLINE")
        {
            inLWPoly = true;
            vertexCount = 0;
            for (int j = i + 2; j + 1 < lines.size() && lines[j] != "0"; j += 2)
            {
                if (lines[j] == "90") vertexCount = lines[j + 1].toInt();
            }
        }
        else if (inLWPoly && (code == "10" || code == "20"))
        {
            // Accumulate x/y pairs while inside a polyline.
            static double pendingX = 0;
            static bool haveX = false;
            if (code == "10") { pendingX = value.toDouble(); haveX = true; }
            else if (code == "20" && haveX)
            {
                polyline.emplace_back(pendingX, value.toDouble(), 0);
                haveX = false;
            }
            if (static_cast<int>(polyline.size()) >= vertexCount && vertexCount > 0)
            {
                inLWPoly = false;
            }
        }
    }

    if (polyline.empty())
    {
        result.error = "No LINE/LWPOLYLINE geometry found in DXF.";
        return result;
    }

    result.toolpath = generateZLevels(polyline, config);
    result.totalPasses = std::max(1, static_cast<int>(std::ceil(config.totalDepth / config.stepDown)));
    result.estimatedTime = calculateEstimatedTime(result.toolpath, config.feedRate);
    result.isSuccess = !result.toolpath.empty();
    return result;
}

ParseResult CadParser::parseSVG(const QFileInfo& fileInfo,
                                const ToolProfile& toolProfile,
                                const ParsingConfig& config)
{
    ParseResult result;
    QFile file(fileInfo.filePath());
    if (!file.open(QIODevice::ReadOnly))
    {
        result.error = "Cannot open SVG: " + fileInfo.filePath();
        return result;
    }

    const QString content = QString::fromUtf8(file.readAll());
    std::vector<QVector3D> polyline;

    // Extract M/L path commands (sufficient for simple 2D contours).
    QRegularExpression re("(M|L|m|l)\\s*([-\\d.]+)[,\\s]+([-\\d.]+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = re.globalMatch(content);

    QVector3D cursor(0, 0, 0);
    while (it.hasNext())
    {
        const QRegularExpressionMatch m = it.next();
        const bool relative = m.captured(1) == "m" || m.captured(1) == "l";
        const double x = m.captured(2).toDouble();
        const double y = m.captured(3).toDouble();
        const QVector3D next = relative ? QVector3D(cursor.x() + x, cursor.y() + y, 0)
                                        : QVector3D(x, y, 0);
        polyline.push_back(next);
        cursor = next;
    }

    if (polyline.empty())
    {
        result.error = "No path data (M/L) found in SVG.";
        return result;
    }

    result.toolpath = generateZLevels(polyline, config);
    result.totalPasses = std::max(1, static_cast<int>(std::ceil(config.totalDepth / config.stepDown)));
    result.estimatedTime = calculateEstimatedTime(result.toolpath, config.feedRate);
    result.isSuccess = !result.toolpath.empty();
    return result;
}

ParseResult CadParser::parseSTEP(const QFileInfo& fileInfo,
                                 const ToolProfile& toolProfile,
                                 const ParsingConfig& config)
{
    ParseResult result;
    result.error = "STEP parsing is not implemented in this host build.";
    return result;
}
