#include "gui/MetricBarChart.h"

#include <QPainter>
#include <QStringList>

#include <algorithm>
#include <cmath>
#include <utility>

namespace gts {

MetricBarChart::MetricBarChart(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(260);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void MetricBarChart::setBars(std::vector<ChartBar> bars)
{
    bars_ = std::move(bars);
    update();
}

std::size_t MetricBarChart::barCount() const
{
    return bars_.size();
}

std::size_t MetricBarChart::xAxisLabelCountForTesting() const
{
    const std::size_t stride = xAxisLabelStride();
    if (bars_.empty() || stride == 0) {
        return 0;
    }
    return (bars_.size() + stride - 1) / stride;
}

std::size_t MetricBarChart::xAxisLabelStride() const
{
    if (bars_.size() <= 12) {
        return 1;
    }

    const QRectF plot = QRectF(rect()).adjusted(44.0, 22.0, -18.0, -76.0);
    const double slotWidth = plot.width() / static_cast<double>(bars_.size());
    constexpr double minimumLabelSpacing = 116.0;
    const auto widthStride = static_cast<std::size_t>(
        std::ceil(minimumLabelSpacing / std::max(1.0, slotWidth)));
    const auto countStride = static_cast<std::size_t>(
        std::ceil(static_cast<double>(bars_.size()) / 6.0));
    return std::max<std::size_t>(1, std::max(widthStride, countStride));
}

void MetricBarChart::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(248, 250, 252));

    const QRectF plot = QRectF(rect()).adjusted(44.0, 22.0, -18.0, -76.0);
    painter.setPen(QPen(QColor(203, 213, 225), 1.0));
    painter.drawRect(plot);

    if (bars_.empty()) {
        painter.setPen(QColor(100, 116, 139));
        painter.drawText(rect(), Qt::AlignCenter, "No data");
        return;
    }

    double maxValue = 0.0;
    for (const ChartBar& bar : bars_) {
        maxValue = std::max(maxValue, bar.value);
    }
    if (maxValue <= 0.0) {
        maxValue = 1.0;
    }

    const double gap = 6.0;
    const double barWidth =
        std::max(4.0, (plot.width() - gap * static_cast<double>(bars_.size() + 1)) /
                           static_cast<double>(bars_.size()));

    painter.setPen(QColor(15, 23, 42));
    painter.drawText(QRectF(4.0, plot.top() - 4.0, 38.0, 20.0),
                     Qt::AlignRight | Qt::AlignVCenter,
                     QString::number(maxValue, 'f', 2));

    const std::size_t labelStride = xAxisLabelStride();
    for (std::size_t i = 0; i < bars_.size(); ++i) {
        const ChartBar& bar = bars_[i];
        const double height = (bar.value / maxValue) * plot.height();
        const double x = plot.left() + gap + static_cast<double>(i) * (barWidth + gap);
        const QRectF barRect(x, plot.bottom() - height, barWidth, height);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(37, 99, 235));
        painter.drawRect(barRect);

        painter.setPen(QColor(15, 23, 42));
        const bool drawAxisLabel = labelStride == 0 || i % labelStride == 0;
        if (drawAxisLabel) {
            painter.drawText(QRectF(x - 8.0, barRect.top() - 18.0, barWidth + 16.0, 16.0),
                             Qt::AlignCenter,
                             QString::number(bar.value, 'f', 2));
        }

        QString label = QString::fromStdString(bar.label);
        if (drawAxisLabel) {
            const double labelWidth =
                std::max(barWidth + 36.0,
                         (barWidth + gap) * static_cast<double>(labelStride) - gap);
            double labelX = x - (labelWidth - barWidth) * 0.5;
            labelX = std::max(plot.left(), std::min(labelX, plot.right() - labelWidth));
            const QRectF labelRect(labelX, plot.bottom() + 6.0, labelWidth, 58.0);

            QStringList lines = label.split('\n');
            while (lines.size() > 2) {
                lines.removeLast();
            }
            const QFont originalFont = painter.font();
            QFont labelFont = originalFont;
            if (bars_.size() > 12) {
                labelFont.setPointSizeF(std::max(7.0, labelFont.pointSizeF() - 1.0));
            }
            painter.setFont(labelFont);
            const QFontMetrics metrics = painter.fontMetrics();
            for (int lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
                const QRectF lineRect(labelRect.left(),
                                      labelRect.top() + static_cast<double>(lineIndex) * 16.0,
                                      labelRect.width(),
                                      16.0);
                painter.drawText(lineRect,
                                 Qt::AlignHCenter | Qt::AlignTop,
                                 metrics.elidedText(lines[lineIndex], Qt::ElideRight,
                                                    static_cast<int>(labelRect.width())));
            }
            painter.setFont(originalFont);
        }
    }
}

} // namespace gts
