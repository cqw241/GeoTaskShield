#include "gui/MetricBarChart.h"

#include <QPainter>

#include <algorithm>
#include <utility>

namespace gts {

MetricBarChart::MetricBarChart(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(220);
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

void MetricBarChart::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(248, 250, 252));

    const QRectF plot = QRectF(rect()).adjusted(44.0, 22.0, -18.0, -46.0);
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

    for (std::size_t i = 0; i < bars_.size(); ++i) {
        const ChartBar& bar = bars_[i];
        const double height = (bar.value / maxValue) * plot.height();
        const double x = plot.left() + gap + static_cast<double>(i) * (barWidth + gap);
        const QRectF barRect(x, plot.bottom() - height, barWidth, height);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(37, 99, 235));
        painter.drawRect(barRect);

        painter.setPen(QColor(15, 23, 42));
        painter.drawText(QRectF(x - 8.0, barRect.top() - 18.0, barWidth + 16.0, 16.0),
                         Qt::AlignCenter,
                         QString::number(bar.value, 'f', 2));

        QString label = QString::fromStdString(bar.label);
        if (label.size() > 24) {
            label = label.left(21) + "...";
        }
        painter.drawText(QRectF(x - 18.0, plot.bottom() + 4.0, barWidth + 36.0, 38.0),
                         Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
                         label);
    }
}

} // namespace gts
