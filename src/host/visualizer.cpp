/*
    visualizer.cpp - implementation of visualizer.h
    Lightweight 2D (top-down + isometric-ish) renderer for the toolpath, machine
    bounds, grid and current tool position. Uses QPainter with a cached pixmap so
    the host stays responsive while streaming telemetry from the MKS DLC32.
*/

#include "visualizer.h"

#include <QMouseEvent>
#include <QPainter>
#include <cmath>

Visualizer::Visualizer(QWidget* parent)
    : QWidget(parent)
    , m_autoRotateTimer(new QTimer(this))
{
    setMinimumSize(320, 240);
    connect(m_autoRotateTimer, &QTimer::timeout, this, [this]() {
        m_rotationZ += 1.0f;
        update();
    });
}

Visualizer::~Visualizer() = default;

void Visualizer::loadToolPath(const std::vector<PathSegment>& path)
{
    m_toolPath = path;
    m_cacheValid = false;
    update();
}

void Visualizer::clearToolPath()
{
    m_toolPath.clear();
    m_cacheValid = false;
    update();
}

void Visualizer::setMachineBounds(const MachineBounds& bounds)
{
    m_bounds = bounds;
    m_cacheValid = false;
    update();
}

void Visualizer::setToolLength(double length)
{
    m_toolLength = length;
    update();
}

void Visualizer::resetView()
{
    m_rotationX = 30.0f;
    m_rotationZ = 45.0f;
    m_zoom = 1.0f;
    m_viewOffset = QPoint();
    m_cacheValid = false;
    update();
}

void Visualizer::setAutoRotate(bool enabled)
{
    if (enabled)
    {
        m_autoRotateTimer->start(33);
    }
    else
    {
        m_autoRotateTimer->stop();
    }
}

void Visualizer::updateToolPosition(const QVector3D& position)
{
    m_currentToolPos = position;
    update();
}

void Visualizer::updateSpindleState(bool state)
{
    m_spindleOn = state;
    update();
}

QPointF Visualizer::project3D(const QVector3D& point) const
{
    const float radX = m_rotationX * M_PI / 180.0f;
    const float radZ = m_rotationZ * M_PI / 180.0f;

    const float cx = point.x() * std::cos(radZ) - point.y() * std::sin(radZ);
    const float cy = point.x() * std::sin(radZ) + point.y() * std::cos(radZ);
    const float cz = point.z();

    const float iso = cy * std::cos(radX) - cz * std::sin(radX);

    const float scale = m_zoom * (height() / 120.0f);
    return QPointF(
        width() / 2.0 + cx * scale + m_viewOffset.x(),
        height() / 2.0 - iso * scale + m_viewOffset.y());
}

void Visualizer::updateProjection()
{
    m_cacheValid = false;
}

void Visualizer::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    drawBackground(painter);
    drawGrid(painter);
    drawBounds(painter);
    drawAxes(painter);
    drawToolPath(painter);
    drawToolHead(painter);
}

void Visualizer::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    m_cache = QPixmap(size());
    m_cacheValid = false;
}

void Visualizer::mousePressEvent(QMouseEvent* event)
{
    m_mousePressed = true;
    m_lastMousePos = event->pos();
}

void Visualizer::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_mousePressed)
    {
        return;
    }
    const QPoint delta = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();

    if (event->buttons() & Qt::LeftButton)
    {
        m_rotationZ += delta.x() * 0.5f;
        m_rotationX += delta.y() * 0.5f;
        m_cacheValid = false;
    }
    else if (event->buttons() & Qt::RightButton)
    {
        m_viewOffset += delta;
        m_cacheValid = false;
    }
    update();
}

void Visualizer::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    m_mousePressed = false;
}

void Visualizer::wheelEvent(QWheelEvent* event)
{
    const double factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    m_zoom = static_cast<float>(m_zoom * factor);
    m_cacheValid = false;
    update();
}

void Visualizer::drawBackground(QPainter& painter)
{
    painter.fillRect(rect(), QColor(28, 30, 34));
}

void Visualizer::drawGrid(QPainter& painter)
{
    painter.setPen(QColor(60, 64, 70));
    const int step = 10;
    const float x0 = m_bounds.min.x();
    const float y0 = m_bounds.min.y();
    const float x1 = m_bounds.max.x();
    const float y1 = m_bounds.max.y();

    for (int gx = static_cast<int>(x0); gx <= static_cast<int>(x1); gx += step)
    {
        const QPointF a = project3D(QVector3D(gx, y0, 0));
        const QPointF b = project3D(QVector3D(gx, y1, 0));
        painter.drawLine(a, b);
    }
    for (int gy = static_cast<int>(y0); gy <= static_cast<int>(y1); gy += step)
    {
        const QPointF a = project3D(QVector3D(x0, gy, 0));
        const QPointF b = project3D(QVector3D(x1, gy, 0));
        painter.drawLine(a, b);
    }
}

void Visualizer::drawBounds(QPainter& painter)
{
    painter.setPen(QColor(120, 200, 120));
    const QPointF p0 = project3D(QVector3D(m_bounds.min.x(), m_bounds.min.y(), 0));
    const QPointF p1 = project3D(QVector3D(m_bounds.max.x(), m_bounds.min.y(), 0));
    const QPointF p2 = project3D(QVector3D(m_bounds.max.x(), m_bounds.max.y(), 0));
    const QPointF p3 = project3D(QVector3D(m_bounds.min.x(), m_bounds.max.y(), 0));
    painter.drawPolygon(QPolygonF() << p0 << p1 << p2 << p3);
}

void Visualizer::drawAxes(QPainter& painter)
{
    const QPointF o = project3D(QVector3D(0, 0, 0));
    painter.setPen(Qt::red);
    painter.drawLine(o, project3D(QVector3D(15, 0, 0)));
    painter.setPen(Qt::green);
    painter.drawLine(o, project3D(QVector3D(0, 15, 0)));
    painter.setPen(Qt::blue);
    painter.drawLine(o, project3D(QVector3D(0, 0, 15)));
}

void Visualizer::drawToolPath(QPainter& painter)
{
    for (const PathSegment& seg : m_toolPath)
    {
        if (seg.type == MoveType::Rapid)
        {
            painter.setPen(QColor(90, 90, 160));
        }
        else if (seg.type == MoveType::Arc)
        {
            painter.setPen(QColor(90, 160, 200));
        }
        else
        {
            painter.setPen(QColor(220, 180, 80));
        }
        painter.drawLine(project3D(seg.start), project3D(seg.end));
    }
}

void Visualizer::drawToolHead(QPainter& painter)
{
    const QPointF p = project3D(m_currentToolPos);
    painter.setBrush(m_spindleOn ? Qt::red : Qt::gray);
    painter.setPen(Qt::white);
    painter.drawEllipse(p, 4, 4);
}
