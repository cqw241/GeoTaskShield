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
    bool hasMarkdownActionsForTesting() const;
    QString markdownReportForTesting() const;
    bool exportMarkdownForTesting(const QString& filePath) const;
    bool hasFilteredCsvExportActionForTesting() const;
    QString filteredCsvForTesting() const;
    bool exportFilteredCsvForTesting(const QString& filePath) const;
    std::vector<BatchResultRecord> currentFilteredRecords() const;

private:
    void openCsv();
    void exportFilteredCsv();
    void previewMarkdown();
    void exportMarkdown();
    void resetFilters();
    void rebuildFilterCombos();
    void refreshView();
    void populateTable(const std::vector<BatchResultRecord>& rows);
    void updateSummaryCards();
    void updateDetailPanelForRow(int row);

    BatchResultMetric selectedMetric() const;
    BatchResultSortField sortFieldForColumn(int column) const;
    int columnForFieldName(const QString& fieldName) const;
    std::string currentMarkdownReport() const;
    std::string currentCsvReport() const;
    bool writeMarkdownToFile(const QString& filePath) const;
    bool writeCsvToFile(const QString& filePath) const;

    BatchResultModel model_;
    std::vector<BatchResultRecord> visibleRecords_;
    std::vector<ChartBar> currentBars_;

    QPushButton* openButton_{};
    QPushButton* exportFilteredCsvButton_{};
    QPushButton* previewMarkdownButton_{};
    QPushButton* exportMarkdownButton_{};
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
