#pragma once 

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QVector3D>
#include <QMatrix4x4>
#include <QTimer>
#include <vector>

/*
    enum class MoveType - describes the type of movement for a visualizer object. It can be either linear or circular.
    Attributes: Rapid - represents a rapid movement, where the object moves quickly from one point to another without following a specific path.
                Linear - represents a linear movement, where the object moves in a straight line from one point to another.
                Arc - represents an arc movement, where the object follows a circular path from one point to another.
*/
enum class MoveType
{
    Rapid,
    Linear,
    Arc
};

/*
    struct PathSegment - a struct that represents a segment of a path. It contains the start and end points of the segment, as well as the type of movement.
    Attributes: QVector3D start - a QVector3D representing the starting point of the path segment.
                QVector3D end - a QVector3D representing the ending point of the path segment.
                MoveType type - a MoveType enum value indicating the type of movement for this path segment (Rapid, Linear, or Arc).
*/
struct PathSegment
{
    QVector3D start;
    QVector3D end;
    MoveType  type;
};

/*
    struct MachineBounds - a struct that represents the boundaries of the machine. It can be used to define the limits within which the visualizer objects can move. The specific attributes and implementation details of this struct are not provided in the given code snippet.
    Attributes: QVector3D min - a QVector3D representing the minimum coordinates of the machine bounds.
                QVector3D max - a QVector3D representing the maximum coordinates of the machine bounds.
*/
struct MachineBounds 
{
    QVector3D min{0, 0, 0};
    QVector3D max{100, 100, 100};
};

/*
    class Visualizer - a QWidget subclass that provides a visual representation of a tool path and machine bounds. It allows for loading and displaying tool paths, updating the current tool position, and visualizing the spindle state. The visualizer also supports user interaction for rotating, zooming, and panning the view.
    Attributes: std::vector<PathSegment> m_toolPath - a vector that holds the segments of the tool path to be visualized.
                MachineBounds m_bounds - an instance of MachineBounds struct that defines the boundaries of the machine.
                QVector3D m_currentToolPos - a QVector3D representing the current position of the tool.
                double m_toolLength - a double value representing the length of the tool.
                bool m_spindleOn - a boolean indicating whether the spindle is currently on or off.
                float m_rotationX - a float value representing the rotation angle around the X-axis for the view.
                float m_rotationZ - a float value representing the rotation angle around the Z-axis for the view.
                float m_zoom - a float value representing the zoom level of the view.
                QPoint m_viewOffset - a QPoint representing the offset of the view for panning.
                QPoint m_lastMousePos - a QPoint representing the last recorded mouse position for handling mouse interactions.
                bool m_mousePressed - a boolean indicating whether the mouse button is currently pressed.
                bool m_mouseRotate - a boolean indicating whether the mouse is in rotate mode for handling rotations.
                QTimer *m_autoRotateTimer - a pointer to a QTimer used for automatically rotating the view when enabled.
                QPixmap m_cache - a QPixmap used for caching the rendered visualizer to improve performance.
                bool m_cacheValid - a boolean indicating whether the cache is valid and can be used for rendering.
    Methods:    explicit Visualizer(QWidget* parent = nullptr) - constructor for the Visualizer class, initializes the visualizer widget.
                ~Visualizer() override - destructor for the Visualizer class, cleans up any resources used by the visualizer.
                void loadToolPath(const std::vector<PathSegment>& path) - loads a tool path into the visualizer for rendering.
                void clearToolPath() - clears the currently loaded tool path from the visualizer.
                void setMachineBounds(const MachineBounds& bounds) - sets the machine bounds for the visualizer.
                void setToolLength(double length) - sets the length of the tool for visualization purposes.
                void resetView() - resets the view to its default orientation and zoom level.
                void setAutoRotate(bool enabled) - enables or disables automatic rotation of the view.
                void updateToolPosition(const QVector3D& position) - updates the current position of the tool in the visualizer.
                void updateSpindleState(bool state) - updates the spindle state (on/off) in the visualizer.
                void paintEvent(QPaintEvent* event) override - overridden method to handle painting of the visualizer widget.
                void resizeEvent(QResizeEvent* event) override - overridden method to handle resizing of the visualizer widget.
                void mousePressEvent(QMouseEvent* event) override - overridden method to handle mouse press events for interaction with the visualizer.
                void mouseMoveEvent(QMouseEvent* event) override - overridden method to handle mouse move events for interaction with the visualizer.
                void mouseReleaseEvent(QMouseEvent* event) override - overridden method to handle mouse release events for interaction with the visualizer.
                void wheelEvent(QWheelEvent* event) override - overridden method to handle mouse wheel events for zooming in and out of the view.
                QPointF project3D(const QVector3D& point) const - a helper method to project a 3D point onto the 2D view for rendering.
                void drawGrid(QPainter& event) - a helper method to draw a grid on the
                void drawBounds(QPainter& event) - a helper method to draw the machine bounds on the visualizer.
                void drawToolPath(QPainter& event) - a helper method to draw the loaded tool
                void drawToolHead(QPainter& event) - a helper method to draw the current position of the tool head on the visualizer.
                void drawAxes(QPainter& event) - a helper method to draw the coordinate axes on
                void drawBackground(QPainter& event) - a helper method to draw the background of the visualizer.
                void updateProjection() - a helper method to update the projection matrix based on the current rotation and zoom settings for accurate rendering of the 3D view.
*/
class Visualizer : public QWidget
{
    Q_OBJECT

public: 

    explicit                 Visualizer(QWidget* parent = nullptr);
                             ~Visualizer() override;
    
    void                     loadToolPath(const std::vector<PathSegment>& path);
    void                     clearToolPath();
    void                     setMachineBounds(const MachineBounds& bounds);
    void                     setToolLength(double length);
    void                     resetView();
    void                     setAutoRotate(bool enabled);

public slots: 

    void                     updateToolPosition(const QVector3D& position);
    void                     updateSpindleState(bool state);

signals: 

    void                     positionClicked(const QVector3D& position);

protected: 

    void                     paintEvent(QPaintEvent* event) override;
    void                     resizeEvent(QResizeEvent* event) override;
    void                     mousePressEvent(QMouseEvent* event) override;
    void                     mouseMoveEvent(QMouseEvent* event) override;
    void                     mouseReleaseEvent(QMouseEvent* event) override;
    void                     wheelEvent(QWheelEvent* event) override;

private:

    QPointF                  project3D(const QVector3D& point) const;

    void                     drawGrid(QPainter& event);
    void                     drawBounds(QPainter& event);
    void                     drawToolPath(QPainter& event);
    void                     drawToolHead(QPainter& event);
    void                     drawAxes(QPainter& event);
    void                     drawBackground(QPainter& event);

    void                     updateProjection();

    std::vector<PathSegment> m_toolPath;
    MachineBounds            m_bounds;
    QVector3D                m_currentToolPos{0, 0, 0};
    double                   m_toolLength{50.0};
    bool                     m_spindleOn{false};

    float                    m_rotationX{30.0f};
    float                    m_rotationZ{45.0f};
    float                    m_zoom{1.0f};
    QPoint                   m_viewOffset;

    QPoint                   m_lastMousePos;
    bool                     m_mousePressed{false};
    bool                     m_mouseRotate{false};

    QTimer                   *m_autoRotateTimer;

    QPixmap                  m_cache;
    bool                     m_cacheValid{false};

};


