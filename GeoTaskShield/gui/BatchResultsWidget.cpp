#include "gui/BatchResultsWidget.h"

#include "experiment/BatchResultCsvLoader.h"
#include "gui/MetricBarChart.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
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
#include <cctype>
#include <iomanip>
#include <set>
#include <sstream>

namespace gts {

namespace {

struct TableHeaderSpec {
    const char* label;
    const char* fieldName;
    int initialWidth;
};

constexpr TableHeaderSpec kTableHeaders[] = {
    {"Scenario", "scenario", 180},
    {"Workers", "workers", 78},
    {"Tasks", "tasks", 72},
    {"Privacy", "privacy", 150},
    {"Algorithm", "algorithm", 135},
    {"Completion", "completionRate", 104},
    {"Avg Distance", "averageTrueDistance", 116},
    {"Total Reward", "totalReward", 112},
    {"Privacy Loss", "averagePrivacyLoss", 116},
    {"Runtime", "runtimeMs", 92},
    {"Load StdDev", "userLoadStdDev", 108},
    {"Fairness", "fairnessIndex", 98},
    {"Privacy/Utility", "privacyUtilityRatio", 128},
    {"Timeout", "timeoutRate", 88}
};

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

bool lowerMetricIsBetter(BatchResultMetric metric)
{
    return metric == BatchResultMetric::AverageTrueDistance ||
           metric == BatchResultMetric::AveragePrivacyLoss ||
           metric == BatchResultMetric::RuntimeMs ||
           metric == BatchResultMetric::UserLoadStdDev ||
           metric == BatchResultMetric::TimeoutRate;
}

std::string lowerCopy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string privacyAbbreviation(const std::string& privacy)
{
    const std::string lower = lowerCopy(privacy);
    if (lower.find("laplace") != std::string::npos) {
        return "lap";
    }
    if (lower.find("anonymity") != std::string::npos) {
        return "k-anon";
    }
    if (lower.find("grid") != std::string::npos) {
        return "grid";
    }
    return privacy;
}

std::string algorithmAbbreviation(const std::string& algorithm)
{
    const std::string lower = lowerCopy(algorithm);
    if (lower.find("nearest") != std::string::npos) {
        return "near";
    }
    if (lower.find("score") != std::string::npos) {
        return "score";
    }
    if (lower.find("hungarian") != std::string::npos) {
        return "hung";
    }
    return algorithm;
}

std::string stressProfileLabel(const std::string& scenario)
{
    if (scenario.find("profile-worker-shortage") != std::string::npos) {
        return "short";
    }
    if (scenario.find("profile-deadline-tight") != std::string::npos) {
        return "dead";
    }
    if (scenario.find("profile-high-privacy-noise") != std::string::npos) {
        return "priv";
    }
    if (scenario.find("profile-reward-skew") != std::string::npos) {
        return "reward";
    }
    if (scenario.find("profile-heterogeneous-speed") != std::string::npos) {
        return "hetero";
    }
    return {};
}

std::string formatCompactDouble(double value)
{
    std::ostringstream output;
    output << std::setprecision(4) << value;
    return output.str();
}

std::string chartLabel(const BatchResultRecord& record)
{
    const std::string profile = stressProfileLabel(record.scenario);
    if (!profile.empty()) {
        std::ostringstream label;
        label << profile << ' ' << privacyAbbreviation(record.privacy)
              << ' ' << algorithmAbbreviation(record.algorithm)
              << "\ng" << formatCompactDouble(record.gridSize)
              << " k" << record.k
              << " e" << formatCompactDouble(record.epsilon);
        return label.str();
    }

    if (record.scenario.size() <= 32) {
        return record.scenario;
    }

    std::ostringstream label;
    label << record.scenario.substr(0, 29) << "...";
    return label.str();
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
    exportFilteredCsvButton_ = new QPushButton("Export Filtered CSV", this);
    exportFilteredCsvButton_->setObjectName("exportFilteredCsvButton");
    previewMarkdownButton_ = new QPushButton("Preview Markdown", this);
    previewMarkdownButton_->setObjectName("previewMarkdownButton");
    exportMarkdownButton_ = new QPushButton("Export Markdown", this);
    exportMarkdownButton_->setObjectName("exportMarkdownButton");
    resetButton_ = new QPushButton("Reset filters", this);
    fileLabel_ = new QLabel("No CSV loaded", this);
    fileLabel_->setWordWrap(true);

    privacyCombo_ = new QComboBox(this);
    algorithmCombo_ = new QComboBox(this);
    metricCombo_ = new QComboBox(this);
    chartLimitCombo_ = new QComboBox(this);
    addMetric(metricCombo_, BatchResultMetric::CompletionRate);
    addMetric(metricCombo_, BatchResultMetric::AverageTrueDistance);
    addMetric(metricCombo_, BatchResultMetric::TotalReward);
    addMetric(metricCombo_, BatchResultMetric::AveragePrivacyLoss);
    addMetric(metricCombo_, BatchResultMetric::RuntimeMs);
    addMetric(metricCombo_, BatchResultMetric::UserLoadStdDev);
    addMetric(metricCombo_, BatchResultMetric::FairnessIndex);
    addMetric(metricCombo_, BatchResultMetric::PrivacyUtilityRatio);
    addMetric(metricCombo_, BatchResultMetric::TimeoutRate);
    chartLimitCombo_->setObjectName("chartLimitCombo");
    chartLimitCombo_->addItem("Top 12", 12);
    chartLimitCombo_->addItem("Top 24", 24);
    chartLimitCombo_->addItem("Top 40", 40);
    chartLimitCombo_->addItem("All", -1);
    chartLimitCombo_->setCurrentIndex(1);

    filterForm->addRow("Privacy", privacyCombo_);
    filterForm->addRow("Algorithm", algorithmCombo_);
    filterForm->addRow("Metric", metricCombo_);
    filterForm->addRow("Chart rows", chartLimitCombo_);

    leftLayout->addWidget(openButton_);
    leftLayout->addWidget(fileLabel_);
    leftLayout->addLayout(filterForm);
    leftLayout->addWidget(resetButton_);
    leftLayout->addWidget(exportFilteredCsvButton_);
    leftLayout->addWidget(previewMarkdownButton_);
    leftLayout->addWidget(exportMarkdownButton_);
    leftLayout->addStretch();
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
    table_->setColumnCount(static_cast<int>(std::size(kTableHeaders)));
    for (int column = 0; column < table_->columnCount(); ++column) {
        const TableHeaderSpec& spec = kTableHeaders[static_cast<std::size_t>(column)];
        auto* headerItem = new QTableWidgetItem(spec.label);
        headerItem->setToolTip(spec.fieldName);
        headerItem->setData(Qt::UserRole, spec.fieldName);
        table_->setHorizontalHeaderItem(column, headerItem);
        table_->setColumnWidth(column, spec.initialWidth);
    }
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table_->horizontalHeader()->setMinimumSectionSize(72);
    table_->horizontalHeader()->setStretchLastSection(false);
    table_->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
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
    connect(exportFilteredCsvButton_, &QPushButton::clicked, this, [this]() {
        exportFilteredCsv();
    });
    connect(previewMarkdownButton_, &QPushButton::clicked, this, [this]() {
        previewMarkdown();
    });
    connect(exportMarkdownButton_, &QPushButton::clicked, this, [this]() {
        exportMarkdown();
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
    connect(chartLimitCombo_, &QComboBox::currentIndexChanged, this, [this]() {
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

int BatchResultsWidget::chartXAxisLabelCountForTesting() const
{
    return chart_ ? static_cast<int>(chart_->xAxisLabelCountForTesting()) : 0;
}

QStringList BatchResultsWidget::chartLabelsForTesting() const
{
    QStringList labels;
    for (const ChartBar& bar : currentBars_) {
        labels.push_back(QString::fromStdString(bar.label));
    }
    return labels;
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

bool BatchResultsWidget::hasMarkdownActionsForTesting() const
{
    return previewMarkdownButton_ != nullptr && exportMarkdownButton_ != nullptr;
}

QString BatchResultsWidget::markdownReportForTesting() const
{
    return QString::fromStdString(currentMarkdownReport());
}

bool BatchResultsWidget::exportMarkdownForTesting(const QString& filePath) const
{
    return writeMarkdownToFile(filePath);
}

bool BatchResultsWidget::hasFilteredCsvExportActionForTesting() const
{
    return exportFilteredCsvButton_ != nullptr;
}

QString BatchResultsWidget::filteredCsvForTesting() const
{
    return QString::fromStdString(currentCsvReport());
}

bool BatchResultsWidget::exportFilteredCsvForTesting(const QString& filePath) const
{
    return writeCsvToFile(filePath);
}

std::vector<BatchResultRecord> BatchResultsWidget::currentFilteredRecords() const
{
    return visibleRecords_;
}

QString BatchResultsWidget::tableHeaderTextForTesting(int column) const
{
    if (column < 0 || column >= table_->columnCount()) {
        return {};
    }
    return table_->horizontalHeaderItem(column)->text();
}

QString BatchResultsWidget::tableHeaderToolTipForTesting(int column) const
{
    if (column < 0 || column >= table_->columnCount()) {
        return {};
    }
    return table_->horizontalHeaderItem(column)->toolTip();
}

void BatchResultsWidget::openCsv()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this, "Open Batch Results CSV", QString(), "CSV files (*.csv);;All files (*.*)");
    if (!filePath.isEmpty()) {
        loadCsvFile(filePath);
    }
}

void BatchResultsWidget::exportFilteredCsv()
{
    if (visibleRecords_.empty()) {
        QMessageBox::information(this, "No export data",
                                 "Load a CSV or adjust filters before exporting CSV.");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Filtered CSV", "filtered-batch-results.csv",
        "CSV files (*.csv);;All files (*.*)");
    if (filePath.isEmpty()) {
        return;
    }
    if (!filePath.endsWith(".csv", Qt::CaseInsensitive)) {
        filePath += ".csv";
    }

    if (!writeCsvToFile(filePath)) {
        QMessageBox::warning(this, "Export failed",
                             "Could not write the filtered CSV to the selected file.");
        return;
    }
    QMessageBox::information(this, "Export complete", "Filtered CSV exported.");
}

void BatchResultsWidget::previewMarkdown()
{
    if (visibleRecords_.empty()) {
        QMessageBox::information(this, "No report data",
                                 "Load a CSV or adjust filters before previewing Markdown.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Markdown Report Preview");
    auto* layout = new QVBoxLayout(&dialog);
    auto* preview = new QTextEdit(&dialog);
    preview->setReadOnly(true);
    preview->setMarkdown(QString::fromStdString(currentMarkdownReport()));
    layout->addWidget(preview);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.resize(760, 560);
    dialog.exec();
}

void BatchResultsWidget::exportMarkdown()
{
    if (visibleRecords_.empty()) {
        QMessageBox::information(this, "No report data",
                                 "Load a CSV or adjust filters before exporting Markdown.");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Markdown Report", "batch-results-report.md",
        "Markdown files (*.md);;All files (*.*)");
    if (filePath.isEmpty()) {
        return;
    }
    if (!filePath.endsWith(".md", Qt::CaseInsensitive)) {
        filePath += ".md";
    }

    if (!writeMarkdownToFile(filePath)) {
        QMessageBox::warning(this, "Export failed",
                             "Could not write the Markdown report to the selected file.");
        return;
    }
    QMessageBox::information(this, "Export complete", "Markdown report exported.");
}

void BatchResultsWidget::resetFilters()
{
    privacyCombo_->setCurrentIndex(0);
    algorithmCombo_->setCurrentIndex(0);
    metricCombo_->setCurrentIndex(0);
    chartLimitCombo_->setCurrentIndex(1);
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
    const BatchResultMetric metric = selectedMetric();
    std::vector<BatchResultRecord> chartRecords = visibleRecords_;
    std::stable_sort(chartRecords.begin(), chartRecords.end(),
                     [metric](const BatchResultRecord& lhs,
                              const BatchResultRecord& rhs) {
                         if (lowerMetricIsBetter(metric)) {
                             return metricValue(lhs, metric) < metricValue(rhs, metric);
                         }
                         return metricValue(rhs, metric) < metricValue(lhs, metric);
                     });

    const int chartLimit = chartLimitCombo_->currentData().toInt();
    if (chartLimit > 0 &&
        chartRecords.size() > static_cast<std::size_t>(chartLimit)) {
        chartRecords.resize(static_cast<std::size_t>(chartLimit));
    }

    currentBars_.clear();
    currentBars_.reserve(chartRecords.size());
    for (const BatchResultRecord& record : chartRecords) {
        currentBars_.push_back(ChartBar{chartLabel(record), metricValue(record, metric)});
    }
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
        const QTableWidgetItem* headerItem = table_->horizontalHeaderItem(column);
        if (headerItem->text() == fieldName ||
            headerItem->data(Qt::UserRole).toString() == fieldName) {
            return column;
        }
    }
    return -1;
}

std::string BatchResultsWidget::currentMarkdownReport() const
{
    return model_.markdownReport();
}

std::string BatchResultsWidget::currentCsvReport() const
{
    return model_.csvReport();
}

bool BatchResultsWidget::writeMarkdownToFile(const QString& filePath) const
{
    if (filePath.isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    const QByteArray content = QString::fromStdString(currentMarkdownReport()).toUtf8();
    return file.write(content) == content.size();
}

bool BatchResultsWidget::writeCsvToFile(const QString& filePath) const
{
    if (filePath.isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    const QByteArray content = QString::fromStdString(currentCsvReport()).toUtf8();
    return file.write(content) == content.size();
}

} // namespace gts
