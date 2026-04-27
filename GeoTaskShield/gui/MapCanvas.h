#pragma once

#include "simulation/SimulationEngine.h"

#include <QWidget>

#include <optional>

namespace gts {

class MapCanvas : public QWidget {
public:
    explicit MapCanvas(QWidget* parent = nullptr);

    void clear();
    void setResult(const SimulationRunResult& result);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::optional<SimulationRunResult> result_;
};

} // namespace gts
