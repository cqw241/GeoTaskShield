#include "gui/ParameterPanel.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
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

    auto* legendTitle = new QLabel("Map legend", this);
    legendTitle->setStyleSheet("font-weight: 600; margin-top: 8px;");

    auto* legend = new QLabel(this);
    legend->setTextFormat(Qt::RichText);
    legend->setWordWrap(true);
    legend->setText(
        "<div style='line-height: 1.45;'>"
        "<div><span style='color:#2563eb; font-size:18px;'>●</span> 蓝色点：真实 worker 位置</div>"
        "<div><span style='color:#dc2626; font-size:18px;'>●</span> 红色点：task 位置</div>"
        "<div><span style='color:#14b8a6; font-size:18px;'>○</span> 绿色圆圈：隐私保护后的暴露位置</div>"
        "<div><span style='color:#64748b;'>- - -</span> 虚线：已分配的 worker-task 连线</div>"
        "</div>");

    runButton_ = new QPushButton("Run Simulation", this);

    root->addLayout(form);
    root->addSpacing(12);
    root->addWidget(legendTitle);
    root->addWidget(legend);
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
