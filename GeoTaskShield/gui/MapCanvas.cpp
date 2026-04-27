#include "gui/MapCanvas.h"

#include <QPainter>

#include <algorithm>
#include <unordered_map>

namespace gts {

namespace {

void includePoint(QRectF& bounds, const Location& location)
{
    if (bounds.isNull()) {
        bounds = QRectF(location.x, location.y, 0.1, 0.1);
        return;
    }

    const double left = std::min(bounds.left(), location.x);
    const double right = std::max(bounds.right(), location.x);
    const double top = std::min(bounds.top(), location.y);
    const double bottom = std::max(bounds.bottom(), location.y);
    bounds = QRectF(QPointF(left, top), QPointF(right, bottom));
}

QRectF mapBounds(const SimulationRunResult& result)
{
    QRectF bounds;
    for (const Worker& worker : result.data.workers) {
        includePoint(bounds, worker.realLocation);
    }
    for (const Worker& worker : result.privacy.workers) {
        includePoint(bounds, worker.exposedLocation);
    }
    for (const Task& task : result.data.tasks) {
        includePoint(bounds, task.location);
    }

    if (bounds.width() < 1.0) {
        bounds.setWidth(1.0);
    }
    if (bounds.height() < 1.0) {
        bounds.setHeight(1.0);
    }
    return bounds.adjusted(-4.0, -4.0, 4.0, 4.0);
}

QPointF project(const Location& location, const QRectF& source, const QRectF& target)
{
    const double xRatio = (location.x - source.left()) / source.width();
    const double yRatio = (location.y - source.top()) / source.height();
    return QPointF(target.left() + xRatio * target.width(),
                   target.bottom() - yRatio * target.height());
}

} // namespace

MapCanvas::MapCanvas(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(520, 360);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void MapCanvas::clear()
{
    result_.reset();
    update();
}

void MapCanvas::setResult(const SimulationRunResult& result)
{
    result_ = result;
    update();
}

void MapCanvas::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(248, 250, 252));

    const QRectF plot = QRectF(rect()).adjusted(24.0, 24.0, -24.0, -24.0);
    painter.setPen(QPen(QColor(203, 213, 225), 1.0));
    painter.drawRect(plot);

    if (!result_) {
        return;
    }

    const QRectF bounds = mapBounds(*result_);
    std::unordered_map<int, Location> tasksById;
    std::unordered_map<int, Location> workersById;
    tasksById.reserve(result_->data.tasks.size());
    workersById.reserve(result_->data.workers.size());

    for (const Task& task : result_->data.tasks) {
        tasksById.emplace(task.id, task.location);
    }
    for (const Worker& worker : result_->data.workers) {
        workersById.emplace(worker.id, worker.realLocation);
    }

    painter.setPen(QPen(QColor(100, 116, 139, 150), 1.2, Qt::DashLine));
    for (const Assignment& assignment : result_->assignment.assignments) {
        const auto taskIt = tasksById.find(assignment.taskId);
        const auto workerIt = workersById.find(assignment.workerId);
        if (taskIt == tasksById.end() || workerIt == workersById.end()) {
            continue;
        }
        painter.drawLine(project(workerIt->second, bounds, plot),
                         project(taskIt->second, bounds, plot));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(37, 99, 235));
    for (const Worker& worker : result_->data.workers) {
        painter.drawEllipse(project(worker.realLocation, bounds, plot), 4.0, 4.0);
    }

    painter.setPen(QPen(QColor(20, 184, 166), 1.4));
    painter.setBrush(Qt::NoBrush);
    for (const Worker& worker : result_->privacy.workers) {
        painter.drawEllipse(project(worker.exposedLocation, bounds, plot), 5.5, 5.5);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(220, 38, 38));
    for (const Task& task : result_->data.tasks) {
        painter.drawEllipse(project(task.location, bounds, plot), 4.8, 4.8);
    }
}

} // namespace gts
