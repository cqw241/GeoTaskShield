#pragma once

#include "simulation/SimulationEngine.h"

#include <QMainWindow>

#include <optional>

namespace gts {

class AgentAssistantWidget;
class BatchResultsWidget;
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
    BatchResultsWidget* batchResultsWidget_{};
    AgentAssistantWidget* agentAssistantWidget_{};
    std::optional<SimulationRunResult> lastResult_;
};

} // namespace gts
