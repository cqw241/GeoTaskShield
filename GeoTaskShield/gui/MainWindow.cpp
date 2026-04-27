#include "gui/MainWindow.h"

#include "assignment/AssignmentAlgorithmFactory.h"
#include "gui/LogPanel.h"
#include "gui/MapCanvas.h"
#include "gui/ParameterPanel.h"
#include "gui/ResultPanel.h"
#include "privacy/PrivacyFactory.h"

#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

#include <exception>
#include <memory>
#include <utility>

namespace gts {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("GeoTaskShield");

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(8, 8, 8, 8);

    auto* contentSplitter = new QSplitter(Qt::Horizontal, central);
    parameterPanel_ = new ParameterPanel(contentSplitter);
    mapCanvas_ = new MapCanvas(contentSplitter);
    resultPanel_ = new ResultPanel(contentSplitter);
    contentSplitter->addWidget(parameterPanel_);
    contentSplitter->addWidget(mapCanvas_);
    contentSplitter->addWidget(resultPanel_);
    contentSplitter->setStretchFactor(0, 0);
    contentSplitter->setStretchFactor(1, 1);
    contentSplitter->setStretchFactor(2, 0);

    logPanel_ = new LogPanel(central);
    root->addWidget(contentSplitter, 1);
    root->addWidget(logPanel_, 0);
    setCentralWidget(central);

    connect(parameterPanel_->runButton(), &QPushButton::clicked, this, [this]() {
        runSimulation();
    });

    resize(1180, 760);
    logPanel_->appendInfo("Ready.");
}

void MainWindow::runSimulation()
{
    logPanel_->clear();
    resultPanel_->clear();
    mapCanvas_->clear();
    lastResult_.reset();

    const std::string privacyType = parameterPanel_->privacyType();
    const std::string algorithmType = parameterPanel_->algorithmType();
    auto privacy = PrivacyFactory::create(privacyType);
    auto algorithm = AssignmentAlgorithmFactory::create(algorithmType);
    if (!privacy || !algorithm) {
        logPanel_->appendInfo("Unknown privacy mechanism or assignment algorithm.");
        return;
    }

    try {
        logPanel_->appendInfo("Running simulation.");
        SimulationEngine engine(std::move(privacy), std::move(algorithm));
        lastResult_ = engine.run(parameterPanel_->simulationConfig());
        resultPanel_->setMetrics(lastResult_->metrics);
        mapCanvas_->setResult(*lastResult_);
        logPanel_->appendInfo(
            QString("Completed %1 of %2 tasks.")
                .arg(lastResult_->metrics.completedTasks)
                .arg(lastResult_->metrics.totalTasks));
    } catch (const std::exception& error) {
        logPanel_->appendInfo(QString("Simulation failed: %1").arg(error.what()));
    }
}

bool MainWindow::hasSimulationResult() const
{
    return lastResult_.has_value();
}

} // namespace gts
