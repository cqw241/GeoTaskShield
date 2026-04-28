# Phase 7 GUI Batch Results Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a `Batch Results` Qt GUI tab that loads existing Phase 5-style CSV files, filters/sorts results, displays summary records, and draws a single-metric bar chart.

**Architecture:** Keep all data parsing and analysis in Qt-free C++ files under `GeoTaskShield/experiment/`, linked into `GeoTaskShieldCore` and covered by non-Qt tests. Keep Qt-specific UI in `GeoTaskShield/gui/` with `BatchResultsWidget` and `MetricBarChart`, integrated into `MainWindow` through a `QTabWidget`.

**Tech Stack:** C++20, CMake, existing custom assertion tests, Qt Widgets, no Qt Charts.

---

## File Map

- Create `GeoTaskShield/experiment/BatchResultRecord.h`
  - Defines metric enum, `BatchResultRecord`, summary result, and chart bar value types.
- Create `GeoTaskShield/experiment/BatchResultCsvLoader.h/.cpp`
  - Reads UTF-8 CSV, supports BOM/CRLF/LF/quoted fields, maps current snake_case headers and aliases, returns records or errors.
- Create `GeoTaskShield/experiment/BatchResultModel.h/.cpp`
  - Owns records, applies privacy/algorithm filters, numeric sort, summary calculations, and chart bar generation.
- Modify `GeoTaskShield/tests/test_core.cpp`
  - Adds tests for loader and model behavior before implementation.
- Modify `GeoTaskShield/CMakeLists.txt`
  - Adds new experiment source files to `GeoTaskShieldCore`.
- Create `GeoTaskShield/gui/MetricBarChart.h/.cpp`
  - Lightweight QWidget that owns `std::vector<ChartBar>` and draws selected metric values.
- Create `GeoTaskShield/gui/BatchResultsWidget.h/.cpp`
  - Qt page with CSV open button, filters, metric selector, summary cards, detail panel, chart, and sortable table.
- Modify `GeoTaskShield/gui/MainWindow.h/.cpp`
  - Adds `QTabWidget`, preserves existing Simulation layout as first tab, adds `Batch Results` tab.
- Modify `GeoTaskShield/gui/tests/test_gui_smoke.cpp`
  - Checks Simulation still runs, Batch Results tab exists, test CSV loads, table/chart state becomes non-empty, and numeric sorting behaves correctly.
- Modify docs/planning files after implementation verification.

## Task 1: Confirm CSV Header and Add Loader Red Tests

**Files:**
- Modify: `GeoTaskShield/tests/test_core.cpp`
- Create later: `GeoTaskShield/experiment/BatchResultRecord.h`
- Create later: `GeoTaskShield/experiment/BatchResultCsvLoader.h/.cpp`

- [ ] **Step 1: Re-read the real Phase 5 CSV header**

Run:

```powershell
Get-Content -First 1 -Encoding UTF8 .\phase5_batch_results.csv
```

Expected header:

```text
scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,completed_tasks,total_tasks,completion_rate,average_moving_distance,total_reward,average_privacy_loss,algorithm_runtime_ms,user_load_stddev,fairness_index,privacy_utility_ratio,timeout_rate
```

- [ ] **Step 2: Write failing loader tests**

Add includes near the existing experiment includes:

```cpp
#include "experiment/BatchResultCsvLoader.h"
#include "experiment/BatchResultModel.h"
#include "experiment/BatchResultRecord.h"
```

Add helper functions in the anonymous namespace:

```cpp
std::string writeTempCsv(const std::string& name, const std::string& content)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / name;
    std::ofstream file(path, std::ios::binary);
    file << content;
    return path.string();
}
```

Also add required standard includes:

```cpp
#include <filesystem>
#include <fstream>
```

Add tests inside `main()` before the final `SimulationEngine` smoke block:

```cpp
const std::string validBatchCsv =
    "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,"
    "completed_tasks,total_tasks,completion_rate,average_moving_distance,"
    "total_reward,average_privacy_loss,algorithm_runtime_ms,user_load_stddev,"
    "fairness_index,privacy_utility_ratio,timeout_rate\r\n"
    "\"scenario,quoted\",10,5,10,3,1,Grid Privacy,Nearest Greedy,"
    "5,5,1,12.5,100,2.5,0.4,0.2,0.9,0.28,0\r\n";
const std::string validBatchCsvPath = writeTempCsv("gts_phase7_valid.csv", validBatchCsv);
const BatchResultLoadResult validLoad =
    BatchResultCsvLoader::loadFromFile(validBatchCsvPath);
require(validLoad.success, validLoad.errorMessage);
require(validLoad.records.size() == 1,
        "BatchResultCsvLoader should load one valid row.");
require(validLoad.records[0].scenario == "scenario,quoted",
        "BatchResultCsvLoader should support quoted fields.");
require(near(validLoad.records[0].averageTrueDistance, 12.5),
        "BatchResultCsvLoader should map average_moving_distance.");
require(near(validLoad.records[0].runtimeMs, 0.4),
        "BatchResultCsvLoader should map algorithm_runtime_ms.");

const std::string aliasCsv =
    "\xEF\xBB\xBFscenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,"
    "completed_tasks,total_tasks,completion_rate,average_true_distance,"
    "total_reward,average_privacy_loss,runtimeMs,user_load_stddev,"
    "fairness_index,privacyUtilityRatio,timeout_rate\n"
    "alias,8,3,5,2,0.5,Laplace Noise Privacy,Score Greedy,"
    "3,3,1,4.5,50,1.25,0.7,0.1,0.8,0.44,0.2\n";
const BatchResultLoadResult aliasLoad =
    BatchResultCsvLoader::loadFromFile(writeTempCsv("gts_phase7_alias.csv", aliasCsv));
require(aliasLoad.success, aliasLoad.errorMessage);
require(near(aliasLoad.records[0].averageTrueDistance, 4.5),
        "BatchResultCsvLoader should accept average_true_distance alias.");
require(near(aliasLoad.records[0].runtimeMs, 0.7),
        "BatchResultCsvLoader should accept runtimeMs alias.");
require(near(aliasLoad.records[0].privacyUtilityRatio, 0.44),
        "BatchResultCsvLoader should accept privacyUtilityRatio alias.");

const BatchResultLoadResult missingColumnLoad =
    BatchResultCsvLoader::loadFromFile(writeTempCsv(
        "gts_phase7_missing.csv",
        "scenario,workers\nonly,1\n"));
require(!missingColumnLoad.success,
        "BatchResultCsvLoader should reject missing required columns.");
require(contains(missingColumnLoad.errorMessage, "tasks"),
        "Missing-column errors should include the missing column name.");

const BatchResultLoadResult invalidNumberLoad =
    BatchResultCsvLoader::loadFromFile(writeTempCsv(
        "gts_phase7_invalid.csv",
        "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,"
        "completed_tasks,total_tasks,completion_rate,average_moving_distance,"
        "total_reward,average_privacy_loss,algorithm_runtime_ms,user_load_stddev,"
        "fairness_index,privacy_utility_ratio,timeout_rate\n"
        "bad,not-a-number,5,10,3,1,Grid Privacy,Nearest Greedy,"
        "5,5,1,12.5,100,2.5,0.4,0.2,0.9,0.28,0\n"));
require(!invalidNumberLoad.success,
        "BatchResultCsvLoader should reject invalid numeric cells.");
require(contains(invalidNumberLoad.errorMessage, "workers") &&
            contains(invalidNumberLoad.errorMessage, "not-a-number"),
        "Invalid numeric errors should include row, column, and raw value.");
```

- [ ] **Step 3: Verify RED**

Run:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --build out\build\x64-debug --target GeoTaskShieldTests'
```

Expected: build fails because `experiment/BatchResultCsvLoader.h` does not exist.

## Task 2: Implement Loader and Record Types

**Files:**
- Create: `GeoTaskShield/experiment/BatchResultRecord.h`
- Create: `GeoTaskShield/experiment/BatchResultCsvLoader.h`
- Create: `GeoTaskShield/experiment/BatchResultCsvLoader.cpp`
- Modify: `GeoTaskShield/CMakeLists.txt`

- [ ] **Step 1: Add `BatchResultRecord.h`**

Define:

```cpp
namespace gts {

enum class BatchResultMetric {
    CompletionRate,
    AverageTrueDistance,
    TotalReward,
    AveragePrivacyLoss,
    RuntimeMs,
    UserLoadStdDev,
    FairnessIndex,
    PrivacyUtilityRatio,
    TimeoutRate
};

struct BatchResultRecord {
    std::string scenario;
    int workerCount{};
    int taskCount{};
    double gridSize{};
    int k{};
    double epsilon{};
    std::string privacy;
    std::string algorithm;
    int completedTasks{};
    int totalTasks{};
    double completionRate{};
    double averageTrueDistance{};
    double totalReward{};
    double averagePrivacyLoss{};
    double runtimeMs{};
    double userLoadStdDev{};
    double fairnessIndex{};
    double privacyUtilityRatio{};
    double timeoutRate{};
};

struct ChartBar {
    std::string label;
    double value{};
};

}
```

- [ ] **Step 2: Add loader API**

Define in `BatchResultCsvLoader.h`:

```cpp
namespace gts {

struct BatchResultLoadResult {
    bool success{};
    std::vector<BatchResultRecord> records;
    std::string errorMessage;
};

class BatchResultCsvLoader {
public:
    static BatchResultLoadResult loadFromFile(const std::string& filePath);
};

}
```

- [ ] **Step 3: Implement minimal robust CSV parsing**

Implement in `BatchResultCsvLoader.cpp`:

- read file in binary mode;
- strip UTF-8 BOM if present;
- parse CSV characters into rows with quoted-field and escaped quote support;
- normalize CRLF/LF;
- map required headers through alias lists;
- convert ints/doubles with full-string validation;
- error format examples:
  - `Missing required column: tasks`
  - `Invalid numeric cell at row 2, column workers: not-a-number`

- [ ] **Step 4: Add source to CMake**

Add to `GEOTASKSHIELD_CORE_SOURCES`:

```cmake
"experiment/BatchResultCsvLoader.cpp"
```

- [ ] **Step 5: Verify GREEN for loader**

Run:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --build out\build\x64-debug --target GeoTaskShieldTests && ctest --test-dir out\build\x64-debug --output-on-failure -R GeoTaskShieldCoreTests'
```

Expected: loader tests pass; model tests are not present yet.

## Task 3: Add Model Red Tests and Implementation

**Files:**
- Modify: `GeoTaskShield/tests/test_core.cpp`
- Create: `GeoTaskShield/experiment/BatchResultModel.h/.cpp`
- Modify: `GeoTaskShield/CMakeLists.txt`

- [ ] **Step 1: Add failing model tests**

Add tests using three `BatchResultRecord` values:

```cpp
BatchResultModel model;
model.setRecords({
    BatchResultRecord{"s1", 10, 5, 10.0, 3, 1.0, "Grid Privacy", "Nearest Greedy",
                      5, 5, 1.0, 5.0, 100.0, 2.0, 0.3, 0.4, 0.7, 0.33, 0.0},
    BatchResultRecord{"s1", 10, 5, 10.0, 3, 1.0, "Laplace Noise Privacy", "Score Greedy",
                      4, 5, 0.8, 3.0, 80.0, 1.0, 0.2, 0.2, 0.9, 0.40, 0.1},
    BatchResultRecord{"s2", 20, 8, 5.0, 5, 0.5, "Grid Privacy", "Hungarian",
                      6, 8, 0.75, 8.0, 120.0, 4.0, 0.1, 0.8, 0.6, 0.15, 0.2}
});
model.setPrivacyFilter("Grid Privacy");
require(model.filteredRecords().size() == 2,
        "BatchResultModel should filter by privacy.");
model.setAlgorithmFilter("Hungarian");
require(model.filteredRecords().size() == 1 &&
            model.filteredRecords()[0].scenario == "s2",
        "BatchResultModel should filter by privacy and algorithm.");
model.clearFilters();

const auto completion = model.bestCompletionRate();
require(completion.has_value() &&
            completion->record.scenario == "s1" &&
            near(completion->value, 1.0),
        "BatchResultModel should report best completion rate with source record.");
const auto utility = model.bestPrivacyUtilityRatio();
require(utility.has_value() &&
            utility->record.privacy == "Laplace Noise Privacy",
        "BatchResultModel should report best privacy-utility source record.");
const auto fairness = model.bestFairnessIndex();
require(fairness.has_value() && near(fairness->value, 0.9),
        "BatchResultModel should report best fairness.");
const auto privacyLoss = model.lowestAveragePrivacyLoss();
require(privacyLoss.has_value() && near(privacyLoss->value, 1.0),
        "BatchResultModel should report lowest average privacy loss.");

const std::vector<BatchResultRecord> sorted =
    model.sortedRecords(BatchResultSortField::AveragePrivacyLoss, true);
require(sorted.front().privacy == "Laplace Noise Privacy",
        "BatchResultModel should sort numeric metrics numerically.");
const std::vector<ChartBar> bars =
    model.chartBars(BatchResultMetric::PrivacyUtilityRatio);
require(bars.size() == 3 &&
            contains(bars[0].label, "s1") &&
            contains(bars[0].label, "Grid Privacy") &&
            contains(bars[0].label, "Nearest Greedy"),
        "BatchResultModel should build chart labels from scenario, privacy, and algorithm.");
```

- [ ] **Step 2: Verify RED**

Run the same build command. Expected: fails because `BatchResultModel` APIs do not exist.

- [ ] **Step 3: Implement `BatchResultModel`**

Define:

```cpp
enum class BatchResultSortField {
    Scenario,
    WorkerCount,
    TaskCount,
    Privacy,
    Algorithm,
    CompletionRate,
    AverageTrueDistance,
    TotalReward,
    AveragePrivacyLoss,
    RuntimeMs,
    UserLoadStdDev,
    FairnessIndex,
    PrivacyUtilityRatio,
    TimeoutRate
};

struct BatchResultSummary {
    BatchResultRecord record;
    double value{};
};
```

Methods:

```cpp
void setRecords(std::vector<BatchResultRecord> records);
const std::vector<BatchResultRecord>& records() const;
void setPrivacyFilter(const std::string& privacy);
void setAlgorithmFilter(const std::string& algorithm);
void clearFilters();
std::vector<BatchResultRecord> filteredRecords() const;
std::vector<BatchResultRecord> sortedRecords(BatchResultSortField field,
                                             bool ascending) const;
std::optional<BatchResultSummary> bestCompletionRate() const;
std::optional<BatchResultSummary> bestPrivacyUtilityRatio() const;
std::optional<BatchResultSummary> bestFairnessIndex() const;
std::optional<BatchResultSummary> lowestAveragePrivacyLoss() const;
std::vector<ChartBar> chartBars(BatchResultMetric metric) const;
```

- [ ] **Step 4: Add source to CMake and verify GREEN**

Add:

```cmake
"experiment/BatchResultModel.cpp"
```

Run non-Qt tests. Expected: all core tests pass.

## Task 4: Add GUI Smoke Red Tests

**Files:**
- Modify: `GeoTaskShield/gui/tests/test_gui_smoke.cpp`
- Create later: `GeoTaskShield/gui/BatchResultsWidget.h/.cpp`
- Create later: `GeoTaskShield/gui/MetricBarChart.h/.cpp`

- [ ] **Step 1: Add GUI test expectations**

Add include:

```cpp
#include "gui/BatchResultsWidget.h"
```

Add after `window.runSimulation()`:

```cpp
auto* batchWidget = window.findChild<gts::BatchResultsWidget*>();
require(batchWidget != nullptr,
        "MainWindow should expose a Batch Results tab.");

const std::string csvPath = writeTempCsv(
    "gts_phase7_gui.csv",
    "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,"
    "completed_tasks,total_tasks,completion_rate,average_moving_distance,"
    "total_reward,average_privacy_loss,algorithm_runtime_ms,user_load_stddev,"
    "fairness_index,privacy_utility_ratio,timeout_rate\n"
    "low,1,1,10,1,1,Grid Privacy,Nearest Greedy,1,1,0.2,1,10,3,0.1,0,0.5,0.05,0\n"
    "high,1,1,10,1,1,Grid Privacy,Nearest Greedy,1,1,0.9,1,10,1,0.1,0,0.8,0.45,0\n");
require(batchWidget->loadCsvFile(QString::fromStdString(csvPath)),
        "BatchResultsWidget should load a valid test CSV.");
require(batchWidget->visibleRowCountForTesting() == 2,
        "BatchResultsWidget should show loaded rows.");
batchWidget->sortByColumnForTesting("completionRate", Qt::AscendingOrder);
require(batchWidget->firstScenarioForTesting() == "low",
        "BatchResultsWidget should sort numeric columns numerically.");
require(batchWidget->chartBarCountForTesting() == 2,
        "BatchResultsWidget should expose chart data after loading.");
```

Also add temp CSV helpers and includes if not already present in this file:

```cpp
#include <filesystem>
#include <fstream>
```

- [ ] **Step 2: Verify RED**

Run:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --build out\build\x64-debug-qt --target GeoTaskShieldGuiSmokeTests'
```

Expected: fails because `BatchResultsWidget.h` does not exist.

## Task 5: Implement MetricBarChart and BatchResultsWidget

**Files:**
- Create: `GeoTaskShield/gui/MetricBarChart.h/.cpp`
- Create: `GeoTaskShield/gui/BatchResultsWidget.h/.cpp`
- Modify: `GeoTaskShield/CMakeLists.txt`

- [ ] **Step 1: Implement `MetricBarChart`**

Public API:

```cpp
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
```

Draw behavior:

- fill background;
- draw border/axes;
- if `bars_` is empty, draw `No data`;
- otherwise scale bars by maximum positive visible value;
- draw short labels and numeric values.

- [ ] **Step 2: Implement `BatchResultsWidget` layout and APIs**

Public API:

```cpp
class BatchResultsWidget : public QWidget {
public:
    explicit BatchResultsWidget(QWidget* parent = nullptr);

    bool loadCsvFile(const QString& filePath);
    int visibleRowCountForTesting() const;
    int chartBarCountForTesting() const;
    QString firstScenarioForTesting() const;
    void sortByColumnForTesting(const QString& fieldName, Qt::SortOrder order);

private:
    void openCsv();
    void refreshFilters();
    void refreshView();
    void populateTable(const std::vector<BatchResultRecord>& rows);
    void updateSummaryCards();
    void updateDetailPanel(const BatchResultRecord* record);
};
```

Required widgets:

- `QPushButton` open/reset buttons;
- `QLabel` loaded file label and four summary card labels;
- `QComboBox` privacy filter, algorithm filter, and metric selector;
- `MetricBarChart`;
- `QTableWidget`;
- `QTextEdit` or `QLabel` detail panel.

Behavior:

- `loadCsvFile()` calls `BatchResultCsvLoader`; on failure shows `QMessageBox` and returns false without replacing current model records.
- On success, replace model records, rebuild filter combos, select all filters, refresh summary/chart/table.
- `refreshView()` applies filters, chart metric, summary, and table.
- Summary labels include value plus `scenario | privacy | algorithm`.
- Table numeric items store values in `Qt::UserRole`.

- [ ] **Step 3: Add GUI sources to CMake**

Add to `GEOTASKSHIELD_GUI_SOURCES`:

```cmake
"gui/MetricBarChart.cpp"
"gui/BatchResultsWidget.cpp"
```

- [ ] **Step 4: Verify widget build**

Run Qt GUI smoke build. Expected: compiler errors only for `MainWindow` integration if not done yet.

## Task 6: Integrate MainWindow Tabs

**Files:**
- Modify: `GeoTaskShield/gui/MainWindow.h`
- Modify: `GeoTaskShield/gui/MainWindow.cpp`

- [ ] **Step 1: Update header**

Add forward declaration:

```cpp
class BatchResultsWidget;
```

Add private member:

```cpp
BatchResultsWidget* batchResultsWidget_{};
```

- [ ] **Step 2: Update constructor**

Replace direct central layout with:

```cpp
auto* tabs = new QTabWidget(central);
auto* simulationPage = new QWidget(tabs);
auto* simulationRoot = new QVBoxLayout(simulationPage);
...
tabs->addTab(simulationPage, "Simulation");
batchResultsWidget_ = new BatchResultsWidget(tabs);
tabs->addTab(batchResultsWidget_, "Batch Results");
root->addWidget(tabs);
```

Keep the existing `QSplitter`, panels, log panel, run button connection, and `runSimulation()` behavior unchanged.

- [ ] **Step 3: Verify GUI GREEN**

Run:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --build out\build\x64-debug-qt --target GeoTaskShieldGuiSmokeTests && ctest --test-dir out\build\x64-debug-qt --output-on-failure -R GeoTaskShieldGuiSmokeTests'
```

Expected: GUI smoke test passes.

## Task 7: Full Verification, Docs, and Commit

**Files:**
- Modify: `README.md`
- Modify: `HANDOFF.md`
- Modify: `task_plan.md`
- Modify: `findings.md`
- Modify: `progress.md`

- [ ] **Step 1: Run full non-Qt verification**

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug && cmake --build out\build\x64-debug && ctest --test-dir out\build\x64-debug --output-on-failure'
```

Expected: `1/1` core tests pass.

- [ ] **Step 2: Run full Qt verification**

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug-qt && cmake --build out\build\x64-debug-qt && ctest --test-dir out\build\x64-debug-qt --output-on-failure'
```

Expected: `2/2` tests pass.

- [ ] **Step 3: Update docs**

README should mention:

- GUI now has a `Batch Results` tab.
- It loads `phase5_batch_results.csv` or same-structure CSV files.
- It supports filters, summary cards, sortable table, and a self-drawn chart.

HANDOFF/progress should record:

- Phase 7 implemented as GUI-only CSV result analysis.
- Core algorithm/Agent/BatchExperiment semantics were not changed.
- Verification commands and results.

- [ ] **Step 4: Run diff checks**

```powershell
git diff --check
git status --short
```

Expected: no whitespace errors; only intended files changed.

- [ ] **Step 5: Commit implementation**

```powershell
git add GeoTaskShield docs README.md HANDOFF.md task_plan.md findings.md progress.md
git commit -m "feat(gui): add batch results visualization tab"
```
