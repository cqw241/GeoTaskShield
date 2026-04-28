#include "gui/BatchResultsWidget.h"

#include "experiment/BatchResultCsvLoader.h"
#include "gui/MetricBarChart.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QVariant>

#include <algorithm>
#include <iomanip>
#include <set>
#include <sstream>

namespace gts {

namespace {

class NumericTableWidgetItem : public QTableWidgetItem {
public:
    explicit NumericTableWidgetItem(double value)
        : QTableWidgetItem(QString::number(value, 'f', 4))
    {
        setData(Qt::UserRole, value);
        setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    bool operator<(const QTableWidgetItem& other) const override
    {
        const QVariant lhs = data(Qt::UserRole);
        const QVariant rhs = other.data(Qt::UserRole);
        if (lhs.isValid() && rhs.isValid()) {
            return lhs.toDouble() < rhs.toDouble();
        }
        return QTableWidgetItem::operator<(other);
    }
};

QLabel* makeSummaryLabel(QWidget* parent)
{
    auto* label = new QLabel("-", parent);
    label->setMinimumWidth(160);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    return label;
}

QString sourceText(const BatchResultRecord& record)
{
    return QString::fromStdString(record.scenario + " | " + record.privacy + " | " +
                                  record.algorithm);
}

QString summaryText(const std::optional<BatchResultSummary>& summary)
{
    if (!summary) {
        return "-";
    }
    return QString("%1\n%2")
        .arg(summary->value, 0, 'f', 4)
        .arg(sourceText(summary->record));
}

QString metricLabel(BatchResultMetric metric)
{
    switch (metric) {
    case BatchResultMetric::CompletionRate:
        return "completionRate";
    case BatchResultMetric::AverageTrueDistance:
        return "averageTrueDistance";
    case BatchResultMetric::TotalReward:
        return "totalReward";
    case BatchResultMetric::AveragePrivacyLoss:
        return "averagePrivacyLoss";
    case BatchResultMetric::RuntimeMs:
        return "runtimeMs";
    case BatchResultMetric::UserLoadStdDev:
        return "userLoadStdDev";
    case BatchResultMetric::FairnessIndex:
        return "fairnessIndex";
    case BatchResultMetric::PrivacyUtilityRatio:
        return "privacyUtilityRatio";
    case BatchResultMetric::TimeoutRate:
        return "timeoutRate";
    }
    return "completionRate";
}

double metricValue(const BatchResultRecord& record, BatchResultMetric metric)
{
    switch (metric) {
    case BatchResultMetric::CompletionRate:
        return record.completionRate;
    case BatchResultMetric::AverageTrueDistance:
        return record.averageTrueDistance;
    case BatchResultMetric::TotalReward:
        return record.totalReward;
    case BatchResultMetric::AveragePrivacyLoss:
        return record.averagePrivacyLoss;
    case BatchResultMetric::RuntimeMs:
        return record.runtimeMs;
    case BatchResultMetric::UserLoadStdDev:
        return record.userLoadStdDev;
    case BatchResultMetric::FairnessIndex:
        return record.fairnessIndex;
    case BatchResultMetric::PrivacyUtilityRatio:
        return record.privacyUtilityRatio;
    case BatchResultMetric::TimeoutRate:
        return record.timeoutRate;
    }
    return 0.0;
}

void addMetric(QComboBox* combo, BatchResultMetric metric)
{
    combo->addItem(metricLabel(metric), static_cast<int>(metric));
}

QTableWidgetItem* textItem(const std::string& value)
{
    return new QTableWidgetItem(QString::fromStdString(value));
}

QTableWidgetItem* intItem(int value)
{
    return new NumericTableWidgetItem(static_cast<double>(value));
}

QTableWidgetItem* doubleItem(double value)
{
    return new NumericTableWidgetItem(value);
}

} // namespace

BatchResultsWidget::BatchResultsWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    auto* content = new QGridLayout;
    auto* leftPanel = new QWidget(this);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    auto* filterForm = new QFormLayout;

    openButton_ = new QPushButton("Open CSV", this);
    resetButton_ = new QPushButton("Reset filters", this);
    fileLabel_ = new QLabel("No CSV loaded", this);
    fileLabel_->setWordWrap(true);

    privacyCombo_ = new QComboBox(this);
    algorithmCombo_ = new QComboBox(this);
    metricCombo_ = new QComboBox(this);
    addMetric(metricCombo_, BatchResultMetric::CompletionRate);
    addMetric(metricCombo_, BatchResultMetric::AverageTrueDistance);
    addMetric(metricCombo_, BatchResultMetric::TotalReward);
    addMetric(metricCombo_, BatchResultMetric::AveragePrivacyLoss);
    addMetric(metricCombo_, BatchResultMetric::RuntimeMs);
    addMetric(metricCombo_, BatchResultMetric::UserLoadStdDev);
    addMetric(metricCombo_, BatchResultMetric::FairnessIndex);
    addMetric(metricCombo_, BatchResultMetric::PrivacyUtilityRatio);
    addMetric(metricCombo_, BatchResultMetric::TimeoutRate);

    filterForm->addRow("Privacy", privacyCombo_);
    filterForm->addRow("Algorithm", algorithmCombo_);
    filterForm->addRow("Metric", metricCombo_);

    leftLayout->addWidget(openButton_);
    leftLayout->addWidget(fileLabel_);
    leftLayout->addLayout(filterForm);
    leftLayout->addStretch();
    leftLayout->addWidget(resetButton_);
    leftPanel->setMinimumWidth(240);
    leftPanel->setMaximumWidth(320);

    auto* summary = new QWidget(this);
    auto* summaryLayout = new QGridLayout(summary);
    bestCompletionLabel_ = makeSummaryLabel(this);
    bestUtilityLabel_ = makeSummaryLabel(this);
    bestFairnessLabel_ = makeSummaryLabel(this);
    lowestPrivacyLossLabel_ = makeSummaryLabel(this);
    summaryLayout->addWidget(new QLabel("Best completion", this), 0, 0);
    summaryLayout->addWidget(new QLabel("Best privacy-utility", this), 0, 1);
    summaryLayout->addWidget(new QLabel("Best fairness", this), 0, 2);
    summaryLayout->addWidget(new QLabel("Lowest privacy loss", this), 0, 3);
    summaryLayout->addWidget(bestCompletionLabel_, 1, 0);
    summaryLayout->addWidget(bestUtilityLabel_, 1, 1);
    summaryLayout->addWidget(bestFairnessLabel_, 1, 2);
    summaryLayout->addWidget(lowestPrivacyLossLabel_, 1, 3);

    chart_ = new MetricBarChart(this);
    detailText_ = new QTextEdit(this);
    detailText_->setReadOnly(true);
    detailText_->setMinimumWidth(280);
    detailText_->setText("No row selected.");

    table_ = new QTableWidget(this);
    table_->setColumnCount(14);
    table_->setHorizontalHeaderLabels({
        "scenario",
        "workers",
        "tasks",
        "privacy",
        "algorithm",
        "completionRate",
        "averageTrueDistance",
        "totalReward",
        "averagePrivacyLoss",
        "runtimeMs",
        "userLoadStdDev",
        "fairnessIndex",
        "privacyUtilityRatio",
        "timeoutRate"
    });
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setSortingEnabled(true);

    auto* mainPanel = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(mainPanel);
    auto* chartAndDetail = new QGridLayout;
    chartAndDetail->addWidget(chart_, 0, 0);
    chartAndDetail->addWidget(detailText_, 0, 1);
    chartAndDetail->setColumnStretch(0, 1);

    mainLayout->addWidget(summary);
    mainLayout->addLayout(chartAndDetail);
    mainLayout->addWidget(table_, 1);

    content->addWidget(leftPanel, 0, 0);
    content->addWidget(mainPanel, 0, 1);
    content->setColumnStretch(1, 1);
    root->addLayout(content);

    rebuildFilterCombos();
    updateSummaryCards();
    chart_->setBars({});

    connect(openButton_, &QPushButton::clicked, this, [this]() {
        openCsv();
    });
    connect(resetButton_, &QPushButton::clicked, this, [this]() {
        resetFilters();
    });
    connect(privacyCombo_, &QComboBox::currentIndexChanged, this, [this]() {
        refreshView();
    });
    connect(algorithmCombo_, &QComboBox::currentIndexChanged, this, [this]() {
        refreshView();
    });
    connect(metricCombo_, &QComboBox::currentIndexChanged, this, [this]() {
        refreshView();
    });
    connect(table_, &QTableWidget::currentCellChanged, this, [this](int currentRow) {
        updateDetailPanelForRow(currentRow);
    });
}

bool BatchResultsWidget::loadCsvFile(const QString& filePath)
{
    const BatchResultLoadResult result =
        BatchResultCsvLoader::loadFromFile(filePath.toStdString());
    if (!result.success) {
        QMessageBox::warning(this, "CSV load failed",
                             QString::fromStdString(result.errorMessage));
        return false;
    }

    model_.setRecords(result.records);
    fileLabel_->setText(filePath);
    rebuildFilterCombos();
    refreshView();
    return true;
}

int BatchResultsWidget::visibleRowCountForTesting() const
{
    return table_->rowCount();
}

int BatchResultsWidget::chartBarCountForTesting() const
{
    return static_cast<int>(currentBars_.size());
}

QString BatchResultsWidget::firstScenarioForTesting() const
{
    if (table_->rowCount() == 0 || !table_->item(0, 0)) {
        return {};
    }
    return table_->item(0, 0)->text();
}

void BatchResultsWidget::sortByColumnForTesting(const QString& fieldName, Qt::SortOrder order)
{
    const int column = columnForFieldName(fieldName);
    if (column >= 0) {
        table_->sortItems(column, order);
    }
}

void BatchResultsWidget::openCsv()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this, "Open Batch Results CSV", QString(), "CSV files (*.csv);;All files (*.*)");
    if (!filePath.isEmpty()) {
        loadCsvFile(filePath);
    }
}

void BatchResultsWidget::resetFilters()
{
    privacyCombo_->setCurrentIndex(0);
    algorithmCombo_->setCurrentIndex(0);
    metricCombo_->setCurrentIndex(0);
    refreshView();
}

void BatchResultsWidget::rebuildFilterCombos()
{
    const QSignalBlocker privacyBlocker(privacyCombo_);
    const QSignalBlocker algorithmBlocker(algorithmCombo_);

    privacyCombo_->clear();
    algorithmCombo_->clear();
    privacyCombo_->addItem("All", "");
    algorithmCombo_->addItem("All", "");

    std::set<std::string> privacyNames;
    std::set<std::string> algorithmNames;
    for (const BatchResultRecord& record : model_.records()) {
        privacyNames.insert(record.privacy);
        algorithmNames.insert(record.algorithm);
    }

    for (const std::string& privacy : privacyNames) {
        privacyCombo_->addItem(QString::fromStdString(privacy), QString::fromStdString(privacy));
    }
    for (const std::string& algorithm : algorithmNames) {
        algorithmCombo_->addItem(QString::fromStdString(algorithm),
                                 QString::fromStdString(algorithm));
    }
}

void BatchResultsWidget::refreshView()
{
    model_.setPrivacyFilter(privacyCombo_->currentData().toString().toStdString());
    model_.setAlgorithmFilter(algorithmCombo_->currentData().toString().toStdString());
    visibleRecords_ = model_.filteredRecords();
    currentBars_ = model_.chartBars(selectedMetric());
    chart_->setBars(currentBars_);
    updateSummaryCards();
    populateTable(visibleRecords_);
}

void BatchResultsWidget::populateTable(const std::vector<BatchResultRecord>& rows)
{
    table_->setSortingEnabled(false);
    table_->clearContents();
    table_->setRowCount(static_cast<int>(rows.size()));

    for (int row = 0; row < static_cast<int>(rows.size()); ++row) {
        const BatchResultRecord& record = rows[static_cast<std::size_t>(row)];
        table_->setItem(row, 0, textItem(record.scenario));
        table_->setItem(row, 1, intItem(record.workerCount));
        table_->setItem(row, 2, intItem(record.taskCount));
        table_->setItem(row, 3, textItem(record.privacy));
        table_->setItem(row, 4, textItem(record.algorithm));
        table_->setItem(row, 5, doubleItem(record.completionRate));
        table_->setItem(row, 6, doubleItem(record.averageTrueDistance));
        table_->setItem(row, 7, doubleItem(record.totalReward));
        table_->setItem(row, 8, doubleItem(record.averagePrivacyLoss));
        table_->setItem(row, 9, doubleItem(record.runtimeMs));
        table_->setItem(row, 10, doubleItem(record.userLoadStdDev));
        table_->setItem(row, 11, doubleItem(record.fairnessIndex));
        table_->setItem(row, 12, doubleItem(record.privacyUtilityRatio));
        table_->setItem(row, 13, doubleItem(record.timeoutRate));
    }

    table_->setSortingEnabled(true);
    if (table_->rowCount() > 0) {
        table_->selectRow(0);
        updateDetailPanelForRow(0);
    } else {
        detailText_->setText("No row selected.");
    }
}

void BatchResultsWidget::updateSummaryCards()
{
    bestCompletionLabel_->setText(summaryText(model_.bestCompletionRate()));
    bestUtilityLabel_->setText(summaryText(model_.bestPrivacyUtilityRatio()));
    bestFairnessLabel_->setText(summaryText(model_.bestFairnessIndex()));
    lowestPrivacyLossLabel_->setText(summaryText(model_.lowestAveragePrivacyLoss()));
}

void BatchResultsWidget::updateDetailPanelForRow(int row)
{
    if (row < 0 || row >= table_->rowCount()) {
        detailText_->setText("No row selected.");
        return;
    }

    std::ostringstream detail;
    detail << "Scenario: " << table_->item(row, 0)->text().toStdString() << "\n"
           << "Workers: " << table_->item(row, 1)->text().toStdString() << "\n"
           << "Tasks: " << table_->item(row, 2)->text().toStdString() << "\n"
           << "Privacy: " << table_->item(row, 3)->text().toStdString() << "\n"
           << "Algorithm: " << table_->item(row, 4)->text().toStdString() << "\n\n"
           << "completionRate: " << table_->item(row, 5)->text().toStdString() << "\n"
           << "averageTrueDistance: " << table_->item(row, 6)->text().toStdString() << "\n"
           << "totalReward: " << table_->item(row, 7)->text().toStdString() << "\n"
           << "averagePrivacyLoss: " << table_->item(row, 8)->text().toStdString() << "\n"
           << "runtimeMs: " << table_->item(row, 9)->text().toStdString() << "\n"
           << "userLoadStdDev: " << table_->item(row, 10)->text().toStdString() << "\n"
           << "fairnessIndex: " << table_->item(row, 11)->text().toStdString() << "\n"
           << "privacyUtilityRatio: " << table_->item(row, 12)->text().toStdString() << "\n"
           << "timeoutRate: " << table_->item(row, 13)->text().toStdString();
    detailText_->setText(QString::fromStdString(detail.str()));
}

BatchResultMetric BatchResultsWidget::selectedMetric() const
{
    return static_cast<BatchResultMetric>(metricCombo_->currentData().toInt());
}

BatchResultSortField BatchResultsWidget::sortFieldForColumn(int column) const
{
    switch (column) {
    case 0:
        return BatchResultSortField::Scenario;
    case 1:
        return BatchResultSortField::WorkerCount;
    case 2:
        return BatchResultSortField::TaskCount;
    case 3:
        return BatchResultSortField::Privacy;
    case 4:
        return BatchResultSortField::Algorithm;
    case 5:
        return BatchResultSortField::CompletionRate;
    case 6:
        return BatchResultSortField::AverageTrueDistance;
    case 7:
        return BatchResultSortField::TotalReward;
    case 8:
        return BatchResultSortField::AveragePrivacyLoss;
    case 9:
        return BatchResultSortField::RuntimeMs;
    case 10:
        return BatchResultSortField::UserLoadStdDev;
    case 11:
        return BatchResultSortField::FairnessIndex;
    case 12:
        return BatchResultSortField::PrivacyUtilityRatio;
    case 13:
        return BatchResultSortField::TimeoutRate;
    default:
        return BatchResultSortField::Scenario;
    }
}

int BatchResultsWidget::columnForFieldName(const QString& fieldName) const
{
    for (int column = 0; column < table_->columnCount(); ++column) {
        if (table_->horizontalHeaderItem(column)->text() == fieldName) {
            return column;
        }
    }
    return -1;
}

} // namespace gts
