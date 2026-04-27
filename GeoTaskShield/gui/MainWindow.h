#pragma once

#include "simulation/SimulationEngine.h"

#include <QMainWindow>

#include <optional>

namespace gts {

class LogPanel;
class MapCanvas;
class ParameterPanel;
class ResultPanel;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

    void runSimulation();
    bool hasSimulationResult() const;

private:
    ParameterPanel* parameterPanel_{};
    MapCanvas* mapCanvas_{};
    ResultPanel* resultPanel_{};
    LogPanel* logPanel_{};
    std::optional<SimulationRunResult> lastResult_;
};

} // namespace gts
