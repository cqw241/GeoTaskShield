#include "gui/ParameterPanel.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace gts {

ParameterPanel::ParameterPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    auto* form = new QFormLayout;

    workerCountSpin_ = new QSpinBox(this);
    workerCountSpin_->setRange(1, 10000);
    workerCountSpin_->setValue(100);
    form->addRow("Workers", workerCountSpin_);

    taskCountSpin_ = new QSpinBox(this);
    taskCountSpin_->setRange(1, 10000);
    taskCountSpin_->setValue(50);
    form->addRow("Tasks", taskCountSpin_);

    gridSizeSpin_ = new QDoubleSpinBox(this);
    gridSizeSpin_->setRange(0.1, 1000.0);
    gridSizeSpin_->setDecimals(2);
    gridSizeSpin_->setValue(10.0);
    form->addRow("Grid size", gridSizeSpin_);

    kSpin_ = new QSpinBox(this);
    kSpin_->setRange(1, 10000);
    kSpin_->setValue(5);
    form->addRow("k", kSpin_);

    epsilonSpin_ = new QDoubleSpinBox(this);
    epsilonSpin_->setRange(0.01, 100.0);
    epsilonSpin_->setDecimals(2);
    epsilonSpin_->setValue(1.0);
    form->addRow("Epsilon", epsilonSpin_);

    privacyCombo_ = new QComboBox(this);
    privacyCombo_->addItem("Grid Privacy", "grid");
    privacyCombo_->addItem("K-Anonymity Privacy", "k-anonymity");
    privacyCombo_->addItem("Laplace Noise Privacy", "laplace");
    form->addRow("Privacy", privacyCombo_);

    algorithmCombo_ = new QComboBox(this);
    algorithmCombo_->addItem("Nearest Greedy", "nearest");
    algorithmCombo_->addItem("Score Greedy", "score");
    algorithmCombo_->addItem("Hungarian", "hungarian");
    form->addRow("Algorithm", algorithmCombo_);

    runButton_ = new QPushButton("Run Simulation", this);

    root->addLayout(form);
    root->addStretch();
    root->addWidget(runButton_);
    setMinimumWidth(240);
    setMaximumWidth(320);
}

SimulationConfig ParameterPanel::simulationConfig() const
{
    SimulationConfig config;
    config.workerCount = workerCountSpin_->value();
    config.taskCount = taskCountSpin_->value();
    config.randomSeed = 42;
    config.areaWidth = 100.0;
    config.areaHeight = 100.0;
    config.privacy.gridSize = gridSizeSpin_->value();
    config.privacy.k = kSpin_->value();
    config.privacy.epsilon = epsilonSpin_->value();
    return config;
}

std::string ParameterPanel::privacyType() const
{
    return privacyCombo_->currentData().toString().toStdString();
}

std::string ParameterPanel::algorithmType() const
{
    return algorithmCombo_->currentData().toString().toStdString();
}

QPushButton* ParameterPanel::runButton() const
{
    return runButton_;
}

} // namespace gts
