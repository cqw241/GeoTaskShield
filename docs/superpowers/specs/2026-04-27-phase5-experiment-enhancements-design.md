# Phase 5 Experiment Enhancements Design

## Scope
Phase 5 adds batch experiment support and additional evaluation metrics for report-oriented demonstrations. It keeps the GUI unchanged and implements the new capability in the Qt-independent core.

## Architecture
- `EvaluationMetrics` gains load and quality metrics.
- `MetricsCalculator` computes those metrics from existing tasks, workers, and assignments.
- A new `experiment` module owns batch scenario definitions, batch execution, and batch CSV/Markdown export.
- A new `GeoTaskShieldBatchDemo` executable runs a deterministic sample batch and writes Phase 5 output files.

## Metrics
- `userLoadStdDev`: standard deviation of assigned task counts per worker. Lower is more balanced.
- `fairnessIndex`: Jain fairness index over worker assignment counts. Higher is more fair.
- `privacyUtilityRatio`: `completionRate / (1 + averagePrivacyLoss)`. Higher means better completion under lower privacy loss.
- `timeoutRate`: assigned tasks whose true travel time exceeds task deadline divided by assigned tasks.

## Batch Experiments
Each `ExperimentScenario` includes a name, `SimulationConfig`, privacy strategy key, and algorithm key. `BatchExperimentRunner` creates strategies through existing factories and runs each scenario through `SimulationEngine`.

## Export
`BatchExperimentExporter` emits CSV and Markdown tables with scenario configuration, strategy names, and the full metric set.

## Testing
Existing custom core tests are extended with focused assertions for new metrics, batch runner behavior, and batch export contents. No network, Python, or external test framework is required.
