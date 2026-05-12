#pragma once 

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QJsonDocument>

/*
    enum class ToolType - defines the type of tool being used in the CNC machine. This can include various types of cutting tools, such as end mills, drills, V-bits, lasers, etc. The ToolType enum helps to categorize and manage different tools based on their specific characteristics and applications in CNC machining.
    Attributes: EndMill - EndMill tool type,
                Drill - Drill tool type,
                VBit - VBit tool type,
                Laser - Laser tool type,
                Unknown - Unknown tool type
*/
enum class ToolType 
{
    EndMill,
    Drill, 
    VBit,
    Laser,
    Unknown
};

/*
    enum class ToolMaterial - defines the material composition of the tool being used in the CNC machine. This can include various materials such as High-Speed Steel (HSS), Carbide, Stainless Steel, Ceramic, Diamond, Carbon, etc. The ToolMaterial enum helps to categorize and manage different tools based on their material properties, which can affect their performance and suitability for specific machining tasks.
    Attributes: HSS - High-Speed Steel material,
                Carbide - Carbide material,
                Stainless - Stainless Steel material,
                Ceramic - Ceramic material,
                Diamond - Diamond material,
                Carbon - Carbon material,
                Unknown - Unknown material
*/
enum class ToolMaterial 
{
    HSS,
    Carbide,
    Stainless,
    Ceramic,
    Diamond,
    Carbon,
    Unknown
};

/*
    struct ToolProfile - defines the profile of a tool used in the CNC machine. It includes various attributes such as id, name, type, material, dimensions (diameter, flute length, total length), performance parameters (max RPM, max feed rate, max depth of cut), and wear information (isActive, currentWear). The ToolProfile struct helps to manage and organize the different tools available in the CNC machine, allowing for easy access and manipulation of tool data.
    Attributes: int id - Unique identifier for the tool profile,
                QString name - Name of the tool profile,
                ToolType type - Type of the tool (e.g., EndMill, Drill, VBit, Laser, Unknown),
                ToolMaterial material - Material of the tool (e.g., HSS, Carbide, Stainless, Ceramic, Diamond, Carbon, Unknown),
                double diameter - Diameter of the tool,
                double fluteLength - Flute length of the tool,
                double totalLength - Total length of the tool,
                double maxRPM - Maximum revolutions per minute for the tool,
                double maxFeedRate - Maximum feed rate for the tool,
                double maxDepthOfCut - Maximum depth of cut for the tool,
                bool isActive - Indicates whether the tool is currently active,
                double currentWear - Current wear level of the tool,
                bool operator== - Equality operator to compare two ToolProfile instances based on their id,
                bool operator< - Less-than operator to compare two ToolProfile instances based on their id for sorting purposes.
*/
struct ToolProfile 
{
    int id{0};
    QString name;
    ToolType type{ToolType::Unknown};
    ToolMaterial material{ToolMaterial::HSS};

    double diameter{0.0};
    double fluteLength{0.0};
    double totalLength{0.0};

    double maxRPM{0.0};
    double maxFeedRate{0.0};
    double maxDepthOfCut{0.0};

    bool isActive{false};
    double currentWear{0.0};

    bool operator==(const ToolProfile& other) const 
    {
        return id == other.id;
    }
    bool operator<(const ToolProfile& other) const 
    {
        return id < other.id;
    }
};

/*
    class ToolManager - manages the tool profiles for a CNC machine. It provides functionalities to load and save tool databases, add and remove tools, set the active tool, validate cutting parameters, and generate G-code for tool changes and length offsets. The ToolManager class uses a QMap to store the tool profiles, with the tool ID as the key. It also emits signals when tools are added, removed, or when the active tool changes, allowing other components of the application to react accordingly.
    Attributes: QMap<int, ToolProfile> toolDatabase - A map to store tool profiles,
                int activeToolId - The ID of the currently active tool,
                QMutex mutex - A mutex to ensure thread safety
    Methods:    explicit ToolManager(QObject* parent = nullptr) - Constructor to initialize the ToolManager,
                ~ToolManager() override - Destructor to clean up resources,
                bool loadDatabase(const QString& filePath) - Loads tool profiles from a JSON file,
                bool saveDatabase(const QString& filePath) const - Saves tool profiles to a JSON file,
                bool addTool(const ToolProfile& tool) - Adds a new tool profile to the database,
                bool removeTool(int toolId) - Removes a tool profile from the database by ID,
                ToolProfile getTool(int toolId) const - Retrieves a tool profile by ID,
                QList<ToolProfile> getAllTools() const - Retrieves all tool profiles,
                void setActiveTool(int toolId) - Sets the active tool by ID,
                ToolProfile getActiveTool() const - Retrieves the currently active tool profile,
                bool validateCuttingParameters(double rpm, double feed) const - Validates cutting parameters against the active tool's specifications,
                QString generateToolChangeCode(int toolId) const - Generates G-code for changing to a specified tool,
                QString generateLengthOffsetCode(double measuredLength) const - Generates G-code for setting length offsets based on measured length.
    Signals:    void toolAdded(const ToolProfile& tool) - Emitted when a new tool is added,
                void toolRemoved(int toolId) - Emitted when a tool is removed,
                void activeToolChanged(int toolId) - Emitted when the active tool changes,
                void validationFailed(const QString& message) - Emitted when cutting parameter validation fails
*/
class ToolManager : public QObject 
{
    Q_OBJECT

public:

    explicit               ToolManager(QObject* parent = nullptr);
                           ~ToolManager() override;
    
    bool                   loadDatabase(const QString& filePath);
    bool                   saveDatabase(const QString& filePath) const;

    bool                   addTool(const ToolProfile& tool);
    bool                   removeTool(int toolId);
    ToolProfile            getTool(int toolId) const;
    QList<ToolProfile>     getAllTools() const;

    void                   setActiveTool(int toolId);
    ToolProfile            getActiveTool() const;

    bool                   validateCuttingParameters(double rpm, double feed) const;
    QString                generateToolChangeCode(int toolId) const;
    QString                generateLengthOffsetCode(double measuredLength) const;
    
signals:

    void                   toolAdded(const ToolProfile& tool);
    void                   toolRemoved(int toolId);
    void                   activeToolChanged(int toolId);
    void                   validationFailed(const QString& message);

private: 

    QMap<int, ToolProfile> toolDatabase;
    int                    activeToolId{0};
    mutable QMutex         mutex;

};

