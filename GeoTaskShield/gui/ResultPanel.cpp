#include "gui/ResultPanel.h"

#include <QFormLayout>
#include <QLabel>

namespace gts {

namespace {

QLabel* makeValueLabel(QWidget* parent)
{
    auto* label = new QLabel("-", parent);
    label->setMinimumWidth(96);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return label;
}

QString number(double value)
{
    return QString::number(value, 'f', 2);
}

} // namespace

ResultPanel::ResultPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* form = new QFormLayout(this);
    completedValue_ = makeValueLabel(this);
    rateValue_ = makeValueLabel(this);
    distanceValue_ = makeValueLabel(this);
    rewardValue_ = makeValueLabel(this);
    privacyLossValue_ = makeValueLabel(this);
    runtimeValue_ = makeValueLabel(this);

    form->addRow("Completed", completedValue_);
    form->addRow("Rate", rateValue_);
    form->addRow("Avg distance", distanceValue_);
    form->addRow("Reward", rewardValue_);
    form->addRow("Privacy loss", privacyLossValue_);
    form->addRow("Runtime ms", runtimeValue_);

    setMinimumWidth(240);
    setMaximumWidth(320);
}

void ResultPanel::clear()
{
    completedValue_->setText("-");
    rateValue_->setText("-");
    distanceValue_->setText("-");
    rewardValue_->setText("-");
    privacyLossValue_->setText("-");
    runtimeValue_->setText("-");
}

void ResultPanel::setMetrics(const EvaluationMetrics& metrics)
{
    completedValue_->setText(
        QString("%1 / %2").arg(metrics.completedTasks).arg(metrics.totalTasks));
    rateValue_->setText(number(metrics.completionRate * 100.0) + "%");
    distanceValue_->setText(number(metrics.averageMovingDistance));
    rewardValue_->setText(number(metrics.totalReward));
    privacyLossValue_->setText(number(metrics.averagePrivacyLoss));
    runtimeValue_->setText(number(metrics.algorithmRuntimeMs));
}

} // namespace gts
