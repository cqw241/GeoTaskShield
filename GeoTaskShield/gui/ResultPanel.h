#pragma once

#include "evaluation/EvaluationMetrics.h"

#include <QWidget>

class QLabel;

namespace gts {

class ResultPanel : public QWidget {
public:
    explicit ResultPanel(QWidget* parent = nullptr);

    void clear();
    void setMetrics(const EvaluationMetrics& metrics);

private:
    QLabel* completedValue_{};
    QLabel* rateValue_{};
    QLabel* distanceValue_{};
    QLabel* rewardValue_{};
    QLabel* privacyLossValue_{};
    QLabel* runtimeValue_{};
};

} // namespace gts
