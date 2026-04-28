# Phase 5 Experiment Enhancements Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add deterministic batch experiments, richer metrics, and CSV/Markdown batch outputs.

**Architecture:** Extend `EvaluationMetrics` and `MetricsCalculator`, then add a focused `experiment` module for batch scenarios, execution, and export. Keep Qt and Agent code unchanged except for consuming the expanded metrics in reports/CSV.

**Tech Stack:** C++20, CMake, existing custom test executable, existing strategy factories and `SimulationEngine`.

---

### Task 1: New Metrics

**Files:**
- Modify: `GeoTaskShield/evaluation/EvaluationMetrics.h`
- Modify: `GeoTaskShield/evaluation/MetricsCalculator.cpp`
- Modify: `GeoTaskShield/tests/test_core.cpp`

- [ ] Add failing tests for load standard deviation, fairness index, privacy-utility ratio, and timeout rate.
- [ ] Implement the minimal metrics fields and calculations.
- [ ] Verify core tests pass.

### Task 2: Batch Experiment Runner

**Files:**
- Create: `GeoTaskShield/experiment/BatchExperiment.h`
- Create: `GeoTaskShield/experiment/BatchExperiment.cpp`
- Modify: `GeoTaskShield/CMakeLists.txt`
- Modify: `GeoTaskShield/tests/test_core.cpp`

- [ ] Add a failing test for running multiple scenarios.
- [ ] Implement `ExperimentScenario`, `BatchExperimentRow`, `BatchExperimentResult`, and `BatchExperimentRunner`.
- [ ] Wire the new source file into `GeoTaskShieldCore`.
- [ ] Verify core tests pass.

### Task 3: Batch Export

**Files:**
- Create: `GeoTaskShield/experiment/BatchExperimentExporter.h`
- Create: `GeoTaskShield/experiment/BatchExperimentExporter.cpp`
- Modify: `GeoTaskShield/CMakeLists.txt`
- Modify: `GeoTaskShield/tests/test_core.cpp`

- [ ] Add failing tests for batch CSV and Markdown contents.
- [ ] Implement CSV and Markdown export.
- [ ] Verify core tests pass.

### Task 4: Batch Demo and Documentation

**Files:**
- Create: `GeoTaskShield/app/batch_demo/main.cpp`
- Modify: `GeoTaskShield/CMakeLists.txt`
- Modify: `README.md`
- Modify: `HANDOFF.md`
- Modify: `task_plan.md`
- Modify: `findings.md`
- Modify: `progress.md`

- [ ] Add `GeoTaskShieldBatchDemo`.
- [ ] Update README and handoff with Phase 5 commands and outputs.
- [ ] Run non-Qt and Qt verification.
- [ ] Commit and push the feature branch.
