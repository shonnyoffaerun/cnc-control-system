#pragma once 

#include <QObject>
#include <QString>
#include <QVector3D>
#include <QFileInfo>
#include <vector>

#include "visualizer.h"

/*
    struct ToolProfile - implementation of ToolProfile struct from visualizer.h, included here to avoid circular dependencies. This struct encapsulates the properties of a cutting tool used for generating toolpaths.
*/
struct ToolProfile;

/*
    struct ParsingConfig - This struct encapsulates all the parameters needed for parsing and generating toolpaths from CAD files.
    Attributes: double totalDepth - The total depth of cut for the toolpath.
                double stepDown - The depth of each individual pass.
                double feedRate - The feed rate for the toolpath.
                double safeHeight - The height at which the tool can safely move without cutting.
                bool isPocketing - Whether to generate a pocketing toolpath (true) or a contouring toolpath (false).
                double leadInDistance - The distance for lead-in moves before starting the cut.
*/
struct ParsingConfig
{
    double totalDepth{5.0};
    double stepDown{1.0};
    double feedRate{500.0};
    double safeHeight{5.0};
    bool   isPocketing{false};
    double leadInDistance{0.0};
};

/*
    struct ParseResult - This struct encapsulates the results of parsing a CAD file and generating a toolpath.
    Attributes: std::vector<PathSegment> toolpath - The toolpath generated from the CAD file.
                int estimatedTime - The estimated time to complete the toolpath.
                int totalPasses - The total number of passes required to complete the toolpath.
                QString error - Any error message encountered during parsing.
                bool isSuccess - Whether the parsing and toolpath generation was successful.

*/
struct ParseResult 
{
    std::vector<PathSegment> toolpath;
    int                      estimatedTime{0};
    int                      totalPasses{0};
    QString                  error;
    bool                     isSuccess{false};
};

/*
    class CadParser - This class is responsible for parsing CAD files (DXF, SVG, STEP) and generating toolpaths based on the provided ToolProfile and ParsingConfig. It emits signals to indicate the progress of parsing and the results.
    Methods: explicit CadParser(QObject *parent = nullptr) - Constructor for the CadParser class.
             ~CadParser() override - Destructor for the CadParser class.
             ParseResult parseFile(const QFileInfo &fileInfo, const ToolProfile &toolProfile, const ParsingConfig &config) - Parses a CAD file and generates a toolpath based on the provided ToolProfile and ParsingConfig.
             ParseResult parseGCodeFile(const QString &filepath) - Parses a G-code file and generates a toolpath.
             bool isSupportedFormat(const QString &fileInfo) const - Checks if the provided file format is supported for parsing.
             QString getSupportedFilters() const - Returns a string of supported file filters for use in file dialogs.
             ParseResult parseDXF(const QFileInfo &fileInfo, const ToolProfile &toolProfile, const ParsingConfig &config) - Parses a DXF file and generates a toolpath.
             ParseResult parseSVG(const QFileInfo &fileInfo, const ToolProfile &toolProfile, const ParsingConfig &config) - Parses an SVG file and generates a toolpath.
             ParseResult parseSTEP(const QFileInfo &fileInfo, const ToolProfile &toolProfile, const ParsingConfig &config) - Parses a STEP file and generates a toolpath.
             std::vector<PathSegment> generateZLevels(const std::vector<QVector3D> &geometry, const ParsingConfig &config) - Generates Z-level toolpaths based on the geometry and parsing configuration.
             std::vector<QVector3D> tesselateCurve(const QVector3D &start, const QVector3D &end, double segmentLength) - Tesselates a curve into segments of a specified length.
             int calculateEstimatedTime(const std::vector<PathSegment> &toolpath, double feedRate) - Calculates the estimated time to complete the toolpath based on the feed rate.
    Signals: void parsingStarted(const QString &filename) - Emitted when parsing starts for a file. 
             void parsingFinished(const ParseResult &result) - Emitted when parsing is finished, providing the results of the parsing process.
             void progressChanged(int progress) - Emitted to indicate the progress of parsing, with progress
*/
class CadParser : public QObject
{
    Q_OBJECT

public: 
    
    explicit                 CadParser(QObject *parent = nullptr);
                             ~CadParser() override;

    ParseResult              parseFile(const QFileInfo &fileInfo, 
                                       const ToolProfile &toolProfile, 
                                       const ParsingConfig &config);

    ParseResult              parseGCodeFile(const QString &filepath);
    bool                     isSupportedFormat(const QString &fileInfo) const;
    QString                  getSupportedFilters() const;

signals: 

    void                     parsingStarted(const QString &filename);
    void                     parsingFinished(const ParseResult &result);
    void                     progressChanged(int progress);

private: 

    ParseResult              parseDXF(const QFileInfo &fileInfo, 
                                      const ToolProfile &toolProfile, 
                                      const ParsingConfig &config);

    ParseResult              parseSVG(const QFileInfo &fileInfo, 
                                      const ToolProfile &toolProfile, 
                                      const ParsingConfig &config);

    ParseResult              parseSTEP(const QFileInfo &fileInfo, 
                                       const ToolProfile &toolProfile, 
                                       const ParsingConfig &config);

    std::vector<PathSegment> generateZLevels(const std::vector<QVector3D> &geometry, 
                                             const ParsingConfig &config);

    std::vector<QVector3D>   tesselateCurve(const QVector3D &start, 
                                          const QVector3D &end, 
                                          double segmentLength);

    int                      calculateEstimatedTime(const std::vector<PathSegment> &toolpath, 
                                                    double feedRate);

};
 
