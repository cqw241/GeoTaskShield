# Phase 7 Design: GUI Batch Results Visualization

## Goal

Phase 7 adds a `Batch Results` tab to the Qt Widgets GUI for analyzing existing batch experiment CSV output. The first version focuses on CSV result loading, filtering, sorting, summary statistics, a single-metric bar chart, and detail-table display.

This phase is a GUI display/reporting enhancement only. It must not change the semantics of `SimulationEngine`, privacy mechanisms, assignment algorithms, factories, the Agent module, or the `BatchExperiment` runner/exporter.

## Scope

In scope:

- Load `phase5_batch_results.csv` or a user-selected CSV with the same structure.
- Validate required CSV columns and parse rows into typed records.
- Filter rows by privacy mechanism and assignment algorithm.
- Select one metric for chart display.
- Sort result rows by visible table columns or model metrics.
- Show summary values:
  - best `completionRate`
  - best `privacyUtilityRatio`
  - best `fairnessIndex`
  - lowest `averagePrivacyLoss`
- Show a lightweight self-drawn bar chart for the selected metric.
- Show a sortable detail table with scenario/configuration/metric columns.
- Preserve the existing Simulation page and its behavior.

Out of scope:

- Re-running batch experiments from the GUI.
- Changing core algorithms, `SimulationEngine`, `PrivacyFactory`, `AssignmentAlgorithmFactory`, Agent behavior, or `BatchExperiment`.
- Introducing Qt Charts. Qt Charts is deprecated in Qt 6.11; if richer charts are needed later, evaluate Qt Graphs separately.
- Adding online model calls or report-generation model integration.

## Architecture

The main window will use a `QTabWidget`:

- `Simulation` tab: existing parameter panel, map canvas, result panel, and log panel.
- `Batch Results` tab: new CSV analysis page.

New files are planned under `GeoTaskShield/gui/`:

- `BatchResultRecord.h`
- `BatchResultCsvLoader.h/.cpp`
- `BatchResultModel.h/.cpp`
- `BatchResultsWidget.h/.cpp`
- `MetricBarChart.h/.cpp`

`BatchResultRecord`, `BatchResultCsvLoader`, and `BatchResultModel` should remain Qt-free C++ even though they live in the GUI module directory. Qt types should only appear in widget classes such as `BatchResultsWidget` and `MetricBarChart`.

## Data Model

`BatchResultRecord` represents one CSV row:

- scenario name
- worker count
- task count
- grid size
- k
- epsilon
- privacy name
- algorithm name
- completed task count
- total task count
- `completionRate`
- `averageTrueDistance`
- `totalReward`
- `averagePrivacyLoss`
- `runtimeMs`
- `userLoadStdDev`
- `fairnessIndex`
- `privacyUtilityRatio`
- `timeoutRate`

The internal metric names use camelCase. The current Phase 5 CSV uses snake_case headers, so the loader will map existing headers into these internal fields.

Required CSV header mapping:

| CSV header | Internal field |
|---|---|
| `scenario` | scenario |
| `workers` | workerCount |
| `tasks` | taskCount |
| `grid_size` | gridSize |
| `k` | k |
| `epsilon` | epsilon |
| `privacy` | privacy |
| `algorithm` | algorithm |
| `completed_tasks` | completedTasks |
| `total_tasks` | totalTasks |
| `completion_rate` | completionRate |
| `average_moving_distance` | averageTrueDistance |
| `total_reward` | totalReward |
| `average_privacy_loss` | averagePrivacyLoss |
| `algorithm_runtime_ms` | runtimeMs |
| `user_load_stddev` | userLoadStdDev |
| `fairness_index` | fairnessIndex |
| `privacy_utility_ratio` | privacyUtilityRatio |
| `timeout_rate` | timeoutRate |

The GUI may display friendly labels such as "Completion" or "Average true distance", but model-facing names should use the internal field names above.

## CSV Loading

`BatchResultCsvLoader` responsibilities:

- Read a CSV file path.
- Parse comma-separated values with quoted-field support.
- Verify all required headers exist.
- Convert each required numeric field to the correct type.
- Return parsed records or a clear error message.

Error behavior:

- Missing required columns: reject the file and report the missing column name.
- Invalid numeric cells: reject the file and report row and column context.
- Empty data file or header-only file: return an empty record set with no crash.
- Failed file open: return an error without changing the current GUI data.

The loader should not depend on Qt.

## Result Model

`BatchResultModel` responsibilities:

- Own the loaded `BatchResultRecord` collection.
- Track privacy and algorithm filters.
- Return filtered rows.
- Sort rows by a selected field and direction.
- Calculate the four summary records/values:
  - max `completionRate`
  - max `privacyUtilityRatio`
  - max `fairnessIndex`
  - min `averagePrivacyLoss`
- Return chart values for the selected metric.

Filtering applies to all summaries, chart values, and table rows. If filters produce no rows, summary cards should show neutral placeholders and the chart/table should be empty.

## GUI Layout

`BatchResultsWidget` will use the approved layout:

- Left panel:
  - Open CSV button
  - loaded file label
  - privacy filter
  - algorithm filter
  - metric selector
  - reset filters button
- Top summary strip:
  - best completion
  - best privacy-utility ratio
  - best fairness
  - lowest average privacy loss
- Middle:
  - `MetricBarChart` for the selected metric by scenario
  - selected-row detail panel
- Bottom:
  - sortable table with scenario/configuration/metric columns

The table should expose enough columns for comparison without hiding the key metrics:

- scenario
- workers
- tasks
- privacy
- algorithm
- `completionRate`
- `averageTrueDistance`
- `totalReward`
- `averagePrivacyLoss`
- `runtimeMs`
- `userLoadStdDev`
- `fairnessIndex`
- `privacyUtilityRatio`
- `timeoutRate`

## Chart

`MetricBarChart` is a lightweight QWidget that draws one metric at a time:

- x-axis labels use scenario names, shortened if necessary.
- y values use the selected metric.
- bars are scaled to the maximum visible value.
- empty data shows a neutral empty state.
- chart data should be set through a small non-owning data structure or value vector without exposing model internals.

Qt Charts must not be introduced in this phase.

## Testing

Add tests for the pure C++ display layer:

- `BatchResultCsvLoader` reads a valid Phase 5 style CSV.
- CSV header mapping populates camelCase internal fields.
- Missing columns and invalid numeric values return errors.
- `BatchResultModel` filters by privacy and algorithm.
- `BatchResultModel` calculates the four required summary values.
- `BatchResultModel` sorts rows by selected metrics.

Extend the GUI smoke test:

- Existing Simulation tab still runs a default simulation.
- `Batch Results` tab exists.
- A test CSV can be loaded programmatically or through a test helper.
- Loaded data produces non-empty table/chart state.

Verification commands:

```powershell
ctest --test-dir out\build\x64-debug --output-on-failure
ctest --test-dir out\build\x64-debug-qt --output-on-failure
```

## Acceptance Criteria

- The GUI can open `phase5_batch_results.csv`.
- The GUI can open a user-selected same-structure CSV.
- The page filters by privacy and algorithm.
- The metric selector controls the single-metric bar chart.
- Summary cards show best `completionRate`, best `privacyUtilityRatio`, best `fairnessIndex`, and lowest `averagePrivacyLoss`.
- The table is sortable.
- The existing Simulation page still works.
- Non-Qt tests pass.
- Qt GUI smoke tests pass.
