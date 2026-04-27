#pragma once

#include "model/ExperimentConfig.h"

#include <QWidget>

#include <string>

class QComboBox;
class QDoubleSpinBox;
class QPushButton;
class QSpinBox;

namespace gts {

class ParameterPanel : public QWidget {
public:
    explicit ParameterPanel(QWidget* parent = nullptr);

    SimulationConfig simulationConfig() const;
    std::string privacyType() const;
    std::string algorithmType() const;
    QPushButton* runButton() const;

private:
    QSpinBox* workerCountSpin_{};
    QSpinBox* taskCountSpin_{};
    QDoubleSpinBox* gridSizeSpin_{};
    QSpinBox* kSpin_{};
    QDoubleSpinBox* epsilonSpin_{};
    QComboBox* privacyCombo_{};
    QComboBox* algorithmCombo_{};
    QPushButton* runButton_{};
};

} // namespace gts
