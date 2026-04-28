#pragma once

#include "experiment/BatchResultRecord.h"

#include <QWidget>

#include <cstddef>
#include <vector>

namespace gts {

class MetricBarChart : public QWidget {
public:
    explicit MetricBarChart(QWidget* parent = nullptr);

    void setBars(std::vector<ChartBar> bars);
    std::size_t barCount() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<ChartBar> bars_;
};

} // namespace gts
