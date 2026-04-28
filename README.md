# GeoTaskShield

GeoTaskShield is a C++20/CMake simulator for privacy-preserving task allocation in mobile crowdsensing. It models workers and sensing tasks on a 2D map, applies location privacy mechanisms, runs task assignment algorithms, evaluates utility/privacy metrics, and presents results through console, CSV, Qt Widgets GUI, and a local experiment-report agent.

Current release: `v0.7.0`. This release hardens the Phase 7 GUI batch-results visualization work.

## Current Status

Completed phases:

- Phase 1: Console MVP simulation pipeline.
- Phase 2: Additional privacy mechanisms, assignment algorithms, comparison experiments, and CSV export.
- Phase 3: Qt Widgets GUI visualization.
- Phase 4: Local rule-based AIAgent and Markdown experiment report generation.
- Phase 5: Batch experiment support, additional metrics, and richer CSV/Markdown export.
- Phase 6: Engineering cleanup, release packaging, style rules, and `v0.6.0` release preparation.
- Phase 7: GUI batch CSV result analysis and visualization, merged into `develop`.
- Phase 8: Release and demo hardening for `v0.7.0`.

## Features

- Synthetic worker/task generation.
- Location privacy mechanisms:
  - Grid Privacy
  - K-Anonymity Privacy
  - Laplace Noise Privacy
- Assignment algorithms:
  - Nearest Greedy
  - Score Greedy
  - Hungarian
- Metrics:
  - completed task count
  - completion rate
  - average true moving distance
  - total reward
  - average privacy loss
  - algorithm runtime
- Console 3 x 3 privacy/algorithm comparison.
- CSV export to `phase2_results.csv`.
- Qt Widgets GUI with a Simulation tab and a Batch Results analysis tab.
- Batch Results CSV loading, filters, summary cards, sortable table, and a lightweight self-drawn bar chart.
- Local natural-language experiment agent.
- Markdown experiment report generation.
- Batch experiment runner.
- Batch CSV and Markdown export.
- Additional metrics: worker load standard deviation, Jain fairness index, privacy-utility ratio, and timeout rate.

## Repository Layout

```text
GeoTaskShield/
  CMakeLists.txt
  CMakePresets.json
  CHANGELOG.md
  HANDOFF.md
  README.md
  git_guide.md
  scripts/
    package_windows.ps1
  phase2_results.csv
  GeoTaskShield/
    app/
      console/
      agent_demo/
      batch_demo/
    model/
    simulation/
    privacy/
    assignment/
    evaluation/
    data/
    agent/
    experiment/
    gui/
    tests/
```

## Requirements

- Windows
- Visual Studio 2026 with MSVC toolchain
- CMake
- Ninja
- Qt 6.11 MSVC kit for GUI builds

The current Qt preset expects:

```text
D:/Qt/6.11.0/msvc2022_64
```

If Qt is installed elsewhere, update `CMAKE_PREFIX_PATH` in `CMakePresets.json`.

## Build and Test

Run commands from the project root in a Visual Studio Developer Command Prompt, or call `VsDevCmd.bat` from PowerShell.

### Core Console Build

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug && cmake --build out\build\x64-debug && ctest --test-dir out\build\x64-debug --output-on-failure'
```

Run the console comparison:

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShield.exe
```

This prints 9 comparison rows and writes:

```text
phase2_results.csv
```

### Agent Report Demo

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShieldAgentDemo.exe
```

With a custom request:

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShieldAgentDemo.exe "对比三种隐私机制，30个用户，10个任务，使用匈牙利算法"
```

Example supported prompts:

```text
100 个用户，50 个任务，k=5，使用匈牙利算法
80 workers, 40 tasks, epsilon=0.5, use laplace and score greedy
对比三种隐私机制，50个用户，20个任务，使用最近贪心
```

### Qt GUI Build

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-debug-qt && cmake --build out\build\x64-debug-qt && ctest --test-dir out\build\x64-debug-qt --output-on-failure'
```

Run the GUI:

```powershell
out\build\x64-debug-qt\GeoTaskShield\GeoTaskShieldGui.exe
```

`GeoTaskShieldGui` runs `windeployqt` after build on Windows. Non-Qt executables such as `GeoTaskShield.exe` and `GeoTaskShieldTests.exe` do not need `windeployqt`.

The GUI contains:

- `Simulation`: run a single simulation with parameter controls, map canvas, metrics, and logs.
- `Batch Results`: open `phase5_batch_results.csv` or a same-structure CSV, filter by privacy/algorithm, select a metric, inspect summary cards, view a bar chart, and sort the detail table.

### Batch Results Demo

The `Batch Results` tab is intended for reviewing batch experiment outputs without rerunning the batch experiment.

Demo flow:

1. Run `GeoTaskShieldGui.exe`.
2. Open the `Batch Results` tab.
3. Click `Open CSV`.
4. Select `phase5_batch_results.csv`.
5. Filter by privacy mechanism or assignment algorithm.
6. Select a metric such as `completionRate`, `privacyUtilityRatio`, `fairnessIndex`, or `averagePrivacyLoss`.
7. Sort the table by clicking column headers and select rows to inspect details.

Example CSV header:

```text
scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,completed_tasks,total_tasks,completion_rate,average_moving_distance,total_reward,average_privacy_loss,algorithm_runtime_ms,user_load_stddev,fairness_index,privacy_utility_ratio,timeout_rate
```

Recommended screenshots for reports or demos:

- `Simulation` tab after running a simulation, showing parameter controls, map canvas, metrics, and logs.
- `Batch Results` tab after loading `phase5_batch_results.csv`, showing filters, summary cards, the bar chart, selected-row details, and the sortable table.

See [docs/demo/v0.7.0-gui-demo-guide.md](docs/demo/v0.7.0-gui-demo-guide.md) for a step-by-step demo script and screenshot checklist.

### Release Build and Package

Build the Release GUI preset:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 && cmake --preset x64-release-qt && cmake --build out\build\x64-release-qt'
```

Create a local Windows ZIP package:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package_windows.ps1
```

The package is generated under:

```text
out/package/GeoTaskShield-v0.7.0-windows-x64.zip
```

The package includes the demo CSV `phase5_batch_results.csv` and the GUI demo guide under `docs/demo/`.

Package outputs under `out/` are generated artifacts and are not committed.

## AIAgent Notes

The current agent is intentionally local and rule-based. It does not call an online model and does not require network access.

If a future version integrates Aliyun Bailian or another model provider, API keys must be read from runtime environment variables such as `DASHSCOPE_API_KEY`. Do not commit API keys, generated secrets, or model credentials.

## Batch Experiment Demo

```powershell
out\build\x64-debug\GeoTaskShield\GeoTaskShieldBatchDemo.exe
```

The batch demo runs deterministic scenarios across worker/task counts, epsilon, k, and grid size. It prints a Markdown table and writes:

```text
phase5_batch_results.csv
phase5_batch_report.md
```

## Git Workflow

This project follows the Git Flow rules in `git_guide.md`:

- `main`: production-ready code
- `develop`: integration branch
- `feature/*`: feature work
- `hotfix/*`: urgent fixes
- `release/*`: release preparation

Commit messages use:

```text
<type>(<scope>): <description>
```

Examples:

```text
feat(agent): add experiment report agent
docs(handoff): update phase 4 status
```

Release branches use `release/*`. Recent release branches:

```text
release/phase6-engineering-release
release/v0.7.0
```

Recent release tags:

```text
v0.6.0
v0.7.0
```

## Code Style

- `.clang-format` defines the project C++ formatting convention.
- MSVC builds use `/W4`; non-MSVC builds use `-Wall -Wextra -Wpedantic`.
- Existing code was not broadly reformatted in Phase 6 to keep the release diff focused.

## Documentation

- `HANDOFF.md`: current project handoff, phase status, verification commands, and next steps.
- `GeoTaskShield.md`: original project concept.
- `CHANGELOG.md`: release notes.
- `docs/superpowers/specs/`: design notes for implemented phases.
- `docs/superpowers/plans/`: implementation plans.

## Current Limitations

- The core tests use a lightweight custom assertion style rather than GoogleTest or Catch2.
- Hungarian assignment supports one task per expanded worker slot, not strict multi-worker task optimization.
- Laplace privacy is a simulator-level coordinate perturbation, not a full formal differential privacy proof.
- The AIAgent parser is rule-based and supports common prompt patterns only.
- The Batch Results tab uses lightweight custom painting rather than Qt Charts.
