#pragma once

#include "experiment/BatchResultModel.h"
#include "experiment/BatchResultRecord.h"

#include <QString>
#include <QWidget>

#include <vector>

class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTextEdit;

namespace gts {

class MetricBarChart;

class BatchResultsWidget : public QWidget {
    Q_OBJECT

public:
    explicit BatchResultsWidget(QWidget* parent = nullptr);

    bool loadCsvFile(const QString& filePath);
    int visibleRowCountForTesting() const;
    int chartBarCountForTesting() const;
    QString firstScenarioForTesting() const;
    void sortByColumnForTesting(const QString& fieldName, Qt::SortOrder order);

private:
    void openCsv();
    void resetFilters();
    void rebuildFilterCombos();
    void refreshView();
    void populateTable(const std::vector<BatchResultRecord>& rows);
    void updateSummaryCards();
    void updateDetailPanelForRow(int row);

    BatchResultMetric selectedMetric() const;
    BatchResultSortField sortFieldForColumn(int column) const;
    int columnForFieldName(const QString& fieldName) const;

    BatchResultModel model_;
    std::vector<BatchResultRecord> visibleRecords_;
    std::vector<ChartBar> currentBars_;

    QPushButton* openButton_{};
    QPushButton* resetButton_{};
    QLabel* fileLabel_{};
    QComboBox* privacyCombo_{};
    QComboBox* algorithmCombo_{};
    QComboBox* metricCombo_{};
    QLabel* bestCompletionLabel_{};
    QLabel* bestUtilityLabel_{};
    QLabel* bestFairnessLabel_{};
    QLabel* lowestPrivacyLossLabel_{};
    MetricBarChart* chart_{};
    QTextEdit* detailText_{};
    QTableWidget* table_{};
};

} // namespace gts
